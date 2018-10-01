/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include "mainwindow.h"
#include <QApplication>


void
myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
      fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
  case QtInfoMsg:
    fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
    abort();
  }
  fflush(stderr);
}


int
main(int argc, char *argv[]) {
  qInstallMessageHandler(myMessageOutput); // Install the handler

  QApplication a(argc, argv);

  QCoreApplication::setOrganizationDomain("Gabriele.Salvato");
  QCoreApplication::setOrganizationName("Gabriele.Salvato");
  QCoreApplication::setApplicationName("Conductivity");
  QCoreApplication::setApplicationVersion("1.0.0");

  MainWindow w;
  w.setWindowIcon(QIcon("qrc:/myLogoT.png"));
  w.show();

  return a.exec();
}
