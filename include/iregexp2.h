

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue Sep 07 04:36:37 2010
 */
/* Compiler settings for vbscript.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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


#ifndef __iregexp2_h__
#define __iregexp2_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IRegExp_FWD_DEFINED__
#define __IRegExp_FWD_DEFINED__
typedef interface IRegExp IRegExp;
#endif 	/* __IRegExp_FWD_DEFINED__ */


#ifndef __IMatch_FWD_DEFINED__
#define __IMatch_FWD_DEFINED__
typedef interface IMatch IMatch;
#endif 	/* __IMatch_FWD_DEFINED__ */


#ifndef __IMatchCollection_FWD_DEFINED__
#define __IMatchCollection_FWD_DEFINED__
typedef interface IMatchCollection IMatchCollection;
#endif 	/* __IMatchCollection_FWD_DEFINED__ */


#ifndef __IRegExp2_FWD_DEFINED__
#define __IRegExp2_FWD_DEFINED__
typedef interface IRegExp2 IRegExp2;
#endif 	/* __IRegExp2_FWD_DEFINED__ */


#ifndef __IMatch2_FWD_DEFINED__
#define __IMatch2_FWD_DEFINED__
typedef interface IMatch2 IMatch2;
#endif 	/* __IMatch2_FWD_DEFINED__ */


#ifndef __IMatchCollection2_FWD_DEFINED__
#define __IMatchCollection2_FWD_DEFINED__
typedef interface IMatchCollection2 IMatchCollection2;
#endif 	/* __IMatchCollection2_FWD_DEFINED__ */


#ifndef __ISubMatches_FWD_DEFINED__
#define __ISubMatches_FWD_DEFINED__
typedef interface ISubMatches ISubMatches;
#endif 	/* __ISubMatches_FWD_DEFINED__ */


#ifndef __RegExp_FWD_DEFINED__
#define __RegExp_FWD_DEFINED__

#ifdef __cplusplus
typedef class RegExp RegExp;
#else
typedef struct RegExp RegExp;
#endif /* __cplusplus */

#endif 	/* __RegExp_FWD_DEFINED__ */


#ifndef __Match_FWD_DEFINED__
#define __Match_FWD_DEFINED__

#ifdef __cplusplus
typedef class Match Match;
#else
typedef struct Match Match;
#endif /* __cplusplus */

#endif 	/* __Match_FWD_DEFINED__ */


#ifndef __MatchCollection_FWD_DEFINED__
#define __MatchCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class MatchCollection MatchCollection;
#else
typedef struct MatchCollection MatchCollection;
#endif /* __cplusplus */

#endif 	/* __MatchCollection_FWD_DEFINED__ */


#ifndef __SubMatches_FWD_DEFINED__
#define __SubMatches_FWD_DEFINED__

#ifdef __cplusplus
typedef class SubMatches SubMatches;
#else
typedef struct SubMatches SubMatches;
#endif /* __cplusplus */

#endif 	/* __SubMatches_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __VBScript_RegExp_55_LIBRARY_DEFINED__
#define __VBScript_RegExp_55_LIBRARY_DEFINED__

/* library VBScript_RegExp_55 */
/* [helpstring][version][uuid] */ 









EXTERN_C const IID LIBID_VBScript_RegExp_55;

#ifndef __IRegExp_INTERFACE_DEFINED__
#define __IRegExp_INTERFACE_DEFINED__

