/*
  Английская локализация частых выражений для консольных приложений.
*/
#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
// Ошибки
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_error                   L"ERROR: "
#define lng_warning                 L"WARNING: "
#define lng_error_unknown           lng_error L"Unknown error.\n"
#define lng_error_not_enough_memory lng_error L"Not enough memory.\n"
#define lng_error_fopen             lng_error L"Failed to open file \"%s\".\n"
#define lng_error_fwrite            lng_error L"Failed to write to file \"%s\".\n"
#define lng_error_wsocket_failed    lng_error L"Failed to initialize WinSocket.\n"
#define lng_error_thread_failed     lng_error L"Failed to create new thread, error %u.\n"
#define lng_error_failed_port_ipv4  lng_error L"Failed to listen on IPv4 port %u.\n"
#define lng_error_failed_port_ipv6  lng_error L"Failed to listen on IPv6 port %u.\n"
#define lng_error_socket            lng_error L"Socket error: %u.\n"
#define lng_error_uknown_command    lng_error L"Unknown command \"%s\".\n"
#define lng_error_uknown_switch     lng_error L"Unknown switch \"%s\".\n"
#define lng_error_syntax_error      lng_error L"Syntax error \"%s\".\n"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Прочее
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_no_switches        lng_warning L"Please choose switches.\n"
#define lng_switch_nologo      L"nologo"
#define lng_switch_nologo_help L"Suppresses display of sign-on banner."
#define lng_help_title         L"Usage: %s <command> -<switch 1> -<switch N>\n\n"
#define lng_result_ready       L"\nReady!\n"
