#ifndef ANNOTATIONSETTINGS_H
#define ANNOTATIONSETTINGS_H

// annotationsettings.h
// 5/24/2012

#include "src/common/acsettings.h"
#include <QtCore/QObject>
#include <QtGui/QColor>
#include <QtGui/QFont>

// QColor(r, g, b, a), sample color RGB palatte: http://www.tayloredmktg.com/rgb/
// Disable alpha for better performance.
//#define ANNOTATION_COLOR_DEFAULT    QColor(255, 250, 250) // Snow: 255,250,250
#define ANNOTATION_COLOR_DEFAULT    QColor(248, 248, 255) // Ghost white: 248,248,255

#define ANNOTATION_SIZE_DEFAULT         20
#define ANNOTATION_SIZE_MARGIN          2

#define ANNOTATION_SIZE_TINY            "12"
#define ANNOTATION_SIZE_SMALL           "18"
#define ANNOTATION_SIZE_NORMAL          "25"
#define ANNOTATION_SIZE_LARGE           "36"
#define ANNOTATION_SIZE_HUGE            "60"

#define ANNOTATION_JAPANESE_FONT_FAMILY "MS Gothic"

#define ANNOTATION_CHINESE_FONT_FAMILY  "YouYuan"

#define ANNOTATION_FONT_FAMILY  "Helvetica"

#define ANNOTATION_FONT_SIZE  18

#define ANNOTATION_SCALE    0.95
#define ANNOTATION_FULLSCREEN_SCALE 1.2

//#define ANNOTATION_OUTLINE_COLOR        QColor(50,100,100)
#define ANNOTATION_OUTLINE_COLOR        QColor(45,95,95)
#define ANNOTATION_OUTLINE_COLOR_HOVER  Qt::red
//#define ANNOTATION_OUTLINE_COLOR_SELF   Qt::darkYellow
#define ANNOTATION_OUTLINE_COLOR_SUB    QColor(170,0,127)

#define ANNOTATION_OPACITY_FACTOR   92
#define ANNOTATION_BACKGROUND_OPACITY_FACTOR    10

#define ANNOTATION_POSITION_RESOLUTION  0

class AnnotationSettings : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(AnnotationSettings)
  typedef AnnotationSettings Self;
  typedef QObject Base;

  qreal scale_,
        fullscreenScale_;
  int opacityFactor_,
      backgroundOpacityFactor_;
  qreal rotation_;
  qreal speedup_;
  int offset_;
  int positionResolution_;
  QFont font_,
        japaneseFont_,
        chineseFont_;

  bool avatarVisible_;
  bool metaVisible_;
  bool preferFloat_;

  bool subtitleOnTop_;

  bool traditionalChinese_;

  QColor outlineColor_,
         subtitleColor_,
         highlightColor_;

public:
  static Self *globalSettings() { static Self g; return &g; }
protected:
  explicit AnnotationSettings(QObject *parent = nullptr)
    : Base(parent), scale_(ANNOTATION_SCALE), fullscreenScale_(ANNOTATION_FULLSCREEN_SCALE),
      opacityFactor_(ANNOTATION_OPACITY_FACTOR), backgroundOpacityFactor_(ANNOTATION_BACKGROUND_OPACITY_FACTOR),
      rotation_(0), speedup_(1.0), offset_(0), positionResolution_(ANNOTATION_POSITION_RESOLUTION),
      avatarVisible_(false), preferFloat_(true), subtitleOnTop_(false), traditionalChinese_(false)
  {
    resetOutlineColor();
    resetSubtitleColor();
    resetHighlightColor();
    resetFont();
    resetJapaneseFont();
    resetChineseFont();
  }

signals:
  void scaleChanged(qreal value);
  void fullscreenScaleChanged(qreal value);
  void opacityFactorChanged(int value);
  void backgroundOpacityFactorChanged(int value);
  void speedupChanged(qreal value);
  void positionResolutionChanged(int value);
  void subtitleOnTopChanged(bool value);
  void rotationChanged(qreal value);
  void offsetChanged(int value);
  void avatarVisibleChanged(bool value);
  void metaVisibleChanged(bool value);
  void preferFloatChanged(bool value);
  void outlineColorChanged(QColor color);
  void highlightColorChanged(QColor color);
  void subtitleColorChanged(QColor color);
  void preferTraditionalChineseChange(bool t);
  //void fontChanged(const QFont &value);
