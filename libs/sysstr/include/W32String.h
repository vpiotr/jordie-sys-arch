/////////////////////////////////////////////////////////////////////////////
// Name:        W32String.h
// Project:     dtpLib
// Purpose:     Utility string functions, Win32 platform
// Author:      Piotr Likus
// Modified by:
// Created:     25/09/2010
// Licence:     BSD
/////////////////////////////////////////////////////////////////////////////

#ifndef _W32STRING_H__
#define _W32STRING_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file W32String.h
/// \brief Utility string functions, Win32 platform
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <string>

//platform
#include <windows.h>
#include <tchar.h>

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------
// Unicode safe char* -> TCHAR* conversion
void charPtrToTCharPtr(PCSTR lpszIn, LPTSTR lpszOut);
void wcharToStdString(WCHAR *wch, std::string &out);
void tcharToStdString(_TCHAR *tch, std::string &out);

#endif