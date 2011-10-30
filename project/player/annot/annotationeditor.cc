// annotationeditor.cc
// 10/4/2011

#include "annotationeditor.h"
#include "textedit.h"
#include "uistyle.h"
#include "tr.h"
#include "global.h"
#include "stylesheet.h"
#include "core/cmd.h"
#include "core/annotationparser.h"
#include "core/gui/toolbutton.h"
#include "core/gui/combobox.h"
#include "core/gui/fontcombobox.h"
#include <boost/tuple/tuple.hpp>
#include <QtGui>

#define DEBUG "AnnotationEditor"
#include "module/debug/debug.h"

#define WINDOW_FLAGS ( \
  Qt::Dialog | \
  Qt::CustomizeWindowHint | \
  Qt::WindowTitleHint | \
  Qt::WindowSystemMenuHint | \
  Qt::WindowMinMaxButtonsHint | \
  Qt::WindowCloseButtonHint )

#define FONTCOMBOBOX_MAXWIDTH           100
#define FONTSIZECOMBOBOX_MAXWIDTH       40
#define ALIGNCOMBOBOX_MAXWIDTH          60
#define MOVESTYLECOMBOBOX_MAXWIDTH      60

// - Constructions -

AnnotationEditor::AnnotationEditor(QWidget *parent)
  : Base(parent, WINDOW_FLAGS), id_(0)
{
  setWindowTitle(TR(T_TITLE_ANNOTATIONEDITOR));
  UiStyle::globalInstance()->setWindowStyle(this);

  textEdit_ = new TextEdit;
  textEdit_->setStyleSheet(SS_TEXTEDIT);

  createActions();
  createRibons();
  createLayout();

  connect(textEdit_, SIGNAL(cursorPositionChanged()), SLOT(invalidateAlignment()));
  connect(textEdit_, SIGNAL(currentCharFormatChanged(QTextCharFormat)), SLOT(setFormat(QTextCharFormat)));
  connect(textEdit_, SIGNAL(currentCharFormatChanged(QTextCharFormat)), SLOT(invalidateCount()));
  connect(textEdit_, SIGNAL(textChanged()), SLOT(invalidateCount()));

  // Start up status
  setFontType(QApplication::font());
  setColorIconColor(QColor("black"));
  setBackgroundColorIconColor(QColor("black"));

  setCodeMode();
  invalidateCount();

  textEdit_->setFocus();
}

