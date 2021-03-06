/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 * @author Adeel Asghar <adeel.asghar@liu.se>
 */

#include "Utilities.h"
#include "Helper.h"
#include "Editors/BaseEditor.h"
#include "Modeling/LibraryTreeWidget.h"

#include <QApplication>
#include <QGridLayout>
#include <QStylePainter>
#include <QPainter>
#include <QColorDialog>
#include <QXmlSchema>

SplashScreen *SplashScreen::mpInstance = 0;

SplashScreen *SplashScreen::instance()
{
  if (!mpInstance) {
    mpInstance = new SplashScreen;
  }
  return mpInstance;
}

TreeSearchFilters::TreeSearchFilters(QWidget *pParent)
  : QWidget(pParent)
{
  // create the filter text box
  mpFilterTextBox = new QLineEdit;
  mpFilterTextBox->installEventFilter(this);
  // show hide button
  mpShowHideButton = new QToolButton;
  QString text = tr("Show/hide filters");
  mpShowHideButton->setText(text);
  mpShowHideButton->setIcon(QIcon(":/Resources/icons/down.svg"));
  mpShowHideButton->setToolTip(text);
  mpShowHideButton->setAutoRaise(true);
  mpShowHideButton->setCheckable(true);
  connect(mpShowHideButton, SIGNAL(toggled(bool)), SLOT(showHideFilters(bool)));
  // filters widget
  mpFiltersWidget = new QWidget;
  // create the case sensitivity checkbox
  mpCaseSensitiveCheckBox = new QCheckBox(tr("Case Sensitive"));
  // create the search syntax combobox
  mpSyntaxComboBox = new QComboBox;
  mpSyntaxComboBox->addItem(tr("Regular Expression"), QRegExp::RegExp);
  mpSyntaxComboBox->setItemData(0, tr("A rich Perl-like pattern matching syntax."), Qt::ToolTipRole);
  mpSyntaxComboBox->addItem(tr("Wildcard"), QRegExp::Wildcard);
  mpSyntaxComboBox->setItemData(1, tr("A simple pattern matching syntax similar to that used by shells (command interpreters) for \"file globbing\"."), Qt::ToolTipRole);
  mpSyntaxComboBox->addItem(tr("Fixed String"), QRegExp::FixedString);
  mpSyntaxComboBox->setItemData(2, tr("Fixed string matching."), Qt::ToolTipRole);
  // expand all button
  mpExpandAllButton = new QPushButton(Helper::expandAll);
  mpExpandAllButton->setAutoDefault(false);
  // collapse all button
  mpCollapseAllButton = new QPushButton(Helper::collapseAll);
  mpCollapseAllButton->setAutoDefault(false);
  // create the layout
  QGridLayout *pFiltersWidgetLayout = new QGridLayout;
  pFiltersWidgetLayout->setContentsMargins(0, 0, 0, 0);
  pFiltersWidgetLayout->setAlignment(Qt::AlignTop);
  pFiltersWidgetLayout->addWidget(mpCaseSensitiveCheckBox, 0, 0);
  pFiltersWidgetLayout->addWidget(mpSyntaxComboBox, 0, 1);
  pFiltersWidgetLayout->addWidget(mpExpandAllButton, 1, 0);
  pFiltersWidgetLayout->addWidget(mpCollapseAllButton, 1, 1);
  mpFiltersWidget->setLayout(pFiltersWidgetLayout);
  mpFiltersWidget->hide();
  // create the layout
  QGridLayout *pMainLayout = new QGridLayout;
  pMainLayout->setContentsMargins(0, 0, 0, 0);
  pMainLayout->setAlignment(Qt::AlignTop);
  pMainLayout->addWidget(mpFilterTextBox, 0, 0);
  pMainLayout->addWidget(mpShowHideButton, 0, 1);
  pMainLayout->addWidget(mpFiltersWidget, 1, 0, 1, 2);
  setLayout(pMainLayout);
}

/*!
 * \brief TreeSearchFilters::eventFilter
 * Handles the ESC key press for search text box
 * \param pObject
 * \param pEvent
 * \return
 */
