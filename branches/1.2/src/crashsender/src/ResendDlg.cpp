/************************************************************************************* 
  This file is a part of CrashRpt library.

  CrashRpt is Copyright (c) 2003, Michael Carruth
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
#include <windows.h>
#include "resource.h"
#include "CrashInfoReader.h"
#include "ResendDlg.h"
#include "Utility.h"
#include "strconv.h"
#include "DetailDlg.h"
#include "ErrorReportSender.h"

LRESULT CProgressMultiDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{   
  DlgResize_Init(false);

  m_listLog = GetDlgItem(IDC_LIST);
  m_listLog.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
  m_listLog.InsertColumn(0, _T("Status"), LVCFMT_LEFT, 2048);

  m_prgProgress = GetDlgItem(IDC_PROGRESS);
  m_prgProgress.SetRange(0, 100);

  m_bMouseCaptured = FALSE;
  m_curSizeNS = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));

  SetTimer(0, 300);
  
  return 0;
}

LRESULT CProgressMultiDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  KillTimer(0);
  return 0;
}

LRESULT CProgressMultiDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{   
  // Get current progress
  int nProgressPct = 0;
  std::vector<CString> messages;
  g_ErrorReportSender.GetStatus(nProgressPct, messages);
    
  // Update progress bar
  m_prgProgress.SetPos(nProgressPct);

  unsigned i;
  for(i=0; i<messages.size(); i++)
  {     
    if(messages[i].CompareNoCase(_T("[status_success]"))==0)
    { 
      BOOL bNext = m_pParent->SendNextReport();
      if(!bNext)
      {
        m_pParent->ShowWindow(SW_SHOW);
      }
    }
    else if(messages[i].CompareNoCase(_T("[status_failed]"))==0)
    { 
      if(!g_CrashInfo.m_bSilentMode)
      {        
        BOOL bNext = m_pParent->SendNextReport();
        if(!bNext)
        {
          m_pParent->ShowWindow(SW_SHOW);
        }
      }
      else
      {
        
      }
    }
    else if(messages[i].CompareNoCase(_T("[exit_silently]"))==0)
    {       
      
      
    }
    else if(messages[i].CompareNoCase(_T("[cancelled_by_user]"))==0)
    { 
      
    }    
    else if(messages[i].CompareNoCase(_T("[confirm_launch_email_client]"))==0)
    {       
      KillTimer(1);        
      if(!g_CrashInfo.m_bSilentMode)
      {
        if(m_pParent->m_MailClientConfirm==NOT_CONFIRMED_YET)
        {
          m_pParent->ShowWindow(SW_SHOW);

          DWORD dwFlags = 0;
          CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
          if(sRTL.CompareNoCase(_T("1"))==0)
            dwFlags = MB_RTLREADING;

          CString sMailClientName;        
          CMailMsg::DetectMailClient(sMailClientName);
          CString msg;
          msg.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ProgressDlg"), _T("ConfirmLaunchEmailClient")), sMailClientName);

          CString sCaption = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ProgressDlg"), _T("DlgCaption"));
          CString sTitle;
          sTitle.Format(sCaption, g_CrashInfo.m_sAppName);
          INT_PTR result = MessageBox(msg, 
            sTitle,
            MB_OKCANCEL|MB_ICONQUESTION|dwFlags);


          if(result==IDOK)
            m_pParent->m_MailClientConfirm = ALLOWED;
          else
            m_pParent->m_MailClientConfirm = NOT_ALLOWED;

          g_ErrorReportSender.FeedbackReady(result==IDOK?0:1);       
          m_pParent->ShowWindow(SW_HIDE);
        }
        else
        {
          g_ErrorReportSender.FeedbackReady(m_pParent->m_MailClientConfirm==ALLOWED?0:1);       
        }
      }      
      else
      { 
        // In silent mode, assume user provides his/her consent
        g_ErrorReportSender.FeedbackReady(0);       
      }        
    }

    int count = m_listLog.GetItemCount();
    int indx = m_listLog.InsertItem(count, messages[i]);
    m_listLog.EnsureVisible(indx, TRUE);

  }
  return 0;
}

LRESULT CProgressMultiDlg::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);
  CPoint ptCursor(x, y);  

  if(ptCursor.y>=0 && ptCursor.y<=7)
  {
    m_ptInitial = ptCursor;
    ::SetCapture(m_hWnd);
    m_bMouseCaptured = TRUE;
  }

  return 0;
}

LRESULT CProgressMultiDlg::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  if(m_bMouseCaptured)
  {
    m_bMouseCaptured = FALSE;
    ::SetCapture(NULL);
  }
  return 0;
}

LRESULT CProgressMultiDlg::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);
  CPoint ptCursor(x, y);  

  if(ptCursor.y>=0 && ptCursor.y<=7)
  {
    SetCursor(m_curSizeNS);
    if(m_bMouseCaptured)
    {
      CPoint offs = ptCursor - m_ptInitial;
      CRect rcWnd;
      GetWindowRect(&rcWnd);
      ScreenToClient(&rcWnd);
      rcWnd.OffsetRect(offs);
      MoveWindow(rcWnd);
    }
  }  

  return 0;
}

BOOL CResendDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CResendDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{ 
  CString sRTL = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("Settings"), _T("RTLReading"));
  if(sRTL.CompareNoCase(_T("1"))==0)
  {
    Utility::SetLayoutRTL(m_hWnd);
  }

  CString sTitle;
  sTitle.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("DlgCaption")), 
    g_CrashInfo.m_sAppName);
  SetWindowText(sTitle);

	// center the dialog on the screen
	CenterWindow();
	
  // Set window icon
  SetIcon(::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME)), 0);
  
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

  m_statText = GetDlgItem(IDC_TEXT);
  m_statText.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("ClickForDetails")));

  m_statSize = GetDlgItem(IDC_SELSIZE);
  m_statSize.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("SelectedSize")));

  m_btnSendNow = GetDlgItem(IDOK);
  m_btnSendNow.SetWindowText(Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("SendNow")));

  m_btnOtherActions = GetDlgItem(IDC_OTHERACTIONS);
  m_btnOtherActions.SetWindowText(Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("OtherActions")));  

  // Init list control
  m_listReportsSort.SubclassWindow(GetDlgItem(IDC_LIST));  
  m_listReports.SubclassWindow(m_listReportsSort.m_hWnd);
  m_listReports.InsertColumn(0, Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("ColumnName")), LVCFMT_LEFT, 240);
  m_listReports.InsertColumn(1, Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("ColumnCreationDate")), LVCFMT_LEFT, 125);
  m_listReports.InsertColumn(2, Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("ColumnSize")), LVCFMT_RIGHT, 70);
  m_listReports.InsertColumn(3, Utility::GetINIString(
    g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("ColumnStatus")), LVCFMT_RIGHT, 90);
  m_listReports.ModifyStyleEx(0, LVS_EX_FULLROWSELECT);
  m_listReportsSort.SetSortColumn(1); // Sort by creation date
  int i;
  for(i=0; i<g_CrashInfo.GetReportCount(); i++)
  {
    ErrorReportInfo& eri = g_CrashInfo.GetReport(i);
    int nItem = m_listReports.InsertItem(i, eri.m_sCrashGUID);
    
    SYSTEMTIME st;
    Utility::UTC2SystemTime(eri.m_sSystemTimeUTC, st);
    CString sCreationDate;
    sCreationDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), 
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    m_listReports.SetItemText(nItem, 1, sCreationDate);

    CString sTotalSize = Utility::FileSizeToStr(eri.m_uTotalSize);
    
    m_listReports.SetItemText(nItem, 2, sTotalSize);
    
    if(eri.m_bSelected)
      m_listReports.SetCheckState(nItem, TRUE);
  }

  UpdateSelectionSize();

  m_statConsent = GetDlgItem(IDC_CONSENT);

  LOGFONT lf;
  memset(&lf, 0, sizeof(LOGFONT));
  lf.lfHeight = 11;
  lf.lfWeight = FW_NORMAL;
  lf.lfQuality = ANTIALIASED_QUALITY;
  _TCSCPY_S(lf.lfFaceName, 32, _T("Tahoma"));
  CFontHandle hConsentFont;
  hConsentFont.CreateFontIndirect(&lf);
  m_statConsent.SetFont(hConsentFont);

  if(g_CrashInfo.m_sPrivacyPolicyURL.IsEmpty())
    m_statConsent.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("MyConsent2")));
  else
    m_statConsent.SetWindowText(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("MyConsent")));

  m_linkPrivacyPolicy.SubclassWindow(GetDlgItem(IDC_PRIVACYPOLICY));
  m_linkPrivacyPolicy.SetHyperLink(g_CrashInfo.m_sPrivacyPolicyURL);
  m_linkPrivacyPolicy.SetLabel(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("MainDlg"), _T("PrivacyPolicy")));

  m_dlgProgress.m_pParent = this;
  m_dlgProgress.Create(m_hWnd);
  m_dlgProgress.SetWindowLong(GWL_ID, IDD_PROGRESSMULTI); 
  
  CRect rc;
  m_listReports.GetWindowRect(&rc);
  ScreenToClient(&rc);
  m_dlgProgress.SetWindowPos(HWND_TOP, rc.left, rc.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

  DlgResize_Init();

  m_bSendingNow = FALSE;
  m_MailClientConfirm = NOT_CONFIRMED_YET;

  // Show balloon in 3 seconds.
  m_nTick = 0;
  SetTimer(0, 3000);

  return TRUE;
}

LRESULT CResendDlg::OnTrayIcon(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
  if(LOWORD(lParam)==WM_LBUTTONDOWN || 
    LOWORD(lParam)==WM_LBUTTONDBLCLK ||
    LOWORD(lParam)==NIN_BALLOONUSERCLICK)
  {
    KillTimer(0);
    ShowWindow(SW_SHOW);
  }

  if(LOWORD(lParam)==WM_RBUTTONDOWN)
  {
    CPoint pt;
    GetCursorPos(&pt);
    CMenu menu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUPMENU));
    CMenu submenu = menu.GetSubMenu(2);

    strconv_t strconv;
    CString sShow = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("PopupShow"));
    CString sExit = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("PopupExit"));
    
    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING;

    mii.dwTypeData = sShow.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU3_SHOW, FALSE, &mii);

    mii.dwTypeData = sExit.GetBuffer(0);  
    submenu.SetMenuItemInfo(ID_MENU3_EXIT, FALSE, &mii);
  
    submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
  }

  return 0;
}

void CResendDlg::CloseDialog(int nVal)
{
	DestroyWindow();
  AddTrayIcon(FALSE);
	::PostQuitMessage(nVal);
}

LRESULT CResendDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{  
  if(m_nTick==0)
  {
    // Show tray icon and balloon.
    AddTrayIcon(TRUE);

    KillTimer(0);

    // Wait for one minute. If user doesn't want to click us, exit.
    SetTimer(0, 60000);
  }
  else if(m_nTick==1)
  {
    KillTimer(0);
    CloseDialog(0);
  }

  m_nTick ++;

  return 0;
}

LRESULT CResendDlg::OnPopupShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  KillTimer(0);
  ShowWindow(SW_SHOW);
  return 0;
}

LRESULT CResendDlg::OnPopupExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  KillTimer(0);
  CloseDialog(0);

  return 0;
}

LRESULT CResendDlg::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  NMLISTVIEW* pnmlv = (NMLISTVIEW *)pnmh;
  if(pnmlv->iItem>=0 && (pnmlv->uChanged&LVIF_STATE))
  {
    UpdateSelectionSize();
  }
  return 0;
}

LRESULT CResendDlg::OnListDblClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  NMITEMACTIVATE* pia = (NMITEMACTIVATE*)pnmh;
  if(pia->iItem>=0)
  {
    CDetailDlg dlg;
    dlg.m_nCurReport = pia->iItem;
    dlg.DoModal(m_hWnd);
  }
  return 0;
}

LRESULT CResendDlg::OnSendNow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if(!m_bSendingNow)
  {
    m_bSendingNow = TRUE;

    m_statSize.ShowWindow(SW_HIDE);
    m_statConsent.ShowWindow(SW_HIDE);
    m_linkPrivacyPolicy.ShowWindow(SW_HIDE);  
    m_btnOtherActions.ShowWindow(SW_HIDE);
    m_dlgProgress.ShowWindow(SW_SHOW);  
    m_btnSendNow.SetWindowText(
      Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ProgressDlg"), _T("Cancel")));

    SendNextReport();
  }
  else
  {

  }

  return 0;
}

BOOL CResendDlg::SendNextReport()
{
  int i;
  for(i=0; i<g_CrashInfo.GetReportCount(); i++)
  {
    BOOL bSelected = m_listReports.GetCheckState(i);

    // Send the first error report
    if(bSelected)
    {
      m_listReports.SetCheckState(i, 0);
      m_listReports.EnsureVisible(i, TRUE);

      g_ErrorReportSender.SetCurReport(i);
      g_ErrorReportSender.DoWork(COMPRESS_REPORT|SEND_REPORT);
      return TRUE;
    }
  }

  return FALSE;
}

LRESULT CResendDlg::OnOtherActions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  CPoint pt;
  GetCursorPos(&pt);
  CMenu menu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUPMENU));  
  CMenu submenu = menu.GetSubMenu(3);

  strconv_t strconv;
  CString sRemindLater = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("PopupRemindLater"));
  CString sNeverRemind = Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("PopupNeverRemind"));
  
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_STRING;

  mii.dwTypeData = sRemindLater.GetBuffer(0);  
  submenu.SetMenuItemInfo(ID_MENU4_REMINDLATER, FALSE, &mii);

  mii.dwTypeData = sNeverRemind.GetBuffer(0);  
  submenu.SetMenuItemInfo(ID_MENU4_NEVERREMIND, FALSE, &mii);

  submenu.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
  return 0;
}

LRESULT CResendDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  g_CrashInfo.SetLastRemindDateToday();
  CloseDialog(0);  
  return 0;
}

LRESULT CResendDlg::OnRemindLater(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
  g_CrashInfo.SetLastRemindDateToday();
  g_CrashInfo.SetRemindPolicy(REMIND_LATER);

  KillTimer(0);
  CloseDialog(0);
  return 0;
}

LRESULT CResendDlg::OnNeverRemind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  g_CrashInfo.SetLastRemindDateToday();
  g_CrashInfo.SetRemindPolicy(NEVER_REMIND);

  KillTimer(0);
  CloseDialog(0);
  return 0;
}

void CResendDlg::AddTrayIcon(BOOL bAdd)
{
  NOTIFYICONDATA nf;
	memset(&nf,0,sizeof(NOTIFYICONDATA));
	nf.cbSize = sizeof(NOTIFYICONDATA);
	nf.hWnd = m_hWnd;
	nf.uID = 0;

  if(bAdd) // Add icon to tray
	{	
		nf.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO ;
		nf.uCallbackMessage = WM_RESENDTRAYICON;
		nf.uVersion = NOTIFYICON_VERSION;

    CString sTip; 
    sTip.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("DlgCaption")), g_CrashInfo.m_sAppName);
		nf.hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
    _TCSCPY_S(nf.szTip, 64, sTip);
	
    CString sInfo;
    sInfo.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("BalloonText")), 
      g_CrashInfo.m_sAppName, g_CrashInfo.m_sAppName);
    _TCSCPY_S(nf.szInfo, 200, sInfo.GetBuffer(0));

    CString sInfoTitle;
    sInfoTitle.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("BalloonCaption")), 
      g_CrashInfo.m_sAppName);
    _TCSCPY_S(nf.szInfoTitle, 64, sInfoTitle.GetBuffer(0));

		Shell_NotifyIcon(NIM_ADD,&nf);
	}
	else // Delete icon
	{
		Shell_NotifyIcon(NIM_DELETE,&nf);
	}	
}

void CResendDlg::UpdateSelectionSize()
{
  int nItemsSelected = 0;
  ULONG64 uSelectedFilesSize = 0;

  int i;
  for(i=0; i<m_listReports.GetItemCount(); i++)
  {
    if(m_listReports.GetCheckState(i))
    {
      nItemsSelected++;
      uSelectedFilesSize += g_CrashInfo.GetReport(i).m_uTotalSize;
    }
  }

  CString sText;
  sText.Format(Utility::GetINIString(g_CrashInfo.m_sLangFileName, _T("ResendDlg"), _T("SelectedSize")), nItemsSelected, 
    Utility::FileSizeToStr(uSelectedFilesSize).GetBuffer(0));
  m_statSize.SetWindowText(sText);

  m_btnSendNow.EnableWindow(nItemsSelected>0?TRUE:FALSE);
}