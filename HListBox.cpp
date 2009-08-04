#include "stdafx.h"
#include "HListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHListBox

CHListBox::CHListBox()
{
 width = 0;
}

CHListBox::~CHListBox()
{
}


BEGIN_MESSAGE_MAP(CHListBox, CListBox)
	//{{AFX_MSG_MAP(CHListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHListBox message handlers
void CHListBox::updateWidth(LPCTSTR s)
    {
     CClientDC dc(this);
     CFont * f = CListBox::GetFont();
     dc.SelectObject(f);
     CSize sz = dc.GetTextExtent(s, _tcslen(s));
     sz.cx += 3 * ::GetSystemMetrics(SM_CXBORDER);
     if(sz.cx > width)
	 { /* extend */
	  width = sz.cx;
	  CListBox::SetHorizontalExtent(width);
	 } /* extend */
    }

int CHListBox::AddString(LPCTSTR s)
    {
     int result = CListBox::AddString(s);
     if(result < 0)
	 return result;
     updateWidth(s);
     return result;
    }

int CHListBox::InsertString(int i, LPCTSTR s)
    {
     int result = CListBox::InsertString(i, s);
     if(result < 0)
	 return result;
     updateWidth(s);
     return result;
    }

void CHListBox::ResetContent()
    {
     CListBox::ResetContent();
     width = 0;
    }

int CHListBox::DeleteString(int n)
    {
     int result = CListBox::DeleteString(n);
     if(result < 0)
	 return result;
     CClientDC dc(this);

     CFont * f = CListBox::GetFont();
     dc.SelectObject(f);

     width = 0;
     for(int i = 0; i < CListBox::GetCount(); i++)
	 { /* scan strings */
	  CString s;
	  CListBox::GetText(i, s);
	  CSize sz = dc.GetTextExtent(s);
          sz.cx += 3 * ::GetSystemMetrics(SM_CXBORDER);
	  if(sz.cx > width)
	      width = sz.cx;
	 } /* scan strings */
     CListBox::SetHorizontalExtent(width);
     return result;
    }
