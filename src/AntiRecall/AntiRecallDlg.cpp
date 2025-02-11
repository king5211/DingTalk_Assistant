/* 
 * Copyright (c) [2019] zhenfei.mzf@gmail.com rights reserved.
 * 
 * DTAssist is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
*/

// AntiRecallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AntiRecall.h"
#include "AntiRecallDlg.h"
#include <Shlwapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "version")  

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PATCH_EXTEND_SIZE	0x20
#define BACKUP_EXTEND		L".mzf"
#define GITHUB				L"https://github.com/mohuihui/DingTalk_Assistant"

const unsigned char MZF_PATCHED[] = {'m','z','f','p','a','t','c','h','e','d'};

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAntiRecallDlg dialog




CAntiRecallDlg::CAntiRecallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAntiRecallDlg::IDD, pParent)
	, mRadioDingtalk(true)
	, mAppPath(_T(""))
	, mCurrentVersion(_T(""))
	, mMainFramePath(_T(""))
	, mNewMainFramePath(_T(""))
	, mPatchStatus(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAntiRecallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_APP_PATH, mAppPath);
	DDX_Text(pDX, IDC_STATIC_VERSION_NUM, mCurrentVersion);
	DDX_Text(pDX, IDC_STATIC_PATCH_STATUS, mPatchStatus);
}

BEGIN_MESSAGE_MAP(CAntiRecallDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_BROWSE_APP, &CAntiRecallDlg::OnBnClickedBtnBrowseApp)
	ON_BN_CLICKED(IDOK, &CAntiRecallDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_MESSAGE, OnUpdateMessage)
	ON_BN_CLICKED(ID_BTN_REVERT, &CAntiRecallDlg::OnBnClickedBtnRevert)
	ON_STN_CLICKED(IDC_STATIC_GITHUB, &CAntiRecallDlg::OnStnClickedStaticGithub)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

LRESULT CAntiRecallDlg::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{
	if (mPatchStatus.Find(L"补丁成功") != -1)
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
	return 0;
}

