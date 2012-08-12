#include "core/app.h"

#include "frameui/framewnd.h"
#include "frameui/fontsys.h"

#include "controlframes.h"

ButtonFrame::ButtonFrame(String text, Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create("Button", text, WS_CHILD | WS_TABSTOP | style, 0);
  setFont(FontSys::getSysFont());
  setId(id);
}
uint32 ButtonFrame::onMessage(uint32 message, uint32 wParam, uint32 lParam)
{
  if (message == WM_ERASEBKGND)
  {
    if ((GetWindowLong(hWnd, GWL_STYLE) & BS_TYPEMASK) == BS_GROUPBOX)
    {
      HDC hDC = (HDC) wParam;
      HBRUSH hBrush = (HBRUSH) GetClassLong(GetParent(hWnd), GCL_HBRBACKGROUND);
      RECT rc;
      GetClientRect(hWnd, &rc);
      FillRect(hDC, &rc, hBrush);
      InvalidateRect(hWnd, NULL, FALSE);
      return TRUE;
    }
  }
  return M_UNHANDLED;
}

///////////////////////////////////////////////////////

LinkFrame::LinkFrame(String text, Frame* parent, int id)
  : WindowFrame(parent)
{
  hFont = NULL;
  uFont = NULL;
  pressed = false;
  hover = false;
  create(text, WS_CHILD, 0);
  setFont(FontSys::getSysFont());
  resetSize();
  setId(id);
}

void LinkFrame::resetSize()
{
  HDC hDC = GetDC(hWnd);
  SelectObject(hDC, hFont);
  SIZE sz;
  String text = getText();
  GetTextExtentPoint32(hDC, text, text.length(), &sz);
  ReleaseDC(hWnd, hDC);
  setSize(sz.cx, sz.cy);
}
void LinkFrame::setColor(uint32 color)
{
  _color = color;
  InvalidateRect(hWnd, NULL, TRUE);
}
void LinkFrame::setFlags(int flags)
{
  _flags = flags;
  InvalidateRect(hWnd, NULL, TRUE);
}

uint32 LinkFrame::onMessage(uint32 message, uint32 wParam, uint32 lParam)
{
  switch (message)
  {
  case WM_SETFONT:
    hFont = (wParam ? (HFONT) wParam : FontSys::getSysFont());
    uFont = FontSys::changeFlags(FontSys::getFlags(hFont) | FONT_UNDERLINE, hFont);
    if (lParam)
      InvalidateRect(hWnd, NULL, TRUE);
    return 0;
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hDC = BeginPaint(hWnd, &ps);

      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hWnd, &pt);

      SetBkMode(hDC, TRANSPARENT);
      SetTextColor(hDC, _color);
      if (pressed || hover)
        SelectObject(hDC, uFont);
      else
        SelectObject(hDC, hFont);
      RECT rc = {0, 0, width(), height()};
      String text = getText();
      DrawText(hDC, text, text.length(), &rc, _flags);

      EndPaint(hWnd, &ps);
    }
    return 0;
  case WM_MOUSEMOVE:
    {
      int x = LOWORD(lParam);
      int y = HIWORD(lParam);
      if (!hover && x > 0 && y > 0 && x < width() && y < height())
      {
        hover = true;
        TRACKMOUSEEVENT tme;
        memset(&tme, 0, sizeof tme);
        tme.cbSize = sizeof tme;
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hWnd;
        TrackMouseEvent(&tme);
        if (!pressed)
          InvalidateRect(hWnd, NULL, FALSE);
      }
    }
    return 0;
  case WM_MOUSELEAVE:
    {
      hover = false;
      TRACKMOUSEEVENT tme;
      memset(&tme, 0, sizeof tme);
      tme.cbSize = sizeof tme;
      tme.dwFlags = TME_CANCEL | TME_LEAVE;
      tme.hwndTrack = hWnd;
      TrackMouseEvent(&tme);
      if (!pressed)
        InvalidateRect(hWnd, NULL, TRUE);
    }
    return 0;
  case WM_LBUTTONDOWN:
    SetCapture(hWnd);
    pressed = true;
    return 0;
  case WM_LBUTTONUP:
    if (pressed)
    {
      ReleaseCapture();
      pressed = false;
      notify(WM_COMMAND, MAKELONG(id(), BN_CLICKED), (uint32) hWnd);
    }
    return 0;
  }
  return M_UNHANDLED;
}

///////////////////////////////////////////////////////

EditFrame::EditFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create("Edit", "", style | WS_CHILD | WS_TABSTOP, WS_EX_CLIENTEDGE);
  setFont(FontSys::getSysFont());
  setId(id);
}

///////////////////////////////////////////////////////

