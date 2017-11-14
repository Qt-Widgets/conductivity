#ifndef CDATASTREAM2D_H
#define CDATASTREAM2D_H

#include <QVector>
#include <QColor>

#include "cdatastream.h"

class CDataStream2D : protected CDataStream
{
public:
  CDataStream2D(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
  CDataStream2D(CDataSetProperties Properties);
  virtual ~CDataStream2D();
  // Operations
  void setMaxPoints(int nPoints);
  int  getMaxPoints();
  void AddPoint(double pointX, double pointY);
  void RemoveAllPoints();
  int  GetId();
  QString GetTitle();
  CDataSetProperties GetProperties();
  void SetProperties(CDataSetProperties newProperties);
  void SetColor(QColor Color);
  void SetShowTitle(bool show);
  void SetTitle(QString myTitle);
  void SetShow(bool);

 // Attributes
 public:
  QVector<double> m_pointArrayX;
  QVector<double> m_pointArrayY;
   double minx;
   double maxx;
   double miny;
   double maxy;
   bool bShowCurveTitle;
   bool isShown;

 protected:
   CDataSetProperties Properties;
   int maxPoints;
};

#endif // CDATASTREAM2D_H