BOOL CAntiRecallDlg::IsAlreadyPatched() 
{
	BOOL bRet = FALSE;
	DWORD dwSize = 0, dwRet = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	unsigned char *pBuffer = NULL;

	if (mMainFramePath.IsEmpty() || PathFileExists(mMainFramePath) == FALSE)
	{
		return bRet;
	}
	
	hFile = CreateFile(mMainFramePath, GENERIC_ALL, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		goto __exit;
	}

	dwSize = GetFileSize(hFile, NULL);
	if (dwSize == INVALID_FILE_SIZE || dwSize < PATCH_EXTEND_SIZE)
	{
		goto __exit;
	}

	pBuffer = (unsigned char *)malloc(PATCH_EXTEND_SIZE);
	if (pBuffer == NULL)
	{
		goto __exit;
	}

	memset(pBuffer, 0, PATCH_EXTEND_SIZE);
	
	if (SetFilePointer(hFile, dwSize - PATCH_EXTEND_SIZE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		goto __exit;
	}

	if (ReadFile(hFile, pBuffer, PATCH_EXTEND_SIZE, &dwRet, NULL))
	{
		for (DWORD i = 0; i < PATCH_EXTEND_SIZE - sizeof(MZF_PATCHED); i++)
		{
			if (memcmp(pBuffer + i, MZF_PATCHED, sizeof(MZF_PATCHED)) == 0) 
			{
				bRet = TRUE;
				break;
			}
		}
	}

__exit:
	if (hFile != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	
	if (pBuffer)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	return bRet;
}

int CAntiRecallDlg::GetSoftWareInstallPath() 
{
	int ret = -1;
	WCHAR default_x64_path[MAX_PATH] = L"X:\\Program Files (x86)\\DingDing\\main\\current\\DingTalk.exe";
	WCHAR default_x86_path[MAX_PATH] = L"X:\\Program Files\\DingDing\\main\\current\\DingTalk.exe";

	for (WCHAR i = 'A'; i <= 'Z'; i++) 
	{
		default_x64_path[0] = i;
		if (PathFileExists(default_x64_path)) 
		{
			mAppPath = default_x64_path;
			ret = 0;
			break;
		}

		default_x86_path[0] = i;
		if (PathFileExists(default_x86_path)) 
		{
			mAppPath = default_x86_path;
			ret = 0;
			break;
		}
	}
	
	if (ret != 0)
	{
		mAppPath = L"无法自动获取钉钉路径，请手动选取。";	
	}

	return ret;
}

int CAntiRecallDlg::GetFileCurrentVersion(CString path) 
{
	int ret = -1;
	
	if (path.IsEmpty())
	{
		return ret;
	}

	DWORD dwLen = GetFileVersionInfoSize(path, NULL); 
	char *pszAppVersion = new char[dwLen + 1];
	if(pszAppVersion)
	{
		memset(pszAppVersion, 0, sizeof(char) * (dwLen + 1));
		GetFileVersionInfo(path, NULL, dwLen, pszAppVersion);
		UINT nLen(0);
		VS_FIXEDFILEINFO *pFileInfo(NULL);
		VerQueryValue(pszAppVersion, L"\\", (LPVOID*)&pFileInfo, &nLen);
		if(pFileInfo)
		{
			mCurrentVersion.Format(L"%d.%d.%d.%d", 
				HIWORD(pFileInfo->dwFileVersionMS),
				LOWORD(pFileInfo->dwFileVersionMS),
				HIWORD(pFileInfo->dwFileVersionLS),
				LOWORD(pFileInfo->dwFileVersionLS));
			ret = 0;
		}

		delete pszAppVersion;
	}

	return ret;
}

void CAntiRecallDlg::CheckPatchStatus()
{
	if (IsAlreadyPatched())
	{
		mPatchStatus = L"已打过补丁，无需再打。";
		((CButton *)GetDlgItem(IDOK))->EnableWindow(FALSE);
	}
	else
	{
		mPatchStatus = L"N/A";
		((CButton *)GetDlgItem(IDOK))->EnableWindow(TRUE);
	}
}

void CAntiRecallDlg::CheckBackup()
{
	CString backup = mMainFramePath + BACKUP_EXTEND;
	CString backup_new = mNewMainFramePath + BACKUP_EXTEND;
	if (PathFileExists(backup) == FALSE && PathFileExists(backup_new) == FALSE)
	{
		((CButton *)GetDlgItem(ID_BTN_REVERT))->EnableWindow(FALSE);
	}
	else
	{
		((CButton *)GetDlgItem(ID_BTN_REVERT))->EnableWindow(TRUE);
	}
}

void CAntiRecallDlg::CheckPatchStatusAndBackup()
{
	GetFileCurrentVersion(mAppPath);
	UpdateMainFramePath();
	CheckPatchStatus();
	CheckBackup();

	UpdateData(FALSE);
}

// CAntiRecallDlg message handlers

BOOL CAntiRecallDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	mPatchStatus = L"N/A";
	mCurrentVersion = L"N/A";

	((CButton *)GetDlgItem(IDC_RADIO_DINGTALK))->SetCheck(TRUE);
	
	GetSoftWareInstallPath();

	if (mAppPath.IsEmpty() == FALSE && PathFileExists(mAppPath))
	{
		CheckPatchStatusAndBackup();
	}

	EnumDingTalkProcess();

	UpdateData(FALSE);

	// TODO: Add extra initialization here
	GetDlgItem(IDC_STATIC_GITHUB)->GetWindowRect(&m_Rect);
	ScreenToClient (&m_Rect);

	m_cfNtr = this->GetFont();
	m_cfNtr->GetLogFont ( &m_lfNtr );
	m_cfNtr->GetLogFont ( &m_lfUL );
	m_lfUL.lfUnderline = TRUE;
	m_cfUL.CreateFontIndirect ( &m_lfUL );

	m_brush.CreateSysColorBrush ( COLOR_MENU );
	m_color = RGB (0,0,0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAntiRecallDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAntiRecallDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAntiRecallDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAntiRecallDlg::OnBnClickedBtnBrowseApp()
{
	CString filePath = _T("");
	CFileDialog dlgFile(TRUE, 
		NULL, 
		NULL, 
		OFN_HIDEREADONLY, 
		_T("DingTalk Exe (DingTalk.exe)|DingTalk.exe|All Files (*.*)|*.*||"), NULL);

	if (dlgFile.DoModal() == IDOK)
	{
		filePath = dlgFile.GetPathName();
		mAppPath = filePath;

		CheckPatchStatusAndBackup();
	}
	
	UpdateData(FALSE);
}

int CAntiRecallDlg::UpdateMainFramePath()
{
	int ret = -1;
	if (mAppPath.IsEmpty() || PathFileExists(mAppPath) == FALSE)
	{
		goto __error;
	}

	int index = mAppPath.ReverseFind('\\');
	if (index == -1)
	{
		goto __error;
	}

	mMainFramePath = mAppPath.Left(index);
	index = mMainFramePath.ReverseFind('\\');
	if (index != -1)
	{
		mNewMainFramePath = mMainFramePath.Left(index);
		mNewMainFramePath += L"\\current_new\\MainFrame.dll";
	}

	mMainFramePath += L"\\MainFrame.dll";
	if (mMainFramePath.IsEmpty() || PathFileExists(mMainFramePath) == FALSE)
	{
		goto __error;
	}
	
	ret = 0;
	return ret;

__error:
	mPatchStatus = L"补丁失败，文件路径可能不对。";
	UpdateData(FALSE);
	return ret;
}

int FindAndPatch(unsigned char *buf, DWORD size)
{
	int ret = -1;
	unsigned char target1[] = {0xCC, 0xCC};
	unsigned char target2[] = {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x6A, 0xFF, 0x68};
	unsigned char target3[] = {0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x81, 0xEC};
	DWORD start = 0x500000;

	if (buf == NULL || size < start )
	{
		return ret;
	}

	for (DWORD i = start; i < size; i++)
	{
		if (memcmp(target1, buf + i, sizeof(target1)) == 0 &&
			memcmp(target2, buf + i + sizeof(target1), sizeof(target2)) == 0 &&
			memcmp(target3, buf + i + sizeof(target1) + sizeof(target2) + 4, sizeof(target3)) == 0)
		{
			unsigned char *buf_tmp = buf + i + sizeof(target1) + sizeof(target2) + 4 + sizeof(target3);
			for (int j = 0; j < 1000; j++)
			{
				if (buf_tmp[j] == 0xe8 && buf_tmp[j+16] == 0xe8 && buf_tmp[j+21] == 0x53 
				&& buf_tmp[j+22] == 0x8d && buf_tmp[j+26] == 0x51
				&& buf_tmp[j+27] == 0x8b && buf_tmp[j+28] == 0x10 
				&& buf_tmp[j+29] == 0x8b && buf_tmp[j+30] == 0xc8)
				{
					buf[i + 2] = 0xc3;
 					ret = 0;
 					break;
				}
			}

			if (ret == 0) 
			{
				break;
			}
		}
	}

	return ret;
}

UINT PatchProc( LPVOID pParam )
{
	UINT ret = 0;
	CAntiRecallDlg *dlg = (CAntiRecallDlg*)pParam;
	if (dlg == NULL)
	{
		return ret;
	}

	// backup the mainframe.dll
	CString pathes[] = {dlg->mMainFramePath, dlg->mNewMainFramePath, L""};
	PVOID pBuffer = NULL;
	DWORD dwSize = 0, dwRet= 0;;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	int i = 0;

	while (pathes[i].IsEmpty() == FALSE)
	{
		CString path = pathes[i++];
		CString backup = path + BACKUP_EXTEND;

		hFile = CreateFile(path, GENERIC_ALL, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			goto __exit;
		}

		dwSize = GetFileSize(hFile, NULL);
		if (dwSize == INVALID_FILE_SIZE || dwSize <= 0)
		{
			goto __exit;
		}

		pBuffer = malloc(dwSize + PATCH_EXTEND_SIZE);
		if (pBuffer == NULL)
		{
			goto __exit;
		}

		memset(pBuffer, 0, dwSize + PATCH_EXTEND_SIZE);

		if (ReadFile(hFile, pBuffer, dwSize, &dwRet, NULL))
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;

			if (FindAndPatch((unsigned char *)pBuffer, dwSize) == 0) 
			{
				memcpy((unsigned char *)pBuffer + dwSize + 1, MZF_PATCHED, sizeof(MZF_PATCHED));

				CopyFile(path, backup, TRUE);

				CFile file;
				TRY 
				{
					if (file.Open(path, CFile::modeCreate | CFile::modeWrite))
					{
						file.Write(pBuffer, dwSize + PATCH_EXTEND_SIZE);
						file.Close();
						ret = 1;
					}
				}
				CATCH_ALL( e )
				{
					file.Abort();
					THROW_LAST();
				}
				END_CATCH_ALL
			}
		}

		if (pBuffer) 
		{
			free(pBuffer);
			pBuffer = NULL;
		}

		if (hFile != INVALID_HANDLE_VALUE) 
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
	}

__exit:
	if (pBuffer) 
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (ret == 1)
	{
		dlg->mPatchStatus = L"补丁成功，重启钉钉后生效。";
	}
	else 
	{
		dlg->mPatchStatus = L"补丁失败，请确认钉钉关闭后重试。";
	}
	
	dlg->PostMessage(WM_UPDATE_MESSAGE, 0, 0);

	return ret;
}

// start patch
void CAntiRecallDlg::OnBnClickedOk()
{
	// OnOK();
	UpdateData();

	if (UpdateMainFramePath() != 0)
	{
		return;
	}
	
	mPatchStatus = L"开始补丁，请稍后...";
	UpdateData(FALSE);
	Sleep(1000);

	CWinThread* MyThread = AfxBeginThread(
		PatchProc, 
		this, 
		THREAD_PRIORITY_NORMAL, 
		0, 
		0, 
		NULL);
	
	Sleep(1000);

	if (IsAlreadyPatched())
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(FALSE);
	}
	else
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(TRUE);
	}

	CheckBackup();
}

