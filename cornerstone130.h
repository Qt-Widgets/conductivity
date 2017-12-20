#ifndef CORNERSTONE130_H
#define CORNERSTONE130_H

#include <QObject>

class CornerStone130 : public QObject
{
    Q_OBJECT
public:
    explicit CornerStone130(int gpio, int address, QObject *parent = nullptr);
    virtual ~CornerStone130();
    int      init();

signals:

public slots:

private:
  int gpibNumber;
  int cs130Address;
  int cs130;
  char spollByte;
  QString sCommand;
  QString sResponse;
};

#endif // CORNERSTONE130_H
