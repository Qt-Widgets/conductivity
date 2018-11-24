#pragma once

#include <QDialog>


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QTabWidget)
QT_FORWARD_DECLARE_CLASS(K236Tab)
QT_FORWARD_DECLARE_CLASS(LS330Tab)
QT_FORWARD_DECLARE_CLASS(CS130Tab)


class IvsVDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IvsVDialog(QWidget *parent = nullptr);

signals:

public slots:

public:
    K236Tab          *pK236Tab;
    LS330Tab         *pLS330Tab;
    CS130Tab         *pCS130Tab;

private:
    QTabWidget       *tabWidget;
    QDialogButtonBox *buttonBox;
};

