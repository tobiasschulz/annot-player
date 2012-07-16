// processview.cc
// 10/16/2011

#include "processview.h"
#include "tr.h"
#include "rc.h"
#include "global.h"
#include "logger.h"
#include "win/qtwin/qtwin.h"
#include "win/qtwinnt/qtwinnt.h"
#ifdef WITH_WIN_TEXTHOOK
# include "win/texthook/texthook.h"
#else
# error "TextHook is indispensible"
#endif // WITH_WIN_TEXTHOOK
#include "project/common/acfilteredtableview.h"
#include "project/common/acui.h"
#include <QtGui>
#include <boost/assign/list_of.hpp>
#include <set>

#define DEBUG "processview"
#include "module/debug/debug.h"

using namespace Logger;

// - Helpers -

namespace { namespace detail {

  bool processNameIsNotGalGame(const QString &procName)
  {
#define H(_cstr) qHash(QString(#_cstr))  // use std::set with qHash to improve search performance
    static const std::set<uint> blacklist = boost::assign::list_of
(H(annot-browser))(H(annot-down))(H(annot-player))
(H(Activator))(H(ApMsgFwd))(H(Apntex))(H(Apoint))(H(AppleMobileDeviceService))(H(APSDaemon))(H(armsvc))(H(AutoHotkey))(H(ApplePhotoStreams))
(H(BitComet))(H(BookmarkDAV_client))(H(BoonSutazio))(H(Bootcamp))(H(BtStackServer))(H(BTTray))(H(btwdins))
(H(CamtasiaStudio))(H(chrome))
(H(distnoted))(H(Dropbox))(H(DTLite))
(H(eclipse))(H(Evernote))(H(EvernoteTray))
(H(firefox))(H(foobar2000))
(H(GoogleIMEJaConverter))(H(GoogleIMEJaRenderer))(H(gvim))
(H(Hamana))(H(HidFind))
(H(iCloudServices))(H(IELowutil))(H(IEXPLOR))(H(iTunes))(H(iTunesHelper))
(H(java))(H(javaw))(H(jusched))
(H(KHALMNPR))(H(KMPlayer))
(H(MacDrive))(H(MacDrive8Service))(H(Maxthon))(H(mDNSResponder))(H(MouseGesture))(H(mpclaunch))(H(mspdbsrv))(H(mysql))(H(mysqld))
(H(netdrive))
(H(oacrmonitor))(H(ONENOTEM))(H(opera))
(H(php-cgi))(H(plugin-container))
(H(QQ))(H(qtcreator))
(H(SecureCRT))(H(SetPoint))(H(sidebar))(H(Skype))(H(SogouCloud))(H(SROSVC))(H(sttray))(H(Switcher))(H(SUService))
(H(thunderbird))(H(TSCHelper))(H(TXPlatform))
(H(VIPAppService))(H(volumouse))
(H(WINWORD))(H(wmpnetwk))
    ;
#undef H
    return blacklist.find(qHash(procName)) != blacklist.end();
  }

} } // anonymous detail

// - Constructions -

#define WINDOW_FLAGS ( \
  Qt::Dialog | \
  Qt::CustomizeWindowHint | \
  Qt::WindowTitleHint | \
  Qt::WindowSystemMenuHint | \
  Qt::WindowMinMaxButtonsHint | \
  Qt::WindowCloseButtonHint )

ProcessView::ProcessView(QWidget *parent)
  : Base(parent, WINDOW_FLAGS), contextMenu_(0)
{
  setWindowTitle(tr("Process view"));

  createModel();
  createLayout();
  //createActions();

  // Set initial states
  tableView_->sortByColumn(HD_Status, Qt::DescendingOrder);
  tableView_->setCurrentColumn(HD_Name);
}

void
ProcessView::createModel()
{
  sourceModel_ = new QStandardItemModel(0, HD_Count, this);
  setProcessHeaderData(sourceModel_);

  proxyModel_ = new QSortFilterProxyModel; {
    proxyModel_->setSourceModel(sourceModel_);
    proxyModel_->setDynamicSortFilter(true);
    proxyModel_->setSortCaseSensitivity(Qt::CaseInsensitive);
  }

  tableView_ = new AcFilteredTableView(sourceModel_, proxyModel_, this);
  connect(tableView_, SIGNAL(currentIndexChanged(QModelIndex)), SLOT(invalidateButtons()));
}

