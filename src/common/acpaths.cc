// ac/paths.cc
// 4/23/2012

#include "src/common/acpaths.h"
#include "src/common/acsettings.h"
#ifdef Q_OS_MAC
# include "mac/qtmac/qtmac.h"
# include "unix/qtunix/qtunix.h"
#endif // Q_OS_MAC
#ifdef Q_OS_WIN
# include "win/qtwin/qtwin.h"
#endif // Q_OS_WIN
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

#define DEBUG "acpath"
#include "qtx/qxdebug.h"

#ifdef Q_OS_MAC
# define REZ_ICON_VIDEO  QCoreApplication::applicationDirPath() + "/../Resources/" "video.IconRez"
#endif // Q_OS_MAC

// - Constructions -

void
AcLocationManager::init()
{ connect(this, SIGNAL(downloadsLocationChanged(QString)), SLOT(createDownloadsLocation())); }

AcSettings*
AcLocationManager::settings() const
{
  if (!settings_) {
    settings_ = AcSettings::globalSettings();
    connect(settings_, SIGNAL(downloadsLocationChanged(QString)), SIGNAL(downloadsLocationChanged(QString)));
  }
  return settings_;
}

// - Downloads -

QString
AcLocationManager::downloadsLocation() const
{
  QString ret = settings()->downloadsLocation();
  if (ret.isEmpty())
    ret = defaultDownloadsLocation();
  return ret;
}

QString
AcLocationManager::defaultDownloadsLocation() const
{
  static QString ret;
  if (ret.isEmpty()) {
    QString desktop = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    ret = desktop + QDir::separator() + tr("Video");
  }
  return ret;
}

void
AcLocationManager::openDownloadsLocation()
{
  QString url = downloadsLocation();
  if (QFile::exists(url)) {
#ifdef Q_OS_WIN
    url = QDir::fromNativeSeparators(url);
    url.prepend('/');
#endif // Q_OS_WIN
    url.prepend("file://");
    QDesktopServices::openUrl(QUrl(url));
  }
}

void
AcLocationManager::createDownloadsLocation()
{
  QString path =  downloadsLocation();

#ifdef Q_OS_MAC
  QString targetIcon = path + "/" "Icon\r";
  if (QFile::exists(targetIcon))
    return;

  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(dir.absolutePath());
  if (!dir.exists()) {
    DOUT("warning: failed to create downloads folder on desktop");
    return;
  }

  QString srcIcon = REZ_ICON_VIDEO;
  bool ok = QFile::exists(srcIcon) &&
    QtUnix::cp(srcIcon, targetIcon) &&
    QtMac::setFileAttributes(path, QtMac::FA_CustomIcon);
  Q_UNUSED(ok)
  DOUT("ok =" << ok);

#elif defined(Q_OS_WIN)
  QString targetIni = path + "/" "desktop.ini";
  if (QFile::exists(targetIni))
    return;

  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(dir.absolutePath());
  if (!dir.exists()) {
    DOUT("warning: failed to create downloads folder on desktop");
    return;
  }

  QString srcIni = QtWin::getVideoPath() + "/" "desktop.ini";
  bool ok = QFile::exists(srcIni) &&
    QFile::copy(srcIni, targetIni) &&
    QtWin::setFileAttributes(targetIni, QtWin::SystemAttribute | QtWin::HiddenAttribute | QtWin::ArchiveAttribute) &&
    QtWin::setFileAttributes(path, QtWin::ReadOnlyAttribute);
  Q_UNUSED(ok)
  DOUT("ok =" << ok);

#else
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(dir.absolutePath());
  if (!dir.exists())
    DOUT("warning: failed to create downloads folder on desktop");
#endif // Q_OS_
}

// EOF
