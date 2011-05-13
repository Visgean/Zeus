#include <windows.h>

#include "wsocket.h"
#include "sslsocket.h"

//Размер буфера сокета.
#define SOCKET_IO_BUFFER 0x10000

//Флаги для InitializeSecurityContext (Необходимость флагов потверждена через IE).
#define ISC_FLAGS (ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_CONFIDENTIALITY | ISC_REQ_EXTENDED_ERROR | ISC_REQ_MANUAL_CRED_VALIDATION |  ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT | ISC_REQ_STREAM)

static void safeFreeContextBuffer(SecBuffer *buffer)
{
  if(buffer->pvBuffer != NULL && buffer->cbBuffer != 0)CWA(secur32, FreeContextBuffer)(buffer->pvBuffer);
  buffer->pvBuffer   = NULL;
  buffer->cbBuffer   = 0;
  buffer->BufferType = SECBUFFER_EMPTY;
}

void SslSocket::init(void)
{

}

void SslSocket::uninit(void)
{

}

SslSocket::SERVERDATA *SslSocket::_createServerData(PCCERT_CONTEXT context, DWORD enabledProtocols)
{
  SERVERDATA sd;
  SCHANNEL_CRED sc;

  sc.dwVersion               = SCHANNEL_CRED_VERSION;
  sc.cCreds                  = 1;
  sc.paCred                  = &context;
  sc.hRootStore              = NULL;
  sc.cMappers                = 0;
  sc.aphMappers              = NULL;
  sc.cSupportedAlgs          = 0;
  sc.palgSupportedAlgs       = NULL;
  sc.grbitEnabledProtocols   = enabledProtocols;
  sc.dwMinimumCipherStrength = 0;
  sc.dwMaximumCipherStrength = 0;
  sc.dwSessionLifespan       = (MAXDWORD - 1);
  sc.dwFlags                 = SCH_CRED_NO_SYSTEM_MAPPER;
  sc.dwCredFormat            = 0;

  if(CWA(secur32, AcquireCredentialsHandleW)(NULL, UNISP_NAME_W, SECPKG_CRED_INBOUND, NULL, &sc, NULL, NULL, &sd.credHandle, NULL) == SEC_E_OK)
  {
    sd.flags = 0;
    
    SERVERDATA *newSd = (SERVERDATA *)Mem::copyEx(&sd, sizeof(SERVERDATA));
    if(newSd != NULL)return newSd;
    CWA(secur32, FreeCredentialsHandle)(&sd.credHandle);
  }
  return NULL;
}

void SslSocket::_freeServerData(SERVERDATA *sd)
{
  CWA(secur32, FreeCredentialsHandle)(&sd->credHandle);
  Mem::free(sd);
}

bool SslSocket::_serverHandshake(SERVERDATA *sd, void *recv, DWORD recvSize, void *send, DWORD *sendSize)
{
  SecBufferDesc inBuffer;
  SecBufferDesc outBuffer;
  SecBuffer     inBuffers[2];
  SecBuffer     outBuffers[2];
  DWORD         contextAttr;

  //recv от клиента.
  inBuffers[0].pvBuffer    = (void *)recv;
  inBuffers[0].cbBuffer    = recvSize;
  inBuffers[0].BufferType  = SECBUFFER_TOKEN;

  inBuffers[1].pvBuffer    = NULL;
  inBuffers[1].cbBuffer    = 0;
  inBuffers[1].BufferType  = SECBUFFER_EMPTY;

  inBuffer.cBuffers        = 2;
  inBuffer.pBuffers        = inBuffers;
  inBuffer.ulVersion       = SECBUFFER_VERSION;
  
  //send клиенту.
  outBuffers[0].pvBuffer   = NULL;
  outBuffers[0].BufferType = SECBUFFER_TOKEN;
  outBuffers[0].cbBuffer   = 0;

  outBuffer.cBuffers       = 1;
  outBuffer.pBuffers       = outBuffers;
  outBuffer.ulVersion      = SECBUFFER_VERSION;

  //Обробатываем этап.
/*  SECURITY_STATUS status = CWA(secur32, AcceptSecurityContext)(&sd->credHandle,
                                                               sd->flags & SDF_STATUS_HANDSHAKE ? &sd->ctxtHandle : NULL,
                                                               &inBuffer,
                                                               ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_EXTENDED_ERROR | ASC_REQ_REPLAY_DETECT | ASC_REQ_SEQUENCE_DETECT | ASC_REQ_STREAM,
                                                               &outBuffer,
                                                               &sd->ctxtHandle, //On subsequent calls, phNewContext can be the same as the handle specified in the phContext parameter.
                                                               &contextAttr,
                                                               NULL
                                                              );
  */
  sd->flags |= SDF_STATUS_HANDSHAKE;



}