void
ProcessView::createLayout()
{
  AcUi *ui = AcUi::globalInstance();
  ui->setWindowStyle(this);

  attachButton_ = ui->makeToolButton(
        AcUi::PushHint | AcUi::HighlightHint, TR(T_ATTACH), TR(T_TOOLTIP_ATTACHPROCESS), this, SLOT(attachProcess()));
  detachButton_ = ui->makeToolButton(
        AcUi::PushHint, TR(T_DETACH), TR(T_TOOLTIP_DETACHPROCESS), this, SLOT(detachProcess()));
  QToolButton *refreshButton = ui->makeToolButton(
        AcUi::PushHint, TR(T_REFRESH), TR(T_TOOLTIP_REFRESHPROCESS), this, SLOT(refresh()));

  // Set layout

  QVBoxLayout *rows = new QVBoxLayout; {
    QHBoxLayout *header = new QHBoxLayout;
    header->addWidget(attachButton_);
    header->addWidget(detachButton_);
    header->addStretch();
    header->addWidget(refreshButton);

    rows->addLayout(header);
    rows->addWidget(tableView_);

    header->setContentsMargins(0, 0, 0, 0);
    rows->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
  } setLayout(rows);
}

void
ProcessView::setProcessHeaderData(QAbstractItemModel *model)
{
  Q_ASSERT(model);
  model->setHeaderData(HD_Name, Qt::Horizontal,TR(T_NAME));
  model->setHeaderData(HD_Pid, Qt::Horizontal, TR(T_PID));
  model->setHeaderData(HD_Status, Qt::Horizontal, TR(T_STATUS));
  model->setHeaderData(HD_Folder, Qt::Horizontal, TR(T_FOLDER));
  model->setHeaderData(HD_Path, Qt::Horizontal, TR(T_PATH));
}

void
ProcessView::createContextMenu()
{
  // Create menus
  contextMenu_ = new QMenu(windowTitle(), this);
  AcUi::globalInstance()->setContextMenuStyle(contextMenu_, true); // persistent = true

  attachAct_ = contextMenu_->addAction(TR(T_MENUTEXT_ATTACHPROCESS), this, SLOT(attachProcess()));
  detachAct_ = contextMenu_->addAction(TR(T_MENUTEXT_DETACHPROCESS), this, SLOT(detachProcess()));
  contextMenu_->addSeparator();
  refreshAct_ = contextMenu_->addAction(TR(T_MENUTEXT_REFRESHPROCESS), this, SLOT(refresh()));
}

// - Properties -

QModelIndex
ProcessView::currentIndex() const
{ return tableView_->currentIndex(); }

ulong
ProcessView::currentPid() const
{
  QModelIndex mi = currentIndex();
  if (!mi.isValid())
    return 0;
  mi = mi.sibling(mi.row(), HD_Pid);
  if (!mi.isValid())
    return 0;
  bool ok;
  ulong ret = mi.data().toUInt(&ok);
  if (!ok)
    return 0;
  return ret;
}

ulong
ProcessView::attachedPid() const
{
  QList<QStandardItem *> items =
      sourceModel_->findItems(TR(T_ATTACHED), Qt::MatchExactly, HD_Status);
  if (items.empty())
    return 0;

  QStandardItem *item = items.first();
  item = sourceModel_->item(item->row(), HD_Pid);
  if (!item)
    return 0;
  bool ok;
  ulong ret = item->data(Qt::EditRole).toUInt(&ok);
  if (!ok)
    return 0;
  return ret;
}

ProcessInfo
ProcessView::attachedProcessInfo() const
{ return pis_[attachedPid()]; }

// - Slots -

void
ProcessView::refresh()
{
  log(tr("refreshing process list ..."));
  invalidateSourceModel();
  invalidateButtons();
}

