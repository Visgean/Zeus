

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Fri Jul 30 00:47:33 2010
 */
/* Compiler settings for msoeapi.idl:
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

#ifndef __msoeapi_h__
#define __msoeapi_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IOutlookExpress_FWD_DEFINED__
#define __IOutlookExpress_FWD_DEFINED__
typedef interface IOutlookExpress IOutlookExpress;
#endif 	/* __IOutlookExpress_FWD_DEFINED__ */


#ifndef __IStoreNamespace_FWD_DEFINED__
#define __IStoreNamespace_FWD_DEFINED__
typedef interface IStoreNamespace IStoreNamespace;
#endif 	/* __IStoreNamespace_FWD_DEFINED__ */


#ifndef __IStoreFolder_FWD_DEFINED__
#define __IStoreFolder_FWD_DEFINED__
typedef interface IStoreFolder IStoreFolder;
#endif 	/* __IStoreFolder_FWD_DEFINED__ */


#ifndef __IStoreNamespace2_FWD_DEFINED__
#define __IStoreNamespace2_FWD_DEFINED__
typedef interface IStoreNamespace2 IStoreNamespace2;
#endif 	/* __IStoreNamespace2_FWD_DEFINED__ */


#ifndef __IStoreFolder2_FWD_DEFINED__
#define __IStoreFolder2_FWD_DEFINED__
typedef interface IStoreFolder2 IStoreFolder2;
#endif 	/* __IStoreFolder2_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "mimeole.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_msoeapi_0000_0000 */
/* [local] */ 

#ifndef MSOEAPI_H
#define MSOEAPI_H



//+-------------------------------------------------------------------------
// GUID Definitions
//--------------------------------------------------------------------------
#pragma comment(lib,"uuid.lib")