/*bool SslSocket::_startServerEncryption(SOCKET socket, SERVERDATA *sd, DWORD enabledProtocols, DWORD timeout)
{
  AcceptSecurityContext(&sd->credHandle, NULL, );


}
*/
bool SslSocket::_startClientEncryption(SOCKET socket, LPWSTR serverName, SOCKETDATA *sd, DWORD enabledProtocols, DWORD timeout)
{
  bool ok = false;
  Mem::_zero(sd, sizeof(SOCKETDATA));

  //Инициализация.
  {
    SCHANNEL_CRED sc;

    sc.dwVersion               = SCHANNEL_CRED_VERSION;
    sc.cCreds                  = 0;
    sc.paCred                  = NULL;
    sc.hRootStore              = NULL;
    sc.cMappers                = 0;
    sc.aphMappers              = NULL;
    sc.cSupportedAlgs          = 0;
    sc.palgSupportedAlgs       = NULL;
    sc.grbitEnabledProtocols   = enabledProtocols;
    sc.dwMinimumCipherStrength = 0;
    sc.dwMaximumCipherStrength = 0;
    sc.dwSessionLifespan       = (MAXDWORD - 1);
    sc.dwFlags                 = SCH_CRED_MANUAL_CRED_VALIDATION | SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SERVERNAME_CHECK;
    sc.dwCredFormat            = 0;

    if(CWA(secur32, AcquireCredentialsHandleW)(NULL, UNISP_NAME_W, SECPKG_CRED_OUTBOUND, NULL, &sc, NULL, NULL, &sd->sh, NULL) != SEC_E_OK)return false;
  }

  DWORD dwContextAttr;
  SecBufferDesc OutBuffer;
  SecBuffer OutBuffers[1];

  OutBuffers[0].pvBuffer   = NULL;
  OutBuffers[0].BufferType = SECBUFFER_TOKEN;
  OutBuffers[0].cbBuffer   = 0;

  OutBuffer.cBuffers  = 1;
  OutBuffer.pBuffers  = OutBuffers;
  OutBuffer.ulVersion = SECBUFFER_VERSION;

  //Начинаем handshake.
  SECURITY_STATUS ssFirst = CWA(secur32, InitializeSecurityContextW)(&sd->sh, NULL, serverName, ISC_FLAGS, 0, 0, NULL, 0, &sd->ch, &OutBuffer, &dwContextAttr, NULL);

  #if defined(WDEBUG1)
  WDEBUG1(WDDT_INFO, "ssFirst=0x%08X", ssFirst);
  #endif

  if(ssFirst == SEC_I_CONTINUE_NEEDED && WSocket::tcpSend(socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer))
  {
    safeFreeContextBuffer(&OutBuffers[0]);

    //Создаем буфер для обмена сообщенияеми.
    LPBYTE recvBuffer = (LPBYTE)Mem::alloc(SOCKET_IO_BUFFER);
    DWORD recvSize   = 0;

    if(recvBuffer != NULL)
    {
      SecBufferDesc   InBuffer;
      SecBuffer       InBuffers[2];
      SECURITY_STATUS ssLoop = SEC_I_CONTINUE_NEEDED;

      while(ssLoop == SEC_I_CONTINUE_NEEDED || ssLoop == SEC_E_INCOMPLETE_MESSAGE)
      {
        //Читаем ответ от сервера.
        if(recvSize == 0 || ssLoop == SEC_E_INCOMPLETE_MESSAGE)
        {
          int r = WSocket::tcpRecv(socket, recvBuffer + recvSize, SOCKET_IO_BUFFER - recvSize, timeout);
          if(r == 0 || r == SOCKET_ERROR)break;
          recvSize += r;
        }

        //Подготавливаем буферы.
        InBuffers[0].pvBuffer    = (void *)recvBuffer;
        InBuffers[0].cbBuffer    = recvSize;
        InBuffers[0].BufferType  = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer    = NULL;
        InBuffers[1].cbBuffer    = 0;
        InBuffers[1].BufferType  = SECBUFFER_EMPTY;

        InBuffer.cBuffers        = 2;
        InBuffer.pBuffers        = InBuffers;
        InBuffer.ulVersion       = SECBUFFER_VERSION;

        OutBuffers[0].pvBuffer   = NULL;
        OutBuffers[0].BufferType = SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer   = 0;

        OutBuffer.cBuffers       = 1;
        OutBuffer.pBuffers       = OutBuffers;
        OutBuffer.ulVersion      = SECBUFFER_VERSION;

        dwContextAttr = 0;
        ssLoop = CWA(secur32, InitializeSecurityContextW)(&sd->sh, &sd->ch, serverName, ISC_FLAGS, 0, 0, &InBuffer, 0, NULL, &OutBuffer, &dwContextAttr, NULL);

        #if defined(WDEBUG1)
        WDEBUG1(WDDT_INFO, "ssLoop=0x%08X", ssLoop);
        #endif

        if(ssLoop == SEC_E_INCOMPLETE_MESSAGE)continue;
        if(ssLoop == SEC_E_OK || ssLoop == SEC_I_CONTINUE_NEEDED || ((FAILED(ssLoop)) && (dwContextAttr & ISC_RET_EXTENDED_ERROR)))
        {
          //Отправляем ответ.
          if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
          {
            if(!WSocket::tcpSend(socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer))break;
            safeFreeContextBuffer(&OutBuffers[0]);
          }

          //Проверяем есть ли лишние данные.
          if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
          {
            #if defined(WDEBUG1)
            WDEBUG1(WDDT_INFO, "SECBUFFER_EXTRA detected with size %u.", InBuffers[1].cbBuffer);
            #endif

            LPBYTE pOffset = recvBuffer + recvSize - InBuffers[1].cbBuffer;

            if(ssLoop == SEC_E_OK)
            {
              sd->extraBuffer = Mem::copyEx(pOffset, InBuffers[1].cbBuffer);
              if(sd->extraBuffer == NULL)break;
              sd->extraBufferSize = InBuffers[1].cbBuffer;
            }
            else
            {
              Mem::_copy(recvBuffer, pOffset, InBuffers[1].cbBuffer);
              recvSize = InBuffers[1].cbBuffer;
            }
          }
          else recvSize = 0;

          if(ssLoop == SEC_E_OK)
          {
            #if defined(WDEBUG1)
            WDEBUG1(WDDT_INFO, "dwContextAttr=0x%08X", dwContextAttr);
            #endif

            ok = true;
          }
        }
      }
      Mem::free(recvBuffer);
    }
  }

  if(ok && CWA(secur32, QueryContextAttributesW)(&sd->ch, SECPKG_ATTR_STREAM_SIZES, &sd->streamSizes) == SEC_E_OK)
  {
    sd->ioBufferSize = sd->streamSizes.cbHeader + sd->streamSizes.cbMaximumMessage + sd->streamSizes.cbTrailer;
    if((sd->ioBuffer = Mem::alloc(sd->ioBufferSize)) != NULL)
    {
      sd->socket = socket;
      return true;
    }
  }

  if(!FAILED(ssFirst))
  {
    Mem::free(sd->extraBuffer);
    sd->extraBuffer = NULL;
    safeFreeContextBuffer(&OutBuffers[0]);
    CWA(secur32, DeleteSecurityContext)(&sd->ch);
  }

  CWA(secur32, FreeCredentialsHandle)(&sd->sh);
  return false;
}

