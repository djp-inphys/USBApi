/** \file VtSysdefs.h

 General system definitions file, contains #defines, and typedefs required by other functions.

 * Copyright (c) 2013 by
 * All Rights Reserved
 *
 * This software comprises unpublished confidential information
 * comprising intellectual property of iX imaging plc and may
 * not be used, copied or made available to anyone, except in
 * accordance with the licence under which it is furnished.
 *
 * REVISIONS: 
 * $Log: VtSysdefs.h,v $
 * Revision 1.2  2013/05/27 12:04:19  david
 * Ver 1
 *
 *
 *
 */

#ifndef _VT_SYSDEFS_H__
#define _VT_SYSDEFS_H__



/**
\def VT_STATIC
\brief link the main library as a static library-
\par
The main API library can be built either as a static link library or a dll.
If the library is to be linked as a static library then the preprocessor symbol VT_STATIC should
be defined.

\def VTAPI_API is the main api library is linked as a static library then the #define VTAPI_API
is define as a blank. However, if the main library is to be linked or built as a dll then VTAPI_API is 
defined as a __declspec(dllexport) or __declspec(dllexport) respectively.

\def VTAPI_EXPORTS is defined when a dll library version of the API us being built.

\par

\def

 The following ifdef block is the standard way of creating macros which make exporting 
 from a DLL simpler. All files within this DLL are compiled with the VTAPI_EXPORTS
 symbol defined on the command line. this symbol should not be defined on any project
 that uses this DLL. This way any other project whose source files include this file see 
 VTAPI_API functions as being imported from a DLL, wheras this DLL sees symbols
 defined with this macro as being exported.

*/
#ifdef VT_STATIC
	#define VTAPI_API
	#pragma message( "Static library link" )
#else

#ifdef VTAPI_EXPORTS
	#define VTAPI_API __declspec(dllexport)
#else
	#define VTAPI_API __declspec(dllimport)
#endif

#endif

/**
\def WIN32
 If compiled under Win32 a pragma to cancel STL truncation warning:
 identifier was truncated to '255' characters in the browser information

 This warning is a well known and fixed problem with the Microsoft 
 implementation of STL
*/

#ifdef WIN32
#pragma warning(disable : 4786) 
#endif // WIN32


//*********************************************************************
// INCLUDES
//*********************************************************************
#include <limits.h>
#include <float.h>
#include <assert.h>

#ifdef __cplusplus
#include <string>
#include <vector>
#include <map>
#endif

//*********************************************************************
// ADDITIONAL DEFINITIONS
//*********************************************************************
#ifndef NULL
#define NULL (0)
#endif


//*********************************************************************
// TYPEDEFS
//*********************************************************************
typedef unsigned char vt_byte;
typedef unsigned char vt_uchar;
typedef char vt_char;
typedef unsigned __int16 vt_word;
typedef unsigned __int32 vt_dword;
typedef unsigned __int64 vt_ddword;
typedef unsigned __int16 vt_uint16;
typedef unsigned __int32 vt_uint32;
typedef unsigned __int64 vt_uint64;
typedef short vt_short;
typedef unsigned short vt_ushort;
typedef int vt_int;
typedef unsigned int vt_uint;
typedef long vt_long;
typedef unsigned long vt_ulong;
typedef float vt_float;
typedef double vt_double;
typedef long double vt_longdouble;

#ifdef __cplusplus
typedef bool vt_bool;
typedef std::string dset_id_type;
typedef std::string im_id_type;
typedef std::pair<dset_id_type, im_id_type> dset_im_id_type;
typedef std::vector<dset_id_type> dset_id_vec_type;
typedef std::vector<im_id_type> im_id_vec_type;
#endif

//*********************************************************************
// MACROS
//*********************************************************************
#define VT_MAXBYTE UCHAR_MAX
#define VT_MAXWORD ((2 ^ (sizeof(vt_word) * 8)) - 1)
#define VT_MAXDWORD ((2 ^ (sizeof(vt_dword) * 8)) - 1)
#define VT_MININT INT_MIN
#define VT_MAXINT INT_MAX
#define VT_MAXUINT UINT_MAX
#define VT_MAXUSHORT USHRT_MAX
#define VT_MINLONG LONG_MIN
#define VT_MAXLONG LONG_MAX
#define VT_MAXULONG ULONG_MAX
#define VT_MAXFLOAT FLT_MAX
#define VT_MAXDOUBLE DBL_MAX
#define VT_MINFLOAT FLT_MIN
#define VT_MINDOUBLE DBL_MIN
#define VT_EPSILONFLOAT FLT_EPSILON
#define VT_EPSILONDOUBLE DBL_EPSILON


//*********************************************************************
//! Using float for real values (to conserve memory usage)
//! Defined here to allow re-compilation of entire libraries
//! to using double instead of float in a single place
//*********************************************************************
#define vt_real vt_float
#define VT_MAXREAL (vt_real)VT_MAXFLOAT
#define VT_MINREAL (vt_real)VT_MINFLOAT
#define VT_REALDIG (vt_int)FLT_DIG
#define VT_EPSILONREAL (vt_real)VT_EPSILONFLOAT
#define VT_TINY ((ix_real)0.000001)


#endif // _VT_SYSDEFS_H__