void ComboFrame::onMove(uint32 data)
{
  if (hWnd)
  {
    uint32 flags = SWP_NOREPOSITION;
    HWND hWndInsertAfter = NULL;
    if (visible())
    {
      if (IsWindowVisible(hWnd))
        flags |= SWP_NOZORDER;
      else
      {
        flags |= SWP_SHOWWINDOW;
        hWndInsertAfter = HWND_TOP;
      }
    }
    else
      flags |= SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW;
    if (data)
      DeferWindowPos((HDWP) data, hWnd, hWndInsertAfter, left(), top(), width(), boxHeight, flags);
    else
      SetWindowPos(hWnd, hWndInsertAfter, left(), top(), width(), boxHeight, flags);
  }
}
ComboFrame::ComboFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  boxHeight = 500;
  create("ComboBox", "", style | WS_CHILD | WS_TABSTOP, 0);
  setFont(FontSys::getSysFont());
  setId(id);
  setHeight(21);
}
int ComboFrame::addString(String text, int data)
{
  int id = SendMessage(hWnd, CB_ADDSTRING, 0, (uint32) text.c_str());
  if (id != CB_ERR)
    SendMessage(hWnd, CB_SETITEMDATA, id, data);
  return id;
}
int ComboFrame::getItemData(int item) const
{
  return SendMessage(hWnd, CB_GETITEMDATA, item, 0);
}
void ComboFrame::setItemData(int item, int data)
{
  SendMessage(hWnd, CB_SETITEMDATA, item, data);
}
int ComboFrame::getCurSel() const
{
  return SendMessage(hWnd, CB_GETCURSEL, 0, 0);
}
void ComboFrame::setCurSel(int sel)
{
  SendMessage(hWnd, CB_SETCURSEL, sel, 0);
}

///////////////////////////////////////////////////////

StaticFrame::StaticFrame(Frame* parent, int id, int style, int exStyle)
  : WindowFrame(parent)
{
  create("Static", "", style | SS_NOTIFY | WS_CHILD | WS_TABSTOP, exStyle);
  setFont(FontSys::getSysFont());
  setId(id);
}
StaticFrame::StaticFrame(String text, Frame* parent, int id, int style, int exStyle)
  : WindowFrame(parent)
{
  create("Static", text, style | SS_NOTIFY | WS_CHILD | WS_TABSTOP, exStyle);
  setFont(FontSys::getSysFont());
  resetSize();
  setId(id);
}
void StaticFrame::setImage(HANDLE image, int type)
{
  SendMessage(hWnd, STM_SETIMAGE, (WPARAM) type, (LPARAM) image);
}
void StaticFrame::resetSize()
{
  HDC hDC = GetDC(hWnd);
  SelectObject(hDC, getFont());
  SIZE sz;
  String text = getText();
  GetTextExtentPoint32(hDC, text, text.length(), &sz);
  ReleaseDC(hWnd, hDC);
  setSize(sz.cx, sz.cy);
}

///////////////////////////////////////////////////////

#include "richedit.h"

struct EditStreamCookie
{
  String str;
  int pos;
};
DWORD CALLBACK RichEditFrame::StreamCallback(DWORD_PTR cookie, LPBYTE buff, LONG cb, LONG* pcb)
{
  EditStreamCookie* ck = (EditStreamCookie*) cookie;
  *pcb = ck->str.length() - ck->pos;
  if (*pcb > cb)
    *pcb = cb;
  if (*pcb)
    memcpy(buff, ck->str.c_str() + ck->pos, *pcb);
  ck->pos += *pcb;
  return 0;
}
RichEditFrame::RichEditFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create(RICHEDIT_CLASS, "", style | WS_CHILD | WS_TABSTOP, WS_EX_CLIENTEDGE);
  setFont(FontSys::getSysFont());
  setId(id);
}
void RichEditFrame::setBackgroundColor(uint32 color)
{
  SendMessage(hWnd, EM_SETBKGNDCOLOR, 0, color);
}
void RichEditFrame::setRichText(String text)
{
  EditStreamCookie cookie;
  cookie.str = text;
  cookie.pos = 0;
  EDITSTREAM es;
  es.dwCookie = (DWORD_PTR) &cookie;
  es.dwError = 0;
  es.pfnCallback = StreamCallback;
  SendMessage(hWnd, EM_STREAMIN, SF_RTF, (uint32) &es);
}

/////////////////////////////////

SliderFrame::SliderFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create(TRACKBAR_CLASS, "", style | WS_CHILD | WS_TABSTOP, 0);
  setFont(FontSys::getSysFont());
  setId(id);
}

