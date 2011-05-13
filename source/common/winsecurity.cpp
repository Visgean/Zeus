#include <windows.h>
#include <sddl.h>
#include <aclapi.h>

#include "winsecurity.h"
#include "process.h"

#define LOWINTEGRITYLEVEL_FULLACCESS         L"S:(ML;;NRNWNX;;;LW)"
#define LOWINTEGRITYLEVEL_FULLACCESS_INHERIT L"S:(ML;CIOI;NRNWNX;;;LW)"

void WinSecurity::init(void)
{

}

void WinSecurity::uninit(void)
{

}

void *WinSecurity::_getFullAccessDescriptors(SECURITY_ATTRIBUTES *sa, SECURITY_DESCRIPTOR *sd)
{
  if(CWA(advapi32, InitializeSecurityDescriptor)(sd, SECURITY_DESCRIPTOR_REVISION) && CWA(advapi32, SetSecurityDescriptorDacl)(sd, TRUE, NULL, FALSE))
  {
    PSECURITY_DESCRIPTOR sdLow;
    if(CWA(advapi32, ConvertStringSecurityDescriptorToSecurityDescriptorW)(LOWINTEGRITYLEVEL_FULLACCESS, SDDL_REVISION_1, &sdLow, NULL) != FALSE) 
    {
      BOOL saclPresent, saclDefaulted;
      PACL sacl = NULL;
      if(CWA(advapi32, GetSecurityDescriptorSacl)(sdLow, &saclPresent, &sacl, &saclDefaulted) == FALSE ||
         CWA(advapi32, SetSecurityDescriptorSacl)(sd, saclPresent, sacl, saclDefaulted) == FALSE)
      {
        CWA(kernel32, LocalFree)(sdLow);
        sdLow = (PSECURITY_DESCRIPTOR)-1;
      }
    }
    else sdLow = (PSECURITY_DESCRIPTOR)-1;

    if(sa)
    {
      sa->nLength              = sizeof(SECURITY_ATTRIBUTES);
      sa->lpSecurityDescriptor = sd;
      sa->bInheritHandle       = FALSE;
    }

    return sdLow;
  }
  return NULL;
}

bool WinSecurity::_setLowIntegrityLevelLabel(LPWSTR name, DWORD objectType, bool inherit)
{
  bool r = false;
  PSECURITY_DESCRIPTOR sd;

  Process::_enablePrivilege(SE_SECURITY_NAME, true);
  if(CWA(advapi32, ConvertStringSecurityDescriptorToSecurityDescriptorW)(inherit ? LOWINTEGRITYLEVEL_FULLACCESS_INHERIT : LOWINTEGRITYLEVEL_FULLACCESS, SDDL_REVISION_1, &sd, NULL) != FALSE) 
  {
    BOOL saclPresent, saclDefaulted;
    PACL sacl = NULL;
    if(CWA(advapi32, GetSecurityDescriptorSacl)(sd, &saclPresent, &sacl, &saclDefaulted) != FALSE)
    {
      if(CWA(advapi32, SetNamedSecurityInfoW)(name, (SE_OBJECT_TYPE)objectType, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, sacl) == ERROR_SUCCESS)r = true;
    }
    CWA(kernel32, LocalFree)(sd);
  }
  return r;
}

TOKEN_USER *WinSecurity::_getUserByToken(HANDLE token)
{
  DWORD size;
  if(CWA(advapi32, GetTokenInformation)(token, TokenUser, NULL, 0, &size) == FALSE && CWA(kernel32, GetLastError)() == ERROR_INSUFFICIENT_BUFFER)
  {
    TOKEN_USER *tu;
    if((tu = (TOKEN_USER *)Mem::alloc(size)) != NULL)
    {
      if(CWA(advapi32, GetTokenInformation)(token, TokenUser, tu, size, &size) != FALSE)return tu;
      Mem::free(tu);
    }
  }
  return NULL;
}