bool TreeSearchFilters::eventFilter(QObject *pObject, QEvent *pEvent)
{
  /* Ticket #3987
   * Clear contents of search field by clicking ESC key.
   */
  QLineEdit *pSearchTextBox = qobject_cast<QLineEdit*>(pObject);
  if (pSearchTextBox && pEvent->type() == QEvent::KeyPress) {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent*>(pEvent);
    if (pKeyEvent && pKeyEvent->key() == Qt::Key_Escape) {
      pSearchTextBox->clear();
      return true;
    }
  }
  return QWidget::eventFilter(pObject, pEvent);
}

void TreeSearchFilters::showHideFilters(bool On)
{
  if (On) {
    mpFiltersWidget->show();
  } else {
    mpFiltersWidget->hide();
  }
}

/*!
 * \class FileDataNotifier
 * \brief Looks for new data on file. When data is available emits bytesAvailable SIGNAL.
 * \brief FileDataNotifier::FileDataNotifier
 * \param fileName
 */
FileDataNotifier::FileDataNotifier(const QString fileName)
{
  mFile.setFileName(fileName);
  mStop = false;
  mBytesAvailable = 0;
}

/*!
 * \brief FileDataNotifier::exit
 * Reimplementation of QThread::exit()
 * Sets mStop to true.
 * \param retcode
 */
void FileDataNotifier::exit(int retcode)
{
  mStop = true;
  QThread::exit(retcode);
}

/*!
 * \brief FileDataNotifier::read
 * Reads the bytes from the file.
 * \param maxlen
 * \return
 */
QByteArray FileDataNotifier::read(qint64 maxlen)
{
  return mFile.read(maxlen);
}

/*!
 * \brief FileDataNotifier::run
 * Reimplentation of QThread::run().
 * Looks for when is new data available for reading on file.
 * Emits the bytesAvailable SIGNAL.
 */
void FileDataNotifier::run()
{
  if (mFile.open(QIODevice::ReadOnly)) {
    while (!mStop) {
      // if file doesn't exist then break.
      if (!mFile.exists()) {
        break;
      }
      // if file has bytes available to read.
      if (mFile.bytesAvailable() > mBytesAvailable) {
        mBytesAvailable = mFile.bytesAvailable();
        emit bytesAvailable(mFile.bytesAvailable());
      }
      Sleep::sleep(1);
    }
  }
}

/*!
 * \brief FileDataNotifier::start
 * Reimplementation of QThread::start()
 * Sets mStop to false.
 * \param priority
 */
void FileDataNotifier::start(Priority priority)
{
  mStop = false;
  QThread::start(priority);
}

Label::Label(QWidget *parent, Qt::WindowFlags flags)
  : QLabel(parent, flags), mElideMode(Qt::ElideNone), mText("")
{
  setTextInteractionFlags(Qt::TextSelectableByMouse);
}

Label::Label(const QString &text, QWidget *parent, Qt::WindowFlags flags)
  : QLabel(text, parent, flags), mElideMode(Qt::ElideNone), mText(text)
{
  setTextInteractionFlags(Qt::TextSelectableByMouse);
  setToolTip(text);
}

QSize Label::minimumSizeHint() const
{
  if (pixmap() != NULL || mElideMode == Qt::ElideNone) {
    return QLabel::minimumSizeHint();
  }
  const QFontMetrics &fm = fontMetrics();
  QSize size(fm.width("..."), fm.height()+5);
  return size;
}

QSize Label::sizeHint() const
{
  if (pixmap() != NULL || mElideMode == Qt::ElideNone)
    return QLabel::sizeHint();
  const QFontMetrics& fm = fontMetrics();
  QSize size(fm.width(mText), fm.height()+5);
  return size;
}

void Label::setText(const QString &text)
{
  mText = text;
  setToolTip(text);
  QLabel::setText(text);
}

void Label::resizeEvent(QResizeEvent *event)
{
  if (mElideMode != Qt::ElideNone) {
    QFontMetrics fm(fontMetrics());
    QString str = fm.elidedText(mText, mElideMode, event->size().width());
    QLabel::setText(str);
  }
  QLabel::resizeEvent(event);
}