void SslSocket::_close(SOCKETDATA *sd)
{
  //Закрываем SSL.
  //FIXME: IE его не закрывает, мы не будем тоже тратить на это время и байты (Не хрена не документиовано как правильно закрывать SSL).

  //Освобождаем память.
  Mem::free(sd->pendingData);
  Mem::free(sd->extraBuffer);
  Mem::free(sd->ioBuffer);
  CWA(secur32, DeleteSecurityContext)(&sd->ch);
  CWA(secur32, FreeCredentialsHandle)(&sd->sh);
}

bool SslSocket::_send(SOCKETDATA *sd, void *buf, int bufSize)
{
  LPBYTE pMessage    = (LPBYTE)sd->ioBuffer + sd->streamSizes.cbHeader;
  LPBYTE pTrailer    = pMessage + sd->streamSizes.cbMaximumMessage;
  DWORD dwDataToSend = (DWORD)bufSize;
  LPBYTE pPos        = (LPBYTE)buf;

  SecBufferDesc Message;
  SecBuffer     Buffers[4];

  while(dwDataToSend != 0)
  {
    Mem::_zero(sd->ioBuffer, sd->ioBufferSize);

    DWORD dwCurLen = min(dwDataToSend, sd->streamSizes.cbMaximumMessage);
    Mem::_copy(pMessage, pPos, dwCurLen);

    dwDataToSend -= dwCurLen;
    pPos += dwCurLen;

    //Заполняем буферы.
    Buffers[0].pvBuffer   = sd->ioBuffer;
    Buffers[0].cbBuffer   = sd->streamSizes.cbHeader;
    Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;

    Buffers[1].pvBuffer   = pMessage;
    Buffers[1].cbBuffer   = dwCurLen;
    Buffers[1].BufferType = SECBUFFER_DATA;

    Buffers[2].pvBuffer   = pTrailer;
    Buffers[2].cbBuffer   = sd->streamSizes.cbTrailer;
    Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;

    Buffers[3].pvBuffer   = NULL;
    Buffers[3].cbBuffer   = 0;
    Buffers[3].BufferType = SECBUFFER_EMPTY;

    Message.ulVersion     = SECBUFFER_VERSION;
    Message.cBuffers      = 4;
    Message.pBuffers      = Buffers;

    //Шифруем.
    if(CWA(secur32, EncryptMessage)(&sd->ch, 0, &Message, 0) != SEC_E_OK)return false;

    //Отправляем.
    for(BYTE i = 0; i < 3; i++)if(Buffers[i].cbBuffer > 0 && !WSocket::tcpSend(sd->socket, Buffers[i].pvBuffer, Buffers[i].cbBuffer))return false;
  }
  return true;
}