void
ProcessView::invalidateSourceModel(bool showAll)
{
  //using QtWin::ProcessInfo;
  static QString windir = QtWin::getWinDirPath();

  clear();

  foreach (const QtWin::ProcessInfo &wpi, QtWin::getProcessesInfo()) {
    ulong pid = wpi.id;
    QString name;
    QString folder;
    QString filePath;
    if (showAll) {
      name = wpi.name;
      filePath = QtWin::getProcessPathById(pid);
      if (!filePath.isEmpty())
        folder = QFileInfo(filePath).dir().dirName();
    } else {
      if (pid == QCoreApplication::applicationPid())
        continue;
      if (wpi.name.isEmpty())
        continue;

      filePath = QtWin::getProcessPathById(pid);
      if (filePath.isEmpty())
        continue;
      if (filePath.startsWith(windir, Qt::CaseInsensitive))
        continue;

      QFileInfo fi(filePath);
      if (!fi.exists())
        continue;

      name = fi.baseName();
      if (detail::processNameIsNotGalGame(name))
        continue;

      folder = fi.dir().dirName();
    }

    QString status;
    if (TextHook::globalInstance()->isProcessAttached(pid))
      status = TR(T_ATTACHED);

    // TODO: hidden PID: pi.pid save as row attribute but hidden
    ProcessInfo pi;
    pi.processId = pid;
    pi.processName = name;
    pi.executablePath = filePath;
    pis_[pid] = pi;

    sourceModel_->insertRow(0);
    sourceModel_->setData(sourceModel_->index(0, HD_Name), name);
    sourceModel_->setData(sourceModel_->index(0, HD_Pid), (quint32)pid); // quint32: potential problem when porting to 64bit machine
    sourceModel_->setData(sourceModel_->index(0, HD_Status), status);
    sourceModel_->setData(sourceModel_->index(0, HD_Folder), folder);
    sourceModel_->setData(sourceModel_->index(0, HD_Path), filePath);

    for (int i = 0; i < HD_Count; i++)
      sourceModel_->setData(sourceModel_->index(0, i), sourceModel_->headerData(i, Qt::Horizontal), Qt::ToolTipRole);
  }
}

void
ProcessView::clear()
{
  pis_.clear();
  tableView_->clear();
}

// - Slots -

void
ProcessView::invalidateButtons()
{
  ulong cur = currentPid();
  if (!cur) {
    attachButton_->setEnabled(false);
    detachButton_->setEnabled(false);
    return;
  }

  ulong a = attachedPid();

  attachButton_->setEnabled(!a);
  detachButton_->setEnabled(a && a == cur);
}

void
ProcessView::attachProcess()
{
  ulong pid = currentPid();
  if (pid && TextHook::globalInstance()->attachProcess(pid)) {
    setCurrentItemAttached(true);
    invalidateButtons();
    log(QString("%1 (pid = %2)").arg(tr("process attached")).arg(QString::number(pid)));
    emit attached(pis_[pid]);
  } else {
    error(tr("failed to attach process ") + QString(" (pid = %1)").arg(QString::number(pid)));
    if (!QtWin::isProcessActiveWithId(pid))
      notify(tr("Is the process running now?"));
    else if (!QtWinnt::isElevated())
      notify(tr("Run me as administrator and try again &gt;_&lt;"));
    else
      notify(tr("Restart the target process might help -_-"));
  }
}

void
ProcessView::detachProcess()
{
  ulong pid = currentPid();
  ProcessInfo pi;
  if (pid) {
    pi = pis_[pid];
    setCurrentItemAttached(false);
  }
  if (pid && TextHook::globalInstance()->detachProcess(pid)) {
    invalidateButtons();
    log(tr("process detached") + QString(" (pid = %1)").arg(QString::number(pid)));
  } else
    warn(tr("failed to detach process") + QString(" (pid = %1)").arg(QString::number(pid)));
  emit detached(pi);
}

// - Events -

//void ProcessView::dragEnterEvent(QDragEnterEvent *event)     { emit dragEnterEventReceived(event); }
//void ProcessView::dragMoveEvent(QDragMoveEvent *event)       { emit dragMoveEventReceived(event); }
//void ProcessView::dragLeaveEvent(QDragLeaveEvent *event)     { emit dragLeaveEventReceived(event); }
//void ProcessView::dropEvent(QDropEvent *event)               { emit dropEventReceived(event); }

void
ProcessView::setCurrentItemAttached(bool attached)
{
  QModelIndex mi = currentIndex();
  if (!mi.isValid())
    return;
  mi = mi.sibling(mi.row(), HD_Status);
  if (!mi.isValid())
    return;
  QString value;
  if (attached)
    value = TR(T_ATTACHED);

  proxyModel_->setData(mi, value);
}

void
ProcessView::setVisible(bool t)
{
  if (t)
    refresh();
  Base::setVisible(t);
}

void
ProcessView::contextMenuEvent(QContextMenuEvent *event)
{
  if (!contextMenu_)
    createContextMenu();
  bool v = currentIndex().isValid();
  attachAct_->setVisible(v && attachButton_->isEnabled());
  detachAct_->setVisible(v && detachButton_->isEnabled());

  contextMenu_->popup(event->globalPos());
  event->accept();
}

// EOF