void CAntiRecallDlg::OnBnClickedBtnRevert()
{
	UpdateData();
	UpdateMainFramePath();

	CString backup = mMainFramePath + BACKUP_EXTEND;
	CString backup_new = mNewMainFramePath + BACKUP_EXTEND;
	BOOL bret = FALSE;

	if (PathFileExists(backup))
	{
		bret = MoveFileEx(backup, mMainFramePath, MOVEFILE_REPLACE_EXISTING);
	}
	
	if (PathFileExists(backup_new))
	{
		bret = MoveFileEx(backup_new, mNewMainFramePath, MOVEFILE_REPLACE_EXISTING);
	}

	if (bret)
	{
		mPatchStatus = L"撤销补丁成功，重启钉钉后生效。";
	}
	else 
	{
		mPatchStatus = L"撤销失败，请确认钉钉关闭后重试。";
	}

	UpdateData(FALSE);

	if (IsAlreadyPatched())
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(FALSE);
	}
	else
	{
		((CButton *)GetDlgItem(IDOK))->EnableWindow(TRUE);
	}

	CheckBackup();
}

void CAntiRecallDlg::EnumDingTalkProcess()
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS,
		0
		);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return;
	}

	PROCESSENTRY32 processEntry32 = { 0 };
	processEntry32.dwSize = sizeof(processEntry32);
	BOOL bFind = Process32First(hProcessSnap, &processEntry32);
	if (!bFind)
	{
		CloseHandle(hProcessSnap);
		return;
	}

	while (bFind)
	{
		const WCHAR *ding = L"DingTalk.exe";
		size_t ding_len = wcslen(ding);
		size_t proc_len = wcslen(processEntry32.szExeFile);

		if (ding_len == proc_len && wcscmp(ding, processEntry32.szExeFile) == 0)
		{
			::MessageBox(NULL,
				L"检测到钉钉正在运行，请关闭后再使用本补丁程序。",
				L"钉钉助手",
				MB_OK | MB_ICONSTOP);
			ExitProcess(-1);
			break;
		}

		bFind = Process32Next(hProcessSnap, &processEntry32);
	}
}

