/////////////////////////////////////////////////////////////////////////////
// Name:        W32String.cpp
// Project:     dtpLib
// Purpose:     Utility, general-purpose functions, Win32 platform
// Author:      Piotr Likus
// Modified by:
// Created:     25/09/2010
// Licence:     BSD
/////////////////////////////////////////////////////////////////////////////

#include "base/btypes.h"
#include "base/W32String.h"

void charPtrToTCharPtr(PCSTR lpszIn, LPTSTR lpszOut)
{
#if defined(UNICODE)||defined(_UNICODE)
   ULONG index = 0;
   PCSTR lpAct = lpszIn;

	for( ; ; lpAct++ )
	{
		lpszOut[index++] = (TCHAR)(*lpAct);
		if ( *lpAct == 0 )
			break;
	}
#else
   // This is trivial :)
	strcpy( lpszOut, lpszIn );
#endif
}

void wcharToStdString(WCHAR *wch, std::string &out)
{
    std::wstring s(wch);
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    char* buf = new char[len];
    try {
      WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
      std::string r(buf);
      out = r;
      delete[] buf;
      buf = NULL;
    }
    catch(...) {
      delete[] buf;
      throw;
    }
}

void tcharToStdString(_TCHAR *tch, std::string &out)
{
#if defined(UNICODE)||defined(_UNICODE)
  wcharToStdString(tch, out);
#else
  out = std::string(tch);
#endif
}
