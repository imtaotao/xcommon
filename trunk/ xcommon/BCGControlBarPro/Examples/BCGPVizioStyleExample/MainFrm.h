// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__D1AF3CB5_2EEC_4A35_A89B_B96BA136F8F9__INCLUDED_)
#define AFX_MAINFRM_H__D1AF3CB5_2EEC_4A35_A89B_B96BA136F8F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ToolBoxBar.h"

#define CFrameWnd CBCGPFrameWnd

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CBCGPMenuBar		m_wndMenuBar;
	CBCGPStatusBar		m_wndStatusBar;
	CBCGPToolBar		m_wndToolBar;
	CToolBoxBar			m_wndToolBoxBar1;
	CToolBoxBar			m_wndToolBoxBar2;
	CToolBoxBar			m_wndToolBoxBar3;
	CToolBoxBar			m_wndToolBoxBar4;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarReset(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnShowPopupMenu (CBCGPPopupMenu* pMenuPopup);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__D1AF3CB5_2EEC_4A35_A89B_B96BA136F8F9__INCLUDED_)