// {3338DF69-4660-11d1-8A8D-00C04FB951F3}
DEFINE_GUID(CLSID_OutlookExpress, 0x3338df69, 0x4660, 0x11d1, 0x8a, 0x8d, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

// {0006F01A-0000-0000-C000-0000000046}
DEFINE_GUID(CLSID_Envelope, 0x0006F01A, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

// {3338DF6A-4660-11d1-8A8D-00C04FB951F3}
DEFINE_GUID(IID_IOutlookExpress, 0x3338df6a, 0x4660, 0x11d1, 0x8a, 0x8d, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

// {E70C92A9-4BFD-11d1-8A95-00C04FB951F3}
DEFINE_GUID(CLSID_StoreNamespace, 0xe70c92a9, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

// {E70C92AA-4BFD-11d1-8A95-00C04FB951F3}
DEFINE_GUID(IID_IStoreNamespace, 0xe70c92aa, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

// {E70C92AC-4BFD-11d1-8A95-00C04FB951F3}
DEFINE_GUID(IID_IStoreFolder, 0xe70c92ac, 0x4bfd, 0x11d1, 0x8a, 0x95, 0x0, 0xc0, 0x4f, 0xb9, 0x51, 0xf3);

//+-------------------------------------------------------------------------
// Errors Definition Macros
//--------------------------------------------------------------------------
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

//+-------------------------------------------------------------------------
// MSOEAPI Failure Return Values
//--------------------------------------------------------------------------
#define MSOEAPI_E_FILE_NOT_FOUND             HR_E(0xCF65)
#define MSOEAPI_E_STORE_INITIALIZE           HR_E(0xCF66)
#define MSOEAPI_E_INVALID_STRUCT_SIZE        HR_E(0xCF67)
#define MSOEAPI_E_CANT_LOAD_MSOERT           HR_E(0xCF68)
#define MSOEAPI_E_CANT_LOAD_INETCOMM         HR_E(0xCF69)
#define MSOEAPI_E_CANT_LOAD_MSOEACCT         HR_E(0xCF70)
#define MSOEAPI_E_CANT_MSOERT_BADVER         HR_E(0xCF71)
#define MSOEAPI_E_CANT_INETCOMM_BADVER       HR_E(0xCF72)
#define MSOEAPI_E_CANT_MSOEACCT_BADVER       HR_E(0xCF73)

//+-------------------------------------------------------------------------
// String Definition Macros
//--------------------------------------------------------------------------
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#ifndef STRCONSTA
#ifdef DEFINE_STRCONST
#define STRCONSTA(x,y)    EXTERN_C const char x[] = y
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[] = L##y
#else
#define STRCONSTA(x,y)    EXTERN_C const char x[]
#define STRCONSTW(x,y)    EXTERN_C const WCHAR x[]
#endif STRCONSTA
#endif

//+-------------------------------------------------------------------------
// Strings
//--------------------------------------------------------------------------
STRCONSTA(STR_MSOEAPI_INSTANCECLASS,      "OutlookExpressHiddenWindow");
STRCONSTA(STR_MSOEAPI_IPSERVERCLASS,      "OutlookExpressInProccessServer");
STRCONSTA(STR_MSOEAPI_INSTANCEMUTEX,      "Local\\OutlookExpress_InstanceMutex_101897");
STRCONSTA(STR_MSOEAPI_DLLNAME,            "MSOE.DLL");
STRCONSTA(STR_MSOEAPI_START,              "CoStartOutlookExpress");
STRCONSTA(STR_MSOEAPI_SHUTDOWN,           "CoShutdownOutlookExpress");
STRCONSTA(STR_MSOEAPI_CREATE,             "CoCreateOutlookExpress");

//+-------------------------------------------------------------------------
// Function Typedefs
//--------------------------------------------------------------------------
typedef HRESULT (APIENTRY *PFNSTART)(DWORD dwFlags, LPCSTR pszCmdLine, INT nCmdShow);
typedef HRESULT (APIENTRY *PFNSHUTDOWN)(DWORD dwReserved);
typedef HRESULT (APIENTRY *PFNCREATE)(IUnknown *pUnkOuter, IUnknown **ppUnknown);

#define	MSOEAPI_ACDM_CMDLINE	( 1 )

#define	MSOEAPI_ACDM_NOTIFY	( 2 )

#define	MSOEAPI_ACDM_ODBNOTIFY	( 3 )

#define	MSOEAPI_ACDM_STGNOTIFY	( 4 )

struct HENUMSTORE__
    {
    DWORD unused;
    } ;
typedef /* [transmit_as] */ struct HENUMSTORE__ *HENUMSTORE;

typedef HENUMSTORE *LPHENUMSTORE;

#ifndef __LPOUTLOOKEXPRESS_DEFINED
#define __LPOUTLOOKEXPRESS_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0000_v0_0_s_ifspec;

#ifndef __IOutlookExpress_INTERFACE_DEFINED__
#define __IOutlookExpress_INTERFACE_DEFINED__

/* interface IOutlookExpress */
/* [unique][uuid][object] */ 

typedef IOutlookExpress *LPOUTLOOKEXPRESS;

#define MSOEAPI_START_SHOWSPLASH         0x00000001
#define MSOEAPI_START_MESSAGEPUMP        0x00000002
#define MSOEAPI_START_ALLOWCOMPACTION    0x00000004
#define MSOEAPI_START_INSTANCEMUTEX      0x00000008
#define MSOEAPI_START_SHOWERRORS         0x00000010
#define MSOEAPI_START_APPWINDOW          0x00000020
#define MSOEAPI_START_DEFAULTIDENTITY    0x00000040
#define MSOEAPI_START_ALREADY_RUNNING    0x00000080
#define MSOEAPI_START_STOREVALIDNODELETE 0x00000100
#define MSOEAPI_START_NOSTORE            0x00000200
#define MSOEAPI_START_APPLICATION \
    (MSOEAPI_START_SHOWSPLASH      | \
     MSOEAPI_START_SHOWERRORS      | \
     MSOEAPI_START_MESSAGEPUMP     | \
     MSOEAPI_START_ALLOWCOMPACTION | \
     MSOEAPI_START_INSTANCEMUTEX   | \
     MSOEAPI_START_APPWINDOW)
#define MSOEAPI_START_COMOBJECT \
     MSOEAPI_START_SHOWERRORS

EXTERN_C const IID IID_IOutlookExpress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3338DF6A-4660-11d1-8A8D-00C04FB951F3")
    IOutlookExpress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ LPCSTR pszCmdLine,
            /* [in] */ INT nCmdShow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOutlookExpressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOutlookExpress * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOutlookExpress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOutlookExpress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IOutlookExpress * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ LPCSTR pszCmdLine,
            /* [in] */ INT nCmdShow);
        
        END_INTERFACE
    } IOutlookExpressVtbl;

    interface IOutlookExpress
    {
        CONST_VTBL struct IOutlookExpressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOutlookExpress_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IOutlookExpress_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IOutlookExpress_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IOutlookExpress_Start(This,dwFlags,pszCmdLine,nCmdShow)	\
    ( (This)->lpVtbl -> Start(This,dwFlags,pszCmdLine,nCmdShow) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IOutlookExpress_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_msoeapi_0000_0001 */
/* [local] */ 

#endif // __LPOUTLOOKEXPRESS_DEFINED
#ifndef __LPSTORENAMESPACE_DEFINED
#define __LPSTORENAMESPACE_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0001_v0_0_s_ifspec;

#ifndef __IStoreNamespace_INTERFACE_DEFINED__
#define __IStoreNamespace_INTERFACE_DEFINED__

/* interface IStoreNamespace */
/* [unique][uuid][object] */ 

typedef IStoreNamespace *LPSTORENAMESPACE;

typedef DWORD STOREFOLDERID;

typedef STOREFOLDERID *LPSTOREFOLDERID;

#define	FOLDERID_ROOT	( 0 )

#define	FOLDERID_INVALID	( 0xffffffff )

typedef DWORD MESSAGEID;

typedef MESSAGEID *LPMESSAGEID;

#define	MESSAGEID_INVALID	( 0xffffffff )

#define	MESSAGEID_FIRST	( 0xffffffff )

#define	CCHMAX_FOLDER_NAME	( 256 )

#define WM_FOLDERNOTIFY      (WM_USER + 1600) // IStoreNamespace Notification - lparam=LPFOLDERNOTIFYEX, client must call CoTaskMemFree(lParam)
#define WM_NEWMSGS           (WM_USER + 1650) // IStoreFolder Notification: wParam=MESSAGEID, lParam=Reserved
#define WM_DELETEMSGS        (WM_USER + 1651) // IStoreFolder Notification: wParam=prgdwMsgId, lParam=cMsgs
#define WM_DELETEFOLDER      (WM_USER + 1652) // IStoreFolder Notification: wParam=STOREFOLDERID or HFOLDER
#define WM_MARKEDASREAD      (WM_USER + 1653) // IStoreFolder Notification: wParamprgdwMsgId, lParam=cMsgs
#define WM_MARKEDASUNREAD    (WM_USER + 1654) // IStoreFolder Notification: wParamprgdwMsgId, lParam=cMsgs
typedef 
enum tagFOLDERNOTIFYTYPE
    {	NEW_FOLDER	= 1,
	DELETE_FOLDER	= ( NEW_FOLDER + 1 ) ,
	RENAME_FOLDER	= ( DELETE_FOLDER + 1 ) ,
	MOVE_FOLDER	= ( RENAME_FOLDER + 1 ) ,
	UNREAD_CHANGE	= ( MOVE_FOLDER + 1 ) ,
	IMAPFLAG_CHANGE	= ( UNREAD_CHANGE + 1 ) ,
	UPDATEFLAG_CHANGE	= ( IMAPFLAG_CHANGE + 1 ) ,
	FOLDER_PROPS_CHANGED	= ( UPDATEFLAG_CHANGE + 1 ) 
    } 	FOLDERNOTIFYTYPE;

typedef struct tagFOLDERNOTIFYEX
    {
    FOLDERNOTIFYTYPE type;
    STOREFOLDERID idFolderOld;
    STOREFOLDERID idFolderNew;
    } 	FOLDERNOTIFYEX;

typedef struct tagFOLDERNOTIFYEX *LPFOLDERNOTIFYEX;

typedef struct tagMESSAGEIDLIST
    {
    DWORD cbSize;
    DWORD cMsgs;
    LPMESSAGEID prgdwMsgId;
    } 	MESSAGEIDLIST;

typedef struct tagMESSAGEIDLIST *LPMESSAGEIDLIST;

typedef 
enum tagSPECIALFOLDER
    {	FOLDER_NOTSPECIAL	= -1,
	FOLDER_INBOX	= ( FOLDER_NOTSPECIAL + 1 ) ,
	FOLDER_OUTBOX	= ( FOLDER_INBOX + 1 ) ,
	FOLDER_SENT	= ( FOLDER_OUTBOX + 1 ) ,
	FOLDER_DELETED	= ( FOLDER_SENT + 1 ) ,
	FOLDER_DRAFT	= ( FOLDER_DELETED + 1 ) ,
	FOLDER_MAX	= ( FOLDER_DRAFT + 1 ) 
    } 	SPECIALFOLDER;

typedef /* [transmit_as] */ DWORD FOLDERPROPSSIZE;

typedef /* [transmit_as] */ SPECIALFOLDER SPECIALFOLDER_XMIT;

typedef struct tagFOLDERPROPS
    {
    FOLDERPROPSSIZE cbSize;
    STOREFOLDERID dwFolderId;
    INT cSubFolders;
    SPECIALFOLDER_XMIT sfType;
    DWORD cUnread;
    DWORD cMessage;
    CHAR szName[ 256 ];
    } 	FOLDERPROPS;

typedef struct tagFOLDERPROPS *LPFOLDERPROPS;

#define NAMESPACE_INITIALIZE_CURRENTIDENTITY 0x00000001
// CLocalStore::CopyMoveMessages flags - dwFlags == 0 is copy
#define CMF_MOVE                     0x0001                  // msgs deleted from src fldr after copy
#define CMF_DELETE                   0x0002                  // same as CMF_MOVE but uses delete string for status

EXTERN_C const IID IID_IStoreNamespace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E70C92AA-4BFD-11d1-8A95-00C04FB951F3")
    IStoreNamespace : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [unique][in] */ HWND hwndOwner,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectory( 
            /* [size_is][out][in] */ LPSTR pszPath,
            /* [in] */ DWORD cchMaxPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenSpecialFolder( 
            /* [in] */ SPECIALFOLDER_XMIT sfType,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenFolder( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFolder( 
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ LPCSTR pszName,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPSTOREFOLDERID pdwFolderId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameFolder( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPCSTR pszNewName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveFolder( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteFolder( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetFolderProps( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyMoveMessages( 
            /* [in] */ IStoreFolder *pSource,
            /* [in] */ IStoreFolder *pDest,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwFlagsRemove,
            /* [in] */ IProgressNotify *pProgress) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE RegisterNotification( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNotification( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompactAll( 
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFirstSubFolder( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [out][in] */ LPFOLDERPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextSubFolder( 
            /* [in] */ HENUMSTORE hEnum,
            /* [out][in] */ LPFOLDERPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubFolderClose( 
            /* [in] */ HENUMSTORE hEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStoreNamespaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreNamespace * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreNamespace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreNamespace * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IStoreNamespace * This,
            /* [unique][in] */ HWND hwndOwner,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectory )( 
            IStoreNamespace * This,
            /* [size_is][out][in] */ LPSTR pszPath,
            /* [in] */ DWORD cchMaxPath);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSpecialFolder )( 
            IStoreNamespace * This,
            /* [in] */ SPECIALFOLDER_XMIT sfType,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *OpenFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ LPCSTR pszName,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPSTOREFOLDERID pdwFolderId);
        
        HRESULT ( STDMETHODCALLTYPE *RenameFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPCSTR pszNewName);
        
        HRESULT ( STDMETHODCALLTYPE *MoveFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *CopyMoveMessages )( 
            IStoreNamespace * This,
            /* [in] */ IStoreFolder *pSource,
            /* [in] */ IStoreFolder *pDest,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwFlagsRemove,
            /* [in] */ IProgressNotify *pProgress);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreNamespace * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreNamespace * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *CompactAll )( 
            IStoreNamespace * This,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstSubFolder )( 
            IStoreNamespace * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [out][in] */ LPFOLDERPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextSubFolder )( 
            IStoreNamespace * This,
            /* [in] */ HENUMSTORE hEnum,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubFolderClose )( 
            IStoreNamespace * This,
            /* [in] */ HENUMSTORE hEnum);
        
        END_INTERFACE
    } IStoreNamespaceVtbl;

    interface IStoreNamespace
    {
        CONST_VTBL struct IStoreNamespaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreNamespace_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IStoreNamespace_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IStoreNamespace_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IStoreNamespace_Initialize(This,hwndOwner,dwReserved)	\
    ( (This)->lpVtbl -> Initialize(This,hwndOwner,dwReserved) ) 

#define IStoreNamespace_GetDirectory(This,pszPath,cchMaxPath)	\
    ( (This)->lpVtbl -> GetDirectory(This,pszPath,cchMaxPath) ) 

#define IStoreNamespace_OpenSpecialFolder(This,sfType,dwReserved,ppFolder)	\
    ( (This)->lpVtbl -> OpenSpecialFolder(This,sfType,dwReserved,ppFolder) ) 

#define IStoreNamespace_OpenFolder(This,dwFolderId,dwReserved,ppFolder)	\
    ( (This)->lpVtbl -> OpenFolder(This,dwFolderId,dwReserved,ppFolder) ) 

#define IStoreNamespace_CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId)	\
    ( (This)->lpVtbl -> CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId) ) 

#define IStoreNamespace_RenameFolder(This,dwFolderId,dwReserved,pszNewName)	\
    ( (This)->lpVtbl -> RenameFolder(This,dwFolderId,dwReserved,pszNewName) ) 

#define IStoreNamespace_MoveFolder(This,dwFolderId,dwParentId,dwReserved)	\
    ( (This)->lpVtbl -> MoveFolder(This,dwFolderId,dwParentId,dwReserved) ) 

#define IStoreNamespace_DeleteFolder(This,dwFolderId,dwReserved)	\
    ( (This)->lpVtbl -> DeleteFolder(This,dwFolderId,dwReserved) ) 

#define IStoreNamespace_GetFolderProps(This,dwFolderId,dwReserved,pProps)	\
    ( (This)->lpVtbl -> GetFolderProps(This,dwFolderId,dwReserved,pProps) ) 

#define IStoreNamespace_CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress)	\
    ( (This)->lpVtbl -> CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress) ) 

#define IStoreNamespace_RegisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd) ) 

#define IStoreNamespace_UnregisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd) ) 

#define IStoreNamespace_CompactAll(This,dwReserved)	\
    ( (This)->lpVtbl -> CompactAll(This,dwReserved) ) 

#define IStoreNamespace_GetFirstSubFolder(This,dwFolderId,pProps,phEnum)	\
    ( (This)->lpVtbl -> GetFirstSubFolder(This,dwFolderId,pProps,phEnum) ) 

#define IStoreNamespace_GetNextSubFolder(This,hEnum,pProps)	\
    ( (This)->lpVtbl -> GetNextSubFolder(This,hEnum,pProps) ) 

#define IStoreNamespace_GetSubFolderClose(This,hEnum)	\
    ( (This)->lpVtbl -> GetSubFolderClose(This,hEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_GetFolderPropsRemote_Proxy( 
    IStoreNamespace * This,
    /* [in] */ STOREFOLDERID dwFolderId,
    /* [in] */ DWORD dwReserved,
    /* [out][in] */ LPFOLDERPROPS pProps);


void __RPC_STUB IStoreNamespace_GetFolderPropsRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_RegisterNotificationRemote_Proxy( 
    IStoreNamespace * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ IUnknown *pUnkObj);


void __RPC_STUB IStoreNamespace_RegisterNotificationRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStoreNamespace_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_msoeapi_0000_0002 */
/* [local] */ 

#endif // __LPSTORENAMESPACE_DEFINED
#ifndef __LPSTOREFOLDER_DEFINED
#define __LPSTOREFOLDER_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0002_v0_0_s_ifspec;

#ifndef __IStoreFolder_INTERFACE_DEFINED__
#define __IStoreFolder_INTERFACE_DEFINED__

/* interface IStoreFolder */
/* [unique][uuid][object] */ 

typedef IStoreFolder *LPSTOREFOLDER;

#define MSG_DELETED                  0x0001  // The message has been deleted, compaction will remove it
#define MSG_UNREAD                   0x0002  // The message is marked as un-read
#define MSG_SUBMITTED                0x0004  // For messages waiting to be sent: OUTBOX ONLY
#define MSG_UNSENT                   0x0008  // For msgs-in-progress saved to a folder
#define MSG_RECEIVED                 0x0010  // For messages that came from a server
#define MSG_NEWSMSG                  0x0020  // For news messages
#define MSG_NOSECUI                  0x0040  // For messages where the user wants to die
#define MSG_VOICEMAIL                0x0080  // The message has the X-Voicemail header set...
#define MSG_REPLIED                  0x0100  // The message has been replied to
#define MSG_FORWARDED                0x0200  // The message has been forwarded to
#define MSG_RCPTSENT                 0x0400  // S/MIME Receipt has been sent
#define MSG_FLAGGED                  0x0800  // The message is currently flaged
#define MSG_LAST                     0x0200  // ** Keep this updated!! **
#define MSG_EXTERNAL_FLAGS           0x00fe
#define MSG_FLAGS                    0x000f
typedef /* [transmit_as] */ DWORD MESSAGEPROPSSIZE;

typedef struct tagMESSAGEPROPS
    {
    MESSAGEPROPSSIZE cbSize;
    DWORD dwReserved;
    MESSAGEID dwMessageId;
    DWORD dwLanguage;
    DWORD dwState;
    DWORD cbMessage;
    IMSGPRIORITY priority;
    FILETIME ftReceived;
    FILETIME ftSent;
    LPSTR pszSubject;
    LPSTR pszDisplayTo;
    LPSTR pszDisplayFrom;
    LPSTR pszNormalSubject;
    DWORD dwFlags;
    IStream *pStmOffsetTable;
    } 	MESSAGEPROPS;

typedef struct tagMESSAGEPROPS *LPMESSAGEPROPS;

struct HBATCHLOCK__
    {
    DWORD unused;
    } ;
typedef /* [transmit_as] */ struct HBATCHLOCK__ *HBATCHLOCK;

typedef HBATCHLOCK *LPHBATCHLOCK;

#define MSGPROPS_FAST                0x00000001              // See MESSAGEPROPS structure, improved performance
#define COMMITSTREAM_REVERT          0x00000001              // Don't add this stream/message to the folder

EXTERN_C const IID IID_IStoreFolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E70C92AC-4BFD-11d1-8A95-00C04FB951F3")
    IStoreFolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFolderProps( 
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetMessageProps( 
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeMessageProps( 
            /* [out][in] */ LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMessages( 
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwReserved,
            /* [in] */ IProgressNotify *pProgress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLanguage( 
            /* [in] */ DWORD dwLanguage,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MarkMessagesAsRead( 
            /* [in] */ BOOL fRead,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetFlags( 
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwState,
            /* [in] */ DWORD dwStatemask,
            /* [unique][out][in] */ LPDWORD prgdwNewFlags) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE OpenMessage( 
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ LPVOID *ppvObject) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE SaveMessage( 
            /* [in] */ REFIID riid,
            /* [iid_is][in] */ LPVOID pvObject,
            /* [in] */ DWORD dwMsgFlags,
            /* [out] */ LPMESSAGEID pdwMessageId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchLock( 
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPHBATCHLOCK phBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchFlush( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BatchUnlock( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateStream( 
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStream **ppStream,
            /* [out] */ LPMESSAGEID pdwMessageId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitStream( 
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ IStream *pStream,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ IMimeMessage *pMessage) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE RegisterNotification( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterNotification( 
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Compact( 
            /* [in] */ DWORD dwReserved) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetFirstMessage( 
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ MESSAGEID dwMsgIdFirst,
            /* [out][in] */ LPMESSAGEPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetNextMessage( 
            /* [in] */ HENUMSTORE hEnum,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMessageClose( 
            /* [in] */ HENUMSTORE hEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStoreFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreFolder * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetMessageProps )( 
            IStoreFolder * This,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *FreeMessageProps )( 
            IStoreFolder * This,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMessages )( 
            IStoreFolder * This,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwReserved,
            /* [in] */ IProgressNotify *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *SetLanguage )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwLanguage,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList);
        
        HRESULT ( STDMETHODCALLTYPE *MarkMessagesAsRead )( 
            IStoreFolder * This,
            /* [in] */ BOOL fRead,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *SetFlags )( 
            IStoreFolder * This,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwState,
            /* [in] */ DWORD dwStatemask,
            /* [unique][out][in] */ LPDWORD prgdwNewFlags);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *OpenMessage )( 
            IStoreFolder * This,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ LPVOID *ppvObject);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *SaveMessage )( 
            IStoreFolder * This,
            /* [in] */ REFIID riid,
            /* [iid_is][in] */ LPVOID pvObject,
            /* [in] */ DWORD dwMsgFlags,
            /* [out] */ LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *BatchLock )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPHBATCHLOCK phBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchFlush )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchUnlock )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            IStoreFolder * This,
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStream **ppStream,
            /* [out] */ LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *CommitStream )( 
            IStoreFolder * This,
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ IStream *pStream,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ IMimeMessage *pMessage);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Compact )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwReserved);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetFirstMessage )( 
            IStoreFolder * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ MESSAGEID dwMsgIdFirst,
            /* [out][in] */ LPMESSAGEPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetNextMessage )( 
            IStoreFolder * This,
            /* [in] */ HENUMSTORE hEnum,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageClose )( 
            IStoreFolder * This,
            /* [in] */ HENUMSTORE hEnum);
        
        END_INTERFACE
    } IStoreFolderVtbl;

    interface IStoreFolder
    {
        CONST_VTBL struct IStoreFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreFolder_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IStoreFolder_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IStoreFolder_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IStoreFolder_GetFolderProps(This,dwReserved,pProps)	\
    ( (This)->lpVtbl -> GetFolderProps(This,dwReserved,pProps) ) 

#define IStoreFolder_GetMessageProps(This,dwMessageId,dwFlags,pProps)	\
    ( (This)->lpVtbl -> GetMessageProps(This,dwMessageId,dwFlags,pProps) ) 

#define IStoreFolder_FreeMessageProps(This,pProps)	\
    ( (This)->lpVtbl -> FreeMessageProps(This,pProps) ) 

#define IStoreFolder_DeleteMessages(This,pMsgIdList,dwReserved,pProgress)	\
    ( (This)->lpVtbl -> DeleteMessages(This,pMsgIdList,dwReserved,pProgress) ) 

#define IStoreFolder_SetLanguage(This,dwLanguage,dwReserved,pMsgIdList)	\
    ( (This)->lpVtbl -> SetLanguage(This,dwLanguage,dwReserved,pMsgIdList) ) 

#define IStoreFolder_MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList)	\
    ( (This)->lpVtbl -> MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList) ) 

#define IStoreFolder_SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags)	\
    ( (This)->lpVtbl -> SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags) ) 

#define IStoreFolder_OpenMessage(This,dwMessageId,riid,ppvObject)	\
    ( (This)->lpVtbl -> OpenMessage(This,dwMessageId,riid,ppvObject) ) 

#define IStoreFolder_SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId)	\
    ( (This)->lpVtbl -> SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId) ) 

#define IStoreFolder_BatchLock(This,dwReserved,phBatchLock)	\
    ( (This)->lpVtbl -> BatchLock(This,dwReserved,phBatchLock) ) 

#define IStoreFolder_BatchFlush(This,dwReserved,hBatchLock)	\
    ( (This)->lpVtbl -> BatchFlush(This,dwReserved,hBatchLock) ) 

#define IStoreFolder_BatchUnlock(This,dwReserved,hBatchLock)	\
    ( (This)->lpVtbl -> BatchUnlock(This,dwReserved,hBatchLock) ) 

#define IStoreFolder_CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId)	\
    ( (This)->lpVtbl -> CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId) ) 

#define IStoreFolder_CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage)	\
    ( (This)->lpVtbl -> CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage) ) 

#define IStoreFolder_RegisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd) ) 

#define IStoreFolder_UnregisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd) ) 

#define IStoreFolder_Compact(This,dwReserved)	\
    ( (This)->lpVtbl -> Compact(This,dwReserved) ) 

#define IStoreFolder_GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum)	\
    ( (This)->lpVtbl -> GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum) ) 

#define IStoreFolder_GetNextMessage(This,hEnum,dwFlags,pProps)	\
    ( (This)->lpVtbl -> GetNextMessage(This,hEnum,dwFlags,pProps) ) 

#define IStoreFolder_GetMessageClose(This,hEnum)	\
    ( (This)->lpVtbl -> GetMessageClose(This,hEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetMessagePropsRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);


void __RPC_STUB IStoreFolder_GetMessagePropsRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SetFlagsRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ LPMESSAGEIDLIST pMsgIdList,
    /* [in] */ DWORD dwState,
    /* [in] */ DWORD dwStatemask,
    /* [in] */ DWORD cMsgs,
    /* [size_is][unique][out][in] */ LPDWORD prgdwNewFlags);


void __RPC_STUB IStoreFolder_SetFlagsRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_OpenMessageRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ LPVOID *ppvObject);


void __RPC_STUB IStoreFolder_OpenMessageRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SaveMessageRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ REFIID riid,
    /* [iid_is][in] */ LPVOID pvObject,
    /* [in] */ DWORD dwMsgFlags,
    /* [out] */ LPMESSAGEID pdwMessageId);


void __RPC_STUB IStoreFolder_SaveMessageRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_RegisterNotificationRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ IUnknown *pUnkObj);


void __RPC_STUB IStoreFolder_RegisterNotificationRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetFirstMessageRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwMsgFlags,
    /* [in] */ MESSAGEID dwMsgIdFirst,
    /* [out][in] */ LPMESSAGEPROPS pProps,
    /* [out] */ LPHENUMSTORE phEnum);


void __RPC_STUB IStoreFolder_GetFirstMessageRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetNextMessageRemote_Proxy( 
    IStoreFolder * This,
    /* [in] */ HENUMSTORE hEnum,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);


void __RPC_STUB IStoreFolder_GetNextMessageRemote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStoreFolder_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_msoeapi_0000_0003 */
/* [local] */ 

#endif // __LPSTOREFOLDER_DEFINED
//+-------------------------------------------------------------------------
// Outlook Express Exported C API Functions
//--------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//+-------------------------------------------------------------------------
// API Name Decoration
//--------------------------------------------------------------------------
#if !defined(_MSOEAPI_)
#define MSOEAPI DECLSPEC_IMPORT HRESULT WINAPI
#define MSOEAPI_(_type_) DECLSPEC_IMPORT _type_ WINAPI
#else
#define MSOEAPI HRESULT WINAPI
#define MSOEAPI_(_type_) _type_ WINAPI
#endif

//+-------------------------------------------------------------------------
// Prototypes
//--------------------------------------------------------------------------
MSOEAPI CoStartOutlookExpress(
             /* IN */        DWORD               dwFlags,
             /* IN */        LPCSTR              pszCmdLine,
             /* IN */        INT                 nCmdShow);

MSOEAPI CoCreateOutlookExpress(
             /* IN */        IUnknown            *pUnkOuter,
             /* OUT */       IUnknown            **ppUnknown);

#ifdef __cplusplus
}
#endif
#if (_WIN32_OE >= 0x0700)


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0003_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0003_v0_0_s_ifspec;

#ifndef __IStoreNamespace2_INTERFACE_DEFINED__
#define __IStoreNamespace2_INTERFACE_DEFINED__

/* interface IStoreNamespace2 */
/* [unique][uuid][object] */ 

typedef IStoreNamespace2 *LPSTORENAMESPACE2;


EXTERN_C const IID IID_IStoreNamespace2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DECAED7F-2DA8-4074-B62A-DB6D8C82C385")
    IStoreNamespace2 : public IStoreNamespace
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMessageID( 
            /* [in] */ LPCWSTR pszPath,
            /* [out] */ LPSTOREFOLDERID pdwFolderId,
            /* [out] */ LPMESSAGEID pdwMessageId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentId( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [out] */ LPSTOREFOLDERID pdwParentId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectoryW( 
            /* [size_is][out][in] */ LPWSTR pwszPath,
            /* [in] */ DWORD cchMaxPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFolderW( 
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ LPCWSTR pwszName,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPSTOREFOLDERID pdwFolderId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RenameFolderW( 
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPCWSTR pwszNewName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStoreNamespace2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreNamespace2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreNamespace2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreNamespace2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IStoreNamespace2 * This,
            /* [unique][in] */ HWND hwndOwner,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectory )( 
            IStoreNamespace2 * This,
            /* [size_is][out][in] */ LPSTR pszPath,
            /* [in] */ DWORD cchMaxPath);
        
        HRESULT ( STDMETHODCALLTYPE *OpenSpecialFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ SPECIALFOLDER_XMIT sfType,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *OpenFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStoreFolder **ppFolder);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ LPCSTR pszName,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPSTOREFOLDERID pdwFolderId);
        
        HRESULT ( STDMETHODCALLTYPE *RenameFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPCSTR pszNewName);
        
        HRESULT ( STDMETHODCALLTYPE *MoveFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *CopyMoveMessages )( 
            IStoreNamespace2 * This,
            /* [in] */ IStoreFolder *pSource,
            /* [in] */ IStoreFolder *pDest,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwFlagsRemove,
            /* [in] */ IProgressNotify *pProgress);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreNamespace2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreNamespace2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *CompactAll )( 
            IStoreNamespace2 * This,
            /* [in] */ DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetFirstSubFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [out][in] */ LPFOLDERPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetNextSubFolder )( 
            IStoreNamespace2 * This,
            /* [in] */ HENUMSTORE hEnum,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubFolderClose )( 
            IStoreNamespace2 * This,
            /* [in] */ HENUMSTORE hEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageID )( 
            IStoreNamespace2 * This,
            /* [in] */ LPCWSTR pszPath,
            /* [out] */ LPSTOREFOLDERID pdwFolderId,
            /* [out] */ LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *GetParentId )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [out] */ LPSTOREFOLDERID pdwParentId);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectoryW )( 
            IStoreNamespace2 * This,
            /* [size_is][out][in] */ LPWSTR pwszPath,
            /* [in] */ DWORD cchMaxPath);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFolderW )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwParentId,
            /* [in] */ LPCWSTR pwszName,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPSTOREFOLDERID pdwFolderId);
        
        HRESULT ( STDMETHODCALLTYPE *RenameFolderW )( 
            IStoreNamespace2 * This,
            /* [in] */ STOREFOLDERID dwFolderId,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPCWSTR pwszNewName);
        
        END_INTERFACE
    } IStoreNamespace2Vtbl;

    interface IStoreNamespace2
    {
        CONST_VTBL struct IStoreNamespace2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreNamespace2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IStoreNamespace2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IStoreNamespace2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IStoreNamespace2_Initialize(This,hwndOwner,dwReserved)	\
    ( (This)->lpVtbl -> Initialize(This,hwndOwner,dwReserved) ) 

#define IStoreNamespace2_GetDirectory(This,pszPath,cchMaxPath)	\
    ( (This)->lpVtbl -> GetDirectory(This,pszPath,cchMaxPath) ) 

#define IStoreNamespace2_OpenSpecialFolder(This,sfType,dwReserved,ppFolder)	\
    ( (This)->lpVtbl -> OpenSpecialFolder(This,sfType,dwReserved,ppFolder) ) 

#define IStoreNamespace2_OpenFolder(This,dwFolderId,dwReserved,ppFolder)	\
    ( (This)->lpVtbl -> OpenFolder(This,dwFolderId,dwReserved,ppFolder) ) 

#define IStoreNamespace2_CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId)	\
    ( (This)->lpVtbl -> CreateFolder(This,dwParentId,pszName,dwReserved,pdwFolderId) ) 

#define IStoreNamespace2_RenameFolder(This,dwFolderId,dwReserved,pszNewName)	\
    ( (This)->lpVtbl -> RenameFolder(This,dwFolderId,dwReserved,pszNewName) ) 

#define IStoreNamespace2_MoveFolder(This,dwFolderId,dwParentId,dwReserved)	\
    ( (This)->lpVtbl -> MoveFolder(This,dwFolderId,dwParentId,dwReserved) ) 

#define IStoreNamespace2_DeleteFolder(This,dwFolderId,dwReserved)	\
    ( (This)->lpVtbl -> DeleteFolder(This,dwFolderId,dwReserved) ) 

#define IStoreNamespace2_GetFolderProps(This,dwFolderId,dwReserved,pProps)	\
    ( (This)->lpVtbl -> GetFolderProps(This,dwFolderId,dwReserved,pProps) ) 

#define IStoreNamespace2_CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress)	\
    ( (This)->lpVtbl -> CopyMoveMessages(This,pSource,pDest,pMsgIdList,dwFlags,dwFlagsRemove,pProgress) ) 

#define IStoreNamespace2_RegisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd) ) 

#define IStoreNamespace2_UnregisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd) ) 

#define IStoreNamespace2_CompactAll(This,dwReserved)	\
    ( (This)->lpVtbl -> CompactAll(This,dwReserved) ) 

#define IStoreNamespace2_GetFirstSubFolder(This,dwFolderId,pProps,phEnum)	\
    ( (This)->lpVtbl -> GetFirstSubFolder(This,dwFolderId,pProps,phEnum) ) 

#define IStoreNamespace2_GetNextSubFolder(This,hEnum,pProps)	\
    ( (This)->lpVtbl -> GetNextSubFolder(This,hEnum,pProps) ) 

#define IStoreNamespace2_GetSubFolderClose(This,hEnum)	\
    ( (This)->lpVtbl -> GetSubFolderClose(This,hEnum) ) 


#define IStoreNamespace2_GetMessageID(This,pszPath,pdwFolderId,pdwMessageId)	\
    ( (This)->lpVtbl -> GetMessageID(This,pszPath,pdwFolderId,pdwMessageId) ) 

#define IStoreNamespace2_GetParentId(This,dwFolderId,pdwParentId)	\
    ( (This)->lpVtbl -> GetParentId(This,dwFolderId,pdwParentId) ) 

#define IStoreNamespace2_GetDirectoryW(This,pwszPath,cchMaxPath)	\
    ( (This)->lpVtbl -> GetDirectoryW(This,pwszPath,cchMaxPath) ) 

#define IStoreNamespace2_CreateFolderW(This,dwParentId,pwszName,dwReserved,pdwFolderId)	\
    ( (This)->lpVtbl -> CreateFolderW(This,dwParentId,pwszName,dwReserved,pdwFolderId) ) 

#define IStoreNamespace2_RenameFolderW(This,dwFolderId,dwReserved,pwszNewName)	\
    ( (This)->lpVtbl -> RenameFolderW(This,dwFolderId,dwReserved,pwszNewName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IStoreNamespace2_INTERFACE_DEFINED__ */


#ifndef __IStoreFolder2_INTERFACE_DEFINED__
#define __IStoreFolder2_INTERFACE_DEFINED__

/* interface IStoreFolder2 */
/* [unique][uuid][object] */ 

typedef IStoreFolder2 *LPSTOREFOLDER2;


EXTERN_C const IID IID_IStoreFolder2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5642F741-46B7-4A22-B496-F064E4C5F302")
    IStoreFolder2 : public IStoreFolder
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMessagePath( 
            /* [in] */ MESSAGEID dwMessageId,
            /* [size_is][out][in] */ LPWSTR pszPath,
            /* [in] */ DWORD cchBufferSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStoreFolder2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStoreFolder2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStoreFolder2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStoreFolder2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFolderProps )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [out][in] */ LPFOLDERPROPS pProps);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetMessageProps )( 
            IStoreFolder2 * This,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *FreeMessageProps )( 
            IStoreFolder2 * This,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMessages )( 
            IStoreFolder2 * This,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwReserved,
            /* [in] */ IProgressNotify *pProgress);
        
        HRESULT ( STDMETHODCALLTYPE *SetLanguage )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwLanguage,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList);
        
        HRESULT ( STDMETHODCALLTYPE *MarkMessagesAsRead )( 
            IStoreFolder2 * This,
            /* [in] */ BOOL fRead,
            /* [in] */ DWORD dwReserved,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *SetFlags )( 
            IStoreFolder2 * This,
            /* [in] */ LPMESSAGEIDLIST pMsgIdList,
            /* [in] */ DWORD dwState,
            /* [in] */ DWORD dwStatemask,
            /* [unique][out][in] */ LPDWORD prgdwNewFlags);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *OpenMessage )( 
            IStoreFolder2 * This,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ LPVOID *ppvObject);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *SaveMessage )( 
            IStoreFolder2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][in] */ LPVOID pvObject,
            /* [in] */ DWORD dwMsgFlags,
            /* [out] */ LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *BatchLock )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [out] */ LPHBATCHLOCK phBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchFlush )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *BatchUnlock )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HBATCHLOCK hBatchLock);
        
        HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            IStoreFolder2 * This,
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwReserved,
            /* [out] */ IStream **ppStream,
            /* [out] */ LPMESSAGEID pdwMessageId);
        
        HRESULT ( STDMETHODCALLTYPE *CommitStream )( 
            IStoreFolder2 * This,
            /* [in] */ HBATCHLOCK hBatchLock,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ IStream *pStream,
            /* [in] */ MESSAGEID dwMessageId,
            /* [in] */ IMimeMessage *pMessage);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *RegisterNotification )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterNotification )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved,
            /* [in] */ HWND hwnd);
        
        HRESULT ( STDMETHODCALLTYPE *Compact )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwReserved);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetFirstMessage )( 
            IStoreFolder2 * This,
            /* [in] */ DWORD dwFlags,
            /* [in] */ DWORD dwMsgFlags,
            /* [in] */ MESSAGEID dwMsgIdFirst,
            /* [out][in] */ LPMESSAGEPROPS pProps,
            /* [out] */ LPHENUMSTORE phEnum);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetNextMessage )( 
            IStoreFolder2 * This,
            /* [in] */ HENUMSTORE hEnum,
            /* [in] */ DWORD dwFlags,
            /* [out][in] */ LPMESSAGEPROPS pProps);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessageClose )( 
            IStoreFolder2 * This,
            /* [in] */ HENUMSTORE hEnum);
        
        HRESULT ( STDMETHODCALLTYPE *GetMessagePath )( 
            IStoreFolder2 * This,
            /* [in] */ MESSAGEID dwMessageId,
            /* [size_is][out][in] */ LPWSTR pszPath,
            /* [in] */ DWORD cchBufferSize);
        
        END_INTERFACE
    } IStoreFolder2Vtbl;

    interface IStoreFolder2
    {
        CONST_VTBL struct IStoreFolder2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStoreFolder2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IStoreFolder2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IStoreFolder2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IStoreFolder2_GetFolderProps(This,dwReserved,pProps)	\
    ( (This)->lpVtbl -> GetFolderProps(This,dwReserved,pProps) ) 

#define IStoreFolder2_GetMessageProps(This,dwMessageId,dwFlags,pProps)	\
    ( (This)->lpVtbl -> GetMessageProps(This,dwMessageId,dwFlags,pProps) ) 

#define IStoreFolder2_FreeMessageProps(This,pProps)	\
    ( (This)->lpVtbl -> FreeMessageProps(This,pProps) ) 

#define IStoreFolder2_DeleteMessages(This,pMsgIdList,dwReserved,pProgress)	\
    ( (This)->lpVtbl -> DeleteMessages(This,pMsgIdList,dwReserved,pProgress) ) 

#define IStoreFolder2_SetLanguage(This,dwLanguage,dwReserved,pMsgIdList)	\
    ( (This)->lpVtbl -> SetLanguage(This,dwLanguage,dwReserved,pMsgIdList) ) 

#define IStoreFolder2_MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList)	\
    ( (This)->lpVtbl -> MarkMessagesAsRead(This,fRead,dwReserved,pMsgIdList) ) 

#define IStoreFolder2_SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags)	\
    ( (This)->lpVtbl -> SetFlags(This,pMsgIdList,dwState,dwStatemask,prgdwNewFlags) ) 

#define IStoreFolder2_OpenMessage(This,dwMessageId,riid,ppvObject)	\
    ( (This)->lpVtbl -> OpenMessage(This,dwMessageId,riid,ppvObject) ) 

#define IStoreFolder2_SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId)	\
    ( (This)->lpVtbl -> SaveMessage(This,riid,pvObject,dwMsgFlags,pdwMessageId) ) 

#define IStoreFolder2_BatchLock(This,dwReserved,phBatchLock)	\
    ( (This)->lpVtbl -> BatchLock(This,dwReserved,phBatchLock) ) 

#define IStoreFolder2_BatchFlush(This,dwReserved,hBatchLock)	\
    ( (This)->lpVtbl -> BatchFlush(This,dwReserved,hBatchLock) ) 

#define IStoreFolder2_BatchUnlock(This,dwReserved,hBatchLock)	\
    ( (This)->lpVtbl -> BatchUnlock(This,dwReserved,hBatchLock) ) 

#define IStoreFolder2_CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId)	\
    ( (This)->lpVtbl -> CreateStream(This,hBatchLock,dwReserved,ppStream,pdwMessageId) ) 

#define IStoreFolder2_CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage)	\
    ( (This)->lpVtbl -> CommitStream(This,hBatchLock,dwFlags,dwMsgFlags,pStream,dwMessageId,pMessage) ) 

#define IStoreFolder2_RegisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> RegisterNotification(This,dwReserved,hwnd) ) 

#define IStoreFolder2_UnregisterNotification(This,dwReserved,hwnd)	\
    ( (This)->lpVtbl -> UnregisterNotification(This,dwReserved,hwnd) ) 

#define IStoreFolder2_Compact(This,dwReserved)	\
    ( (This)->lpVtbl -> Compact(This,dwReserved) ) 

#define IStoreFolder2_GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum)	\
    ( (This)->lpVtbl -> GetFirstMessage(This,dwFlags,dwMsgFlags,dwMsgIdFirst,pProps,phEnum) ) 

#define IStoreFolder2_GetNextMessage(This,hEnum,dwFlags,pProps)	\
    ( (This)->lpVtbl -> GetNextMessage(This,hEnum,dwFlags,pProps) ) 

#define IStoreFolder2_GetMessageClose(This,hEnum)	\
    ( (This)->lpVtbl -> GetMessageClose(This,hEnum) ) 


#define IStoreFolder2_GetMessagePath(This,dwMessageId,pszPath,cchBufferSize)	\
    ( (This)->lpVtbl -> GetMessagePath(This,dwMessageId,pszPath,cchBufferSize) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IStoreFolder2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_msoeapi_0000_0005 */
/* [local] */ 

#endif // (_WIN32_OE >= 0x0700)
#endif // MSOEAPI_H


extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_msoeapi_0000_0005_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */


void __RPC_USER HENUMSTORE_to_xmit( HENUMSTORE *, INT  * * );
void __RPC_USER HENUMSTORE_from_xmit( INT  *, HENUMSTORE * );
void __RPC_USER HENUMSTORE_free_inst( HENUMSTORE * );
void __RPC_USER HENUMSTORE_free_xmit( INT  * );

void __RPC_USER FOLDERPROPSSIZE_to_xmit( FOLDERPROPSSIZE *, DWORD  * * );
void __RPC_USER FOLDERPROPSSIZE_from_xmit( DWORD  *, FOLDERPROPSSIZE * );
void __RPC_USER FOLDERPROPSSIZE_free_inst( FOLDERPROPSSIZE * );
void __RPC_USER FOLDERPROPSSIZE_free_xmit( DWORD  * );

void __RPC_USER SPECIALFOLDER_XMIT_to_xmit( SPECIALFOLDER_XMIT *, DWORD  * * );
void __RPC_USER SPECIALFOLDER_XMIT_from_xmit( DWORD  *, SPECIALFOLDER_XMIT * );
void __RPC_USER SPECIALFOLDER_XMIT_free_inst( SPECIALFOLDER_XMIT * );
void __RPC_USER SPECIALFOLDER_XMIT_free_xmit( DWORD  * );

void __RPC_USER MESSAGEPROPSSIZE_to_xmit( MESSAGEPROPSSIZE *, DWORD  * * );
void __RPC_USER MESSAGEPROPSSIZE_from_xmit( DWORD  *, MESSAGEPROPSSIZE * );
void __RPC_USER MESSAGEPROPSSIZE_free_inst( MESSAGEPROPSSIZE * );
void __RPC_USER MESSAGEPROPSSIZE_free_xmit( DWORD  * );

void __RPC_USER HBATCHLOCK_to_xmit( HBATCHLOCK *, INT  * * );
void __RPC_USER HBATCHLOCK_from_xmit( INT  *, HBATCHLOCK * );
void __RPC_USER HBATCHLOCK_free_inst( HBATCHLOCK * );
void __RPC_USER HBATCHLOCK_free_xmit( INT  * );

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_GetFolderProps_Proxy( 
    IStoreNamespace * This,
    /* [in] */ STOREFOLDERID dwFolderId,
    /* [in] */ DWORD dwReserved,
    /* [out][in] */ LPFOLDERPROPS pProps);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_GetFolderProps_Stub( 
    IStoreNamespace * This,
    /* [in] */ STOREFOLDERID dwFolderId,
    /* [in] */ DWORD dwReserved,
    /* [out][in] */ LPFOLDERPROPS pProps);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_RegisterNotification_Proxy( 
    IStoreNamespace * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ HWND hwnd);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreNamespace_RegisterNotification_Stub( 
    IStoreNamespace * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ IUnknown *pUnkObj);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetMessageProps_Proxy( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetMessageProps_Stub( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SetFlags_Proxy( 
    IStoreFolder * This,
    /* [in] */ LPMESSAGEIDLIST pMsgIdList,
    /* [in] */ DWORD dwState,
    /* [in] */ DWORD dwStatemask,
    /* [unique][out][in] */ LPDWORD prgdwNewFlags);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SetFlags_Stub( 
    IStoreFolder * This,
    /* [in] */ LPMESSAGEIDLIST pMsgIdList,
    /* [in] */ DWORD dwState,
    /* [in] */ DWORD dwStatemask,
    /* [in] */ DWORD cMsgs,
    /* [size_is][unique][out][in] */ LPDWORD prgdwNewFlags);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_OpenMessage_Proxy( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ LPVOID *ppvObject);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_OpenMessage_Stub( 
    IStoreFolder * This,
    /* [in] */ MESSAGEID dwMessageId,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ LPVOID *ppvObject);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SaveMessage_Proxy( 
    IStoreFolder * This,
    /* [in] */ REFIID riid,
    /* [iid_is][in] */ LPVOID pvObject,
    /* [in] */ DWORD dwMsgFlags,
    /* [out] */ LPMESSAGEID pdwMessageId);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_SaveMessage_Stub( 
    IStoreFolder * This,
    /* [in] */ REFIID riid,
    /* [iid_is][in] */ LPVOID pvObject,
    /* [in] */ DWORD dwMsgFlags,
    /* [out] */ LPMESSAGEID pdwMessageId);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_RegisterNotification_Proxy( 
    IStoreFolder * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ HWND hwnd);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_RegisterNotification_Stub( 
    IStoreFolder * This,
    /* [in] */ DWORD dwReserved,
    /* [in] */ IUnknown *pUnkObj);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetFirstMessage_Proxy( 
    IStoreFolder * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwMsgFlags,
    /* [in] */ MESSAGEID dwMsgIdFirst,
    /* [out][in] */ LPMESSAGEPROPS pProps,
    /* [out] */ LPHENUMSTORE phEnum);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetFirstMessage_Stub( 
    IStoreFolder * This,
    /* [in] */ DWORD dwFlags,
    /* [in] */ DWORD dwMsgFlags,
    /* [in] */ MESSAGEID dwMsgIdFirst,
    /* [out][in] */ LPMESSAGEPROPS pProps,
    /* [out] */ LPHENUMSTORE phEnum);

/* [local] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetNextMessage_Proxy( 
    IStoreFolder * This,
    /* [in] */ HENUMSTORE hEnum,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);


/* [call_as] */ HRESULT STDMETHODCALLTYPE IStoreFolder_GetNextMessage_Stub( 
    IStoreFolder * This,
    /* [in] */ HENUMSTORE hEnum,
    /* [in] */ DWORD dwFlags,
    /* [out][in] */ LPMESSAGEPROPS pProps);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


