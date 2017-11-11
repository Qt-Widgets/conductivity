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
#ifndef STRIPCHART_H
#define STRIPCHART_H

#include <QWidget>
#include <QDialog>
#include <QPen>

#include "AxisLimits.h"
#include "AxisFrame.h"
#include "cdatastream.h"

//class StripChart : public QWidget
class StripChart : public QDialog
{
  Q_OBJECT
public:
  explicit StripChart(QWidget *parent=0, QString Title="Strip Chart");
  ~StripChart();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

public:
  void SetLimits (double XMin, double XMax, double YMin, double YMax,
                  bool AutoX, bool AutoY, bool LogX, bool LogY);
  CDataStream* NewDataSet(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
  bool DelDataSet(int Id);
  void NewPoint(int Id, double y);
  void SetShowDataSet(int Id, bool Show);
  void SetShowTitle(int Id, bool show);
  void UpdateChart();
  void ClearChart();
  void setMaxPoints(int nPoints);
  int  getMaxPoints();

public:
  static const int ipoint;
  static const int iline;
  static const int iplus;
  static const int iper;
  static const int istar;
  static const int iuptriangle;
  static const int idntriangle;
  static const int icircle;

signals:

public slots:

protected:
  void closeEvent(QCloseEvent *event);
  void paintEvent(QPaintEvent *event);
  void DrawPlot(QPainter* painter, QPaintEvent *event);
  void DrawFrame(QPainter* painter, QFontMetrics fontMetrics);
  void XTicLin(QPainter* painter, QFontMetrics fontMetrics);
  void XTicLog(QPainter* painter, QFontMetrics fontMetrics);
  void YTicLin(QPainter* painter, QFontMetrics fontMetrics);
  void YTicLog(QPainter* painter, QFontMetrics fontMetrics);
  void DrawData(QPainter* painter, QFontMetrics fontMetrics);
  void LinePlot(QPainter* painter, CDataStream *pData);
  void PointPlot(QPainter* painter, CDataStream* pData);
  void ScatterPlot(QPainter* painter, CDataStream* pData);
  void DrawLastPoint(QPainter* painter, CDataStream* pData);
  void ShowTitle(QPainter* painter, QFontMetrics fontMetrics, CDataStream* pData);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
//  void wheelEvent(QWheelEvent* event);


protected:
  QPen labelPen;
  QPen gridPen;
  QPen framePen;

  int maxDataPoints;
  bool bZooming;
  bool bShowMarker;
  double xMarker, yMarker;
  QList<CDataStream*> dataSetList;
  CAxisLimits Ax;
  CAxisFrame Pf;

private:
  QString sTitle;
  QString sXCoord, sYCoord;
  double xfact, yfact;
  QPoint lastPos, zoomStart, zoomEnd;
};

#endif // STRIPCHART_H
