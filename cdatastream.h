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
  // Operations
  void setMaxPoints(int nPoints);
  int  getMaxPoints();
  void AddPoint(double point);
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
   QVector<double> m_pointArray;
   double miny;
   double maxy;
   bool bShowCurveTitle;
   bool isShown;

 protected:
   CDataSetProperties Properties;
   int maxPoints;
};

#endif // CDATASTREAM_H
