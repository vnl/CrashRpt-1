/************************************************************************************* 
  This file is a part of CrashRpt library.

  Copyright (c) 2003, Michael Carruth
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.
 
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 
   * Neither the name of the author nor the names of its contributors 
     may be used to endorse or promote products derived from this software without 
     specific prior written permission.
 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

#ifndef __SCREENCAP_H__
#define __SCREENCAP_H__

#include "stdafx.h"

extern "C" {
#include "png.h"
}

class CScreenCapture
{
public:

  CScreenCapture();

  void GetScreenRect(LPRECT rcScreen);
  BOOL CaptureScreenRect(RECT rcCapture, POINT ptCursorPos, CString sSaveDirName, int nIdStartFrom, std::vector<CString>& out_file_list);

  BOOL PngInit(int nWidth, int nHeight, CString sFileName);
  BOOL PngWriteRow(LPBYTE pRow);
  BOOL PngFinalize();

  /* Member variables. */

  CPoint m_ptCursorPos;
  CURSORINFO m_CursorInfo;
  int m_nIdStartFrom;
  CString m_sSaveDirName;
  FILE* m_fp;
  png_structp m_png_ptr;
  png_infop m_info_ptr;  
  std::vector<CString> m_out_file_list;
};

#endif //__SCREENCAP_H__


