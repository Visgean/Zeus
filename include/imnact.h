

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Jul 27 04:11:19 2010
 */
/* Compiler settings for imnact.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __imnact_h__
#define __imnact_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IImnAdviseAccount_FWD_DEFINED__
#define __IImnAdviseAccount_FWD_DEFINED__
typedef interface IImnAdviseAccount IImnAdviseAccount;
#endif 	/* __IImnAdviseAccount_FWD_DEFINED__ */


#ifndef __IImnAdviseMigrateServer_FWD_DEFINED__
#define __IImnAdviseMigrateServer_FWD_DEFINED__
typedef interface IImnAdviseMigrateServer IImnAdviseMigrateServer;
#endif 	/* __IImnAdviseMigrateServer_FWD_DEFINED__ */


#ifndef __IImnEnumAccounts_FWD_DEFINED__
#define __IImnEnumAccounts_FWD_DEFINED__
typedef interface IImnEnumAccounts IImnEnumAccounts;
#endif 	/* __IImnEnumAccounts_FWD_DEFINED__ */


#ifndef __IImnAccountManager_FWD_DEFINED__
#define __IImnAccountManager_FWD_DEFINED__
typedef interface IImnAccountManager IImnAccountManager;
#endif 	/* __IImnAccountManager_FWD_DEFINED__ */


#ifndef __IImnAccountManager2_FWD_DEFINED__
#define __IImnAccountManager2_FWD_DEFINED__
typedef interface IImnAccountManager2 IImnAccountManager2;
#endif 	/* __IImnAccountManager2_FWD_DEFINED__ */


#ifndef __IPropertyContainer_FWD_DEFINED__
#define __IPropertyContainer_FWD_DEFINED__
typedef interface IPropertyContainer IPropertyContainer;
#endif 	/* __IPropertyContainer_FWD_DEFINED__ */


#ifndef __IImnAccount_FWD_DEFINED__
#define __IImnAccount_FWD_DEFINED__
typedef interface IImnAccount IImnAccount;
#endif 	/* __IImnAccount_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_imnact_0000_0000 */
/* [local] */ 


//=--------------------------------------------------------------------------=
// Imnact.h
//=--------------------------------------------------------------------------=
// (C) Copyright 1995-1998 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=

#pragma comment(lib,"uuid.lib")

//---------------------------------------------------------------------------=
// Internet Mail and News Account Manager Interfaces.

