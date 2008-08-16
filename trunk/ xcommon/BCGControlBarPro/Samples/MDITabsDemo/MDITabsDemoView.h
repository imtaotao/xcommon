// MDITabsDemoView.h : interface of the CMDITabsDemoView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDITABSDEMOVIEW_H__BB79B267_3E6D_4DEF_B0CA_30B87FA1EDD3__INCLUDED_)
#define AFX_MDITABSDEMOVIEW_H__BB79B267_3E6D_4DEF_B0CA_30B87FA1EDD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CMDITabsDemoView : public CView
{
protected: // create from serialization only
	CMDITabsDemoView();
	DECLARE_DYNCREATE(CMDITabsDemoView)

// Attributes
public:
	CMDITabsDemoDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDITabsDemoView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMDITabsDemoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMDITabsDemoView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MDITabsDemoView.cpp
inline CMDITabsDemoDoc* CMDITabsDemoView::GetDocument()
   { return (CMDITabsDemoDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDITABSDEMOVIEW_H__BB79B267_3E6D_4DEF_B0CA_30B87FA1EDD3__INCLUDED_)