FixedCheckBox::FixedCheckBox(QWidget *parent)
 : QCheckBox(parent)
{
  setCheckable(false);
  mDefaultValue = false;
  mTickState = false;
}

void FixedCheckBox::setTickState(bool defaultValue, bool tickState)
{
  mDefaultValue = defaultValue;
  mTickState = tickState;
}

QString FixedCheckBox::tickStateString()
{
  if (mDefaultValue) {
    return "";
  } else if (mTickState) {
    return "true";
  } else {
    return "false";
  }
}

/*!
  Reimplementation of QCheckBox::paintEvent.\n
  Draws a custom checkbox suitable for fixed modifier.
  */
void FixedCheckBox::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QStylePainter p(this);
  QStyleOptionButton opt;
  opt.initFrom(this);
  if (mDefaultValue) {
    p.setBrush(QColor(225, 225, 225));
  } else {
    p.setBrush(Qt::white);
  }
  p.drawRect(opt.rect.adjusted(0, 0, -1, -1));
  // if is checked then draw a tick
  if (mTickState) {
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen = p.pen();
    pen.setWidthF(1.5);
    p.setPen(pen);
    QVector<QPoint> lines;
    lines << QPoint(opt.rect.left() + 3, opt.rect.center().y());
    lines << QPoint(opt.rect.center().x() - 1, opt.rect.bottom() - 3);
    lines << QPoint(opt.rect.center().x() - 1, opt.rect.bottom() - 3);
    lines << QPoint(opt.rect.width() - 3, opt.rect.top() + 3);
    p.drawLines(lines);
  }
}

PreviewPlainTextEdit::PreviewPlainTextEdit(QWidget *parent)
 : QPlainTextEdit(parent)
{
  QTextDocument *pTextDocument = document();
  pTextDocument->setDocumentMargin(2);
  BaseEditorDocumentLayout *pModelicaTextDocumentLayout = new BaseEditorDocumentLayout(pTextDocument);
  pTextDocument->setDocumentLayout(pModelicaTextDocumentLayout);
  setDocument(pTextDocument);
  // parentheses matcher
  mParenthesesMatchFormat = Utilities::getParenthesesMatchFormat();
  mParenthesesMisMatchFormat = Utilities::getParenthesesMisMatchFormat();

  updateHighlights();
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateHighlights()));
}

/*!
 * \brief PreviewPlainTextEdit::highlightCurrentLine
 * Hightlights the current line.
 */
void PreviewPlainTextEdit::highlightCurrentLine()
{
  Utilities::highlightCurrentLine(this);
}

/*!
 * \brief PreviewPlainTextEdit::highlightParentheses
 * Highlights the matching parentheses.
 */
void PreviewPlainTextEdit::highlightParentheses()
{
  Utilities::highlightParentheses(this, mParenthesesMatchFormat, mParenthesesMisMatchFormat);
}

void PreviewPlainTextEdit::updateHighlights()
{
  QList<QTextEdit::ExtraSelection> selections;
  setExtraSelections(selections);
  highlightCurrentLine();
  highlightParentheses();
}

ListWidgetItem::ListWidgetItem(QString text, QColor color, QListWidget *pParentListWidget)
  : QListWidgetItem(pParentListWidget)
{
  setText(text);
  mColor = color;
  setForeground(mColor);
}

