#include "filetab.h"
#include <QDir>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QSettings>
#include <QMessageBox>

FileTab::FileTab(QWidget *parent)
    : QWidget(parent)
    , sBaseDir(QDir::homePath())
    , sOutFileName("junction.dat")
{
    pSampleInformationEdit = new QPlainTextEdit();
    pOutPathEdit = new QLineEdit();
    pOutFileEdit = new QLineEdit();;

    restoreSettings();
    setToolTips();
    initUI();
}



void
FileTab::initUI() {
    pSampleInformationEdit->setPlainText(sSampleInfo);
    pOutPathEdit->setText(sBaseDir);
    pOutFileEdit->setText(sOutFileName);
}


void
FileTab::restoreSettings() {
    QSettings settings;
    sSampleInfo    = settings.value("FileTabSampleInfo", "").toString();
    sBaseDir       = settings.value("FileTabBaseDir", sBaseDir).toString();
    sOutFileName   = settings.value("FileTabOutFileName", sOutFileName).toString();
}


void
FileTab::saveSettings() {
    QSettings settings;
    sSampleInfo = pSampleInformationEdit->toPlainText();
    settings.setValue("FileTabSampleInfo", sSampleInfo);
    settings.setValue("FileTabBaseDir", sBaseDir);
    settings.setValue("FileTabOutFileName", sOutFileName);
}


void
FileTab::on_outFilePathButton_clicked() {
    QFileDialog chooseDirDialog;
    QDir outDir(sBaseDir);
    chooseDirDialog.setFileMode(QFileDialog::DirectoryOnly);
    if(outDir.exists())
        chooseDirDialog.setDirectory(outDir);
    else
        chooseDirDialog.setDirectory(QDir::homePath());
    if(chooseDirDialog.exec() == QDialog::Accepted)
        sBaseDir = chooseDirDialog.selectedFiles().at(0);
    pOutPathEdit->setText(sBaseDir);
}


void
FileTab::focusOutEvent(QFocusEvent* event) {
    if(!event->lostFocus()) return;
    pOutFileEdit->text();
    if(sOutFileName == QString()) {
        QMessageBox::information(
                    this,
                    QString("Empty Output Filename"),
                    QString("Please enter a Valid Output File Name"));
        pOutFileEdit->setFocus();
        return;
    }
    if(QDir(sBaseDir).exists(sOutFileName)) {
        int iAnswer = QMessageBox::question(
                    this,
                    QString("File Already exists"),
                    QString("Overwrite %1 ?").arg(sOutFileName),
                    QMessageBox::Yes,
                    QMessageBox::No,
                    QMessageBox::NoButton);
        if(iAnswer == QMessageBox::No) {
            pOutFileEdit->setFocus();
            return;
        }
    }
}
