//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2000 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

// BCGPToolBox.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_TOOLBOX

#include "BCGPToolBox.h"
#include "BCGPOutlookWnd.h"
#include "BCGPButton.h"
#include "BCGPVisualManager.h"
#include "BCGPPopupMenu.h"
#include "BCGPTooltipManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxPage

#define TOOLBOX_IMAGE_MARGIN 4

IMPLEMENT_DYNCREATE(CBCGPToolBoxPage, CBCGPControlBar)

CBCGPToolBoxPage::CBCGPToolBoxPage()
{
	m_nCheckedButton = -1;
	m_nHighlightedButton = -1;
	m_sizeImage = CSize (0, 0);
	m_pToolBox = NULL;
	m_pToolBoxEx = NULL;
	m_nVertScrollOffset = 0;
	m_nVertScrollSize = 0;
	m_bCheckFirstButton = TRUE;
	m_Mode = ToolBoxPageMode_Default;
	m_pButtonClass = NULL;
	m_pToolTip = NULL;
}

CBCGPToolBoxPage::~CBCGPToolBoxPage()
{
	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		delete m_arButtons [i];
	}
}

BEGIN_MESSAGE_MAP(CBCGPToolBoxPage, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPToolBoxPage)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_NCDESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
END_MESSAGE_MAP()

