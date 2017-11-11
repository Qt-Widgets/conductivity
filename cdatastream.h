/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
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
