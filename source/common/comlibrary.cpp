#include <windows.h>
#include <objbase.h>

#include "comlibrary.h"

void ComLibrary::init(void)
{

}

void ComLibrary::uninit(void)
{

}

bool ComLibrary::_initThread(HRESULT *result)
{
  HRESULT hr = CWA(ole32, CoInitializeEx)(NULL, COINIT_APARTMENTTHREADED);
  *result = hr;
  return (hr == S_OK || hr == S_FALSE || hr == RPC_E_CHANGED_MODE);
}

void ComLibrary::_uninitThread(HRESULT initResult)
{
  if(initResult == S_OK || initResult == S_FALSE)CWA(ole32, CoUninitialize)();
}

void *ComLibrary::_createInterface(REFCLSID clsid, REFIID iid)
{
  void *interfacePointer = NULL;
  HRESULT hr = CWA(ole32, CoCreateInstance)(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_NO_FAILURE_LOG | CLSCTX_NO_CODE_DOWNLOAD, iid, &interfacePointer);
  
  if(hr == S_OK && interfacePointer != NULL)return interfacePointer;

#if defined WDEBUG1
  WDEBUG1(WDDT_ERROR, "CoCreateInstance failed, result=0x%08X.", hr);
#endif

  return NULL;
}
