// CheckBoxTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Font_Creation.h"
#include "CheckBoxTreeCtrl.h"


// CCheckBoxTreeCtrl

IMPLEMENT_DYNAMIC(CCheckBoxTreeCtrl, CTreeCtrl)

CCheckBoxTreeCtrl::CCheckBoxTreeCtrl()
{

}

CCheckBoxTreeCtrl::~CCheckBoxTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CCheckBoxTreeCtrl, CTreeCtrl)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(NM_CLICK, &CCheckBoxTreeCtrl::OnNMClick)
END_MESSAGE_MAP()



void CCheckBoxTreeCtrl::RecursiveCheckChildren(HTREEITEM hmyItem, bool check)
{
	if (ItemHasChildren(hmyItem))
	{
	   HTREEITEM hChildItem = GetChildItem(hmyItem);

	   while (hChildItem != NULL)
	   {
			SetCheck(hChildItem, check);
			hChildItem = GetNextItem(hChildItem, TVGN_NEXT);
	   }
	}
}

void CCheckBoxTreeCtrl::RecursiveCheckParent(HTREEITEM hItem_start, bool check)
{
	HTREEITEM p = GetParentItem(hItem_start);
	if(p)
	{	
		if(check)
		{	SetCheck(p,check);
			RecursiveCheckParent(p,check);
		}
		else
		{
			HTREEITEM hChildItem = GetChildItem(p);

			while (hChildItem != NULL)
			{
				if(GetCheck(hChildItem))return;
				hChildItem = GetNextItem(hChildItem, TVGN_NEXT);
			}

			SetCheck(p,false);
			RecursiveCheckParent(p,false);
		}
	}
}

void CCheckBoxTreeCtrl::UpdateRelatedItem(HTREEITEM item)
{
	bool newly_checked = GetCheck(item);
	RecursiveCheckChildren(item,newly_checked);
	RecursiveCheckParent(item,newly_checked);
}

void CCheckBoxTreeCtrl::SendCheckNotification()
{
	NMHDR info;
	info.code = TVN_CHECKCHANGING;
	info.hwndFrom = GetSafeHwnd();
	info.idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
	GetParent()->SendMessage(WM_NOTIFY,info.idFrom,(LPARAM)&info);
}

// CCheckBoxTreeCtrl message handlers
void CCheckBoxTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(GetStyle()&TVS_CHECKBOXES && nChar == VK_SPACE)
	{	
		HTREEITEM cur = GetSelectedItem();
		if(cur)
		{	
			bool check = !GetCheck(cur);
			SetCheck(cur, check);
			UpdateRelatedItem(cur);

			SendCheckNotification();
		}

		return;
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CCheckBoxTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	UINT wh;
	HTREEITEM item = HitTest(point, &wh);
	if(wh == TVHT_ONITEMSTATEICON && item)
	{	
		SelectItem(item);
		
		bool check = !GetCheck(item);
		SetCheck(item, check);
		UpdateRelatedItem(item);
		SendCheckNotification();

		SetCheck(item, !check);
	}

	*pResult = 0;
}


