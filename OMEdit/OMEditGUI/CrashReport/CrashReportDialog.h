/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S ACCEPTANCE
 * OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3, ACCORDING TO RECIPIENTS CHOICE.
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
 *
 * @author Adeel Asghar <adeel.asghar@liu.se>
 *
 * RCS: $Id$
 *
 */

#ifndef CRASHREPORTDIALOG_H
#define CRASHREPORTDIALOG_H

#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#include <QNetworkReply>
#include <QHttpMultiPart>
#else
#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#endif

#include "Utilities.h"

class CrashReportDialog : public QDialog
{
  Q_OBJECT
public:
  CrashReportDialog();
private:
  Label *mpCrashReportHeading;
  QFrame *mpHorizontalLine;
  Label *mpEmailLabel;
  QLineEdit *mpEmailTextBox;
  Label *mpBugDescriptionLabel;
  QPlainTextEdit *mpBugDescriptionTextBox;
  Label *mpFilesDescriptionLabel;
  QCheckBox *mpOMEditCommunicationLogFileCheckBox;
  QCheckBox *mpOMEditCommandsMosFileCheckBox;
  QCheckBox *mpOMStackTraceFileCheckBox;
  QPushButton *mpSendReportButton;
  QPushButton *mpCancelButton;
  QDialogButtonBox *mpButtonBox;
public slots:
  void sendReport();
  void reportSent(QNetworkReply *pNetworkReply);
};

#endif // CRASHREPORTDIALOG_H