CodeColorsWidget::CodeColorsWidget(QWidget *pParent)
  : QWidget(pParent)
{
  // colors groupbox
  mpColorsGroupBox = new QGroupBox(Helper::Colors);
  // Item color label and pick color button
  mpItemColorLabel = new Label(tr("Item Color:"));
  mpItemColorPickButton = new QPushButton(Helper::pickColor);
  mpItemColorPickButton->setAutoDefault(false);
  connect(mpItemColorPickButton, SIGNAL(clicked()), SLOT(pickColor()));
  // Items list
  mpItemsLabel = new Label(tr("Items:"));
  mpItemsListWidget = new QListWidget;
  mpItemsListWidget->setItemDelegate(new ItemDelegate(mpItemsListWidget));
  mpItemsListWidget->setMaximumHeight(90);
  // text (black)
  new ListWidgetItem("Text", QColor(0, 0, 0), mpItemsListWidget);
  // make first item in the list selected
  mpItemsListWidget->setCurrentRow(0, QItemSelectionModel::Select);
  // preview textbox
  mpPreviewLabel = new Label(tr("Preview:"));
  mpPreviewPlainTextEdit = new PreviewPlainTextEdit;
  mpPreviewPlainTextEdit->setTabStopWidth(Helper::tabWidth);
  // set colors groupbox layout
  QGridLayout *pColorsGroupBoxLayout = new QGridLayout;
  pColorsGroupBoxLayout->addWidget(mpItemsLabel, 1, 0);
  pColorsGroupBoxLayout->addWidget(mpItemColorLabel, 1, 1);
  pColorsGroupBoxLayout->addWidget(mpItemsListWidget, 2, 0);
  pColorsGroupBoxLayout->addWidget(mpItemColorPickButton, 2, 1, Qt::AlignTop);
  pColorsGroupBoxLayout->addWidget(mpPreviewLabel, 3, 0, 1, 2);
  pColorsGroupBoxLayout->addWidget(mpPreviewPlainTextEdit, 4, 0, 1, 2);
  mpColorsGroupBox->setLayout(pColorsGroupBoxLayout);
  // set the layout
  QVBoxLayout *pMainLayout = new QVBoxLayout;
  pMainLayout->setContentsMargins(0, 0, 0, 0);
  pMainLayout->addWidget(mpColorsGroupBox);
  setLayout(pMainLayout);
}

/*!
 * \brief CodeColorsWidget::pickColor
 * Picks a color for one of the Text Settings rules.
 * This method is called when mpColorPickButton clicked SIGNAL raised.
 */
void CodeColorsWidget::pickColor()
{
  QListWidgetItem *pItem = mpItemsListWidget->currentItem();
  ListWidgetItem *pListWidgetItem = dynamic_cast<ListWidgetItem*>(pItem);
  if (!pListWidgetItem) {
    return;
  }
  QColor color = QColorDialog::getColor(pListWidgetItem->getColor());
  if (!color.isValid()) {
    return;
  }
  pListWidgetItem->setColor(color);
  pListWidgetItem->setForeground(color);
  emit colorUpdated();
}

/*!
 * \brief Utilities::tempDirectory
 * Returns the application temporary directory.
 * \return
 */
QString& Utilities::tempDirectory()
{
  static int init = 0;
  static QString tmpPath;
  if (!init) {
    init = 1;
#ifdef WIN32
    tmpPath = QDir::tempPath() + "/OpenModelica/OMEdit/";
#else // UNIX environment
    char *user = getenv("USER");
    tmpPath = QDir::tempPath() + "/OpenModelica_" + QString(user ? user : "nobody") + "/OMEdit/";
#endif
    tmpPath.remove("\"");
    if (!QDir().exists(tmpPath))
      QDir().mkpath(tmpPath);
  }
  return tmpPath;
}

/*!
 * \brief Utilities::getApplicationSettings
 * Returns the application settings object.
 * \return
 */
QSettings* Utilities::getApplicationSettings()
{
  static int init = 0;
  static QSettings *pSettings;
  if (!init) {
    init = 1;
    pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, Helper::organization, Helper::application);
    pSettings->setIniCodec(Helper::utf8.toStdString().data());
  }
  return pSettings;
}


/*!
 * \brief Utilities::parseMetaModelText
 * Parses the MetaModel text against the schema.
 * \param pMessageHandler
 * \param contents
 */
void Utilities::parseMetaModelText(MessageHandler *pMessageHandler, QString contents)
{
  QFile schemaFile(QString(":/Resources/XMLSchema/tlmModelDescription.xsd"));
  schemaFile.open(QIODevice::ReadOnly);
  const QString schemaText(QString::fromUtf8(schemaFile.readAll()));
  schemaFile.close();
  const QByteArray schemaData = schemaText.toUtf8();

  QXmlSchema schema;
  schema.setMessageHandler(pMessageHandler);
  schema.load(schemaData);
  if (!schema.isValid()) {
    pMessageHandler->setFailed(true);
  } else {
    QXmlSchemaValidator validator(schema);
    if (!validator.validate(contents.toUtf8())) {
      pMessageHandler->setFailed(true);
    }
  }
}

