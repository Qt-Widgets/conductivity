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
    explicit FileTab(QWidget *parent = nullptr);
    ~FileTab() Q_DECL_OVERRIDE;
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
};

#endif // FILETAB_H