public:
  qreal scale() const { return scale_; }
  qreal fullscreenScale() const { return fullscreenScale_; }
  int opacityFactor() const { return opacityFactor_; }
  qreal opacity() const { return opacityFactor_ / 100.0; }
  int backgroundOpacityFactor() const { return backgroundOpacityFactor_; }
  qreal backgroundOpacity() const { return backgroundOpacityFactor_ / 100.0; }
  qreal rotation() const { return rotation_; }
  qreal speedup() const { return speedup_; }
  int offset() const { return offset_; }
  int positionResolution() const { return positionResolution_; }
  bool isAvatarVisible() const { return avatarVisible_; }
  bool isMetaVisible() const { return metaVisible_; }

  QFont &font() { return font_; }
  QFont font() const { return font_; }
  QString fontFamily() const { return font_.family(); }

  QFont &japaneseFont() { return japaneseFont_; }
  QFont japaneseFont() const { return japaneseFont_; }
  QString japaneseFontFamily() const { return japaneseFont_.family(); }

  QFont &chineseFont() { return chineseFont_; }
  QFont chineseFont() const { return chineseFont_; }
  QString chineseFontFamily() const { return chineseFont_.family(); }

  const QColor &outlineColor() const { return outlineColor_; }
  const QColor &subtitleColor() const { return subtitleColor_; }
  const QColor &highlightColor() const { return highlightColor_; }

  bool isSubtitleOnTop() const { return subtitleOnTop_; }

  bool preferFloat() const { return preferFloat_; }

  bool preferTraditionalChinese() const { return traditionalChinese_; }