BOOL CBCGPToolBoxPage::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return  TRUE;
}
//*************************************************************************************
void CBCGPToolBoxPage::OnDestroy() 
{
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CBCGPControlBar::OnDestroy();
}
//*************************************************************************************
void CBCGPToolBoxPage::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CBCGPMemDC memDC (*pDCPaint, this);
	CDC* pDC = &memDC.GetDC ();

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
		rectClient, rectClient);

	m_Images.SetTransparentColor (globalData.clrBtnFace);

	CBCGPDrawState ds;
	m_Images.PrepareDrawImage (ds);

	pDC->SetBkMode (TRANSPARENT);

	CFont* pOldFont = pDC->SelectObject (&globalData.fontRegular);

	ASSERT(pOldFont != NULL);
	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPToolBoxButton* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		pButton->OnDraw (pDC);
	}

	m_Images.EndDrawImage (ds);
	pDC->SelectObject (pOldFont);
}
//****************************************************************************************
void CBCGPToolBoxPage::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPControlBar::OnSize(nType, cx, cy);
	AdjustLayout ();
}
//****************************************************************************************
void CBCGPToolBoxPage::AdjustLayout ()
{
	CBCGPControlBar::AdjustLayout ();

	m_nVertScrollOffset = 0;
	m_nVertScrollSize = 0;

	ReposButtons ();

	if (m_pToolBox != NULL)
	{
		CBCGPOutlookWnd* pOlWnd = DYNAMIC_DOWNCAST (CBCGPOutlookWnd, m_pToolBox->GetTabWnd ());
		ASSERT_VALID (pOlWnd);

		pOlWnd->EnableScrollButtons (TRUE, m_nVertScrollOffset > 0, m_nVertScrollSize > 0);
	}

	RedrawWindow ();
}
//****************************************************************************************
int CBCGPToolBoxPage::HitTest(CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen (&ptScreen);

	if (CWnd::WindowFromPoint (ptScreen)->GetSafeHwnd () != GetSafeHwnd ())
	{
		return -1;
	}

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPToolBoxButton* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		if (pButton->GetRect ().PtInRect (point))
		{
			return i;
		}
	}

	return -1;
}
//****************************************************************************************
void CBCGPToolBoxPage::ReposButtons ()
{
	CRect rectClient;
	GetClientRect (rectClient);

	m_nVertScrollSize = 0;

	const int nButtonWidth = m_Mode == ToolBoxPageMode_Images ? 
		m_sizeImage.cx + 3 * TOOLBOX_IMAGE_MARGIN :
		rectClient.Width ();

	const int nButtonHeight = m_Mode == ToolBoxPageMode_Images ? 
		m_sizeImage.cy + 3 * TOOLBOX_IMAGE_MARGIN :
		max (globalData.GetTextHeight (), m_sizeImage.cy) + TOOLBOX_IMAGE_MARGIN;

	int x = rectClient.left;
	int y = rectClient.top + TOOLBOX_IMAGE_MARGIN - 
		m_nVertScrollOffset * nButtonHeight;

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPToolBoxButton* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		CRect rectButton = CRect (
						CPoint (x, y),
						CSize (nButtonWidth, nButtonHeight));

		if (rectButton.top >= rectClient.top && rectButton.bottom <= rectClient.bottom)
		{
			pButton->SetRect (rectButton);

			if (m_pToolTip->GetSafeHwnd () != NULL)
			{
				m_pToolTip->SetToolRect (this, pButton->GetID (), rectButton);
			}
		}
		else
		{
			pButton->Hide ();

			if (m_pToolTip->GetSafeHwnd () != NULL)
			{
				m_pToolTip->SetToolRect (this, pButton->GetID (), CRect (0, 0, 0, 0));
			}
		}

		x += nButtonWidth;

		if (x + nButtonWidth > rectClient.right)
		{
			x = rectClient.left;
			y += nButtonHeight;

			if (y >= rectClient.bottom)
			{
				m_nVertScrollSize++;
			}
		}
	}

	RedrawWindow ();
}
//***************************************************************************************
void CBCGPToolBoxPage::HighlightButton (int nButton)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPToolBoxButton* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		if (pButton->IsHighlighted ())
		{
			if (i == nButton)
			{
				// Already highlighted
				return;
			}

			pButton->Highlight (FALSE);
			InvalidateRect (pButton->GetRect ());
		}
		
		if (i == nButton)
		{
			pButton->Highlight ();
			InvalidateRect (pButton->GetRect ());
		}
	}

	UpdateWindow ();
}
//***************************************************************************************
void CBCGPToolBoxPage::RedrawButton (int nButton)
{
	ASSERT_VALID (this);

	if (nButton < 0 || nButton >= m_arButtons.GetSize ())
	{
		return;
	}

	CBCGPToolBoxButton* pButton = m_arButtons [nButton];
	ASSERT_VALID (pButton);

	RedrawWindow (pButton->GetRect ());
}
//***************************************************************************************
CBCGPToolBoxButton* CBCGPToolBoxPage::GetButton (int nButton)
{
	ASSERT_VALID (this);

	if (nButton < 0 || nButton >= m_arButtons.GetSize ())
	{
		return NULL;
	}

	CBCGPToolBoxButton* pButton = m_arButtons [nButton];
	ASSERT_VALID (pButton);

	return pButton;
}
//***************************************************************************************
CBCGPToolBoxButton* CBCGPToolBoxPage::GetButtonByID (int nID)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arButtons.GetSize (); i++)
	{
		CBCGPToolBoxButton* pButton = m_arButtons [i];
		ASSERT_VALID (pButton);

		if (pButton->GetID () == nID)
		{
			return pButton;
		}
	}

	return NULL;
}
//***************************************************************************************
int CBCGPToolBoxPage::GetMaxHeight () const
{
	int nButtons = (int) m_arButtons.GetSize ();
	int nRows = nButtons;
	int nButtonHeight = 0;

	if (m_Mode != ToolBoxPageMode_Images)
	{
		nButtonHeight = max (globalData.GetTextHeight (), m_sizeImage.cy) + TOOLBOX_IMAGE_MARGIN;
	}
	else
	{
		nButtonHeight = m_sizeImage.cy + 3 * TOOLBOX_IMAGE_MARGIN;
		const int nButtonWidth = m_sizeImage.cx + 3 * TOOLBOX_IMAGE_MARGIN;

		CRect rectClient;
		GetClientRect (rectClient);

		const int nButtonsInRow = rectClient.Width () / nButtonWidth;

		if (nButtonsInRow > 0)
		{
			nRows = nButtons / nButtonsInRow;
		}
		else
		{
			nRows = 0;
		}

		if (nButtonsInRow * nRows < nButtons)
		{
			nRows++;
		}
	}

	return nRows * nButtonHeight + 2 * TOOLBOX_IMAGE_MARGIN;
}
//***************************************************************************************
BOOL CBCGPToolBoxPage::InitPage (UINT uiBmpResID, int nImageWidth, 
								 const CStringList& lstLabels,
								 CRuntimeClass* pButtonClass)
{
	ASSERT_VALID (this);

	m_pButtonClass = pButtonClass;

	if (!m_Images.Load (uiBmpResID))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	// Enable tooltips:
	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
		BCGP_TOOLTIP_TYPE_TOOLBOX);

	HBITMAP hBitmap = m_Images.GetImageWell ();

	BITMAP bmp;
	::GetObject (hBitmap, sizeof (BITMAP), (LPVOID) &bmp);

	m_sizeImage.cx = nImageWidth;
	m_sizeImage.cy = bmp.bmHeight;

	m_Images.SetImageSize (m_sizeImage, TRUE);

	for (int i = 0; i < m_Images.GetCount (); i++)
	{
		CBCGPToolBoxButton* pButton = DYNAMIC_DOWNCAST (CBCGPToolBoxButton, pButtonClass->CreateObject ());
		if (pButton == NULL)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		ASSERT_VALID (pButton);

		pButton->SetParentPage (this);
		pButton->SetImageList (&m_Images);
		pButton->SetImageIndex (i);
		pButton->SetID (i + 1);

		POSITION posLabel = lstLabels.FindIndex (i);
		if (posLabel != NULL)
		{
			pButton->SetLabel (lstLabels.GetAt (posLabel));
		}

		if (i == 0 && m_bCheckFirstButton)
		{
			m_nCheckedButton = 0;
			pButton->SetCheck (TRUE);
			RedrawButton (m_nCheckedButton);
		}

		m_arButtons.Add (pButton);

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty ();

			m_pToolTip->AddTool (	this, LPSTR_TEXTCALLBACK, &rectDummy, 
									pButton->GetID ());
		}
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPToolBoxPage::SetSelected (int nItem)
{
	ASSERT_VALID (this);

	if (nItem >= m_arButtons.GetSize ())
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_nCheckedButton >= 0)
	{
		if (nItem == m_nCheckedButton)
		{
			return TRUE;
		}

		m_arButtons [m_nCheckedButton]->SetCheck (FALSE);
		RedrawButton (m_nCheckedButton);
	}

	if (nItem >= 0)
	{
		m_arButtons [nItem]->SetCheck (TRUE);
		RedrawButton (nItem);
	}

	m_nCheckedButton = nItem;
	return TRUE;
}
//*****************************************************************************************
void CBCGPToolBoxPage::SetMode (ToolBoxPageMode mode)
{
	ASSERT_VALID (this);

	m_Mode = mode;

	if (GetSafeHwnd () != NULL)
	{
		ReposButtons ();
	}
}
//*****************************************************************************************
int CBCGPToolBoxPage::AddButton (LPCTSTR lpszText, HICON hIcon)
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);
	ASSERT (hIcon != NULL);

	// Add icon to the image list:
	int iIconIdx = m_Images.AddIcon (hIcon);

	CBCGPToolBoxButton* pButton = DYNAMIC_DOWNCAST (CBCGPToolBoxButton, 
		m_pButtonClass->CreateObject ());
	if (pButton == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT_VALID (pButton);

	pButton->SetParentPage (this);
	pButton->SetImageList (&m_Images);
	pButton->SetImageIndex (iIconIdx);
	pButton->SetLabel (lpszText);
	pButton->SetID ((int) m_arButtons.GetSize () + 1);

	m_arButtons.Add (pButton);

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		CRect rectDummy;
		rectDummy.SetRectEmpty ();

		m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, pButton->GetID ());
	}

	if (m_pToolBox != NULL)
	{
		m_pToolBox->AdjustLayout ();
	}
	else if (m_pToolBoxEx != NULL)
	{
		m_pToolBoxEx->AdjustLayout ();
	}

	AdjustLayout ();
	return iIconIdx;
}
//*****************************************************************************************
BOOL CBCGPToolBoxPage::DeleteButton (int nItem)
{
	ASSERT_VALID (this);

	if (nItem < 0 || nItem >= (int) m_arButtons.GetSize ())
	{
		return FALSE;
	}
	
	ASSERT_VALID (m_arButtons [nItem]);

	m_Images.DeleteImage (nItem);
	
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->DelTool (this, m_arButtons [nItem]->GetID ());
	}
	
	delete m_arButtons [nItem];
	m_arButtons.RemoveAt (nItem);
	
	if (m_pToolBox != NULL)
	{
		m_pToolBox->AdjustLayout ();
	}
	else if (m_pToolBoxEx != NULL)
	{
		m_pToolBoxEx->AdjustLayout ();
	}
	
	m_nCheckedButton = m_nHighlightedButton = -1;

	AdjustLayout ();
	return TRUE;
}
//*****************************************************************************************
void CBCGPToolBoxPage::DeleteAllButons ()
{
	ASSERT_VALID (this);

	const int nSize = (int) m_arButtons.GetSize ();

	for (int i = 0; i < nSize; i++)
	{
		ASSERT_VALID (m_arButtons [i]);

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->DelTool (this, m_arButtons [i]->GetID ());
		}

		delete m_arButtons [i];
	}
	
	m_arButtons.RemoveAll ();
	m_Images.Clear ();

	if (m_pToolBox != NULL)
	{
		m_pToolBox->AdjustLayout ();
	}
	else if (m_pToolBoxEx != NULL)
	{
		m_pToolBoxEx->AdjustLayout ();
	}
	
	m_nCheckedButton = m_nHighlightedButton = -1;
	AdjustLayout ();
}
//*****************************************************************************************
void CBCGPToolBoxPage::OnNcDestroy() 
{
	CBCGPControlBar::OnNcDestroy();
	delete this;
}
//*****************************************************************************************
void CBCGPToolBoxPage::SetScrollValue (int nScrollValue)
{
	ASSERT (nScrollValue >= 0);

	int nButtonHeight = max (globalData.GetTextHeight (), m_sizeImage.cy) + TOOLBOX_IMAGE_MARGIN;

	if (nButtonHeight > 0)
	{
		m_nVertScrollOffset = nScrollValue / nButtonHeight;
		if (nScrollValue % nButtonHeight > 0)
		{
			m_nVertScrollOffset++;
		}
	}

	ReposButtons ();
}
//*********************************************************************************
void CBCGPToolBoxPage::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/) 
{
}
//*********************************************************************************
void CBCGPToolBoxPage::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	SetFocus ();

	ReleaseCapture ();
	m_nHighlightedButton = -1;

	int nButtonIndex = HitTest (point);
	
	if (nButtonIndex < 0)
	{
		return;
	}

	if (m_nCheckedButton >= 0)
	{
		m_arButtons [m_nCheckedButton]->SetCheck (FALSE);
		RedrawButton (m_nCheckedButton);
	}

	m_nCheckedButton = nButtonIndex;

	if (m_nCheckedButton < 0)
	{
		return;
	}

	m_arButtons [m_nCheckedButton]->SetCheck (TRUE);
	RedrawButton (m_nCheckedButton);

	if (m_pToolBox != NULL)
	{
		ASSERT_VALID (m_pToolBox);
		int nPage = m_pToolBox->GetPageNumber (this);

		m_pToolBox->OnClickTool (nPage, m_nCheckedButton);
	}

	if (m_pToolBoxEx != NULL)
	{
		ASSERT_VALID (m_pToolBoxEx);
		int nPage = m_pToolBoxEx->GetPageNumber (this);

		m_pToolBoxEx->OnClickTool (nPage, m_nCheckedButton);
	}
}
//*********************************************************************************
void CBCGPToolBoxPage::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CBCGPControlBar::OnLButtonUp(nFlags, point);
}
//*********************************************************************************
void CBCGPToolBoxPage::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPControlBar::OnMouseMove(nFlags, point);

	int nPrevHighlightedButton = m_nHighlightedButton;
	m_nHighlightedButton = HitTest (point);

	if (m_nHighlightedButton != nPrevHighlightedButton)
	{
		HighlightButton (m_nHighlightedButton);

		if (m_nHighlightedButton != -1)
		{
			if (nPrevHighlightedButton == -1)
			{
				SetCapture ();
			}
		}
		else
		{
			ReleaseCapture ();
		}
	}
}
//*********************************************************************************
BOOL CBCGPToolBoxPage::PreTranslateMessage(MSG* pMsg) 
{
   	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	return CBCGPControlBar::PreTranslateMessage(pMsg);
}
//**********************************************************************************
BOOL CBCGPToolBoxPage::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	if (m_Mode != ToolBoxPageMode_Images)
	{
		return FALSE;
	}

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	CBCGPToolBoxButton* pButton = GetButtonByID ((int) pNMH->idFrom);
	if (pButton == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pButton);

	strTipText = pButton->GetLabel ();
	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
	return TRUE;
}
//**************************************************************************
LRESULT CBCGPToolBoxPage::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & BCGP_TOOLTIP_TYPE_TOOLBOX)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_TOOLBOX);

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty ();

			for (int i = 0; i < m_arButtons.GetSize (); i++)
			{
				ASSERT_VALID (m_arButtons [i]);

				m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectDummy, 
					m_arButtons [i]->GetID ());
			}

			ReposButtons ();
		}
	}

	return 0;
}
//**************************************************************************
void CBCGPToolBoxPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	ASSERT_VALID (this);

	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	const int nHit = HitTest (ptClient);

	ReleaseCapture ();
	m_nHighlightedButton = -1;

	if (m_nCheckedButton >= 0)
	{
		m_arButtons [m_nCheckedButton]->SetCheck (FALSE);
		RedrawButton (m_nCheckedButton);
	}

	m_nCheckedButton = nHit;

	if (m_nCheckedButton >= 0)
	{
		m_arButtons [m_nCheckedButton]->SetCheck (TRUE);
		RedrawButton (m_nCheckedButton);
	}

	if (m_pToolBox != NULL)
	{
		ASSERT_VALID (m_pToolBox);

		if (m_pToolBox->OnShowToolboxMenu (point, this, nHit))
		{
			return;
		}
	}
	else if (m_pToolBoxEx != NULL)
	{
		ASSERT_VALID (m_pToolBoxEx);

		if (m_pToolBoxEx->OnShowToolboxMenu (point, this, nHit))
		{
			return;
		}
	}

	CBCGPControlBar::OnContextMenu (pWnd, point);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBox

