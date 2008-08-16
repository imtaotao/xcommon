// UserTools.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "UserTools.h"

#include "MainFrm.h"
#include "UserToolsDoc.h"
#include "UserToolsView.h"

#include "SampleUserTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserToolsApp

BEGIN_MESSAGE_MAP(CUserToolsApp, CWinApp)
	//{{AFX_MSG_MAP(CUserToolsApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserToolsApp construction

CUserToolsApp::CUserToolsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CUserToolsApp object

CUserToolsApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CUserToolsApp initialization

BOOL CUserToolsApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("BCGSoft\\BCGControlBarPro\\Samples"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	SetRegistryBase (_T("Settings"));

	// Initialize all Managers for usage. They are automatically constructed
	// if not yet present
	InitContextMenuManager();
	InitKeyboardManager();

	// Enable user-defined tools. If you want allow more than 10 tools,
	// add tools entry to resources (ID_USER_TOOL11, ID_USER_TOOL12,...)
	EnableUserTools (ID_TOOLS_ENTRY, ID_USER_TOOL1, ID_USER_TOOL10,
		RUNTIME_CLASS (CSampleUserTool));

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CUserToolsDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CUserToolsView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	LoadCustomState ();

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CUserToolsApp message handlers

int CUserToolsApp::ExitInstance() 
{
	BCGCBProCleanUp();

	return CWinApp::ExitInstance();
}

void CUserToolsApp::OnAppAbout()
{
	BCGPShowAboutDlg (AFX_IDS_APP_TITLE);
}


void CUserToolsApp::PreLoadState ()
{

	GetContextMenuManager()->AddMenu (_T("My menu"), IDR_CONTEXT_MENU);

	// TODO: add another context menus here
}


/////////////////////////////////////////////////////////////////////////////
// CUserToolsApp message handlers
