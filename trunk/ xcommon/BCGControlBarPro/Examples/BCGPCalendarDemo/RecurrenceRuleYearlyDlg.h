#if !defined(AFX_RECURRENCERULEYEARLYDLG_H__6147AC53_64C7_47C4_A779_C79B5E9EE82A__INCLUDED_)
#define AFX_RECURRENCERULEYEARLYDLG_H__6147AC53_64C7_47C4_A779_C79B5E9EE82A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecurrenceRuleYearlyDlg.h : header file
//

#include "RecurrenceRuleDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CRecurrenceRuleYearlyDlg dialog

class CRecurrenceRuleYearlyDlg : public CRecurrenceRuleDlg
{
// Construction
public:
	CRecurrenceRuleYearlyDlg();   // standard constructor

	virtual void SetRule (CBCGPRecurrenceBaseRule* pRule);

	virtual BOOL UpdateRule (const COleDateTime& dtStart);

// Dialog Data
	//{{AFX_DATA(CRecurrenceRuleYearlyDlg)
	enum { IDD = IDD_RECURRENCE_RULE_YEARLY };
	CComboBox	m_cbDayTypeWeekDay;
	CComboBox	m_cbDayTypeMonth;
	CComboBox	m_cbDayMonth;
	CButton m_btnTypeDay;
	CButton m_btnTypeDayType;
	DWORD	m_dwDay;
	int		m_nDayMonth;
	int		m_nDayTypeMonth;
	int		m_nDayTypeDay;
	int		m_nDayTypeWeekDay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecurrenceRuleYearlyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual UINT GetID () const
	{
		return IDD;
	}

	// Generated message map functions
	//{{AFX_MSG(CRecurrenceRuleYearlyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTypeDay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECURRENCERULEYEARLYDLG_H__6147AC53_64C7_47C4_A779_C79B5E9EE82A__INCLUDED_)