/* interface IRegExp */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IRegExp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACA0-160D-11D2-A8E9-00104B365C9F")
    IRegExp : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Pattern( 
            /* [retval][out] */ BSTR *pPattern) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Pattern( 
            /* [in] */ BSTR pPattern) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IgnoreCase( 
            /* [retval][out] */ VARIANT_BOOL *pIgnoreCase) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_IgnoreCase( 
            /* [in] */ VARIANT_BOOL pIgnoreCase) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Global( 
            /* [retval][out] */ VARIANT_BOOL *pGlobal) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Global( 
            /* [in] */ VARIANT_BOOL pGlobal) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Execute( 
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ IDispatch **ppMatches) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Test( 
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ VARIANT_BOOL *pMatch) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Replace( 
            /* [in] */ BSTR sourceString,
            /* [in] */ BSTR replaceString,
            /* [retval][out] */ BSTR *pDestString) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRegExpVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegExp * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegExp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegExp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRegExp * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRegExp * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRegExp * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRegExp * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Pattern )( 
            IRegExp * This,
            /* [retval][out] */ BSTR *pPattern);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Pattern )( 
            IRegExp * This,
            /* [in] */ BSTR pPattern);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IgnoreCase )( 
            IRegExp * This,
            /* [retval][out] */ VARIANT_BOOL *pIgnoreCase);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_IgnoreCase )( 
            IRegExp * This,
            /* [in] */ VARIANT_BOOL pIgnoreCase);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Global )( 
            IRegExp * This,
            /* [retval][out] */ VARIANT_BOOL *pGlobal);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Global )( 
            IRegExp * This,
            /* [in] */ VARIANT_BOOL pGlobal);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IRegExp * This,
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ IDispatch **ppMatches);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Test )( 
            IRegExp * This,
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ VARIANT_BOOL *pMatch);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Replace )( 
            IRegExp * This,
            /* [in] */ BSTR sourceString,
            /* [in] */ BSTR replaceString,
            /* [retval][out] */ BSTR *pDestString);
        
        END_INTERFACE
    } IRegExpVtbl;

    interface IRegExp
    {
        CONST_VTBL struct IRegExpVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegExp_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRegExp_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRegExp_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRegExp_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRegExp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRegExp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRegExp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRegExp_get_Pattern(This,pPattern)	\
    ( (This)->lpVtbl -> get_Pattern(This,pPattern) ) 

#define IRegExp_put_Pattern(This,pPattern)	\
    ( (This)->lpVtbl -> put_Pattern(This,pPattern) ) 

#define IRegExp_get_IgnoreCase(This,pIgnoreCase)	\
    ( (This)->lpVtbl -> get_IgnoreCase(This,pIgnoreCase) ) 

#define IRegExp_put_IgnoreCase(This,pIgnoreCase)	\
    ( (This)->lpVtbl -> put_IgnoreCase(This,pIgnoreCase) ) 

#define IRegExp_get_Global(This,pGlobal)	\
    ( (This)->lpVtbl -> get_Global(This,pGlobal) ) 

#define IRegExp_put_Global(This,pGlobal)	\
    ( (This)->lpVtbl -> put_Global(This,pGlobal) ) 

#define IRegExp_Execute(This,sourceString,ppMatches)	\
    ( (This)->lpVtbl -> Execute(This,sourceString,ppMatches) ) 

#define IRegExp_Test(This,sourceString,pMatch)	\
    ( (This)->lpVtbl -> Test(This,sourceString,pMatch) ) 

#define IRegExp_Replace(This,sourceString,replaceString,pDestString)	\
    ( (This)->lpVtbl -> Replace(This,sourceString,replaceString,pDestString) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRegExp_INTERFACE_DEFINED__ */


#ifndef __IMatch_INTERFACE_DEFINED__
#define __IMatch_INTERFACE_DEFINED__

/* interface IMatch */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IMatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACA1-160D-11D2-A8E9-00104B365C9F")
    IMatch : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ BSTR *pValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_FirstIndex( 
            /* [retval][out] */ long *pFirstIndex) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ long *pLength) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMatch * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMatch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMatch * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMatch * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMatch * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMatch * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMatch * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IMatch * This,
            /* [retval][out] */ BSTR *pValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FirstIndex )( 
            IMatch * This,
            /* [retval][out] */ long *pFirstIndex);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMatch * This,
            /* [retval][out] */ long *pLength);
        
        END_INTERFACE
    } IMatchVtbl;

    interface IMatch
    {
        CONST_VTBL struct IMatchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMatch_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMatch_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMatch_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMatch_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IMatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IMatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IMatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IMatch_get_Value(This,pValue)	\
    ( (This)->lpVtbl -> get_Value(This,pValue) ) 

#define IMatch_get_FirstIndex(This,pFirstIndex)	\
    ( (This)->lpVtbl -> get_FirstIndex(This,pFirstIndex) ) 

#define IMatch_get_Length(This,pLength)	\
    ( (This)->lpVtbl -> get_Length(This,pLength) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMatch_INTERFACE_DEFINED__ */


#ifndef __IMatchCollection_INTERFACE_DEFINED__
#define __IMatchCollection_INTERFACE_DEFINED__

/* interface IMatchCollection */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IMatchCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACA2-160D-11D2-A8E9-00104B365C9F")
    IMatchCollection : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long index,
            /* [retval][out] */ IDispatch **ppMatch) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMatchCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMatchCollection * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMatchCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMatchCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMatchCollection * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMatchCollection * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMatchCollection * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMatchCollection * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMatchCollection * This,
            /* [in] */ long index,
            /* [retval][out] */ IDispatch **ppMatch);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMatchCollection * This,
            /* [retval][out] */ long *pCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMatchCollection * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        END_INTERFACE
    } IMatchCollectionVtbl;

    interface IMatchCollection
    {
        CONST_VTBL struct IMatchCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMatchCollection_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMatchCollection_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMatchCollection_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMatchCollection_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IMatchCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IMatchCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IMatchCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IMatchCollection_get_Item(This,index,ppMatch)	\
    ( (This)->lpVtbl -> get_Item(This,index,ppMatch) ) 

#define IMatchCollection_get_Count(This,pCount)	\
    ( (This)->lpVtbl -> get_Count(This,pCount) ) 

#define IMatchCollection_get__NewEnum(This,ppEnum)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMatchCollection_INTERFACE_DEFINED__ */


#ifndef __IRegExp2_INTERFACE_DEFINED__
#define __IRegExp2_INTERFACE_DEFINED__

/* interface IRegExp2 */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IRegExp2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACB0-160D-11D2-A8E9-00104B365C9F")
    IRegExp2 : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Pattern( 
            /* [retval][out] */ BSTR *pPattern) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Pattern( 
            /* [in] */ BSTR pPattern) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IgnoreCase( 
            /* [retval][out] */ VARIANT_BOOL *pIgnoreCase) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_IgnoreCase( 
            /* [in] */ VARIANT_BOOL pIgnoreCase) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Global( 
            /* [retval][out] */ VARIANT_BOOL *pGlobal) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Global( 
            /* [in] */ VARIANT_BOOL pGlobal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Multiline( 
            /* [retval][out] */ VARIANT_BOOL *pMultiline) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Multiline( 
            /* [in] */ VARIANT_BOOL pMultiline) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Execute( 
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ IDispatch **ppMatches) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Test( 
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ VARIANT_BOOL *pMatch) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Replace( 
            /* [in] */ BSTR sourceString,
            /* [in] */ VARIANT replaceVar,
            /* [retval][out] */ BSTR *pDestString) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRegExp2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegExp2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegExp2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegExp2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRegExp2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRegExp2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRegExp2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRegExp2 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Pattern )( 
            IRegExp2 * This,
            /* [retval][out] */ BSTR *pPattern);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Pattern )( 
            IRegExp2 * This,
            /* [in] */ BSTR pPattern);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IgnoreCase )( 
            IRegExp2 * This,
            /* [retval][out] */ VARIANT_BOOL *pIgnoreCase);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_IgnoreCase )( 
            IRegExp2 * This,
            /* [in] */ VARIANT_BOOL pIgnoreCase);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Global )( 
            IRegExp2 * This,
            /* [retval][out] */ VARIANT_BOOL *pGlobal);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Global )( 
            IRegExp2 * This,
            /* [in] */ VARIANT_BOOL pGlobal);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Multiline )( 
            IRegExp2 * This,
            /* [retval][out] */ VARIANT_BOOL *pMultiline);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Multiline )( 
            IRegExp2 * This,
            /* [in] */ VARIANT_BOOL pMultiline);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IRegExp2 * This,
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ IDispatch **ppMatches);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Test )( 
            IRegExp2 * This,
            /* [in] */ BSTR sourceString,
            /* [retval][out] */ VARIANT_BOOL *pMatch);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Replace )( 
            IRegExp2 * This,
            /* [in] */ BSTR sourceString,
            /* [in] */ VARIANT replaceVar,
            /* [retval][out] */ BSTR *pDestString);
        
        END_INTERFACE
    } IRegExp2Vtbl;

    interface IRegExp2
    {
        CONST_VTBL struct IRegExp2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegExp2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRegExp2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRegExp2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRegExp2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRegExp2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRegExp2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRegExp2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IRegExp2_get_Pattern(This,pPattern)	\
    ( (This)->lpVtbl -> get_Pattern(This,pPattern) ) 

#define IRegExp2_put_Pattern(This,pPattern)	\
    ( (This)->lpVtbl -> put_Pattern(This,pPattern) ) 

#define IRegExp2_get_IgnoreCase(This,pIgnoreCase)	\
    ( (This)->lpVtbl -> get_IgnoreCase(This,pIgnoreCase) ) 

#define IRegExp2_put_IgnoreCase(This,pIgnoreCase)	\
    ( (This)->lpVtbl -> put_IgnoreCase(This,pIgnoreCase) ) 

#define IRegExp2_get_Global(This,pGlobal)	\
    ( (This)->lpVtbl -> get_Global(This,pGlobal) ) 

#define IRegExp2_put_Global(This,pGlobal)	\
    ( (This)->lpVtbl -> put_Global(This,pGlobal) ) 

#define IRegExp2_get_Multiline(This,pMultiline)	\
    ( (This)->lpVtbl -> get_Multiline(This,pMultiline) ) 

#define IRegExp2_put_Multiline(This,pMultiline)	\
    ( (This)->lpVtbl -> put_Multiline(This,pMultiline) ) 

#define IRegExp2_Execute(This,sourceString,ppMatches)	\
    ( (This)->lpVtbl -> Execute(This,sourceString,ppMatches) ) 

#define IRegExp2_Test(This,sourceString,pMatch)	\
    ( (This)->lpVtbl -> Test(This,sourceString,pMatch) ) 

#define IRegExp2_Replace(This,sourceString,replaceVar,pDestString)	\
    ( (This)->lpVtbl -> Replace(This,sourceString,replaceVar,pDestString) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRegExp2_INTERFACE_DEFINED__ */


#ifndef __IMatch2_INTERFACE_DEFINED__
#define __IMatch2_INTERFACE_DEFINED__

/* interface IMatch2 */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IMatch2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACB1-160D-11D2-A8E9-00104B365C9F")
    IMatch2 : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ BSTR *pValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_FirstIndex( 
            /* [retval][out] */ long *pFirstIndex) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Length( 
            /* [retval][out] */ long *pLength) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_SubMatches( 
            /* [retval][out] */ IDispatch **ppSubMatches) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMatch2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMatch2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMatch2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMatch2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMatch2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMatch2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMatch2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMatch2 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IMatch2 * This,
            /* [retval][out] */ BSTR *pValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FirstIndex )( 
            IMatch2 * This,
            /* [retval][out] */ long *pFirstIndex);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMatch2 * This,
            /* [retval][out] */ long *pLength);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_SubMatches )( 
            IMatch2 * This,
            /* [retval][out] */ IDispatch **ppSubMatches);
        
        END_INTERFACE
    } IMatch2Vtbl;

    interface IMatch2
    {
        CONST_VTBL struct IMatch2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMatch2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMatch2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMatch2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMatch2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IMatch2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IMatch2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IMatch2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IMatch2_get_Value(This,pValue)	\
    ( (This)->lpVtbl -> get_Value(This,pValue) ) 

#define IMatch2_get_FirstIndex(This,pFirstIndex)	\
    ( (This)->lpVtbl -> get_FirstIndex(This,pFirstIndex) ) 

#define IMatch2_get_Length(This,pLength)	\
    ( (This)->lpVtbl -> get_Length(This,pLength) ) 

#define IMatch2_get_SubMatches(This,ppSubMatches)	\
    ( (This)->lpVtbl -> get_SubMatches(This,ppSubMatches) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMatch2_INTERFACE_DEFINED__ */


#ifndef __IMatchCollection2_INTERFACE_DEFINED__
#define __IMatchCollection2_INTERFACE_DEFINED__

/* interface IMatchCollection2 */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_IMatchCollection2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACB2-160D-11D2-A8E9-00104B365C9F")
    IMatchCollection2 : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long index,
            /* [retval][out] */ IDispatch **ppMatch) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMatchCollection2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMatchCollection2 * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMatchCollection2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMatchCollection2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMatchCollection2 * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMatchCollection2 * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMatchCollection2 * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMatchCollection2 * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMatchCollection2 * This,
            /* [in] */ long index,
            /* [retval][out] */ IDispatch **ppMatch);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMatchCollection2 * This,
            /* [retval][out] */ long *pCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMatchCollection2 * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        END_INTERFACE
    } IMatchCollection2Vtbl;

    interface IMatchCollection2
    {
        CONST_VTBL struct IMatchCollection2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMatchCollection2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMatchCollection2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMatchCollection2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMatchCollection2_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IMatchCollection2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IMatchCollection2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IMatchCollection2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IMatchCollection2_get_Item(This,index,ppMatch)	\
    ( (This)->lpVtbl -> get_Item(This,index,ppMatch) ) 

#define IMatchCollection2_get_Count(This,pCount)	\
    ( (This)->lpVtbl -> get_Count(This,pCount) ) 

#define IMatchCollection2_get__NewEnum(This,ppEnum)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMatchCollection2_INTERFACE_DEFINED__ */


#ifndef __ISubMatches_INTERFACE_DEFINED__
#define __ISubMatches_INTERFACE_DEFINED__

/* interface ISubMatches */
/* [object][oleautomation][nonextensible][dual][hidden][uuid] */ 


EXTERN_C const IID IID_ISubMatches;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3F4DACB3-160D-11D2-A8E9-00104B365C9F")
    ISubMatches : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long index,
            /* [retval][out] */ VARIANT *pSubMatch) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISubMatchesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISubMatches * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISubMatches * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISubMatches * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISubMatches * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISubMatches * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISubMatches * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISubMatches * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISubMatches * This,
            /* [in] */ long index,
            /* [retval][out] */ VARIANT *pSubMatch);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISubMatches * This,
            /* [retval][out] */ long *pCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISubMatches * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        END_INTERFACE
    } ISubMatchesVtbl;

    interface ISubMatches
    {
        CONST_VTBL struct ISubMatchesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISubMatches_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISubMatches_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISubMatches_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISubMatches_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ISubMatches_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ISubMatches_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ISubMatches_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ISubMatches_get_Item(This,index,pSubMatch)	\
    ( (This)->lpVtbl -> get_Item(This,index,pSubMatch) ) 

#define ISubMatches_get_Count(This,pCount)	\
    ( (This)->lpVtbl -> get_Count(This,pCount) ) 

#define ISubMatches_get__NewEnum(This,ppEnum)	\
    ( (This)->lpVtbl -> get__NewEnum(This,ppEnum) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISubMatches_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_RegExp;

#ifdef __cplusplus

class DECLSPEC_UUID("3F4DACA4-160D-11D2-A8E9-00104B365C9F")
RegExp;
#endif

EXTERN_C const CLSID CLSID_Match;

#ifdef __cplusplus

class DECLSPEC_UUID("3F4DACA5-160D-11D2-A8E9-00104B365C9F")
Match;
#endif

EXTERN_C const CLSID CLSID_MatchCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("3F4DACA6-160D-11D2-A8E9-00104B365C9F")
MatchCollection;
#endif

EXTERN_C const CLSID CLSID_SubMatches;

#ifdef __cplusplus

class DECLSPEC_UUID("3F4DACC0-160D-11D2-A8E9-00104B365C9F")
SubMatches;
#endif
#endif /* __VBScript_RegExp_55_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


