#ifndef ANNOTATIONCODEC_H
#define ANNOTATIONCODEC_H

// annotationcodec.h
// 2/3/2012

#include "module/annotcloud/annotation.h"

class AnnotationCodec : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(AnnotationCodec)
  typedef AnnotationCodec Self;
  typedef QObject Base;

  typedef AnnotCloud::Annotation Annotation;
  typedef AnnotCloud::AnnotationList AnnotationList;

public:
  explicit AnnotationCodec(QObject *parent = 0)
    : Base(parent) { }

signals:
  void error(QString msg);
  void message(QString msg);

  void fetched(AnnotationList annots, QString url, QString originalUrl);

public:
  virtual bool match(const QString &url) const = 0;
  virtual void fetch(const QString &url, const QString &originalUrl) = 0;

  // - Helpers -
protected:
  static QByteArray skipLeadingCommentAndNewLine(const QByteArray &data, const char *start, const char *stop);

  static QByteArray skipXmlLeadingComment(const QByteArray &data)
  { return skipLeadingCommentAndNewLine(data, "<!--", "-->"); }

  static QByteArray skipJsonLeadingComment(const QByteArray &data)
  { return skipLeadingCommentAndNewLine(data, "/*", "*/"); }
};

#endif // ANNOTATIONCODEC_H
