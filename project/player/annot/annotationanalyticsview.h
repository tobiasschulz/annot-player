#ifndef ANNOTATIONANALYTICSVIEW_H
#define ANNOTATIONANALYTICSVIEW_H

// annotationanalyticsview.h
// 11/16/2011

#include "project/common/acwebwindow.h"
#include "module/annotcloud/annotation.h"

QT_FORWARD_DECLARE_CLASS(QUrl)

class DataManager;
class SignalHub;

class AnnotationAnalyticsView: public AcWebWindow
{
  Q_OBJECT
  Q_DISABLE_COPY(AnnotationAnalyticsView)
  typedef AnnotationAnalyticsView Self;
  typedef AcWebWindow Base;

  typedef AnnotCloud::AnnotationList AnnotationList;

  // - Constructions -
public:
  AnnotationAnalyticsView(DataManager *data, SignalHub *hub, QWidget *parent = 0);

signals:
  void contentChanged(const QString &html);
  void windowTitleChanged(const QString &title);
  void urlChanged(const QUrl &url);
  void visibleChanged(bool visible);

public slots:
  void refresh();

  // - Events -
public:
  virtual void setVisible(bool visible); ///< \reimp

protected slots:
  void setContent(const QString &html);
  void setUrl(const QUrl &url);

  // - Implementations
public slots:
  void invalidateAnnotations(bool async = true);
private:
  void setupActions();

private:
  DataManager *data_;
  SignalHub *hub_;
  bool refreshing_;
};

#endif // ANNOTATIONANALYTICSVIEW_H
