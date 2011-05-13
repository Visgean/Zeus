/*
  язык.
*/
#pragma once

#include "..\common\cui[en].h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Ћоготип.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_logo BO_NAME L" build tools.\nBuild time: " BO_BUILDTIME L".\n\n"

////////////////////////////////////////////////////////////////////////////////////////////////////
//  оманды и опции.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_command_ror13             L"ror13"
#define lng_command_ror13_help        L"ROR13 hash for input string."

#define lng_switch_inputstring        L"is"
#define lng_switch_inputstring_help   L"Input string."

#define lng_command_peinfo            L"peinfo"
#define lng_command_peinfo_help       L"Get information from PE file."

#define lng_switch_file               L"file"
#define lng_switch_file_help          L"PE file."

#define lng_switch_as_va              L"va"
#define lng_switch_as_va_help         L"Use VA inside RVA for input and ouput addresses."

#define lng_switch_datadirectory      L"dd"
#define lng_switch_datadirectory_help L"Get RVA and size from IMAGE_OPTIONAL_HEADER::DataDirectory[index]."

#define lng_switch_patch_dw           L"pdw"
#define lng_switch_patch_dw_help      L"Patch DWORD in section at specified RVA of image."

////////////////////////////////////////////////////////////////////////////////////////////////////
// ROR13.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_ror13_no_inputstring lng_error L"Input string not defined.\n"

////////////////////////////////////////////////////////////////////////////////////////////////////
// PEINFO.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_peinfo_no_pefile         lng_error L"PE file not specified.\n"
#define lng_peinfo_pefile_corrupted  lng_error L"PE file corrupted.\n"
#define lng_peinfo_invalid_rva       lng_error L"Invalid RVA specified.\n"
#define lng_peinfo_patched           L"Patched!\n"
#define lng_peinfo_not_patched       lng_error L"RVA not found.\n"