/*!
 * \brief Utilities::convertUnit
 * Converts the value using the unit offset and scale factor.
 * \param value
 * \param offset
 * \param scaleFactor
 * \return
 */
qreal Utilities::convertUnit(qreal value, qreal offset, qreal scaleFactor)
{
  return (value - offset) / scaleFactor;
}

Label* Utilities::getHeadingLabel(QString heading)
{
  Label *pHeadingLabel = new Label(heading);
  pHeadingLabel->setFont(QFont(Helper::systemFontInfo.family(), Helper::headingFontSize));
  return pHeadingLabel;
}

QFrame* Utilities::getHeadingLine()
{
  QFrame *pHeadingLine = new QFrame();
  pHeadingLine->setFrameShape(QFrame::HLine);
  pHeadingLine->setFrameShadow(QFrame::Sunken);
  return pHeadingLine;
}

/*!
 * \brief Utilities::detectBOM
 * Detects if the file has byte order mark (BOM) or not.
 * \param fileName
 * \return
 */
bool Utilities::detectBOM(QString fileName)
{
  QFile file(fileName);
  if (file.exists()) {
    if (file.open(QIODevice::ReadOnly)) {
      QByteArray data = file.readAll();
      const int bytesRead = data.size();
      const unsigned char *buf = reinterpret_cast<const unsigned char *>(data.constData());
      // code taken from qtextstream
      if (bytesRead >= 3 && ((buf[0] == 0xef && buf[1] == 0xbb) && buf[2] == 0xbf)) {
        return true;
      } else {
        return false;
      }
      file.close();
    } else {
      qDebug() << QString("Failed to detect byte order mark. Unable to open file %1.").arg(fileName);
    }
  }
  return false;
}

QTextCharFormat Utilities::getParenthesesMatchFormat()
{
  QTextCharFormat parenthesesMatchFormat;
  parenthesesMatchFormat.setForeground(Qt::red);
  parenthesesMatchFormat.setBackground(QColor(160, 238, 160));
  return parenthesesMatchFormat;
}

QTextCharFormat Utilities::getParenthesesMisMatchFormat()
{
  QTextCharFormat parenthesesMisMatchFormat;
  parenthesesMisMatchFormat.setBackground(Qt::red);
  return parenthesesMisMatchFormat;
}

void Utilities::highlightCurrentLine(QPlainTextEdit *pPlainTextEdit)
{
  QList<QTextEdit::ExtraSelection> selections = pPlainTextEdit->extraSelections();
  QTextEdit::ExtraSelection selection;
  QColor lineColor = QColor(232, 242, 254);
  selection.format.setBackground(lineColor);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = pPlainTextEdit->textCursor();
  selection.cursor.clearSelection();
  selections.append(selection);
  pPlainTextEdit->setExtraSelections(selections);
}

