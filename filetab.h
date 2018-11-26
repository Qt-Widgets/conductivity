#ifndef FILETAB_H
#define FILETAB_H

#include <QObject>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)

class FileTab : public QWidget
{
    Q_OBJECT
public:
    explicit FileTab(QWidget *parent = nullptr);
    void restoreSettings();
    void saveSettings();

signals:

public slots:
    void on_outFilePathButton_clicked();
    void focusOutEvent(QFocusEvent* event);

protected:
    void initUI();
    void setToolTips();

public:
    QString sSampleInfo;
    QString sBaseDir;
    QString sOutFileName;

private:
    QPlainTextEdit *pSampleInformationEdit;
    QLineEdit      *pOutPathEdit;
    QLineEdit      *pOutFileEdit;
};

#endif // FILETAB_H