void
AnnotationEditor::createRibons()
{
#define MAKE_RIBON_BUTTON(_button, _tr, _slot) \
  _button = new Core::Gui::ToolButton; { \
    _button->setStyleSheet(SS_TOOLBUTTON_TEXT); \
    _button->setToolButtonStyle(Qt::ToolButtonTextOnly); \
    _button->setText(QString("- %1 -").arg(_tr)); \
    _button->setCheckable(true); \
    connect(_button, SIGNAL(clicked()), _slot); \
  }

#define MAKE_UNCHECKABLE_BUTTON(_button, _title, _tip, _slot) \
    _button = new Core::Gui::ToolButton; { \
      _button->setStyleSheet(SS_TOOLBUTTON_TEXT); \
      _button->setToolButtonStyle(Qt::ToolButtonTextOnly); \
      _button->setText(_title); \
      _button->setToolTip(_tip); \
      connect(_button, SIGNAL(clicked()), _slot); \
    }
#define MAKE_CHECKABLE_BUTTON(_button, _title, _tip, _slot) \
    _button = new Core::Gui::ToolButton; { \
      _button->setStyleSheet(SS_TOOLBUTTON_TEXT); \
      _button->setToolButtonStyle(Qt::ToolButtonTextOnly); \
      _button->setText(_title); \
      _button->setToolTip(_tip); \
      _button->setCheckable(true); \
      connect(_button, SIGNAL(clicked(bool)), _slot); \
    }

  // Code ribon

  // - verbatimButton_
  MAKE_UNCHECKABLE_BUTTON(verbatimButton_, tr("verbatim"), TR(T_VERBATIM), SLOT(setVerbatim()))

  // - subtitleButton_
  MAKE_UNCHECKABLE_BUTTON(subtitleButton_, tr("subtitle"), TR(T_SUBTITLE), SLOT(setSubtitle()))

  // - moveStyleComboBox_
  moveStyleComboBox_ = new Core::Gui::ComboBox; {
    UiStyle::globalInstance()->setComboBoxStyle(moveStyleComboBox_);
    moveStyleComboBox_->setEditable(true);
    moveStyleComboBox_->setMaximumWidth(MOVESTYLECOMBOBOX_MAXWIDTH);
    moveStyleComboBox_->setToolTip(tr("Style tag"));

    // Must be consistent with MoveStyleIndex
    moveStyleComboBox_->addItem(tr("fly"));
    moveStyleComboBox_->addItem(tr("top"));
    moveStyleComboBox_->addItem(tr("bottom"));
  }
  connect(moveStyleComboBox_, SIGNAL(activated(int)), SLOT(setMoveStyle(int)));

  // - Code ribon
  QGroupBox *codeRibon = new QGroupBox; {
    QVBoxLayout *rows = new QVBoxLayout;
    QHBoxLayout *row0 = new QHBoxLayout,
                *row1 = new QHBoxLayout;
    rows->addLayout(row0);
    rows->addLayout(row1);
    codeRibon->setLayout(rows);

    row0->addWidget(verbatimButton_);
    row0->addWidget(subtitleButton_);
    row0->addStretch();

    row1->addWidget(moveStyleComboBox_);
    row1->addStretch();

    rows->setContentsMargins(0, 0, 0, 0);
    row0->setContentsMargins(0, 0, 0, 0);
    row1->setContentsMargins(0, 0, 0, 0);
    codeRibon->setContentsMargins(0, 0, 0, 0);
  }

  // Html ribon

  // - boldButton_
  MAKE_CHECKABLE_BUTTON(boldButton_, tr("B"), TR(T_BOLD), SLOT(setBold(bool))) {
    QFont font = boldButton_->font();
    font.setBold(true);
    boldButton_->setFont(font);
  }

  // - italicButton_
  MAKE_CHECKABLE_BUTTON(italicButton_, tr("I"), TR(T_ITALIC), SLOT(setItalic(bool))) {
    QFont font = italicButton_->font();
    font.setItalic(true);
    italicButton_->setFont(font);
  }

  // - underlineButton_
  MAKE_CHECKABLE_BUTTON(underlineButton_, tr("U"), TR(T_UNDERLINE), SLOT(setUnderline(bool))) {
    QFont font = underlineButton_->font();
    font.setUnderline(true);
    underlineButton_->setFont(font);
  }

  // - strikeOutButton_
  MAKE_CHECKABLE_BUTTON(strikeOutButton_, tr("S"), TR(T_STRIKEOUT), SLOT(setStrikeOut(bool))) {
    QFont font = strikeOutButton_->font();
    font.setStrikeOut(true);
    strikeOutButton_->setFont(font);
  }

  // - colorButton_
  colorButton_ = new Core::Gui::ToolButton; {
    colorButton_->setStyleSheet(SS_TRANSPARENT);
    colorButton_->setToolTip(TR(T_FOREGROUNDCOLOR));
  }
  connect(colorButton_, SIGNAL(clicked()), SLOT(showColorDialog()));

  // - backgroundColorButton_
  backgroundColorButton_ = new Core::Gui::ToolButton; {
    backgroundColorButton_->setStyleSheet(SS_TRANSPARENT);
    backgroundColorButton_->setToolTip(TR(T_BACKGROUNDCOLOR));
  }
  connect(backgroundColorButton_, SIGNAL(clicked()), SLOT(showBackgroundColorDialog()));

  // - alignComboBox_
  alignComboBox_ = new Core::Gui::ComboBox; {
    UiStyle::globalInstance()->setComboBoxStyle(alignComboBox_);
    alignComboBox_->setEditable(false);
    alignComboBox_->setMaximumWidth(ALIGNCOMBOBOX_MAXWIDTH);
    alignComboBox_->setToolTip(tr("Alignment"));

    // Must be consisitent with AlignIndex
    alignComboBox_->addItem(tr("Left"));
    alignComboBox_->addItem(tr("Right"));
    alignComboBox_->addItem(tr("Center"));
    alignComboBox_->addItem(tr("Justify"));
  }
  connect(alignComboBox_, SIGNAL(activated(int)), SLOT(setAlignment(int)));

  // - fontComboBox_
  fontComboBox_ = new Core::Gui::FontComboBox; {
    UiStyle::globalInstance()->setComboBoxStyle(fontComboBox_);
    fontComboBox_->setEditable(true);
    fontComboBox_->setMaximumWidth(FONTCOMBOBOX_MAXWIDTH);
    fontComboBox_->setToolTip(tr("Font family"));
  }
  connect(fontComboBox_, SIGNAL(activated(QString)), SLOT(setFontFamily(QString)));

  // - sizeComboBox_
  fontSizeComboBox_ = new Core::Gui::ComboBox; {
    UiStyle::globalInstance()->setComboBoxStyle(fontSizeComboBox_);
    fontSizeComboBox_->setEditable(true);
    fontSizeComboBox_->setMaximumWidth(FONTSIZECOMBOBOX_MAXWIDTH);
    fontSizeComboBox_->setToolTip(tr("Font size"));

    QFontDatabase db;
    foreach(int size, db.standardSizes())
      fontSizeComboBox_->addItem(QString::number(size));
  }
  connect(fontSizeComboBox_, SIGNAL(activated(QString)), SLOT(setFontSize(QString)));

  // - Html ribon
  QGroupBox *htmlRibon = new QGroupBox; {
    QVBoxLayout *rows = new QVBoxLayout;
    QHBoxLayout *row0 = new QHBoxLayout,
                *row1 = new QHBoxLayout;
    rows->addLayout(row0);
    rows->addLayout(row1);
    htmlRibon->setLayout(rows);

    row0->addWidget(boldButton_);
    row0->addWidget(italicButton_);
    row0->addWidget(underlineButton_);
    row0->addWidget(strikeOutButton_);
    row0->addWidget(colorButton_);
    row0->addWidget(backgroundColorButton_);
    row0->addStretch();

    row1->addWidget(fontComboBox_);
    row1->addWidget(fontSizeComboBox_);
    row1->addWidget(alignComboBox_);
    row1->addStretch();

    rows->setContentsMargins(0, 0, 0, 0);
    row0->setContentsMargins(0, 0, 0, 0);
    row1->setContentsMargins(0, 0, 0, 0);
    htmlRibon->setContentsMargins(0, 0, 0, 0);
  }

  // Ribon layout

  ribonStackLayout_ = new QStackedLayout; {
    // Must be consistent with RibonIndex
    ribonStackLayout_->addWidget(codeRibon);
    ribonStackLayout_->addWidget(htmlRibon);
    ribonStackLayout_->setContentsMargins(0, 0, 0, 0);
  }

  // Header

  MAKE_RIBON_BUTTON(codeRibonButton_, tr("code"), SLOT(setCodeMode()))
  MAKE_RIBON_BUTTON(htmlRibonButton_, tr("html"), SLOT(setHtmlMode()))

  formatButton_ = new Core::Gui::ToolButton; {
    formatButton_->setStyleSheet(SS_TOOLBUTTON_TEXT);
    formatButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    formatButton_->setText(QString("%1 ").arg(tr("format")));
    formatButton_->setToolTip(tr("Format"));

    QMenu *menu = new QMenu; {
      UiStyle::globalInstance()->setMenuStyle(menu);

      // - Styles
      menu->addAction(boldAct_);
      menu->addAction(italicAct_);
      menu->addAction(underlineAct_);
      menu->addAction(strikeOutAct_);

      // - Colors
      menu->addSeparator();
      menu->addAction(colorAct_);
      menu->addAction(backgroundColorAct_);

      #define ADD_COLOR(_Color, _COLOR) \
        QAction *_Color = new QAction(this); \
        _Color->setText(TR(T_##_COLOR)); \
        _Color->setToolTip(TR(T_##_COLOR)); \
        connect(_Color, SIGNAL(triggered()), SLOT(set##_Color##Color())); \
        menu->addAction(_Color);

        ADD_COLOR(Black, BLACK)
        ADD_COLOR(Blue, BLUE)
        ADD_COLOR(Brown, BROWN)
        ADD_COLOR(Cyan, CYAN)
        ADD_COLOR(Gray, GRAY)
        ADD_COLOR(Magenta, MAGENTA)
        ADD_COLOR(Orange, ORANGE)
        ADD_COLOR(Pink, PINK)
        ADD_COLOR(Red, RED)
        ADD_COLOR(White, WHITE)
        ADD_COLOR(Yellow, YELLOW)
      #undef ADD_COLOR
    }

    formatButton_->setMenu(menu);
    connect(formatButton_, SIGNAL(clicked()), formatButton_, SLOT(showMenu()));
  }

  // Footer

  countLabel_ = new QLabel; {
    countLabel_->setStyleSheet(SS_LABEL);
    countLabel_->setToolTip(TR(T_WORDCOUNT));
  }

  tidyButton_ = new Core::Gui::ToolButton; {
    tidyButton_->setStyleSheet(SS_TOOLBUTTON_TEXT);
    tidyButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tidyButton_->setText(tr("tidy"));
    tidyButton_->setToolTip(tr("Tidy HTML"));
    tidyButton_->setCheckable(true);
    tidyButton_->setChecked(true);
  }
  connect(tidyButton_, SIGNAL(clicked(bool)),SLOT(setTidyEnabled(bool)));

  saveButton_ = new Core::Gui::ToolButton; {
    saveButton_->setStyleSheet(SS_TOOLBUTTON_TEXT);
    saveButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    saveButton_->setText(QString("[ %1 ]").arg(TR(T_SAVE)));
    saveButton_->setToolTip(TR(T_SAVE));
  }
  connect(saveButton_, SIGNAL(clicked()), SLOT(save()));

  cancelButton_ = new Core::Gui::ToolButton; {
    cancelButton_->setStyleSheet(SS_TOOLBUTTON_TEXT);
    cancelButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    cancelButton_->setText(QString("[ %1 ]").arg(TR(T_CANCEL)));
    cancelButton_->setToolTip(TR(T_CANCEL));
  }
  connect(cancelButton_, SIGNAL(clicked()), SLOT(cancel()));


#undef MAKE_RIBON_BUTTON
#undef MAKE_UNCHECKABLE_BUTTON
#undef MAKE_CHECKABLE_BUTTON
}

void
AnnotationEditor::createActions()
{
#define MAKE_ACTION(_action, _tr, _slot) \
  _action = new QAction(this); \
  _action->setText(_tr); \
  _action->setStatusTip(_tr); \
  connect(_action, SIGNAL(triggered()), _slot);

#define MAKE_TOGGLE(_action, _tr, _slot) \
  _action = new QAction(this); \
  _action->setText(_tr); \
  _action->setStatusTip(_tr); \
  _action->setCheckable(true); \
  connect(_action, SIGNAL(triggered(bool)), _slot);

  MAKE_TOGGLE(boldAct_, TR(T_BOLD), SLOT(setBold(bool)))
  MAKE_TOGGLE(italicAct_, TR(T_ITALIC), SLOT(setItalic(bool)))
  MAKE_TOGGLE(underlineAct_, TR(T_UNDERLINE), SLOT(setUnderline(bool)))
  MAKE_TOGGLE(strikeOutAct_, TR(T_STRIKEOUT), SLOT(setStrikeOut(bool)))

  MAKE_ACTION(colorAct_, TR(T_FOREGROUNDCOLOR), SLOT(showColorDialog()))
  MAKE_ACTION(backgroundColorAct_, TR(T_BACKGROUNDCOLOR), SLOT(showBackgroundColorDialog()))

#undef MAKE_ACTION
#undef MAKE_TOGGLE
}

void
AnnotationEditor::createLayout()
{
  // Layout
  QVBoxLayout *rows = new QVBoxLayout; {
    QHBoxLayout *header = new QHBoxLayout,
                *footer = new QHBoxLayout;
    rows->addLayout(header);
    rows->addLayout(ribonStackLayout_);
    rows->addWidget(textEdit_);
    rows->addLayout(footer);

    header->addWidget(codeRibonButton_);
    header->addWidget(htmlRibonButton_);
    header->addStretch();
    header->addWidget(formatButton_);

    footer->addWidget(countLabel_);
    footer->addWidget(tidyButton_);
    footer->addStretch();
    footer->addWidget(saveButton_);
    footer->addWidget(cancelButton_);

    // left, top, right, bottom
    header->setContentsMargins(0, 0, 0, 0);
    footer->setContentsMargins(0, 0, 0, 0);
    rows->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(9, 0, 9, 0);
  }
  setLayout(rows);
}

void
AnnotationEditor::clear()
{ textEdit_->clear(); }

// - Properties -

qint64
AnnotationEditor::id() const
{ return id_; }

void
AnnotationEditor::setId(qint64 id)
{  id_ = id; }

bool
AnnotationEditor::isTidyEnabled() const
{ return tidyButton_->isChecked(); }

void
AnnotationEditor::setTidyEnabled(bool t)
{
  tidyButton_->setChecked(t);
  if (t)
    textEdit_->setPlainText(ANNOT_REDUCE_HTML(textEdit_->toPlainText()));
  invalidateCount();
}

AnnotationEditor::Mode
AnnotationEditor::mode() const
{ return isCodeMode() ? CodeMode : HtmlMode; }

bool
AnnotationEditor::isCodeMode() const
{ return ribonStackLayout_->currentIndex() == CodeRibonIndex; }

bool
AnnotationEditor::isHtmlMode() const
{ return ribonStackLayout_->currentIndex() == HtmlRibonIndex; }

void
AnnotationEditor::setMode(Mode mode)
{
  switch (mode) {
  case CodeMode: setCodeMode(); break;
  case HtmlMode: setHtmlMode(); break;
  default: Q_ASSERT(0);
  }
}

QString
AnnotationEditor::text() const
{
  switch (mode()) {
  case CodeMode:
    return textEdit_->toPlainText();

  case HtmlMode:
    if (textEdit_->toPlainText().trimmed().isEmpty())
      return "";
    else {
      QString html = textEdit_->toHtml();
      if (isTidyEnabled())
        return ANNOT_REDUCE_HTML(html);
      else
        return html;
    }

  default:
    Q_ASSERT(0);
    return QString();
  }
}

void
AnnotationEditor::setText(const QString &text)
{
  if (text.isEmpty() || text.trimmed().isEmpty()) {
    clear();
    return;
  }

  textEdit_->setPlainText(text);
  if (isHtmlMode())
    code2html();
}

void
AnnotationEditor::code2html()
{
  DOUT("code2html:enter: text follows");
  DOUT(textEdit_->toPlainText());
  QString html;
  QStringList tags;
  boost::tie(html, tags) = ANNOT_PARSE_CODE(textEdit_->toPlainText());
  if (!tags.empty())
    html = tags.join("") + " " + html.trimmed();

  textEdit_->setHtml(html);
  DOUT("code2html:tags follows");
  DOUT(tags);
  DOUT("code2html:html follows");
  DOUT(html);
  DOUT("code2html:exit: html follows");
}

void
AnnotationEditor::html2code()
{
  DOUT("html2code:enter: tidy =" << isTidyEnabled());
  QString html = textEdit_->toHtml();
  DOUT("html2code: html follows");
  DOUT(html);
  if (isTidyEnabled())
    textEdit_->setPlainText(ANNOT_REDUCE_HTML(html));
  else
    textEdit_->setPlainText(html);
  DOUT("html2code: text follows");
  DOUT(textEdit_->toPlainText());
  clearFormat();
  DOUT("html2code:exit");
}

// - Slots -

void
AnnotationEditor::tag(const QString &tag)
{
  removeTag(tag);
  addTag(tag);
}

void
AnnotationEditor::addTag(const QString &tag)
{
  if (isCodeMode())
    textEdit_->setPlainText(QString("%1 %2").arg(tag).arg(textEdit_->toPlainText()).trimmed());
}

void
AnnotationEditor::removeTag(const QString &tag)
{
  if (isCodeMode())
    textEdit_->setPlainText(textEdit_->toPlainText().remove(tag).trimmed());
}

void
AnnotationEditor::setVerbatim()
{ tag(CORE_CMD_VERBATIM); }

void
AnnotationEditor::setSubtitle()
{ tag(CORE_CMD_SUB); }

// TODO: use moveStyleComboBox as Tag combo box?
void
AnnotationEditor::setMoveStyle(int index)
{
  if (index < 0 || index >= MoveStyleCount) {
    QString newTag = moveStyleComboBox_->currentText().trimmed();
    if (!newTag.isEmpty())
      tag(CORE_CMDSTR + newTag);
    return;
  }

  removeTag(CORE_CMD_VIEW_FLY);
  removeTag(CORE_CMD_VIEW_TOP);
  removeTag(CORE_CMD_VIEW_BOTTOM);

  switch (index) {
  case MoveStyleFlyIndex:    addTag(CORE_CMD_VIEW_FLY); break;
  case MoveStyleTopIndex:    addTag(CORE_CMD_VIEW_TOP); break;
  case MoveStyleBottomIndex: addTag(CORE_CMD_VIEW_BOTTOM); break;
  }
}

void
AnnotationEditor::save()
{
  hide();
  emit textSaved(text());
}

void
AnnotationEditor::cancel()
{ hide(); }

void
AnnotationEditor::setCodeMode()
{
  if (!isCodeMode())
    html2code();

  ribonStackLayout_->setCurrentIndex(CodeRibonIndex);
  htmlRibonButton_->setChecked(false);
  codeRibonButton_->setChecked(true);

  formatButton_->setEnabled(false);
}

void
AnnotationEditor::setHtmlMode()
{
  if (!isHtmlMode())
    code2html();

  ribonStackLayout_->setCurrentIndex(HtmlRibonIndex);
  htmlRibonButton_->setChecked(true);
  codeRibonButton_->setChecked(false);

  formatButton_->setEnabled(true);
}


void
AnnotationEditor::setBold(bool t)
{
  boldButton_->setCheckable(t);
  boldAct_->setCheckable(t);

  QTextCharFormat fmt;
  fmt.setFontWeight(t ? QFont::Bold : QFont::Normal);
  mergeFormat(fmt);
}

void
AnnotationEditor::setItalic(bool t)
{
  italicButton_->setCheckable(t);
  italicAct_->setCheckable(t);

  QTextCharFormat fmt;
  fmt.setFontItalic(t);
  mergeFormat(fmt);
}

void
AnnotationEditor::setUnderline(bool t)
{
  underlineButton_->setCheckable(t);
  underlineAct_->setCheckable(t);

  QTextCharFormat fmt;
  fmt.setFontUnderline(t);
  mergeFormat(fmt);
}

void
AnnotationEditor::setStrikeOut(bool t)
{
  strikeOutButton_->setCheckable(t);
  strikeOutAct_->setCheckable(t);

  QTextCharFormat fmt;
  fmt.setFontStrikeOut(t);
  mergeFormat(fmt);
}

void
AnnotationEditor::setFontFamily(const QString &name)
{
  QTextCharFormat fmt;
  fmt.setFontFamily(name);
  if (fmt.isValid())
    mergeFormat(fmt);
}

void
AnnotationEditor::setFontSize(const QString &size)
{
  bool ok;
  qreal fp = size.toFloat(&ok);
  if (ok && fp > 0) {
    QTextCharFormat fmt;
    fmt.setFontPointSize(fp);
    if (fmt.isValid())
      mergeFormat(fmt);
  }
}

void
AnnotationEditor::setColor(const QColor &c)
{
  QTextCharFormat fmt;
  fmt.setForeground(c);
  mergeFormat(fmt);

  setColorIconColor(c);
}

void
AnnotationEditor::setBackgroundColor(const QColor &c)
{
  QTextCharFormat fmt;
  fmt.setBackground(c);
  mergeFormat(fmt);

  setBackgroundColorIconColor(c);
}

void
AnnotationEditor::setFormat(const QTextCharFormat &fmt)
{
  setFontType(fmt.font());
  setColorIconColor(fmt.foreground().color());
  setBackgroundColorIconColor(fmt.background().color());
}

void
AnnotationEditor::clearFormat()
{
  QTextCharFormat fmt;

  setFontType(fmt.font());
  setColorIconColor(fmt.foreground().color());
  setBackgroundColorIconColor(fmt.foreground().color());

  QTextCursor cursor = textEdit_->textCursor();
  cursor.select(QTextCursor::Document);
  cursor.setCharFormat(fmt);
  textEdit_->setCurrentCharFormat(fmt);
}

void
AnnotationEditor::setFontType(const QFont &font)
{
  fontComboBox_->setCurrentIndex(fontComboBox_->findText(QFontInfo(font).family()));
  fontSizeComboBox_->setCurrentIndex(fontSizeComboBox_->findText(QString::number(font.pointSize())));

  boldButton_->setChecked(font.bold());
  italicButton_->setChecked(font.italic());
  underlineButton_->setChecked(font.underline());
}

void
AnnotationEditor::setColorIconColor(const QColor &input)
{
  QColor c = input;
  if (!c.isValid())
    c = QColor("black");
  QPixmap icon(16, 16);
  icon.fill(c);
  colorButton_->setIcon(icon);
  colorAct_->setIcon(icon);
}

void
AnnotationEditor::setBackgroundColorIconColor(const QColor &input)
{
  QColor c = input;
  if (!c.isValid())
    c = QColor("black");
  QPixmap icon(16, 16);
  icon.fill(c);
  backgroundColorButton_->setIcon(icon);
  backgroundColorAct_->setIcon(icon);
}

void
AnnotationEditor::showColorDialog()
{
  QColor c = QColorDialog::getColor(textEdit_->textColor(), this);
  if (c.isValid())
    setColor(c);
}

void
AnnotationEditor::showBackgroundColorDialog()
{
  QColor c = QColorDialog::getColor(textEdit_->textBackgroundColor(), this);
  if (c.isValid())
    setBackgroundColor(c);
}

void
AnnotationEditor::setAlignment(int alignIndex)
{
  Qt::Alignment a = 0;
  switch (alignIndex) {
  case AlignLeftIndex:    a = Qt::AlignLeft | Qt::AlignAbsolute; break;
  case AlignRightIndex:   a = Qt::AlignRight | Qt::AlignAbsolute; break;
  case AlignHCenterIndex: a = Qt::AlignHCenter; break;
  case AlignJustifyIndex: a = Qt::AlignJustify; break;
  }
  if (a)
    textEdit_->setAlignment(a);
}

void
AnnotationEditor::invalidateAlignment()
{
  Qt::Alignment a = textEdit_->alignment();

  int index = AlignIndexCount;
  if (a & Qt::AlignLeft)
    index = AlignLeftIndex;
  else if (a & Qt::AlignRight)
    index = AlignRightIndex;
  else if (a & Qt::AlignHCenter)
    index = AlignHCenterIndex;
  else if (a & Qt::AlignJustify)
    index = AlignJustifyIndex;

  if (index < AlignIndexCount)
    alignComboBox_->setCurrentIndex(index);
}

void
AnnotationEditor::invalidateCount()
{
  int cur = text().size();
  int max = G_ANNOTATION_MAXSIZE;
  countLabel_->setText(QString("%1/%2").arg(cur).arg(max));

  if (cur > max) {
    QString warning = tr("Text is too long >_<");
    countLabel_->setToolTip(warning);
    if (saveButton_->isEnabled()) {
      saveButton_->setEnabled(false);
      saveButton_->setToolTip(warning);
    }
  } else {
    countLabel_->setToolTip(TR(T_WORDCOUNT));
    if (!saveButton_->isEnabled()) {
      saveButton_->setEnabled(true);
      saveButton_->setToolTip(TR(T_SAVE));
    }
  }
}

// - Helpers -

void
AnnotationEditor::mergeFormat(const QTextCharFormat &format)
{
  QTextCursor cursor = textEdit_->textCursor();
  if (!cursor.hasSelection())
    cursor.select(QTextCursor::WordUnderCursor);
  cursor.mergeCharFormat(format);
  textEdit_->mergeCurrentCharFormat(format);
}

// - Events -

/*
void
AnnotationEditor::contextMenuEvent(QContextMenuEvent *event)
{
  if (!event)
    return;

  event->accept();
}
*/

// EOF

/*
void TextEdit::textStyle(int styleIndex)
{
  QTextCursor cursor = textEdit_->textCursor();

  if (styleIndex != 0) {
    QTextListFormat::Style style = QTextListFormat::ListDisc;

    switch (styleIndex) {
      default:
      case 1:
        style = QTextListFormat::ListDisc;
        break;
      case 2:
        style = QTextListFormat::ListCircle;
        break;
      case 3:
        style = QTextListFormat::ListSquare;
        break;
      case 4:
        style = QTextListFormat::ListDecimal;
        break;
      case 5:
        style = QTextListFormat::ListLowerAlpha;
        break;
      case 6:
        style = QTextListFormat::ListUpperAlpha;
        break;
      case 7:
        style = QTextListFormat::ListLowerRoman;
        break;
      case 8:
        style = QTextListFormat::ListUpperRoman;
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;

    if (cursor.currentList()) {
      listFmt = cursor.currentList()->format();
    } else {
      listFmt.setIndent(blockFmt.indent() + 1);
      blockFmt.setIndent(0);
      cursor.setBlockFormat(blockFmt);
    }

    listFmt.setStyle(style);

    cursor.createList(listFmt);

    cursor.endEditBlock();
  } else {
    // ####
    QTextBlockFormat bfmt;
    bfmt.setObjectIndex(-1);
    cursor.mergeBlockFormat(bfmt);
  }
}
*/