void CAntiRecallDlg::OnStnClickedStaticGithub()
{
	ShellExecute(m_hWnd, NULL, GITHUB, NULL, NULL, SW_SHOWMAXIMIZED);   
}

void CAntiRecallDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (point.x > m_Rect.left && point.x < m_Rect.right && point.y < m_Rect.bottom && point.y > m_Rect.top )
	{
		ShellExecute (NULL, NULL, GITHUB, NULL, NULL, SW_NORMAL);
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CAntiRecallDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( point.x > m_Rect.left && point.x < m_Rect.right && point.y < m_Rect.bottom && point.y > m_Rect.top )
	{
		HCURSOR hCursor;
		hCursor = ::LoadCursor ( NULL, IDC_HAND );
		::SetCursor ( hCursor );

		GetDlgItem(IDC_STATIC_GITHUB)->SetFont ( &m_cfUL );

		m_color = RGB (0,0,225);
		CStatic* m_pStatic = (CStatic*)GetDlgItem(IDC_STATIC_GITHUB);
		m_pStatic->RedrawWindow ();
	}
	else
	{
		GetDlgItem(IDC_STATIC_GITHUB)->SetFont ( m_cfNtr );

		m_color = RGB (0,0,0);
		CStatic* m_pStatic = (CStatic*)GetDlgItem(IDC_STATIC_GITHUB);
		m_pStatic->RedrawWindow();
	}

	CDialog::OnMouseMove(nFlags, point);
}

HBRUSH CAntiRecallDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( nCtlColor == CTLCOLOR_STATIC )
	{
		pDC->SetBkMode ( TRANSPARENT );
		pDC->SetTextColor ( m_color );

		return (HBRUSH)m_brush.GetSafeHandle ();
	}

	return hbr;
}
