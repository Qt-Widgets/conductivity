#ifndef FILETAB_H
#define FILETAB_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>


class FileTab : public QWidget
{
    Q_OBJECT
public:
    explicit FileTab(int iConfiguration, QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();
    bool checkFileName();

signals:

public slots:
    void on_outFilePathButton_clicked();

protected:
    void initUI();
    void setToolTips();
    void connectSignals();

public:
    QString sSampleInfo;
    QString sBaseDir;
    QString sOutFileName;

private:
    QPlainTextEdit sampleInformationEdit;
    QLineEdit      outPathEdit;
    QLineEdit      outFileEdit;
    QPushButton    outFilePathButton;

    int            myConfiguration;
};

#endif // FILETAB_H