// --------------------------------------------------------------------------------
// GUIDS
// --------------------------------------------------------------------------------
#if !defined( WIN16 ) || defined( __cplusplus )
// {8D4B04E1-1331-11d0-81B8-00C04FD85AB4}
DEFINE_GUID(CLSID_ImnAccountManager, 0x8d4b04e1, 0x1331, 0x11d0, 0x81, 0xb8, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

// {FD465484-1384-11d0-ABBD-0020AFDFD10A}
DEFINE_GUID(IID_IPropertyContainer, 0xfd465484, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

// {FD465481-1384-11d0-ABBD-0020AFDFD10A}
DEFINE_GUID(IID_IImnAccountManager, 0xfd465481, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

// {C43DFC6F-62BB-11d2-A727-00C04F79E7C8}
DEFINE_GUID(IID_IImnAccountManager2, 0xc43dfc6f, 0x62bb, 0x11d2, 0xa7, 0x27, 0x0, 0xc0, 0x4f, 0x79, 0xe7, 0xc8);

// {FD465482-1384-11d0-ABBD-0020AFDFD10A}
DEFINE_GUID(IID_IImnAccount, 0xfd465482, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

// {FD465483-1384-11d0-ABBD-0020AFDFD10A}
DEFINE_GUID(IID_IImnEnumAccounts, 0xfd465483, 0x1384, 0x11d0, 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa);

// {8D0AED11-1638-11d0-81B9-00C04FD85AB4}
DEFINE_GUID(IID_IImnAdviseMigrateServer, 0x8d0aed11, 0x1638, 0x11d0, 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

// {0A06BD31-166F-11d0-81B9-00C04FD85AB4}
DEFINE_GUID(IID_IImnAdviseAccount, 0xa06bd31, 0x166f, 0x11d0, 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

#endif //!WIN16 || __cplusplus

// --------------------------------------------------------------------------------
// Exported C Functions
// --------------------------------------------------------------------------------
#if !defined(_IMNACCT_)
#define IMNACCTAPI DECLSPEC_IMPORT HRESULT WINAPI
#define IMNACCTAPI_(_type_) DECLSPEC_IMPORT _type_ WINAPI
#else
#define IMNACCTAPI HRESULT WINAPI
#define IMNACCTAPI_(_type_) _type_ WINAPI
#endif
#ifdef __cplusplus
extern "C" {
#endif

IMNACCTAPI HrCreateAccountManager(IImnAccountManager **ppAccountManager);
IMNACCTAPI ValidEmailAddress(LPCSTR lpAddress);

#ifdef __cplusplus
}
#endif

// --------------------------------------------------------------------------------
// Errors
// --------------------------------------------------------------------------------
#ifndef FACILITY_INTERNET
#define FACILITY_INTERNET 12
#endif
#ifndef HR_E
#define HR_E(n) MAKE_SCODE(SEVERITY_ERROR, FACILITY_INTERNET, n)
#endif
#ifndef HR_S
#define HR_S(n) MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_INTERNET, n)
#endif
#ifndef HR_CODE
#define HR_CODE(hr) (INT)(hr & 0xffff)
#endif

// --------------------------------------------------------------------------------
// General Errors
// --------------------------------------------------------------------------------
#define E_RegCreateKeyFailed         HR_E(0xCD00)
#define E_RegQueryInfoKeyFailed      HR_E(0xCD01)
#define E_UserCancel                 HR_E(0xCD02)
#define E_RegOpenKeyFailed           HR_E(0xCD03)
#define E_RegSetValueFailed          HR_E(0xCD04)
#define E_RegDeleteKeyFailed         HR_E(0xCD05)
#define E_DuplicateAccountName       HR_E(0xCD06)
#define S_NonStandardValue           HR_S(0xCD07)
#define E_InvalidValue               HR_E(0xCD08)
#define S_AlreadyInitialized         HR_S(0xCD09)

// --------------------------------------------------------------------------------
// IPropertyContainer Errors
// --------------------------------------------------------------------------------
#define E_NoPropData                 HR_E(0xCDA0)
#define E_BufferTooSmall             HR_E(0xCDA1)
#define E_BadPropType                HR_E(0xCDA2)
#define E_BufferSizeMismatch         HR_E(0xCDA3)
#define E_InvalidBooleanValue        HR_E(0xCDA4)
#define E_InvalidMinMaxValue         HR_E(0xCDA5)
#define E_PropNotFound               HR_E(0xCDA6)
#define E_InvalidPropTag             HR_E(0xCDA7)
#define E_InvalidPropertySet         HR_E(0xCDA8)
#define E_EnumFinished               HR_E(0xCDA9)
#define S_NoSaveNeeded               HR_S(0xCDAA)
#define S_PasswordDeleted            HR_S(0xCDAB)
#define S_PasswordNotFound           HR_S(0xCDAC)

// --------------------------------------------------------------------------------
// ImnAccountManager errors
// --------------------------------------------------------------------------------
#define E_NoAccounts                 HR_E(0xCDD0)
#define E_BadFriendlyName            HR_E(0xCDD1)
#define E_NoIdentities               HR_E(0xCDD2)

// --------------------------------------------------------------------------------
// RAS Connection Types
// --------------------------------------------------------------------------------
#define CF_AUTO_CONNECT          FLAG02 // Automatically connect

// --------------------------------------------------------------------------------
// Property Macros
// --------------------------------------------------------------------------------
#define PROPTAG_MASK                 ((ULONG)0x0000FFFF)
#define PROPTAG_TYPE(ulPropTag)      (PROPTYPE)(((ULONG)(ulPropTag)) & PROPTAG_MASK)
#define PROPTAG_ID(ulPropTag)        (((ULONG)(ulPropTag))>>16)
#define PROPTAG(ulPropType,ulPropID) ((((ULONG)(ulPropID))<<16)|((ULONG)(ulPropType)))
#define MAX_PROPID                   ((ULONG) 0xFFFF)

// --------------------------------------------------------------------------------
// IImnAccount Properties
// --------------------------------------------------------------------------------
#define AP_FIRST                    0X1234
// per account properties
#define AP_ACCOUNT_NAME             PROPTAG(TYPE_STRING,    AP_FIRST+1)
#define AP_ACCOUNT_FIRST            AP_ACCOUNT_NAME
#define AP_TEMP_ACCOUNT             PROPTAG(TYPE_DWORD,     AP_FIRST+3)
#define AP_LAST_UPDATED             PROPTAG(TYPE_FILETIME,  AP_FIRST+4)
#define AP_RAS_CONNECTION_TYPE      PROPTAG(TYPE_DWORD,     AP_FIRST+5)
#define AP_RAS_CONNECTOID           PROPTAG(TYPE_STRING,    AP_FIRST+6)
#define AP_RAS_CONNECTION_FLAGS     PROPTAG(TYPE_DWORD,     AP_FIRST+7)
#define AP_ACCOUNT_ID               PROPTAG(TYPE_STRING,    AP_FIRST+9)
#define AP_RAS_BACKUP_CONNECTOID    PROPTAG(TYPE_STRING,    AP_FIRST+10)
#define AP_SERVICE                  PROPTAG(TYPE_STRING,    AP_FIRST+11)
#define AP_AVAIL_OFFLINE            PROPTAG(TYPE_DWORD,     AP_FIRST+12)
#define AP_UNIQUE_ID                PROPTAG(TYPE_BINARY,    AP_FIRST+13)
#define AP_SERVER_READ_ONLY         PROPTAG(TYPE_BOOL,      AP_FIRST+14)
#define AP_ACCOUNT_LAST             AP_SERVER_READ_ONLY

// IMAP properties
#define AP_IMAP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+100)
#define AP_IMAP_FIRST               AP_IMAP_SERVER
#define AP_IMAP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+101)
#define AP_IMAP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+102)
#define AP_IMAP_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+104)
#define AP_IMAP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+105)
#define AP_IMAP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+106)
#define AP_IMAP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+107)
#define AP_IMAP_ROOT_FOLDER         PROPTAG(TYPE_STRING,    AP_FIRST+108)
#define AP_IMAP_DATA_DIR            PROPTAG(TYPE_STRING,    AP_FIRST+109)
#define AP_IMAP_USE_LSUB            PROPTAG(TYPE_BOOL,      AP_FIRST+111)
#define AP_IMAP_POLL                PROPTAG(TYPE_BOOL,      AP_FIRST+112)
#define AP_IMAP_FULL_LIST           PROPTAG(TYPE_BOOL,      AP_FIRST+113)
#define AP_IMAP_NOOP_INTERVAL       PROPTAG(TYPE_DWORD,     AP_FIRST+114)
#define AP_IMAP_SVRSPECIALFLDRS     PROPTAG(TYPE_BOOL,      AP_FIRST+116)
#define AP_IMAP_SENTITEMSFLDR       PROPTAG(TYPE_STRING,    AP_FIRST+117)
#define AP_IMAP_DRAFTSFLDR          PROPTAG(TYPE_STRING,    AP_FIRST+119)
#define AP_IMAP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+124)
#define AP_IMAP_DIRTY               PROPTAG(TYPE_DWORD,     AP_FIRST+125)
#define AP_IMAP_POLL_ALL_FOLDERS    PROPTAG(TYPE_BOOL,      AP_FIRST+126)
#define AP_IMAP_DELETEDITEMSFLDR    PROPTAG(TYPE_STRING,    AP_FIRST+127)
#define AP_IMAP_JUNKFLDR            PROPTAG(TYPE_STRING,    AP_FIRST+128)
#define AP_IMAP_LAST                AP_IMAP_JUNKFLDR

// LDAP properties
#define AP_LDAP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+200)
#define AP_LDAP_FIRST               AP_LDAP_SERVER
#define AP_LDAP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+201)
#define AP_LDAP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+202)
#define AP_LDAP_AUTHENTICATION      PROPTAG(TYPE_DWORD,     AP_FIRST+203)
#define AP_LDAP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+204)
#define AP_LDAP_SEARCH_RETURN       PROPTAG(TYPE_DWORD,     AP_FIRST+205)
#define AP_LDAP_SEARCH_BASE         PROPTAG(TYPE_STRING,    AP_FIRST+206)
#define AP_LDAP_SERVER_ID           PROPTAG(TYPE_DWORD,     AP_FIRST+207)
#define AP_LDAP_RESOLVE_FLAG        PROPTAG(TYPE_DWORD,     AP_FIRST+208)
#define AP_LDAP_URL                 PROPTAG(TYPE_STRING,    AP_FIRST+209)
#define AP_LDAP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+210)
#define AP_LDAP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+211)
#define AP_LDAP_LOGO                PROPTAG(TYPE_STRING,    AP_FIRST+212)
#define AP_LDAP_USE_BIND_DN         PROPTAG(TYPE_DWORD,     AP_FIRST+213)
#define AP_LDAP_SIMPLE_SEARCH       PROPTAG(TYPE_DWORD,     AP_FIRST+214)
#define AP_LDAP_ADVANCED_SEARCH_ATTR PROPTAG(TYPE_STRING,   AP_FIRST+215)
#define AP_LDAP_PAGED_RESULTS       PROPTAG(TYPE_DWORD,     AP_FIRST+216)
#define AP_LDAP_NTDS                PROPTAG(TYPE_DWORD,     AP_FIRST+217)
#define AP_LDAP_LAST                AP_LDAP_NTDS

// HTTPMail properties
#define AP_HTTPMAIL_SERVER           PROPTAG(TYPE_STRING,    AP_FIRST+250)
#define AP_HTTPMAIL_FIRST            AP_HTTPMAIL_SERVER
#define AP_HTTPMAIL_USERNAME         PROPTAG(TYPE_STRING,    AP_FIRST+251)
#define AP_HTTPMAIL_PASSWORD         PROPTAG(TYPE_PASS,      AP_FIRST+252)
#define AP_HTTPMAIL_PROMPT_PASSWORD  PROPTAG(TYPE_BOOL,      AP_FIRST+253)
#define AP_HTTPMAIL_USE_SICILY       PROPTAG(TYPE_DWORD,     AP_FIRST+254)
#define AP_HTTPMAIL_FRIENDLY_NAME    PROPTAG(TYPE_STRING,    AP_FIRST+255)
#define AP_HTTPMAIL_DOMAIN_MSN       PROPTAG(TYPE_BOOL,      AP_FIRST+256)
#define AP_HTTPMAIL_POLL             PROPTAG(TYPE_BOOL,      AP_FIRST+257)
#define AP_HTTPMAIL_ADURL              PROPTAG(TYPE_STRING,          AP_FIRST+258)
#define AP_HTTPMAIL_SHOW_ADBAR         PROPTAG(TYPE_BOOL,            AP_FIRST+259)
#define AP_HTTPMAIL_MINPOLLINGINTERVAL PROPTAG(TYPE_ULARGEINTEGER,   AP_FIRST+260)
#define AP_HTTPMAIL_GOTPOLLINGINTERVAL PROPTAG(TYPE_BOOL,            AP_FIRST+261)
#define AP_HTTPMAIL_LASTPOLLEDTIME     PROPTAG(TYPE_ULARGEINTEGER,   AP_FIRST+262)
#define AP_HTTPMAIL_ROOTTIMESTAMP      PROPTAG(TYPE_STRING,          AP_FIRST+263)
#define AP_HTTPMAIL_ROOTINBOXTIMESTAMP PROPTAG(TYPE_STRING,          AP_FIRST+264)
#define AP_HTTPMAIL_INBOXTIMESTAMP     PROPTAG(TYPE_STRING,          AP_FIRST+265)
#define AP_HTTPMAIL_MAXPOLLINGINTERVAL PROPTAG(TYPE_ULARGEINTEGER,   AP_FIRST+266)
#define AP_HTTPMAIL_ADBAR              PROPTAG(TYPE_STRING,          AP_FIRST+267)
#define AP_HTTPMAIL_CONTACTS           PROPTAG(TYPE_STRING,          AP_FIRST+268)
#define AP_HTTPMAIL_INBOX              PROPTAG(TYPE_STRING,          AP_FIRST+269)
#define AP_HTTPMAIL_OUTBOX             PROPTAG(TYPE_STRING,          AP_FIRST+270)
#define AP_HTTPMAIL_SENDMSG            PROPTAG(TYPE_STRING,          AP_FIRST+271)
#define AP_HTTPMAIL_SENTITEMS          PROPTAG(TYPE_STRING,          AP_FIRST+272)
#define AP_HTTPMAIL_DELETEDITEMS       PROPTAG(TYPE_STRING,          AP_FIRST+273)
#define AP_HTTPMAIL_DRAFTS             PROPTAG(TYPE_STRING,          AP_FIRST+274)
#define AP_HTTPMAIL_MSGFOLDERROOT      PROPTAG(TYPE_STRING,          AP_FIRST+275)
#define AP_HTTPMAIL_SIG                PROPTAG(TYPE_STRING,          AP_FIRST+276)
#define AP_HTTPMAIL_HASROOTPROPS       PROPTAG(TYPE_BOOL,            AP_FIRST+277)
#define AP_HTTPMAIL_LAST               AP_HTTPMAIL_HASROOTPROPS

// NNTP properties
#define AP_NNTP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+300)
#define AP_NNTP_FIRST               AP_NNTP_SERVER
#define AP_NNTP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+301)
#define AP_NNTP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+302)
#define AP_NNTP_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+304)
#define AP_NNTP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+305)
#define AP_NNTP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+306)
#define AP_NNTP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+307)
#define AP_NNTP_DISPLAY_NAME        PROPTAG(TYPE_STRING,    AP_FIRST+308)
#define AP_NNTP_ORG_NAME            PROPTAG(TYPE_STRING,    AP_FIRST+309)
#define AP_NNTP_EMAIL_ADDRESS       PROPTAG(TYPE_STRING,    AP_FIRST+310)
#define AP_NNTP_REPLY_EMAIL_ADDRESS PROPTAG(TYPE_STRING,    AP_FIRST+311)
#define AP_NNTP_SPLIT_MESSAGES      PROPTAG(TYPE_BOOL,      AP_FIRST+312)
#define AP_NNTP_SPLIT_SIZE          PROPTAG(TYPE_DWORD,     AP_FIRST+313)
#define AP_NNTP_USE_DESCRIPTIONS    PROPTAG(TYPE_BOOL,      AP_FIRST+314)
#define AP_NNTP_DATA_DIR            PROPTAG(TYPE_STRING,    AP_FIRST+315)
#define AP_NNTP_POLL                PROPTAG(TYPE_BOOL,      AP_FIRST+316)
#define AP_NNTP_POST_FORMAT         PROPTAG(TYPE_DWORD,     AP_FIRST+317)
#define AP_NNTP_SIGNATURE           PROPTAG(TYPE_STRING,    AP_FIRST+318)
#define AP_NNTP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+319)
#define AP_NNTP_COMMUNITIES         PROPTAG(TYPE_DWORD,     AP_FIRST+320)
#define AP_NNTP_PASSPORT_MEMBERNAME PROPTAG(TYPE_STRING,    AP_FIRST+321)
#define AP_NNTP_COMMUNITYSERVERDATA PROPTAG(TYPE_BINARY,    AP_FIRST+322)
#define AP_NNTP_PASSPORTSESSIONDATA PROPTAG(TYPE_BINARY,    AP_FIRST+323)
#define AP_NNTP_USER_INFORMATION    PROPTAG(TYPE_DWORD,     AP_FIRST+324)
#define AP_NNTP_LAST                AP_NNTP_USER_INFORMATION

// POP3 properties
#define AP_POP3_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+400)
#define AP_POP3_FIRST               AP_POP3_SERVER
#define AP_POP3_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+401)
#define AP_POP3_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+402)
#define AP_POP3_USE_SICILY          PROPTAG(TYPE_BOOL,      AP_FIRST+404)
#define AP_POP3_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+405)
#define AP_POP3_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+406)
#define AP_POP3_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+407)
#define AP_POP3_LEAVE_ON_SERVER     PROPTAG(TYPE_BOOL,      AP_FIRST+408)
#define AP_POP3_REMOVE_DELETED      PROPTAG(TYPE_BOOL,      AP_FIRST+409)
#define AP_POP3_REMOVE_EXPIRED      PROPTAG(TYPE_BOOL,      AP_FIRST+410)
#define AP_POP3_EXPIRE_DAYS         PROPTAG(TYPE_DWORD,     AP_FIRST+411)
#define AP_POP3_SKIP                PROPTAG(TYPE_BOOL,      AP_FIRST+412)
#define AP_POP3_OUTLOOK_CACHE_NAME  PROPTAG(TYPE_STRING,    AP_FIRST+413)
#define AP_POP3_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+414)
#define AP_POP3_LAST                AP_POP3_PROMPT_PASSWORD

// SMTP properties
#define AP_SMTP_SERVER              PROPTAG(TYPE_STRING,    AP_FIRST+500)
#define AP_SMTP_FIRST               AP_SMTP_SERVER
#define AP_SMTP_USERNAME            PROPTAG(TYPE_STRING,    AP_FIRST+501)
#define AP_SMTP_PASSWORD            PROPTAG(TYPE_PASS,      AP_FIRST+502)
#define AP_SMTP_USE_SICILY          PROPTAG(TYPE_DWORD,     AP_FIRST+504) // SMTPAUTHTYEP
#define AP_SMTP_PORT                PROPTAG(TYPE_DWORD,     AP_FIRST+505)
#define AP_SMTP_SSL                 PROPTAG(TYPE_BOOL,      AP_FIRST+506)
#define AP_SMTP_TIMEOUT             PROPTAG(TYPE_DWORD,     AP_FIRST+507)
#define AP_SMTP_DISPLAY_NAME        PROPTAG(TYPE_STRING,    AP_FIRST+508)
#define AP_SMTP_ORG_NAME            PROPTAG(TYPE_STRING,    AP_FIRST+509)
#define AP_SMTP_EMAIL_ADDRESS       PROPTAG(TYPE_STRING,    AP_FIRST+510)
#define AP_SMTP_REPLY_EMAIL_ADDRESS PROPTAG(TYPE_STRING,    AP_FIRST+511)
#define AP_SMTP_SPLIT_MESSAGES      PROPTAG(TYPE_BOOL,      AP_FIRST+512)
#define AP_SMTP_SPLIT_SIZE          PROPTAG(TYPE_DWORD,     AP_FIRST+513)
#define AP_SMTP_CERTIFICATE         PROPTAG(TYPE_BINARY,    AP_FIRST+514)
#define AP_SMTP_SIGNATURE           PROPTAG(TYPE_STRING,    AP_FIRST+515)
#define AP_SMTP_PROMPT_PASSWORD     PROPTAG(TYPE_BOOL,      AP_FIRST+516)
#define AP_SMTP_ENCRYPT_CERT           PROPTAG(TYPE_BINARY,      AP_FIRST+517)
#define AP_SMTP_ENCRYPT_ALGTH       PROPTAG(TYPE_BINARY,      AP_FIRST+518)
#define AP_SMTP_LAST                AP_SMTP_ENCRYPT_ALGTH

// --------------------------------------------------------------------------------
// Account Flags
// --------------------------------------------------------------------------------
#define ACCT_FLAG_NEWS           ((DWORD)1)
#define ACCT_FLAG_MAIL           ((DWORD)2)
#define ACCT_FLAG_DIR_SERV       ((DWORD)4)
#define ACCT_FLAG_ALL            (ACCT_FLAG_NEWS | ACCT_FLAG_MAIL | ACCT_FLAG_DIR_SERV)

// --------------------------------------------------------------------------------
// Server Types
// --------------------------------------------------------------------------------
#define SRV_NNTP                 ((DWORD)1)
#define SRV_IMAP                 ((DWORD)2)
#define SRV_POP3                 ((DWORD)4)
#define SRV_SMTP                 ((DWORD)8)
#define SRV_LDAP                 ((DWORD)16)
#define SRV_HTTPMAIL             ((DWORD)32)
#define SRV_MAIL                 ((DWORD)(SRV_IMAP | SRV_POP3 | SRV_SMTP | SRV_HTTPMAIL))
#define SRV_ALL                  ((DWORD)(SRV_NNTP | SRV_IMAP | SRV_POP3 | SRV_SMTP | SRV_LDAP | SRV_HTTPMAIL))

// --------------------------------------------------------------------------------
// LDAP Authentication Types
// --------------------------------------------------------------------------------
#define LDAP_AUTH_ANONYMOUS      ((DWORD)0)
#define LDAP_AUTH_PASSWORD       ((DWORD)1)
#define LDAP_AUTH_MEMBER_SYSTEM  ((DWORD)2)
#define LDAP_AUTH_MAX            ((DWORD)2)

// --------------------------------------------------------------------------------
// LDAP Paged Result Support Types
// --------------------------------------------------------------------------------
#define LDAP_PRESULT_UNKNOWN         ((DWORD)0)
#define LDAP_PRESULT_SUPPORTED       ((DWORD)1)
#define LDAP_PRESULT_NOTSUPPORTED    ((DWORD)2)
#define LDAP_PRESULT_MAX             ((DWORD)2)

// --------------------------------------------------------------------------------
// LDAP NTDS Types
// --------------------------------------------------------------------------------
#define LDAP_NTDS_UNKNOWN      ((DWORD)0)
#define LDAP_NTDS_IS           ((DWORD)1)
#define LDAP_NTDS_ISNOT        ((DWORD)2)
#define LDAP_NTDS_MAX          ((DWORD)2)

// --------------------------------------------------------------------------------
// AP_NNTP_POST_FORMAT types
// --------------------------------------------------------------------------------
#define POST_USE_DEFAULT         ((DWORD)0)
#define POST_USE_PLAIN_TEXT      ((DWORD)1)
#define POST_USE_HTML            ((DWORD)2)

// --------------------------------------------------------------------------------
// AP_NNTP_USER_INFORMATION types
// --------------------------------------------------------------------------------
#define NNTP_USER_AUTOCONFIG     ((DWORD)0)
#define NNTP_USER_NOTIFIED       ((DWORD)1)
#define NNTP_USER_DONTSHOW       ((DWORD)2)
#define NNTP_USER_APPROVED       ((DWORD)3)

// -----------------------------------------------------------------------------
// Account Manager Notification Types
// -----------------------------------------------------------------------------
#define AN_ACCOUNT_DELETED           WM_USER + 1
#define AN_ACCOUNT_ADDED             WM_USER + 2
#define AN_ACCOUNT_CHANGED           WM_USER + 3
#define AN_DEFAULT_CHANGED           WM_USER + 4
#define AN_SERVERTYPES_CHANGED       WM_USER + 5
#define AN_ACCOUNT_PREDELETE         WM_USER + 6

// IImnAccountManager::AccountListDialog flags
// IImnAccount::ShowProperties flags
#define ACCTDLG_NO_IMAP          0x0001
#define ACCTDLG_NO_REMOVEDELETE  0x0002
#define ACCTDLG_NO_BREAKMESSAGES 0x0004
#define ACCTDLG_NO_REMOVEAFTER   0x0008
#define ACCTDLG_NO_SENDRECEIVE   0x0010
#define ACCTDLG_NO_NEWSPOLL      0x0020
#define ACCTDLG_NO_SECURITY      0x0040
#define ACCTDLG_BACKUP_CONNECT   0x0080
#define ACCTDLG_NO_IMAPPOLL      0x0100
#define ACCTDLG_NO_NEW_POP       0x0200
#define ACCTDLG_SHOWIMAPSPECIAL  0x0400
#define ACCTDLG_INTERNETCONNECTION   0x0800
#define ACCTDLG_HTTPMAIL             0x1000
#define ACCTDLG_REVOCATION           0x2000
#define ACCTDLG_OE                   0x4000
#define ACCTDLG_PASSPORT             0x8000
#define ACCTDLG_ALL                  0xffff

// AP_RAS_CONNECTION_TYPE            values
#define CONNECTION_TYPE_LAN               0
#define CONNECTION_TYPE_MANUAL            1
#define CONNECTION_TYPE_RAS               2
#define CONNECTION_TYPE_INETSETTINGS      3

// IImnAccount::DoWizard flags
#define ACCT_WIZ_MIGRATE             0x0001
#define ACCT_WIZ_MAILIMPORT          0x0002
#define ACCT_WIZ_OUTLOOK             0x0004
#define ACCT_WIZ_NEWSIMPORT          0x0008
#define ACCT_WIZ_NO_NEW_POP          0x0010
#define ACCT_WIZ_INTERNETCONNECTION  0x0020
#define ACCT_WIZ_HTTPMAIL            0x0040
#define ACCT_WIZ_OE                  0x0080
#define ACCT_WIZ_COMMUNITIES         0x0100

// IImnAccount::InitEx flags
#define ACCT_INIT_ATHENA         0x0001
#define ACCT_INIT_OUTLOOK        0x0002

 // AP_IMAP_DIRTY flags
#define IMAP_FLDRLIST_DIRTY      0x0001
#define IMAP_OE4MIGRATE_DIRTY    0x0002
#define IMAP_SENTITEMS_DIRTY     0x0004
#define IMAP_DRAFTS_DIRTY        0x0008
#define IMAP_DELETEDITEMS_DIRTY  0x0010
#define IMAP_JUNK_DIRTY          0x0020
typedef 
enum tagSMTPAUTHTYPE
    {	SMTP_AUTH_NONE	= 0,
	SMTP_AUTH_SICILY	= ( SMTP_AUTH_NONE + 1 ) ,
	SMTP_AUTH_USE_POP3ORIMAP_SETTINGS	= ( SMTP_AUTH_SICILY + 1 ) ,
	SMTP_AUTH_USE_SMTP_SETTINGS	= ( SMTP_AUTH_USE_POP3ORIMAP_SETTINGS + 1 ) 
    } 	SMTPAUTHTYPE;

#define	CCHMAX_ORG_NAME	( 256 )

#define	CCHMAX_DISPLAY_NAME	( 256 )

#define	CCHMAX_ACCOUNT_NAME	( 256 )

#define	CCHMAX_SERVER_NAME	( 256 )

#define	CCHMAX_PASSWORD	( 256 )

#define	CCHMAX_USERNAME	( 256 )

#define	CCHMAX_EMAIL_ADDRESS	( 256 )

#define	CCHMAX_CONNECTOID	( 256 )

#define	CCHMAX_SEARCH_BASE	( 256 )

#define	CCHMAX_ROOT_FOLDER	( 256 )

#define	CCHMAX_SIGNATURE	( 16 )

#define	CCHMAX_SERVICE	( 256 )



extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_0000_v0_0_s_ifspec;

#ifndef __IImnAdviseAccount_INTERFACE_DEFINED__
#define __IImnAdviseAccount_INTERFACE_DEFINED__

/* interface IImnAdviseAccount */
/* [unique][uuid][object] */ 

typedef /* [public][public][public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL_IImnAdviseAccount_0001
    {	ACCT_NEWS	= 0,
	ACCT_MAIL	= ( ACCT_NEWS + 1 ) ,
	ACCT_DIR_SERV	= ( ACCT_MAIL + 1 ) ,
	ACCT_LAST	= ( ACCT_DIR_SERV + 1 ) 
    } 	ACCTTYPE;

typedef struct tagAccountContext
    {
    ACCTTYPE AcctType;
    LPSTR pszAccountID;
    LPSTR pszOldName;
    DWORD dwServerType;
    } 	ACTX;


EXTERN_C const IID IID_IImnAdviseAccount;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0A06BD31-166F-11d0-81B9-00C04FD85AB4")
    IImnAdviseAccount : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AdviseAccount( 
            /* [in] */ DWORD dwAdviseType,
            /* [in] */ ACTX *pAcctCtx) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnAdviseAccountVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAdviseAccount * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAdviseAccount * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAdviseAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *AdviseAccount )( 
            IImnAdviseAccount * This,
            /* [in] */ DWORD dwAdviseType,
            /* [in] */ ACTX *pAcctCtx);
        
        END_INTERFACE
    } IImnAdviseAccountVtbl;

    interface IImnAdviseAccount
    {
        CONST_VTBL struct IImnAdviseAccountVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAdviseAccount_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnAdviseAccount_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnAdviseAccount_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnAdviseAccount_AdviseAccount(This,dwAdviseType,pAcctCtx)	\
    ( (This)->lpVtbl -> AdviseAccount(This,dwAdviseType,pAcctCtx) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnAdviseAccount_INTERFACE_DEFINED__ */


#ifndef __IImnAdviseMigrateServer_INTERFACE_DEFINED__
#define __IImnAdviseMigrateServer_INTERFACE_DEFINED__

/* interface IImnAdviseMigrateServer */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IImnAdviseMigrateServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8D0AED11-1638-11d0-81B9-00C04FD85AB4")
    IImnAdviseMigrateServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MigrateServer( 
            /* [in] */ DWORD dwSrvType,
            /* [in] */ IImnAccount *pAccount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnAdviseMigrateServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAdviseMigrateServer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAdviseMigrateServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAdviseMigrateServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *MigrateServer )( 
            IImnAdviseMigrateServer * This,
            /* [in] */ DWORD dwSrvType,
            /* [in] */ IImnAccount *pAccount);
        
        END_INTERFACE
    } IImnAdviseMigrateServerVtbl;

    interface IImnAdviseMigrateServer
    {
        CONST_VTBL struct IImnAdviseMigrateServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAdviseMigrateServer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnAdviseMigrateServer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnAdviseMigrateServer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnAdviseMigrateServer_MigrateServer(This,dwSrvType,pAccount)	\
    ( (This)->lpVtbl -> MigrateServer(This,dwSrvType,pAccount) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnAdviseMigrateServer_INTERFACE_DEFINED__ */


#ifndef __IImnEnumAccounts_INTERFACE_DEFINED__
#define __IImnEnumAccounts_INTERFACE_DEFINED__

/* interface IImnEnumAccounts */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IImnEnumAccounts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465483-1384-11d0-ABBD-0020AFDFD10A")
    IImnEnumAccounts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
            /* [out] */ ULONG *pcItems) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SortByAccountName( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNext( 
            /* [out] */ IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnEnumAccountsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnEnumAccounts * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnEnumAccounts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnEnumAccounts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IImnEnumAccounts * This,
            /* [out] */ ULONG *pcItems);
        
        HRESULT ( STDMETHODCALLTYPE *SortByAccountName )( 
            IImnEnumAccounts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetNext )( 
            IImnEnumAccounts * This,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IImnEnumAccounts * This);
        
        END_INTERFACE
    } IImnEnumAccountsVtbl;

    interface IImnEnumAccounts
    {
        CONST_VTBL struct IImnEnumAccountsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnEnumAccounts_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnEnumAccounts_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnEnumAccounts_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnEnumAccounts_GetCount(This,pcItems)	\
    ( (This)->lpVtbl -> GetCount(This,pcItems) ) 

#define IImnEnumAccounts_SortByAccountName(This)	\
    ( (This)->lpVtbl -> SortByAccountName(This) ) 

#define IImnEnumAccounts_GetNext(This,ppAccount)	\
    ( (This)->lpVtbl -> GetNext(This,ppAccount) ) 

#define IImnEnumAccounts_Reset(This)	\
    ( (This)->lpVtbl -> Reset(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnEnumAccounts_INTERFACE_DEFINED__ */


#ifndef __IImnAccountManager_INTERFACE_DEFINED__
#define __IImnAccountManager_INTERFACE_DEFINED__

/* interface IImnAccountManager */
/* [unique][uuid][object] */ 

typedef struct tagACCTLISTINFO
    {
    DWORD cbSize;
    ACCTTYPE AcctTypeInit;
    DWORD dwAcctFlags;
    DWORD dwFlags;
    } 	ACCTLISTINFO;




EXTERN_C const IID IID_IImnAccountManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465481-1384-11d0-ABBD-0020AFDFD10A")
    IImnAccountManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAccountObject( 
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enumerate( 
            /* [in] */ DWORD dwSrvTypes,
            /* [out] */ IImnEnumAccounts **ppEnumAccounts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAccountCount( 
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ ULONG *pcServers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindAccount( 
            /* [in] */ DWORD dwPropTag,
            /* [in] */ LPCSTR pszSearchData,
            /* [out] */ IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAccount( 
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultAccountName( 
            /* [in] */ ACCTTYPE AcctType,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR pszAccount,
            /* [in] */ ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessNotification( 
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateDefaultSendAccount( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AccountListDialog( 
            /* [in] */ HWND hwnd,
            /* [in] */ ACCTLISTINFO *pinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
            /* [in] */ IImnAdviseAccount *pAdviseAccount,
            /* [out] */ DWORD *pdwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
            /* [in] */ DWORD dwConnection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUniqueAccountName( 
            /* [in] */ LPSTR szName,
            /* [in] */ UINT cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitEx( 
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer,
            /* [in] */ DWORD dwFlags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnAccountManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccountManager * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccountManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccountManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IImnAccountManager * This,
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAccountObject )( 
            IImnAccountManager * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Enumerate )( 
            IImnAccountManager * This,
            /* [in] */ DWORD dwSrvTypes,
            /* [out] */ IImnEnumAccounts **ppEnumAccounts);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountCount )( 
            IImnAccountManager * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ ULONG *pcServers);
        
        HRESULT ( STDMETHODCALLTYPE *FindAccount )( 
            IImnAccountManager * This,
            /* [in] */ DWORD dwPropTag,
            /* [in] */ LPCSTR pszSearchData,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccount )( 
            IImnAccountManager * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccountName )( 
            IImnAccountManager * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR pszAccount,
            /* [in] */ ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessNotification )( 
            IImnAccountManager * This,
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateDefaultSendAccount )( 
            IImnAccountManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccountListDialog )( 
            IImnAccountManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ACCTLISTINFO *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IImnAccountManager * This,
            /* [in] */ IImnAdviseAccount *pAdviseAccount,
            /* [out] */ DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IImnAccountManager * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueAccountName )( 
            IImnAccountManager * This,
            /* [in] */ LPSTR szName,
            /* [in] */ UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *InitEx )( 
            IImnAccountManager * This,
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer,
            /* [in] */ DWORD dwFlags);
        
        END_INTERFACE
    } IImnAccountManagerVtbl;

    interface IImnAccountManager
    {
        CONST_VTBL struct IImnAccountManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccountManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnAccountManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnAccountManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnAccountManager_Init(This,pAdviseMigrateServer)	\
    ( (This)->lpVtbl -> Init(This,pAdviseMigrateServer) ) 

#define IImnAccountManager_CreateAccountObject(This,AcctType,ppAccount)	\
    ( (This)->lpVtbl -> CreateAccountObject(This,AcctType,ppAccount) ) 

#define IImnAccountManager_Enumerate(This,dwSrvTypes,ppEnumAccounts)	\
    ( (This)->lpVtbl -> Enumerate(This,dwSrvTypes,ppEnumAccounts) ) 

#define IImnAccountManager_GetAccountCount(This,AcctType,pcServers)	\
    ( (This)->lpVtbl -> GetAccountCount(This,AcctType,pcServers) ) 

#define IImnAccountManager_FindAccount(This,dwPropTag,pszSearchData,ppAccount)	\
    ( (This)->lpVtbl -> FindAccount(This,dwPropTag,pszSearchData,ppAccount) ) 

#define IImnAccountManager_GetDefaultAccount(This,AcctType,ppAccount)	\
    ( (This)->lpVtbl -> GetDefaultAccount(This,AcctType,ppAccount) ) 

#define IImnAccountManager_GetDefaultAccountName(This,AcctType,pszAccount,cchMax)	\
    ( (This)->lpVtbl -> GetDefaultAccountName(This,AcctType,pszAccount,cchMax) ) 

#define IImnAccountManager_ProcessNotification(This,uMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessNotification(This,uMsg,wParam,lParam) ) 

#define IImnAccountManager_ValidateDefaultSendAccount(This)	\
    ( (This)->lpVtbl -> ValidateDefaultSendAccount(This) ) 

#define IImnAccountManager_AccountListDialog(This,hwnd,pinfo)	\
    ( (This)->lpVtbl -> AccountListDialog(This,hwnd,pinfo) ) 

#define IImnAccountManager_Advise(This,pAdviseAccount,pdwConnection)	\
    ( (This)->lpVtbl -> Advise(This,pAdviseAccount,pdwConnection) ) 

#define IImnAccountManager_Unadvise(This,dwConnection)	\
    ( (This)->lpVtbl -> Unadvise(This,dwConnection) ) 

#define IImnAccountManager_GetUniqueAccountName(This,szName,cch)	\
    ( (This)->lpVtbl -> GetUniqueAccountName(This,szName,cch) ) 

#define IImnAccountManager_InitEx(This,pAdviseMigrateServer,dwFlags)	\
    ( (This)->lpVtbl -> InitEx(This,pAdviseMigrateServer,dwFlags) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnAccountManager_INTERFACE_DEFINED__ */


#ifndef __IImnAccountManager2_INTERFACE_DEFINED__
#define __IImnAccountManager2_INTERFACE_DEFINED__

/* interface IImnAccountManager2 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IImnAccountManager2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C43DFC6F-62BB-11d2-A727-00C04F79E7C8")
    IImnAccountManager2 : public IImnAccountManager
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitUser( 
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer,
            /* [in] */ REFGUID rguidID,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIncompleteAccount( 
            /* [in] */ ACCTTYPE AcctType,
            /* [ref][in] */ LPSTR pszAccountId,
            /* [in] */ ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetIncompleteAccount( 
            /* [in] */ ACCTTYPE AcctType,
            /* [unique][in] */ LPCSTR pszAccountId) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnAccountManager2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccountManager2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccountManager2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccountManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IImnAccountManager2 * This,
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAccountObject )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *Enumerate )( 
            IImnAccountManager2 * This,
            /* [in] */ DWORD dwSrvTypes,
            /* [out] */ IImnEnumAccounts **ppEnumAccounts);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountCount )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ ULONG *pcServers);
        
        HRESULT ( STDMETHODCALLTYPE *FindAccount )( 
            IImnAccountManager2 * This,
            /* [in] */ DWORD dwPropTag,
            /* [in] */ LPCSTR pszSearchData,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccount )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [out] */ IImnAccount **ppAccount);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultAccountName )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR pszAccount,
            /* [in] */ ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessNotification )( 
            IImnAccountManager2 * This,
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateDefaultSendAccount )( 
            IImnAccountManager2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AccountListDialog )( 
            IImnAccountManager2 * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ACCTLISTINFO *pinfo);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            IImnAccountManager2 * This,
            /* [in] */ IImnAdviseAccount *pAdviseAccount,
            /* [out] */ DWORD *pdwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            IImnAccountManager2 * This,
            /* [in] */ DWORD dwConnection);
        
        HRESULT ( STDMETHODCALLTYPE *GetUniqueAccountName )( 
            IImnAccountManager2 * This,
            /* [in] */ LPSTR szName,
            /* [in] */ UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *InitEx )( 
            IImnAccountManager2 * This,
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *InitUser )( 
            IImnAccountManager2 * This,
            /* [in] */ IImnAdviseMigrateServer *pAdviseMigrateServer,
            /* [in] */ REFGUID rguidID,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetIncompleteAccount )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [ref][in] */ LPSTR pszAccountId,
            /* [in] */ ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetIncompleteAccount )( 
            IImnAccountManager2 * This,
            /* [in] */ ACCTTYPE AcctType,
            /* [unique][in] */ LPCSTR pszAccountId);
        
        END_INTERFACE
    } IImnAccountManager2Vtbl;

    interface IImnAccountManager2
    {
        CONST_VTBL struct IImnAccountManager2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccountManager2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnAccountManager2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnAccountManager2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnAccountManager2_Init(This,pAdviseMigrateServer)	\
    ( (This)->lpVtbl -> Init(This,pAdviseMigrateServer) ) 

#define IImnAccountManager2_CreateAccountObject(This,AcctType,ppAccount)	\
    ( (This)->lpVtbl -> CreateAccountObject(This,AcctType,ppAccount) ) 

#define IImnAccountManager2_Enumerate(This,dwSrvTypes,ppEnumAccounts)	\
    ( (This)->lpVtbl -> Enumerate(This,dwSrvTypes,ppEnumAccounts) ) 

#define IImnAccountManager2_GetAccountCount(This,AcctType,pcServers)	\
    ( (This)->lpVtbl -> GetAccountCount(This,AcctType,pcServers) ) 

#define IImnAccountManager2_FindAccount(This,dwPropTag,pszSearchData,ppAccount)	\
    ( (This)->lpVtbl -> FindAccount(This,dwPropTag,pszSearchData,ppAccount) ) 

#define IImnAccountManager2_GetDefaultAccount(This,AcctType,ppAccount)	\
    ( (This)->lpVtbl -> GetDefaultAccount(This,AcctType,ppAccount) ) 

#define IImnAccountManager2_GetDefaultAccountName(This,AcctType,pszAccount,cchMax)	\
    ( (This)->lpVtbl -> GetDefaultAccountName(This,AcctType,pszAccount,cchMax) ) 

#define IImnAccountManager2_ProcessNotification(This,uMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessNotification(This,uMsg,wParam,lParam) ) 

#define IImnAccountManager2_ValidateDefaultSendAccount(This)	\
    ( (This)->lpVtbl -> ValidateDefaultSendAccount(This) ) 

#define IImnAccountManager2_AccountListDialog(This,hwnd,pinfo)	\
    ( (This)->lpVtbl -> AccountListDialog(This,hwnd,pinfo) ) 

#define IImnAccountManager2_Advise(This,pAdviseAccount,pdwConnection)	\
    ( (This)->lpVtbl -> Advise(This,pAdviseAccount,pdwConnection) ) 

#define IImnAccountManager2_Unadvise(This,dwConnection)	\
    ( (This)->lpVtbl -> Unadvise(This,dwConnection) ) 

#define IImnAccountManager2_GetUniqueAccountName(This,szName,cch)	\
    ( (This)->lpVtbl -> GetUniqueAccountName(This,szName,cch) ) 

#define IImnAccountManager2_InitEx(This,pAdviseMigrateServer,dwFlags)	\
    ( (This)->lpVtbl -> InitEx(This,pAdviseMigrateServer,dwFlags) ) 


#define IImnAccountManager2_InitUser(This,pAdviseMigrateServer,rguidID,dwFlags)	\
    ( (This)->lpVtbl -> InitUser(This,pAdviseMigrateServer,rguidID,dwFlags) ) 

#define IImnAccountManager2_GetIncompleteAccount(This,AcctType,pszAccountId,cchMax)	\
    ( (This)->lpVtbl -> GetIncompleteAccount(This,AcctType,pszAccountId,cchMax) ) 

#define IImnAccountManager2_SetIncompleteAccount(This,AcctType,pszAccountId)	\
    ( (This)->lpVtbl -> SetIncompleteAccount(This,AcctType,pszAccountId) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnAccountManager2_INTERFACE_DEFINED__ */


#ifndef __IPropertyContainer_INTERFACE_DEFINED__
#define __IPropertyContainer_INTERFACE_DEFINED__

/* interface IPropertyContainer */
/* [unique][uuid][object] */ 

typedef /* [public] */ 
enum __MIDL_IPropertyContainer_0001
    {	TYPE_ERROR	= 1000,
	TYPE_DWORD	= ( TYPE_ERROR + 1 ) ,
	TYPE_LONG	= ( TYPE_DWORD + 1 ) ,
	TYPE_WORD	= ( TYPE_LONG + 1 ) ,
	TYPE_SHORT	= ( TYPE_WORD + 1 ) ,
	TYPE_BYTE	= ( TYPE_SHORT + 1 ) ,
	TYPE_CHAR	= ( TYPE_BYTE + 1 ) ,
	TYPE_FILETIME	= ( TYPE_CHAR + 1 ) ,
	TYPE_STRING	= ( TYPE_FILETIME + 1 ) ,
	TYPE_BINARY	= ( TYPE_STRING + 1 ) ,
	TYPE_FLAGS	= ( TYPE_BINARY + 1 ) ,
	TYPE_STREAM	= ( TYPE_FLAGS + 1 ) ,
	TYPE_WSTRING	= ( TYPE_STREAM + 1 ) ,
	TYPE_BOOL	= ( TYPE_WSTRING + 1 ) ,
	TYPE_PASS	= ( TYPE_BOOL + 1 ) ,
	TYPE_ULARGEINTEGER	= ( TYPE_PASS + 1 ) ,
	TYPE_LAST	= ( TYPE_ULARGEINTEGER + 1 ) 
    } 	PROPTYPE;






EXTERN_C const IID IID_IPropertyContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465484-1384-11d0-ABBD-0020AFDFD10A")
    IPropertyContainer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProp( 
            /* [in] */ DWORD dwPropTag,
            /* [unique][ref][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG *pcb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropDw( 
            /* [in] */ DWORD dwPropTag,
            /* [out] */ DWORD *pdw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropSz( 
            /* [in] */ DWORD dwPropTag,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR psz,
            /* [in] */ ULONG cchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProp( 
            /* [in] */ DWORD dwPropTag,
            /* [unique][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropDw( 
            /* [in] */ DWORD dwPropTag,
            /* [in] */ DWORD dw) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropSz( 
            /* [in] */ DWORD dwPropTag,
            /* [unique][in] */ 
            __in  LPSTR psz) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPropertyContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyContainer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][ref][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropDw )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [out] */ DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropSz )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR psz,
            /* [in] */ ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropDw )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [in] */ DWORD dw);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropSz )( 
            IPropertyContainer * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][in] */ 
            __in  LPSTR psz);
        
        END_INTERFACE
    } IPropertyContainerVtbl;

    interface IPropertyContainer
    {
        CONST_VTBL struct IPropertyContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyContainer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPropertyContainer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPropertyContainer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPropertyContainer_GetProp(This,dwPropTag,pb,pcb)	\
    ( (This)->lpVtbl -> GetProp(This,dwPropTag,pb,pcb) ) 

#define IPropertyContainer_GetPropDw(This,dwPropTag,pdw)	\
    ( (This)->lpVtbl -> GetPropDw(This,dwPropTag,pdw) ) 

#define IPropertyContainer_GetPropSz(This,dwPropTag,psz,cchMax)	\
    ( (This)->lpVtbl -> GetPropSz(This,dwPropTag,psz,cchMax) ) 

#define IPropertyContainer_SetProp(This,dwPropTag,pb,cb)	\
    ( (This)->lpVtbl -> SetProp(This,dwPropTag,pb,cb) ) 

#define IPropertyContainer_SetPropDw(This,dwPropTag,dw)	\
    ( (This)->lpVtbl -> SetPropDw(This,dwPropTag,dw) ) 

#define IPropertyContainer_SetPropSz(This,dwPropTag,psz)	\
    ( (This)->lpVtbl -> SetPropSz(This,dwPropTag,psz) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPropertyContainer_INTERFACE_DEFINED__ */


#ifndef __IImnAccount_INTERFACE_DEFINED__
#define __IImnAccount_INTERFACE_DEFINED__

/* interface IImnAccount */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IImnAccount;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD465482-1384-11d0-ABBD-0020AFDFD10A")
    IImnAccount : public IPropertyContainer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Exist( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAsDefault( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveChanges( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAccountType( 
            /* [out] */ ACCTTYPE *pAcctType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetServerTypes( 
            /* [out] */ DWORD *pdwSrvTypes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShowProperties( 
            /* [in] */ HWND hwnd,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateProperty( 
            /* [in] */ DWORD dwPropTag,
            /* [size_is][in] */ BYTE *pb,
            /* [in] */ ULONG cb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoWizard( 
            /* [in] */ HWND hwnd,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoImportWizard( 
            /* [in] */ HWND hwnd,
            /* [in] */ CLSID clsid,
            /* [in] */ DWORD dwFlags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImnAccountVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImnAccount * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImnAccount * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetProp )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][ref][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG *pcb);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropDw )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [out] */ DWORD *pdw);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropSz )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [size_is][out] */ 
            __out_ecount(cchMax)  LPSTR psz,
            /* [in] */ ULONG cchMax);
        
        HRESULT ( STDMETHODCALLTYPE *SetProp )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][size_is][in] */ BYTE *pb,
            /* [in] */ ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropDw )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [in] */ DWORD dw);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropSz )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [unique][in] */ 
            __in  LPSTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *Exist )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAsDefault )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *SaveChanges )( 
            IImnAccount * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetAccountType )( 
            IImnAccount * This,
            /* [out] */ ACCTTYPE *pAcctType);
        
        HRESULT ( STDMETHODCALLTYPE *GetServerTypes )( 
            IImnAccount * This,
            /* [out] */ DWORD *pdwSrvTypes);
        
        HRESULT ( STDMETHODCALLTYPE *ShowProperties )( 
            IImnAccount * This,
            /* [in] */ HWND hwnd,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ValidateProperty )( 
            IImnAccount * This,
            /* [in] */ DWORD dwPropTag,
            /* [size_is][in] */ BYTE *pb,
            /* [in] */ ULONG cb);
        
        HRESULT ( STDMETHODCALLTYPE *DoWizard )( 
            IImnAccount * This,
            /* [in] */ HWND hwnd,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DoImportWizard )( 
            IImnAccount * This,
            /* [in] */ HWND hwnd,
            /* [in] */ CLSID clsid,
            /* [in] */ DWORD dwFlags);
        
        END_INTERFACE
    } IImnAccountVtbl;

    interface IImnAccount
    {
        CONST_VTBL struct IImnAccountVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImnAccount_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImnAccount_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImnAccount_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImnAccount_GetProp(This,dwPropTag,pb,pcb)	\
    ( (This)->lpVtbl -> GetProp(This,dwPropTag,pb,pcb) ) 

#define IImnAccount_GetPropDw(This,dwPropTag,pdw)	\
    ( (This)->lpVtbl -> GetPropDw(This,dwPropTag,pdw) ) 

#define IImnAccount_GetPropSz(This,dwPropTag,psz,cchMax)	\
    ( (This)->lpVtbl -> GetPropSz(This,dwPropTag,psz,cchMax) ) 

#define IImnAccount_SetProp(This,dwPropTag,pb,cb)	\
    ( (This)->lpVtbl -> SetProp(This,dwPropTag,pb,cb) ) 

#define IImnAccount_SetPropDw(This,dwPropTag,dw)	\
    ( (This)->lpVtbl -> SetPropDw(This,dwPropTag,dw) ) 

#define IImnAccount_SetPropSz(This,dwPropTag,psz)	\
    ( (This)->lpVtbl -> SetPropSz(This,dwPropTag,psz) ) 


#define IImnAccount_Exist(This)	\
    ( (This)->lpVtbl -> Exist(This) ) 

#define IImnAccount_SetAsDefault(This)	\
    ( (This)->lpVtbl -> SetAsDefault(This) ) 

#define IImnAccount_Delete(This)	\
    ( (This)->lpVtbl -> Delete(This) ) 

#define IImnAccount_SaveChanges(This)	\
    ( (This)->lpVtbl -> SaveChanges(This) ) 

#define IImnAccount_GetAccountType(This,pAcctType)	\
    ( (This)->lpVtbl -> GetAccountType(This,pAcctType) ) 

#define IImnAccount_GetServerTypes(This,pdwSrvTypes)	\
    ( (This)->lpVtbl -> GetServerTypes(This,pdwSrvTypes) ) 

#define IImnAccount_ShowProperties(This,hwnd,dwFlags)	\
    ( (This)->lpVtbl -> ShowProperties(This,hwnd,dwFlags) ) 

#define IImnAccount_ValidateProperty(This,dwPropTag,pb,cb)	\
    ( (This)->lpVtbl -> ValidateProperty(This,dwPropTag,pb,cb) ) 

#define IImnAccount_DoWizard(This,hwnd,dwFlags)	\
    ( (This)->lpVtbl -> DoWizard(This,hwnd,dwFlags) ) 

#define IImnAccount_DoImportWizard(This,hwnd,clsid,dwFlags)	\
    ( (This)->lpVtbl -> DoImportWizard(This,hwnd,clsid,dwFlags) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IImnAccount_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_imnact_0000_0007 */
/* [local] */ 

#if defined( WIN16 ) && !defined( __cplusplus )
// {8D4B04E1-1331-11d0-81B8-00C04FD85AB4}
EXTERN_C const IID CLSID_ImnAccountManager =
    { 0x8d4b04e1, 0x1331, 0x11d0, { 0x81, 0xb8, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

// {FD465484-1384-11d0-ABBD-0020AFDFD10A}
EXTERN_C const IID IID_IPropertyContainer =
    { 0xfd465484, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

// {FD465481-1384-11d0-ABBD-0020AFDFD10A}
EXTERN_C const IID IID_IImnAccountManager =
    { 0xfd465481, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

// {C43DFC6F-62BB-11d2-A727-00C04F79E7C8}
EXTERN_C const IID IID_IImnAccountManager2 =
    { 0xc43dfc6f, 0x62bb, 0x11d2, { 0xa7, 0x27, 0x0, 0xc0, 0x4f, 0x79, 0xe7, 0xc8 } };

// {FD465482-1384-11d0-ABBD-0020AFDFD10A}
EXTERN_C const IID IID_IImnAccount =
    { 0xfd465482, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

// {FD465483-1384-11d0-ABBD-0020AFDFD10A}
EXTERN_C const IID IID_IImnEnumAccounts =
    { 0xfd465483, 0x1384, 0x11d0, { 0xab, 0xbd, 0x0, 0x20, 0xaf, 0xdf, 0xd1, 0xa } };

// {8D0AED11-1638-11d0-81B9-00C04FD85AB4}
EXTERN_C const IID IID_IImnAdviseMigrateServer =
    { 0x8d0aed11, 0x1638, 0x11d0, { 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

// {0A06BD31-166F-11d0-81B9-00C04FD85AB4}
EXTERN_C const IID IID_IImnAdviseAccount =
    { 0xa06bd31, 0x166f, 0x11d0, { 0x81, 0xb9, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4 } };

#endif //WIN16 && !__cplusplus


extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_0007_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_imnact_0000_0007_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


