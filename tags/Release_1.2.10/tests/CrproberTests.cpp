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

#include "stdafx.h"
#include "Tests.h"
#include "CrashRpt.h"
#include "Utility.h"
#include "strconv.h"
#include "Shellapi.h"

// Helper function prototype
BOOL CreateErrorReport(CString sTmpFolder, CString& sErrorReportName, CString& sMD5Hash);

class CrproberTests : public CTestSuite
{
  BEGIN_TEST_MAP(CrproberTests, "crprober.exe tests")
    REGISTER_TEST(Test_output)
    REGISTER_TEST(Test_extract_file)
  END_TEST_MAP()

public:

  void SetUp();
  void TearDown();
  
  void Test_output();
  void Test_extract_file();

  CString m_sTmpFolder;
  CString m_sErrorReportName;
  CString m_sMD5Hash;
};

REGISTER_TEST_SUITE( CrproberTests );

void CrproberTests::SetUp()
{
  CString sAppDataFolder;  

  // Create a temporary folder  
  Utility::GetSpecialFolder(CSIDL_APPDATA, sAppDataFolder);
  m_sTmpFolder = sAppDataFolder+_T("\\CrashRpt 应用程序名称");
  BOOL bCreate = Utility::CreateFolder(m_sTmpFolder);
  TEST_ASSERT(bCreate);

  // Create error report ZIP
  BOOL bCreateReport = CreateErrorReport(m_sTmpFolder, m_sErrorReportName, m_sMD5Hash);
  TEST_ASSERT(bCreateReport);

  __TEST_CLEANUP__;


}

void CrproberTests::TearDown()
{
  // Delete tmp folder
  Utility::RecycleFile(m_sTmpFolder, TRUE);
}

void CrproberTests::Test_output()
{ 
  // This test calls crprober.exe with /f, fmd5 and /o flags to
  // generate a text output file from error report ZIP archive

  CString sExeName;
  CString sParams;
  BOOL bExecute = FALSE;
  SHELLEXECUTEINFO sei;
  memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
  DWORD dwExitCode = 1;
  DWORD dwFileAttrs = INVALID_FILE_ATTRIBUTES;
  
#ifdef _DEBUG
  sExeName = Utility::GetModulePath(NULL)+_T("\\crproberd.exe");
#else
  sExeName = Utility::GetModulePath(NULL)+_T("\\crprober.exe");
#endif

  sParams.Format(_T("/f \"%s\" /fmd5 \"%s\" /o \"%s\""), 
    m_sErrorReportName, 
    m_sErrorReportName+_T(".md5"),
    m_sTmpFolder+_T("\\out.txt"));
    
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI;
  sei.lpVerb = _T("open");
  sei.lpFile = sExeName;
  sei.lpParameters = sParams;
  sei.lpDirectory = m_sTmpFolder;
  
  bExecute = ShellExecuteEx(&sei);
  TEST_ASSERT(bExecute);

  // Wait until process exits
  WaitForSingleObject(sei.hProcess, 10000);

  // Check crprober.exe process exit code - it should equal to 0
  GetExitCodeProcess(sei.hProcess, &dwExitCode);
  TEST_ASSERT(dwExitCode==0);

  // Check that out.txt file exists
  dwFileAttrs = GetFileAttributes(m_sTmpFolder+_T("\\out.txt"));
  TEST_ASSERT(dwFileAttrs!=INVALID_FILE_ATTRIBUTES);

  __TEST_CLEANUP__;

  if(sei.hProcess)
  {
    CloseHandle(sei.hProcess);
    sei.hProcess = NULL;
  }
  
}

void CrproberTests::Test_extract_file()
{ 
  // This test calls crprober.exe with /ext flag to
  // extract all files from error report ZIP archive

  CString sExeName;
  CString sParams;
  BOOL bExecute = FALSE;
  SHELLEXECUTEINFO sei;
  memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
  DWORD dwExitCode = 1;
  DWORD dwFileAttrs = INVALID_FILE_ATTRIBUTES;
  std::vector<CString> asFileList;
  UINT i;
  
  asFileList.push_back(_T("crashrpt.xml"));
  asFileList.push_back(_T("crashdump.dmp"));
  asFileList.push_back(_T("regkey.xml"));  
  
#ifdef _DEBUG
  sExeName = Utility::GetModulePath(NULL)+_T("\\crproberd.exe");
#else
  sExeName = Utility::GetModulePath(NULL)+_T("\\crprober.exe");
#endif

  sParams.Format(_T("/f \"%s\" /ext \"%s\""), m_sErrorReportName, m_sTmpFolder);
    
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI;
  sei.lpVerb = _T("open");
  sei.lpFile = sExeName;
  sei.lpParameters = sParams;
  sei.lpDirectory = m_sTmpFolder;
  
  bExecute = ShellExecuteEx(&sei);
  TEST_ASSERT(bExecute);

  // Wait until process exits
  WaitForSingleObject(sei.hProcess, 10000);

  // Check crprober.exe process exit code - it should equal to 0
  GetExitCodeProcess(sei.hProcess, &dwExitCode);
  TEST_ASSERT(dwExitCode==0);

  // Check that extracted files exist
  for(i=0; i<asFileList.size(); i++)
  {
    dwFileAttrs = GetFileAttributes(m_sTmpFolder+_T("\\")+asFileList[i]);
    TEST_ASSERT(dwFileAttrs!=INVALID_FILE_ATTRIBUTES);
  }

  __TEST_CLEANUP__;

  if(sei.hProcess)
  {
    CloseHandle(sei.hProcess);
    sei.hProcess = NULL;
  }  
}