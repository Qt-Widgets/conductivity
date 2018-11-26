#pragma once

#include <QDialog>
#include "k236tab.h"
#include "ls330tab.h"
#include "cs130tab.h"
#include "filetab.h"


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QTabWidget)
QT_FORWARD_DECLARE_CLASS(K236Tab)
QT_FORWARD_DECLARE_CLASS(LS330Tab)
QT_FORWARD_DECLARE_CLASS(CS130Tab)
QT_FORWARD_DECLARE_CLASS(FileTab)


class IvsVDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IvsVDialog(QWidget *parent = nullptr);

signals:

public slots:
    void onCancel();
    void onOk();

public:
    K236Tab          TabK236;
    LS330Tab         TabLS330;
    CS130Tab         TabCS130;
    FileTab          TabFile;

private:
    QTabWidget       *tabWidget;
    QDialogButtonBox *buttonBox;
};

