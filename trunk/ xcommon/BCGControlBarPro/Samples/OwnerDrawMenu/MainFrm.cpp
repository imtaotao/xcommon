// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "OwnerDrawMenu.h"
#include "LineStyleMenuItem.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_REGISTERED_MESSAGE(BCGM_TOOLBARMENU, OnToolbarContextMenu)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGPToolBar::EnableQuickCustomization ();
	CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2003));

	if (!m_wndMenuBar.Create (this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CString strMainToolbarTitle;
	strMainToolbarTitle.LoadString (IDS_MAIN_TOOLBAR);
	m_wndToolBar.SetWindowText (strMainToolbarTitle);
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMenuBar);
	DockControlBar(&m_wndToolBar);

	//----------------------------------------------------------
	// Line styles will be protected (unable to customize them):
	//----------------------------------------------------------
	CList<UINT, UINT> lstLineStyles;
	for (UINT uiLineStyleCmd = ID_LINE_STYLE_1; uiLineStyleCmd <= ID_LINE_STYLE_10; 
		uiLineStyleCmd++)
	{
		lstLineStyles.AddTail (uiLineStyleCmd);
	}

	CBCGPToolbarButton::SetProtectedCommands (lstLineStyles);
	m_wndToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE, _T("Customize..."));

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnViewCustomize()
{
	//------------------------------------
	// Create a customize toolbars dialog:
	//------------------------------------
	CBCGPToolbarCustomize* pDlgCust = new CBCGPToolbarCustomize (this,
		TRUE /* Automatic menus scaning */);

	pDlgCust->Create ();
}

LRESULT CMainFrame::OnToolbarContextMenu(WPARAM,LPARAM lp)
{
	CPoint point (BCG_GET_X_LPARAM(lp), BCG_GET_Y_LPARAM(lp));

	CMenu menu;
	VERIFY(menu.LoadMenu (IDR_POPUP_TOOLBAR));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CBCGPPopupMenu* pPopupMenu = new CBCGPPopupMenu;
	pPopupMenu->Create (this, point.x, point.y, pPopup->Detach ());

	return 0;
}

afx_msg LRESULT CMainFrame::OnToolbarReset(WPARAM wp, LPARAM)
{
	UINT uiToolBarId = (UINT) wp;
	if (uiToolBarId == IDR_MAINFRAME)
	{
		//--------------------------------------------------------
		// Replace the regular toolbar button 'ID_LINE_STYLE_MENU'
		// by menu button:
		//--------------------------------------------------------
		CMenu menuLineStyles;
		menuLineStyles.LoadMenu (IDR_LINE_STYLE_POPUP);

		m_wndToolBar.ReplaceButton (ID_LINE_STYLE_MENU,
			CBCGPToolbarMenuButton (
				(UINT)-1, // No default command
				menuLineStyles.GetSafeHmenu (),
				CImageHash::GetImageOfCommand (ID_LINE_STYLE_MENU, FALSE),
				_T("&Line Style")));
	}

	return 0;
}

BOOL CMainFrame::OnShowPopupMenu (CBCGPPopupMenu* pMenuPopup)
{
    CFrameWnd::OnShowPopupMenu (pMenuPopup);

	if (pMenuPopup == NULL || pMenuPopup->GetMenuBar () == NULL)
	{
		return TRUE;
	}

	//---------------------------------------------------------
	// Replace ID_VIEW_TOOLBARS menu item to the toolbars list:
	//---------------------------------------------------------
    if (pMenuPopup->GetMenuBar ()->CommandToIndex (ID_VIEW_TOOLBARS) >= 0)
    {
		if (CBCGPToolBar::IsCustomizeMode ())
		{
			//----------------------------------------------------
			// Don't show toolbars list in the cuztomization mode!
			//----------------------------------------------------
			return FALSE;
		}

		pMenuPopup->RemoveAllItems ();

		CMenu menu;
		VERIFY(menu.LoadMenu (IDR_POPUP_TOOLBAR));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		pMenuPopup->GetMenuBar ()->ImportFromMenu (*pPopup, TRUE);
    }

	//-------------------------------------------------------------------
	// Replace all "line style" menu items by CLineStyleMenuItem objects:
	//-------------------------------------------------------------------
	for (UINT uiLineStyleCmd = ID_LINE_STYLE_1; 
		uiLineStyleCmd <= ID_LINE_STYLE_10; 
		uiLineStyleCmd++)
	{
		int iIndex = pMenuPopup->GetMenuBar ()->CommandToIndex (uiLineStyleCmd);
		if (iIndex >= 0)
		{
			pMenuPopup->GetMenuBar ()->m_bDisableSideBarInXPMode = TRUE;
			//----------------------------
			// Obtain item text and style:
			//----------------------------
			CString strText	= pMenuPopup->GetMenuBar ()->GetButtonText (iIndex);
			UINT uiStyle	= pMenuPopup->GetMenuBar ()->GetButtonStyle (iIndex);

			//------------------
			// Calc. line width:
			//------------------
			int nWidth = uiLineStyleCmd - ID_LINE_STYLE_1 + 1;

			//-------------------
			// Replace menu item:
			//-------------------
			pMenuPopup->GetMenuBar ()->ReplaceButton (uiLineStyleCmd,
				CLineStyleMenuItem (nWidth,
									uiLineStyleCmd,
									strText,
									uiStyle & TBBS_CHECKED));
		}
	}

	return TRUE;
}