void SliderFrame::setPos(int pos)
{
  SendMessage(hWnd, TBM_SETPOS, TRUE, pos);
}
void SliderFrame::setRange(int minValue, int maxValue)
{
  SendMessage(hWnd, TBM_SETRANGEMIN, TRUE, minValue);
  SendMessage(hWnd, TBM_SETRANGEMAX, TRUE, maxValue);
}
void SliderFrame::setLineSize(int size)
{
  SendMessage(hWnd, TBM_SETLINESIZE, 0, size);
}
void SliderFrame::setPageSize(int size)
{
  SendMessage(hWnd, TBM_SETPAGESIZE, 0, size);
}
void SliderFrame::setTicFreq(int freq)
{
  SendMessage(hWnd, TBM_SETTICFREQ, freq, 0);
}
int SliderFrame::getPos()
{
  return SendMessage(hWnd, TBM_GETPOS, 0, 0);
}

/////////////////////////////////////////

UpDownFrame::UpDownFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create(UPDOWN_CLASS, "", WS_CHILD | style, 0);
  setId(id);
}

/////////////////////////////////////////

TabFrame::TabFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create(WC_TABCONTROL, "", WS_CHILD | style, 0);
  setFont(FontSys::getSysFont());
  setId(id);
}
Frame* TabFrame::addTab(int pos, String text, Frame* frame)
{
  if (pos >= tabs.length())
    tabs.resize(pos + 1, 0);
  if (frame == NULL)
    frame = new Frame(this);
  tabs[pos] = frame;

  TCITEM item;
  memset(&item, 0, sizeof item);
  item.mask = TCIF_TEXT;
  item.pszText = text.getBuffer();
  TabCtrl_InsertItem(hWnd, pos, &item);
  frame->show(pos == TabCtrl_GetCurSel(hWnd));

  RECT rc;
  GetClientRect(hWnd, &rc);
  int prevWidth = rc.right;
  int prevHeight = rc.bottom;
  TabCtrl_AdjustRect(hWnd, FALSE, &rc);
  frame->setPoint(PT_TOPLEFT, rc.left, rc.top);
  frame->setPoint(PT_BOTTOMRIGHT, rc.right - prevWidth, rc.bottom - prevHeight);

  return frame;
}

uint32 TabFrame::onMessage(uint32 message, uint32 wParam, uint32 lParam)
{
  if (message == WM_NOTIFY)
  {
    NMHDR* hdr = (NMHDR*) lParam;
    if (hdr->hwndFrom == hWnd && hdr->code == TCN_SELCHANGE)
    {
      int sel = TabCtrl_GetCurSel(hWnd);
      for (int i = 0; i < tabs.length(); i++)
        if (i != sel)
          tabs[i]->hide();
      tabs[sel]->show();
      return 0;
    }
  }
  return M_UNHANDLED;
}
void TabFrame::setCurSel(int sel)
{
  TabCtrl_SetCurSel(hWnd, sel);
  for (int i = 0; i < tabs.length(); i++)
    if (i != sel)
      tabs[i]->hide();
  tabs[sel]->show();
}

/////////////////////////////////////

uint32 ImageFrame::onMessage(uint32 message, uint32 wParam, uint32 lParam)
{
  if (message == WM_PAINT)
  {
    PAINTSTRUCT ps;
    HDC hPaintDC = BeginPaint(hWnd, &ps);
    if (hBitmap)
      BitBlt(hPaintDC, 0, 0, width(), height(), hDC, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
    return 0;
  }
  return M_UNHANDLED;
}
ImageFrame::ImageFrame(Frame* parent, Image* img)
  : WindowFrame(parent)
{
  create("", WS_CHILD, 0);

  hDC = CreateCompatibleDC(NULL);
  hBitmap = NULL;
  setImage(img);
}
ImageFrame::~ImageFrame()
{
  if (hBitmap)
    DeleteObject(hBitmap);
  DeleteDC(hDC);
}

void ImageFrame::setImage(Image* image)
{
  if (hBitmap)
    DeleteObject(hBitmap);
  hBitmap = NULL;
  if (image)
  {
    setSize(image->width(), image->height());
//    hBitmap = CreateCompatibleBitmap(hDC, image->width(), image->height());
//    image->fillBitmap(hBitmap, hDC);
    hBitmap = image->createBitmap(NULL);
    SelectObject(hDC, hBitmap);
  }
  invalidate();
}

////////////////////////////////////

TreeViewFrame::TreeViewFrame(Frame* parent, int id, int style)
  : WindowFrame(parent)
{
  create(WC_TREEVIEW, "", style | WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);
  setId(id);
  setFont(FontSys::getSysFont());
}
void TreeViewFrame::setItemText(HTREEITEM item, String text)
{
  TVITEM tvi;
  memset(&tvi, 0, sizeof tvi);
  tvi.hItem = item;
  tvi.mask = TVIF_TEXT;
  tvi.pszText = text.getBuffer();
  TreeView_SetItem(hWnd, &tvi);
}