void Utilities::highlightParentheses(QPlainTextEdit *pPlainTextEdit, QTextCharFormat parenthesesMatchFormat,
                                     QTextCharFormat parenthesesMisMatchFormat)
{
  if (pPlainTextEdit->isReadOnly()) {
    return;
  }

  QTextCursor backwardMatch = pPlainTextEdit->textCursor();
  QTextCursor forwardMatch = pPlainTextEdit->textCursor();
  if (pPlainTextEdit->overwriteMode()) {
    backwardMatch.movePosition(QTextCursor::Right);
  }

  const TextBlockUserData::MatchType backwardMatchType = TextBlockUserData::matchCursorBackward(&backwardMatch);
  const TextBlockUserData::MatchType forwardMatchType = TextBlockUserData::matchCursorForward(&forwardMatch);
  QList<QTextEdit::ExtraSelection> selections = pPlainTextEdit->extraSelections();

  if (backwardMatchType == TextBlockUserData::NoMatch && forwardMatchType == TextBlockUserData::NoMatch) {
    pPlainTextEdit->setExtraSelections(selections);
    return;
  }

  if (backwardMatch.hasSelection()) {
    QTextEdit::ExtraSelection selection;
    if (backwardMatchType == TextBlockUserData::Mismatch) {
      selection.cursor = backwardMatch;
      selection.format = parenthesesMisMatchFormat;
      selections.append(selection);
    } else {
      selection.cursor = backwardMatch;
      selection.format = parenthesesMatchFormat;

      selection.cursor.setPosition(backwardMatch.selectionStart());
      selection.cursor.setPosition(selection.cursor.position() + 1, QTextCursor::KeepAnchor);
      selections.append(selection);

      selection.cursor.setPosition(backwardMatch.selectionEnd());
      selection.cursor.setPosition(selection.cursor.position() - 1, QTextCursor::KeepAnchor);
      selections.append(selection);
    }
  }

  if (forwardMatch.hasSelection()) {
    QTextEdit::ExtraSelection selection;
    if (forwardMatchType == TextBlockUserData::Mismatch) {
      selection.cursor = forwardMatch;
      selection.format = parenthesesMisMatchFormat;
      selections.append(selection);
    } else {
      selection.cursor = forwardMatch;
      selection.format = parenthesesMatchFormat;

      selection.cursor.setPosition(forwardMatch.selectionStart());
      selection.cursor.setPosition(selection.cursor.position() + 1, QTextCursor::KeepAnchor);
      selections.append(selection);

      selection.cursor.setPosition(forwardMatch.selectionEnd());
      selection.cursor.setPosition(selection.cursor.position() - 1, QTextCursor::KeepAnchor);
      selections.append(selection);
    }
  }
  pPlainTextEdit->setExtraSelections(selections);
}

/*!
 * \brief Utilities::getProcessId
 * Returns the process id.
 * \param pProcess
 * \return
 */
qint64 Utilities::getProcessId(QProcess *pProcess)
{
  qint64 processId = 0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
  processId = pProcess->processId();
#else /* Qt4 */
#ifdef WIN32
  _PROCESS_INFORMATION *procInfo = pProcess->pid();
  if (procInfo) {
    processId = procInfo->dwProcessId;
  }
#else
  processId = pProcess->pid();
#endif /* WIN32 */
#endif /* QT_VERSION */
  return processId;
}

#ifdef WIN32
/* adrpo: found this on http://stackoverflow.com/questions/1173342/terminate-a-process-tree-c-for-windows
 * thanks go to: mjmarsh & Firas Assaad
 * adapted to recurse on children ids
 */
void Utilities::killProcessTreeWindows(DWORD myprocID)
{
  PROCESSENTRY32 pe;
  HANDLE hSnap = NULL, hProc = NULL;

  memset(&pe, 0, sizeof(PROCESSENTRY32));
  pe.dwSize = sizeof(PROCESSENTRY32);

  hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (Process32First(hSnap, &pe))
  {
    BOOL bContinue = TRUE;

    // kill child processes
    while (bContinue)
    {
      // only kill child processes
      if (pe.th32ParentProcessID == myprocID)
      {
        HANDLE hChildProc = NULL;

        // recurse
        killProcessTreeWindows(pe.th32ProcessID);

        hChildProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);

        if (hChildProc)
        {
          TerminateProcess(hChildProc, 1);
          CloseHandle(hChildProc);
        }
      }

      bContinue = Process32Next(hSnap, &pe);
    }

    // kill the main process
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, myprocID);

    if (hProc)
    {
      TerminateProcess(hProc, 1);
      CloseHandle(hProc);
    }
  }
}
#endif

/*!
 * \brief Utilities::isCFile
 * Returns true if extension is of C file.
 * \param extension
 * \return
 */
bool Utilities::isCFile(QString extension)
{
  if (extension.compare("c") == 0 ||
      extension.compare("cpp") == 0 ||
      extension.compare("cc") == 0 ||
      extension.compare("h") == 0 ||
      extension.compare("hpp") == 0) {
    return true;
  } else {
    return false;
  }
}

/*!
 * \brief Utilities::isModelicaFile
 * Returns true if extension is of Modelica file.
 * \param extension
 * \return
 */
