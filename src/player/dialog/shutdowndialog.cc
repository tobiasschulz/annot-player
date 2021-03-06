// shutdowndialog.cc
// 2/13/2012

#include "shutdowndialog.h"
#include "tr.h"
#include "htmlutil/htmltags.h"
#ifdef Q_OS_WIN
# include "win/qtwin/qtwin.h"
#elif defined(Q_OS_MAC)
# include "mac/qtmac/qtmac.h"
#else
# include "unix/qtunix/qtunix.h"
#endif // Q_OS_
#include <QtGui>

#define DEBUG "shutdowndialog"
#include "qtx/qxdebug.h"

ShutdownDialog::ShutdownDialog(QWidget *parent)
  : Base(parent)
{
  setWindowTitle(tr("Shutdown?"));
  setText(tr("Shutdown the computer after %1 seconds ...")
    .arg(HTML_SS_OPEN(color:red) "%1" HTML_SS_CLOSE())
  );
  QApplication::beep();
}

void
ShutdownDialog::run()
{
  DOUT("enter");
  QApplication::beep();
  hide();
  emit warning(tr("shutting down the computer ..."));
  bool ok = false;
#ifdef Q_OS_WIN
  ok = QtWin::halt();
#elif defined(Q_OS_MAC)
  ok = QtMac::halt();
#else
  ok = QtUnix::halt();
#endif // Q_OS_
  if (!ok)
    emit warning(tr("failed to shut down the computer"));
  DOUT("exit");
}

// EOF
