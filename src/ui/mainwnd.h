#ifndef __UI_MAINWND_H__
#define __UI_MAINWND_H__

#include "frameui/framewnd.h"

#include "ui/replaytree.h"
#include "ui/viewitem.h"

#define MAINWND_SETTINGS      0
#define MAINWND_REPLAY        1
#define MAINWND_FOLDER        2
#define MAINWND_NUM_VIEWS     3

class MainWnd : public FrameWindow
{
  ReplayTree* replayTree;

  ExtWindowFrame* views[MAINWND_NUM_VIEWS];
  ViewItem* history;

  // splitter
  int dragPos;
  HCURSOR sizeCursor;

  uint32 onMessage(uint32 message, uint32 wParam, uint32 lParam);
public:
  MainWnd();
  ~MainWnd();

  Window* setView(int view);
  void pushView(ViewItem* item);

  void postLoad();
};

#endif // __UI_MAINWND_H__