bool Utilities::isModelicaFile(QString extension)
{
  if (extension.compare("mo") == 0) {
    return true;
  } else {
    return false;
  }
}

/*!
 * \brief Utilities::insertText
 * Inserts the text to QPlainTextEdit.
 * \param pPlainTextEdit
 * \param text
 * \param color
 */
void Utilities::insertText(QPlainTextEdit *pPlainTextEdit, QString text, QTextCharFormat format)
{
  // move the cursor down before adding to the logger.
  QTextCursor textCursor = pPlainTextEdit->textCursor();
  const bool atBottom = pPlainTextEdit->verticalScrollBar()->value() == pPlainTextEdit->verticalScrollBar()->maximum();
  if (!textCursor.atEnd()) {
    textCursor.movePosition(QTextCursor::End);
  }
  // insert the text
  textCursor.beginEditBlock();
  if (format.isValid()) {
    textCursor.insertText(text, format);
  } else {
    textCursor.insertText(text);
  }
  textCursor.endEditBlock();
  // move the cursor
  if (atBottom) {
    pPlainTextEdit->verticalScrollBar()->setValue(pPlainTextEdit->verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    pPlainTextEdit->verticalScrollBar()->setValue(pPlainTextEdit->verticalScrollBar()->maximum());
  }
}

/*!
 * \brief Utilities::getRotationMatrix
 * Computes the corresponding rotation matrix for specified rotation vector
 * \param rotation Rotation vector with Euler angles
 * \return
 */
QGenericMatrix<3,3, double> Utilities::getRotationMatrix(QGenericMatrix<3,1,double> rotation)
{
  double c1 = cos(rotation(0,0));
  double s1 = sin(rotation(0,0));
  double c2 = cos(rotation(0,1));
  double s2 = sin(rotation(0,1));
  double c3 = cos(rotation(0,2));
  double s3 = sin(rotation(0,2));

  double R_data[9];
  R_data[0] = c2*c3;             R_data[1] = c2*s3;              R_data[2] = -s2;
  R_data[3] = -c1*s3+s1*s2*c3;   R_data[4] = c1*c3+s1*s2*s3;     R_data[5] = s1*c2;
  R_data[6] = s1*s3+c1*s2*c3;    R_data[7] = -s1*c3+c1*s2*s3;    R_data[8] = c1*c2;

  QGenericMatrix<3,3,double> R(R_data);

  return R;
}

Utilities::FileIconProvider::FileIconProviderImplementation *instance()
{
  static Utilities::FileIconProvider::FileIconProviderImplementation theInstance;
  return &theInstance;
}

QFileIconProvider* Utilities::FileIconProvider::iconProvider()
{
  return instance();
}

Utilities::FileIconProvider::FileIconProviderImplementation::FileIconProviderImplementation()
   : mUnknownFileIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon))
{

}

QIcon Utilities::FileIconProvider::FileIconProviderImplementation::icon(const QFileInfo &fileInfo)
{
  // Check for cached overlay icons by file suffix.
  bool isDir = fileInfo.isDir();
  QString suffix = !isDir ? fileInfo.suffix() : QString();
  if (!mIconsHash.isEmpty() && !isDir && !suffix.isEmpty()) {
    if (mIconsHash.contains(suffix)) {
      return mIconsHash.value(suffix);
    }
  }
  // Get icon from OS.
  QIcon icon;
  // File icons are unknown on linux systems.
#if defined(Q_OS_LINUX)
  icon = isDir ? QFileIconProvider::icon(fileInfo) : mUnknownFileIcon;
#else
  icon = QFileIconProvider::icon(fileInfo);
#endif
  if (!isDir && !suffix.isEmpty()) {
    mIconsHash.insert(suffix, icon);
  }
  return icon;
}

/*!
 * \brief icon
 * Returns the icon associated with the file suffix in fileInfo. If there is none,
 * the default icon of the operating system is returned.
 * \param info
 * \return
 */
QIcon Utilities::FileIconProvider::icon(const QFileInfo &info)
{
  return instance()->icon(info);
}
