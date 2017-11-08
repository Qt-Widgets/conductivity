#ifndef CDATASTREAM_H
#define CDATASTREAM_H

#include <QVector>
#include <QColor>

#include "DataSetProperties.h"

class CDataStream {
public:
  CDataStream(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
  CDataStream(CDataSetProperties Properties);
  virtual ~CDataStream(void);

 // Attributes
 public:
   QVector<double> m_pointArray;

 private:
   CDataSetProperties Properties;
   int maxPoints;

 // Operations
 public:
   void setMaxPoints(int nPoints);
   int  getMaxPoints();
   QString GetTitle();
   CDataSetProperties GetProperties();
   void SetProperties(CDataSetProperties newProperties);
   void RemoveAllPoints();
   int  GetId();
   void SetColor(QColor Color);
   void AddPoint(double point);
   bool isShown;
   void SetShow(bool);
   void SetShowTitle(bool show);
   void SetTitle(QString myTitle);
   double miny, maxy;
   bool bShowCurveTitle;
};

#endif // CDATASTREAM_H