int SslSocket::_recv(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible)
{
  //Не полученные данные с предыдушего вызова функции.
  if(sd->pendingDataSize > 0)
  {
    #if defined(WDEBUG0)
    WDEBUG0(WDDT_INFO, "Copying pending data.");
    #endif

    DWORD size = min(sd->pendingDataSize, (DWORD)bufSize);
    Mem::_copy(buf, sd->pendingData, size);
    sd->pendingDataSize -= size;

    if(sd->pendingDataSize == 0)
    {
      Mem::free(sd->pendingData);
      sd->pendingData = NULL;
    }
    else Mem::_copy(sd->pendingData, (LPBYTE)sd->pendingData + size, sd->pendingDataSize);
    *extraAvalible = (sd->extraBufferSize > 0 || sd->pendingDataSize > 0) ? true : false;
    return (int)size;
  }

  //Полчение из сокета.
  SecBufferDesc Message;
  SecBuffer     Buffers[4];
  DWORD dwReaded = 0;
  int r = SOCKET_ERROR;

  for(;;)
  {
    DWORD dwCurLen;

    //Читаем данные.
    if(sd->extraBufferSize > 0)
    {
      #if defined(WDEBUG0)
      WDEBUG0(WDDT_INFO, "Copying extra data.");
      #endif

      dwCurLen = sd->extraBufferSize;
      Mem::_copy(sd->ioBuffer, sd->extraBuffer, sd->extraBufferSize);
      Mem::free(sd->extraBuffer);
      sd->extraBuffer = NULL;
      sd->extraBufferSize = 0;
    }
    else
    {
      dwCurLen = (DWORD)WSocket::tcpRecv(sd->socket, (LPBYTE)sd->ioBuffer + dwReaded, sd->ioBufferSize - dwReaded, timeout);
      if(dwCurLen == 0 || dwCurLen == (DWORD)SOCKET_ERROR){r = (int)dwCurLen; break;}
    }

    dwReaded += dwCurLen; //Всего байт в буфере.

    //Заполняем буферы.
    Buffers[0].pvBuffer   = sd->ioBuffer;
    Buffers[0].cbBuffer   = dwReaded;
    Buffers[0].BufferType = SECBUFFER_DATA;

    for(BYTE i = 1; i < 4; i++)
    {
      Buffers[i].pvBuffer   = NULL;
      Buffers[i].cbBuffer   = 0;
      Buffers[i].BufferType = SECBUFFER_EMPTY;
    }

    Message.ulVersion = SECBUFFER_VERSION;
    Message.cBuffers  = 4;
    Message.pBuffers  = Buffers;

    //Расшифровываем данные.
    SECURITY_STATUS ss = CWA(secur32, DecryptMessage)(&sd->ch, &Message, 0, NULL);
    if(ss == SEC_E_INCOMPLETE_MESSAGE)
    {
#     if defined(WDEBUG0)
      WDEBUG0(WDDT_INFO, "SEC_E_INCOMPLETE_MESSAGE detected.");
#     endif
      continue;
    }

    //FIXME: коды SEC_I_CONTEXT_EXPIRED (не делаю по причине как и в _Close), SEC_I_RENEGOTIATE не на чем пока тестировать.
    if(ss == SEC_E_OK)
    {
      //Ищим обработанные данные.
      SecBuffer *pData  = NULL;
      SecBuffer *pExtra = NULL;
      for(BYTE i = 1; i < 4; i++)
      {
        if(pData == NULL && Buffers[i].BufferType == SECBUFFER_DATA && Buffers[i].cbBuffer > 0)pData = &Buffers[i];
        else if(pExtra == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA && Buffers[i].cbBuffer > 0)pExtra = &Buffers[i];
      }

      //Копируем данные в pBuf.
      if(pData)
      {
        #if defined(WDEBUG1)
        WDEBUG1(WDDT_INFO, "SECBUFFER_DATA detected %u.", pData->cbBuffer);
        #endif
        if(pData->cbBuffer > (DWORD)bufSize)
        {
          dwCurLen = (DWORD)bufSize;
          sd->pendingDataSize = pData->cbBuffer - dwCurLen;
          if((sd->pendingData = Mem::copyEx((LPBYTE)pData->pvBuffer + dwCurLen, sd->pendingDataSize)) == NULL)break;
        }
        else dwCurLen = pData->cbBuffer;
        Mem::_copy(buf, pData->pvBuffer, dwCurLen);
      }

      //Сохраняем лишние данные.
      if(pExtra != NULL)
      {
        #if defined(WDEBUG1)
        WDEBUG1(WDDT_INFO, "SECBUFFER_EXTRA detected %u.", pExtra->cbBuffer);
        #endif

        if((sd->extraBuffer = Mem::copyEx((LPBYTE)sd->ioBuffer + dwReaded - pExtra->cbBuffer, pExtra->cbBuffer)) == NULL)break;
        sd->extraBufferSize = pExtra->cbBuffer;

        /*
          Sometimes an application will read data from the remote party, attempt to decrypt it by
          using DecryptMessage (Schannel), and discover that DecryptMessage (Schannel) succeeded
          but the output buffers are empty. This is normal behavior, and applications must be able
          to deal with it.

          Видемо оно.
        */
        if(pData == NULL)continue;
      }

      if(pData)r = (int)dwCurLen;
    }
    break;
  }

  *extraAvalible = (sd->extraBufferSize > 0 || sd->pendingDataSize > 0) ? true : false;
  return r;
}

bool SslSocket::_recvAll(SOCKETDATA *sd, void *buf, int bufSize, DWORD timeout, bool *extraAvalible)
{
  int len;
  do
  {
    if((len = _recv(sd, buf, bufSize, timeout, extraAvalible)) <= 0)return false;

    buf      = ((LPBYTE)buf) + len;
    bufSize -= len;
  }
  while(bufSize > 0);

  return bufSize == 0 ? true : false;
}