public slots:
  void setScale(qreal value) { if (!qFuzzyCompare(scale_, value)) emit scaleChanged(scale_ = value); }
  void resetScale() { setScale(ANNOTATION_SCALE); }

  void setFullscreenScale(qreal value) { if (!qFuzzyCompare(fullscreenScale_, value)) emit fullscreenScaleChanged(fullscreenScale_ = value); }
  void resetFullscreenScale() { setFullscreenScale(ANNOTATION_FULLSCREEN_SCALE); }

  void setOpacityFactor(int value) { if (opacityFactor_ != value) emit opacityFactorChanged(opacityFactor_ = value); }
  void resetOpacityFactor() { setOpacityFactor(ANNOTATION_OPACITY_FACTOR); }

  void setBackgroundOpacityFactor(int value) { if (backgroundOpacityFactor_ != value) emit backgroundOpacityFactorChanged(backgroundOpacityFactor_ = value); }
  void resetBackgroundOpacityFactor() { setBackgroundOpacityFactor(ANNOTATION_BACKGROUND_OPACITY_FACTOR); }

  void setRotation(qreal value) { if (!qFuzzyCompare(rotation_+1, value+1)) emit rotationChanged(rotation_ = value); }
  void resetRotation() { setRotation(0); }

  void setSpeedup(qreal value) { if (!qFuzzyCompare(speedup_, value)) emit speedupChanged(speedup_ = value); }
  void resetSpeedup() { setSpeedup(1.0); }

  void setOffset(int value) { if (offset_ != value) emit offsetChanged(offset_ = value); }
  void resetOffset() { setOffset(0); }

  void setPositionResolution(int value) { if (positionResolution_ != value) emit positionResolutionChanged(positionResolution_ = value); }
  void resetPositionResolution() { setPositionResolution(ANNOTATION_POSITION_RESOLUTION); }

  void setSubtitleOnTop(bool value) { if (subtitleOnTop_ != value) emit subtitleOnTopChanged(subtitleOnTop_ = value); }

  void setPreferTraditionalChinese(bool t = true)
  {
    if (traditionalChinese_ != t)
      emit preferTraditionalChineseChange(traditionalChinese_ = t);
  }

  void setOutlineColor(const QColor &c)
  {
    if (c.isValid()) {
      if (outlineColor_ != c)
        emit outlineColorChanged(outlineColor_ = c);
    } else if (outlineColor_ != ANNOTATION_OUTLINE_COLOR)
      emit outlineColorChanged(outlineColor_ = ANNOTATION_OUTLINE_COLOR);
  }

  void resetOutlineColor() { setOutlineColor(QColor()); }

  void setSubtitleColor(const QColor &c)
  {
    if (c.isValid()) {
      if (subtitleColor_ != c)
        emit subtitleColorChanged(subtitleColor_ = c);
    } else if (subtitleColor_ != ANNOTATION_OUTLINE_COLOR_SUB)
      emit subtitleColorChanged(subtitleColor_ = ANNOTATION_OUTLINE_COLOR_SUB);
  }

  void resetSubtitleColor() { setSubtitleColor(QColor()); }

  void setHighlightColor(const QColor &c)
  {
    if (c.isValid()) {
      if (highlightColor_ != c)
        emit highlightColorChanged(highlightColor_ = c);
    } else if (highlightColor_ != ANNOTATION_OUTLINE_COLOR_HOVER)
      emit highlightColorChanged(highlightColor_ = ANNOTATION_OUTLINE_COLOR_HOVER);
  }

  void resetHighlightColor() { setHighlightColor(QColor()); }

  void setFont(const QFont &value) { font_ = value; }
  void setJapaneseFont(const QFont &value) { japaneseFont_ = value; }
  void setChineseFont(const QFont &value) { chineseFont_ = value; }

  void setAvatarVisible(bool value)
  { if (avatarVisible_ != value) emit avatarVisibleChanged(avatarVisible_ = value); }

  void setMetaVisible(bool value)
  { if (metaVisible_ != value) emit metaVisibleChanged(metaVisible_ = value); }

  void setPreferFloat(bool value)
  { if (preferFloat_ != value) emit preferFloatChanged(preferFloat_ = value);}

  //void resetFontFamily() { resetFont(); }

  void setFontFamily(const QString &family)
  {
    QString f = family.isEmpty() ? defaultFont().family() : family;
    if (f.compare(f, font_.family(), Qt::CaseInsensitive))
      setFont(makeFont(f));
  }

  void setJapaneseFontFamily(const QString &family)
  {
    QString f = family.isEmpty() ? defaultJapaneseFont().family() : family;
    if (f.compare(f, font_.family(), Qt::CaseInsensitive))
      setJapaneseFont(makeFont(f));
  }

  void setChineseFontFamily(const QString &family)
  {
    QString f = family.isEmpty() ? defaultChineseFont().family() : family;
    if (f.compare(f, font_.family(), Qt::CaseInsensitive))
      setChineseFont(makeFont(f));
  }

  void resetFont()
  { setFont(defaultFont()); }

  void resetJapaneseFont()
  { setFont(defaultJapaneseFont()); }

  void resetChineseFont()
  { setFont(defaultChineseFont()); }

public:
  static const QFont &defaultFont()
  {
    static QFont ret = makeFont(ANNOTATION_FONT_FAMILY);
    return ret;
  }

  static const QFont &defaultJapaneseFont()
  {
    static QFont ret = makeFont(ANNOTATION_JAPANESE_FONT_FAMILY);
    return ret;
  }

  static const QFont &defaultChineseFont()
  {
    static QFont ret = makeFont(ANNOTATION_CHINESE_FONT_FAMILY);
    return ret;
  }

protected:
  static QFont makeFont(const QString &family)
  {
    QFont ret(family, ANNOTATION_FONT_SIZE);
    ret.setWeight(QFont::DemiBold);
    ret.setStyleStrategy((QFont::StyleStrategy)(
      QFont::PreferAntialias | QFont::PreferQuality
    ));
    return ret;
  }
};

#endif // ANNOTATIONSETTINGS_H