IMPLEMENT_DYNCREATE(CBCGPToolBox, CBCGPDockingControlBar)

CBCGPToolBox::CBCGPToolBox()
{
}

CBCGPToolBox::~CBCGPToolBox()
{
}

BEGIN_MESSAGE_MAP(CBCGPToolBox, CBCGPDockingControlBar)
	//{{AFX_MSG_MAP(CBCGPToolBox)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBox message handlers

int CBCGPToolBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPDockingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectDummy (0, 0, 0, 0);

	// Create ToolBox pane:
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT;
	DWORD dwBCGStyle = 0;

	if (!m_wndOutlook.Create (_T(""), this, rectDummy, 
		0, dwStyle, dwBCGStyle))
	{
		TRACE0("Failed to create ToolBox window\n");
		return -1;      // fail to create
	}

    m_wndOutlook.SetBarStyle (CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY |
								CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndOutlook.EnableSetCaptionTextToTabName (FALSE);

	return 0;
}
//****************************************************************************************
void CBCGPToolBox::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	AdjustLayout ();
	RedrawWindow ();
}
//****************************************************************************************
BOOL CBCGPToolBox::AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
								 LPCTSTR lpszLabels,
								 CRuntimeClass* pPageClass,
								 CRuntimeClass* pButtonClass)
{
	ASSERT (lpszLabels != NULL);

	CString strLabels = lpszLabels;
	CStringList lstLabels;

	for (int i = 0; i < strLabels.GetLength ();)
	{
		CString strLabel;
			
		int iNextWord = strLabels.Find (_T('\n'), i);
		if (iNextWord == -1)
		{
			lstLabels.AddTail (strLabels.Mid (i));
			break;
		}

		lstLabels.AddTail (strLabels.Mid (i, iNextWord - i));
		i = iNextWord + 1;
	}

	return AddToolsPage (lpszPageName, uiBmpResID, nImageWidth,
								 lstLabels, pPageClass, pButtonClass);
}
//****************************************************************************************
BOOL CBCGPToolBox::AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
								 const CStringList& lstLabels, 
								 CRuntimeClass* pPageClass,
								 CRuntimeClass* pButtonClass)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);
	ASSERT (pPageClass != NULL);
	ASSERT (pPageClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolBoxPage)));
	ASSERT (pButtonClass != NULL);
	ASSERT (pButtonClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolBoxButton)));
	ASSERT (lpszPageName != NULL);

	CBCGPToolBoxPage* pPage = DYNAMIC_DOWNCAST (
		CBCGPToolBoxPage, pPageClass->CreateObject ());
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CRect rectDummy (0, 0, 0, 0);
	pPage->Create (NULL, WS_VISIBLE | WS_CHILD, rectDummy, this, 0);

	pPage->SetWindowText (lpszPageName);
	if (!pPage->InitPage (uiBmpResID, nImageWidth, lstLabels, pButtonClass))
	{
		delete pPage;
		return FALSE;
	}

	m_wndOutlook.AddTab (pPage, TRUE, TRUE, FALSE);
	pPage->m_pToolBox = this;

	OnActivatePage (m_wndOutlook.GetTabsNum () - 1);

	return TRUE;
}
//*****************************************************************************************
int CBCGPToolBox::GetActivePage () const
{
	ASSERT_VALID (this);

	CBCGPBaseTabWnd* pTab = GetTabWnd ();
	if (pTab == NULL)
	{
		ASSERT (FALSE);
		return  -1;
	}

	ASSERT_VALID (pTab);

	return pTab->GetActiveTab ();
}
//*****************************************************************************************
int CBCGPToolBox::GetLastClickedTool (int nPage) const
{
	ASSERT_VALID (this);

	CBCGPBaseTabWnd* pTab = GetTabWnd ();
	if (pTab == NULL)
	{
		ASSERT (FALSE);
		return  -1;
	}

	ASSERT_VALID (pTab);

	if (nPage < 0 || nPage >= pTab->GetTabsNum ())
	{
		ASSERT (FALSE);
		return -1;
	}

	CBCGPToolBoxPage* pPage = DYNAMIC_DOWNCAST (CBCGPToolBoxPage, pTab->GetTabWnd (nPage));
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return -1;
	}

	ASSERT_VALID (pPage);
	return pPage->m_nCheckedButton;
}
//******************************************************************************************
int CBCGPToolBox::GetPageNumber (CBCGPToolBoxPage* pPage) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);

	CBCGPBaseTabWnd* pTab = GetTabWnd ();
	if (pTab == NULL)
	{
		ASSERT (FALSE);
		return  -1;
	}

	ASSERT_VALID (pTab);

	for (int i = 0; i < pTab->GetTabsNum (); i++)
	{
		CBCGPToolBoxPage* pListPage = DYNAMIC_DOWNCAST (CBCGPToolBoxPage, pTab->GetTabWnd (i));
		if (pListPage->GetSafeHwnd () == pPage->GetSafeHwnd ())
		{
			return i;
		}
	}

	return -1;
}
//****************************************************************************************
CBCGPToolBoxPage* CBCGPToolBox::GetPage (int nPage) const
{
	ASSERT_VALID (this);

	CBCGPBaseTabWnd* pTab = GetTabWnd ();
	if (pTab == NULL)
	{
		ASSERT (FALSE);
		return  NULL;
	}

	ASSERT_VALID (pTab);

	return DYNAMIC_DOWNCAST (CBCGPToolBoxPage, pTab->GetTabWnd (nPage));
}
//****************************************************************************************
void CBCGPToolBox::OnClickTool (int /*nPage*/, int /*nIndex*/)
{
	ASSERT_VALID (this);

	CWnd* pOwner = GetOwner ();
	if (pOwner != NULL)
	{
		pOwner->PostMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}
}
//*****************************************************************************************
BOOL CBCGPToolBox::OnShowToolboxMenu (CPoint /*point*/, 
									  CBCGPToolBoxPage* /*pPage*/, int /*nHit*/)
{
	return FALSE;
}
//*****************************************************************************************
CBCGPBaseTabWnd* CBCGPToolBox::GetTabWnd () const
{
	ASSERT_VALID (this);
	return ((CBCGPToolBox*) this)->m_wndOutlook.GetUnderlinedWindow ();
}

