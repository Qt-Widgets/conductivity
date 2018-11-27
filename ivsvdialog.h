#pragma once

#include "k236tab.h"
#include "ls330tab.h"
#include "cs130tab.h"
#include "filetab.h"

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>


class IvsVDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IvsVDialog(QWidget *parent = nullptr);

signals:

public slots:
    void onCancel();
    void onOk();

protected:
    void connectSignals();
    void setToolTips();

public:
    K236Tab  TabK236;
    LS330Tab TabLS330;
    CS130Tab TabCS130;
    FileTab  TabFile;

private:
    QTabWidget       tabWidget;
    QDialogButtonBox buttonBox;
    int iSourceIndex;
    int iThermIndex;
    int iMonoIndex;
    int iFileIndex;
};

