#pragma once


// CCheckBoxTreeCtrl

#define TVN_CHECKCHANGING	(TVN_ENDLABELEDIT-11)

class CCheckBoxTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CCheckBoxTreeCtrl)

public:
	CCheckBoxTreeCtrl();
	virtual ~CCheckBoxTreeCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	void	RecursiveCheckChildren(HTREEITEM hItem_start, bool check);
	void	RecursiveCheckParent(HTREEITEM hItem_start, bool check);
	void	UpdateRelatedItem(HTREEITEM hItem_start);
	void	SendCheckNotification();


public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