BEGIN_MESSAGE_MAP(CBCGPToolBoxBar, CBCGPOutlookBar)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CBCGPToolBoxBar::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CBCGPToolBox* pParentBar = DYNAMIC_DOWNCAST (CBCGPToolBox, GetParent ());
	if (pParentBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	CRect rectClient;
	GetClientRect (rectClient);

	if (!rectClient.PtInRect (ptClient) || 
		!pParentBar->OnShowToolboxMenu (point, NULL, -1))
	{
		CBCGPOutlookBar::OnContextMenu (pWnd, point);
	}
}
//***************************************************************************************
void CBCGPToolBoxBar::OnActivateTab (int iTabNum)
{
	ASSERT_VALID (this);

	CBCGPOutlookBar::OnActivateTab (iTabNum);

	CBCGPToolBox* pParentBar = DYNAMIC_DOWNCAST (CBCGPToolBox, GetParent ());
	if (pParentBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pParentBar->OnActivatePage (iTabNum);
}
//****************************************************************************************
void CBCGPToolBoxBar::OnScroll (BOOL bDown)
{
	ASSERT_VALID (this);

	CBCGPOutlookBar::OnScroll (bDown);

	CBCGPToolBox* pParentBar = DYNAMIC_DOWNCAST (CBCGPToolBox, GetParent ());
	if (pParentBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pParentBar->OnScroll (bDown);
}
//****************************************************************************************
void CBCGPToolBox::OnActivatePage (int /*nPage*/)
{
	ASSERT_VALID (this);

	CBCGPOutlookWnd* pOlWnd = DYNAMIC_DOWNCAST (CBCGPOutlookWnd, GetTabWnd ());
	ASSERT_VALID (pOlWnd);

	CBCGPToolBoxPage* pPage = GetPage (GetActivePage ());
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	pOlWnd->EnableScrollButtons (TRUE, pPage->m_nVertScrollOffset > 0, pPage->m_nVertScrollSize > 0);
}
//***************************************************************************************
void CBCGPToolBox::OnScroll (BOOL bDown)
{
	ASSERT_VALID (this);

	CBCGPOutlookWnd* pOlWnd = DYNAMIC_DOWNCAST (CBCGPOutlookWnd, GetTabWnd ());
	ASSERT_VALID (pOlWnd);

	CBCGPToolBoxPage* pPage = GetPage (GetActivePage ());
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	if (bDown)
	{
		pPage->m_nVertScrollOffset++;
	}
	else
	{
		pPage->m_nVertScrollOffset--;
	}

	pPage->ReposButtons ();
	pOlWnd->EnableScrollButtons (TRUE, pPage->m_nVertScrollOffset > 0, pPage->m_nVertScrollSize > 0);
}
//***************************************************************************************
void CBCGPToolBox::AdjustLayout ()
{
	CBCGPDockingControlBar::AdjustLayout ();

	CRect rectClient;
	GetClientRect (rectClient);

	m_wndOutlook.SetWindowPos (NULL,
            -1, -1,
            rectClient.Width (), rectClient.Height (),
            SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	int nActivePage = GetActivePage ();
	if (nActivePage >= 0)
	{
		OnActivatePage (nActivePage);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxButton

IMPLEMENT_DYNCREATE(CBCGPToolBoxButton, CBCGPToolBoxButton)

CBCGPToolBoxButton::CBCGPToolBoxButton ()
{
	m_iImageIndex = -1;
	m_bIsHighlighted = FALSE;
	m_bIsChecked = FALSE;
	m_Rect.SetRectEmpty ();
	m_pImages = NULL;
	m_pPage = NULL;
	m_nID = -1;
}
//***************************************************************************************
void CBCGPToolBoxButton::OnFillBackground (CDC* pDC, const CRect& rectClient)
{
	CBCGPVisualManager::GetInstance ()->OnEraseToolBoxButton (pDC, rectClient, this);
}
//***************************************************************************************
void CBCGPToolBoxButton::OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState)
{
	CBCGPVisualManager::GetInstance ()->OnDrawToolBoxButtonBorder (pDC, rectClient,
		this, uiState);
}
//***************************************************************************************
void CBCGPToolBoxButton::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pPage);

	if (m_Rect.IsRectEmpty ())
	{
		return;
	}

	OnFillBackground (pDC, m_Rect);
	OnDrawBorder (pDC, m_Rect, 0);

	CRect rectText = m_Rect;

	if (m_pImages != NULL && m_iImageIndex >= 0)
	{
		ASSERT_VALID (m_pImages);

		CSize sizeImage = m_pImages->GetImageSize ();
		CRect rectImage = rectText;

		if (m_pPage->GetMode () != CBCGPToolBoxPage::ToolBoxPageMode_Images)
		{
			rectImage.right = rectImage.left + sizeImage.cx + 10;
		}

		m_pImages->Draw (pDC,
				rectImage.left + (rectImage.Width () - sizeImage.cx) / 2,
				rectImage.top + (rectImage.Height () - sizeImage.cy) / 2,
				m_iImageIndex);

		rectText.left = rectImage.right;
	}

	if (m_pPage->GetMode () == CBCGPToolBoxPage::ToolBoxPageMode_Images)
	{
		return;
	}

	rectText.DeflateRect (10, 0);
	pDC->DrawText (m_strLabel, rectText, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxEx

IMPLEMENT_DYNCREATE(CBCGPToolBoxEx, CBCGPTasksPane)

CBCGPToolBoxEx::CBCGPToolBoxEx()
{
	m_nVertMargin = 0;
	m_nHorzMargin = 0;
	m_nGroupVertOffset = 0;
	m_nGroupCaptionHorzOffset = 0;
	m_nGroupCaptionVertOffset = 0;

	m_bOffsetCustomControls = FALSE;
	m_bAnimationEnabled = TRUE;
}

CBCGPToolBoxEx::~CBCGPToolBoxEx()
{
}

BEGIN_MESSAGE_MAP(CBCGPToolBoxEx, CBCGPTasksPane)
	//{{AFX_MSG_MAP(CBCGPToolBoxEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolBoxEx message handlers

int CBCGPToolBoxEx::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPTasksPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableScrollButtons (FALSE);
	return 0;
}
//****************************************************************************************
void CBCGPToolBoxEx::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPTasksPane::OnSize(nType, cx, cy);
	AdjustLayout ();
}
//****************************************************************************************
BOOL CBCGPToolBoxEx::AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
								 LPCTSTR lpszLabels,
								 CRuntimeClass* pPageClass,
								 CRuntimeClass* pButtonClass)
{
	ASSERT (lpszLabels != NULL);

	CString strLabels = lpszLabels;
	CStringList lstLabels;

	for (int i = 0; i < strLabels.GetLength ();)
	{
		CString strLabel;
			
		int iNextWord = strLabels.Find (_T('\n'), i);
		if (iNextWord == -1)
		{
			lstLabels.AddTail (strLabels.Mid (i));
			break;
		}

		lstLabels.AddTail (strLabels.Mid (i, iNextWord - i));
		i = iNextWord + 1;
	}

	return AddToolsPage (lpszPageName, uiBmpResID, nImageWidth,
								 lstLabels, pPageClass, pButtonClass);
}
//****************************************************************************************
BOOL CBCGPToolBoxEx::AddToolsPage (LPCTSTR lpszPageName, UINT uiBmpResID, int nImageWidth,
								 const CStringList& lstLabels, 
								 CRuntimeClass* pPageClass,
								 CRuntimeClass* pButtonClass)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);
	ASSERT (pPageClass != NULL);
	ASSERT (pPageClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolBoxPage)));
	ASSERT (pButtonClass != NULL);
	ASSERT (pButtonClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolBoxButton)));
	ASSERT (lpszPageName != NULL);

	CBCGPToolBoxPage* pPage = DYNAMIC_DOWNCAST (
		CBCGPToolBoxPage, pPageClass->CreateObject ());
	if (pPage == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	pPage->m_bCheckFirstButton = FALSE;

	CRect rectDummy (0, 0, 0, 0);
	pPage->Create (NULL, WS_VISIBLE | WS_CHILD, rectDummy, this, 0);

	pPage->SetWindowText (lpszPageName);
	if (!pPage->InitPage (uiBmpResID, nImageWidth, lstLabels, pButtonClass))
	{
		delete pPage;
		return FALSE;
	}

	int nHeight = pPage->GetMaxHeight ();
	int nGroup = AddGroup (0, lpszPageName);
	AddWindow (nGroup, pPage->GetSafeHwnd (), nHeight);

	pPage->m_pToolBoxEx = this;

	return TRUE;
}
//*****************************************************************************************
int CBCGPToolBoxEx::GetLastClickedTool (int& nClickedPage) const
{
	ASSERT_VALID (this);

	for (int nPage = 0; nPage < GetPageCount (); nPage++)
	{
		CBCGPToolBoxPage* pPage = GetPage (nPage);
		ASSERT_VALID (pPage);

		if (pPage->GetSelected () >= 0)
		{
			nClickedPage = nPage;
			return pPage->GetSelected ();
		}
	}

	nClickedPage = -1;
	return -1;
}
//******************************************************************************************
int CBCGPToolBoxEx::GetPageNumber (CBCGPToolBoxPage* pPage) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);

	int nPage = 0;

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL; nPage++)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetHead ();
		if (pTask != NULL && pTask->m_hwndTask == pPage->GetSafeHwnd ())
		{
			return nPage;
		}
	}

	return -1;
}
//****************************************************************************************
int CBCGPToolBoxEx::GetPageCount () const
{
	ASSERT_VALID (this);
	return (int) m_lstTaskGroups.GetCount ();
}
//****************************************************************************************
CBCGPToolBoxPage* CBCGPToolBoxEx::GetPage (int nPage) const
{
	ASSERT_VALID (this);

	POSITION posGroup = m_lstTaskGroups.FindIndex (nPage);
	if (posGroup == NULL)
	{
		return NULL;
	}

	CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetAt (posGroup);
	ASSERT_VALID (pGroup);

	CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetHead ();
	if (pTask == NULL || pTask->m_hwndTask == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	return DYNAMIC_DOWNCAST (CBCGPToolBoxPage,
		CWnd::FromHandle (pTask->m_hwndTask));
}
//****************************************************************************************
void CBCGPToolBoxEx::OnClickTool (int nPage, int /*nIndex*/)
{
	ASSERT_VALID (this);

	CWnd* pOwner = GetOwner ();
	if (pOwner != NULL)
	{
		pOwner->PostMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}

	for (int i = 0; i < GetPageCount (); i++)
	{
		if (i != nPage)
		{
			CBCGPToolBoxPage* pPage = GetPage (i);
			ASSERT_VALID (pPage);

			pPage->SetSelected (-1);
			pPage->m_nHighlightedButton = -1;
			pPage->HighlightButton (-1);
		}
	}
}
//*****************************************************************************************
BOOL CBCGPToolBoxEx::OnShowToolboxMenu (CPoint /*point*/, 
									  CBCGPToolBoxPage* /*pPage*/, int /*nHit*/)
{
	return FALSE;
}
//*********************************************************************************
void CBCGPToolBoxEx::OnFillBackground (CDC* pDC, CRect rectFill)
{
	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this,
			rectFill, rectFill);
}
//*********************************************************************************
void CBCGPToolBoxEx::ScrollChild (HWND hwndChild, int nScrollValue)
{
	ASSERT_VALID (this);

	CWnd* pChildWnd = CWnd::FromHandle (hwndChild);
	CBCGPToolBoxPage* pPage = DYNAMIC_DOWNCAST (CBCGPToolBoxPage, pChildWnd);
	ASSERT_VALID (pPage);

	pPage->SetScrollValue (nScrollValue);
}
//*********************************************************************************
void CBCGPToolBoxEx::AdjustLayout ()
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition (); pos != NULL;)
	{
		CBCGPTasksGroup* pGroup = (CBCGPTasksGroup*) m_lstTaskGroups.GetNext (pos);
		ASSERT_VALID (pGroup);

		CBCGPTask* pTask = (CBCGPTask*) pGroup->m_lstTasks.GetHead ();
		ASSERT_VALID (pTask);

		CBCGPToolBoxPage* pPage = (CBCGPToolBoxPage*) CWnd::FromHandle (pTask->m_hwndTask);
		ASSERT_VALID (pPage);

		pTask->m_nWindowHeight = pPage->GetMaxHeight ();
	}

	RecalcLayout ();
	RedrawWindow ();
}
//*********************************************************************************
void CBCGPToolBoxEx::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	CRect rectClient;
	GetClientRect (rectClient);

	if (!rectClient.PtInRect (ptClient) || !OnShowToolboxMenu (point, NULL, -1))
	{
		CBCGPTasksPane::OnContextMenu (pWnd, point);
	}
}

#endif // BCGP_EXCLUDE_TOOLBOX

