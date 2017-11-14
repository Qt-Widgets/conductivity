#ifndef PLOT2D_H
#define PLOT2D_H
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

#include "stripchart.h"
#include "cdatastream2d.h"
#include "AxisLimits.h"
#include "AxisFrame.h"

#include <QDialog>
#include <QPen>


class Plot2D : public QDialog
{
  Q_OBJECT
public:
  explicit Plot2D(QWidget *parent=0, QString Title="Plot 2D");
  ~Plot2D();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  void SetLimits (double XMin, double XMax, double YMin, double YMax,
                  bool AutoX, bool AutoY, bool LogX, bool LogY);
  CDataStream2D* NewDataSet(int Id, int PenWidth, QColor Color, int Symbol, QString Title);
  bool DelDataSet(int Id);
  void NewPoint(int Id, double x, double y);
  void SetShowDataSet(int Id, bool Show);
  void SetShowTitle(int Id, bool show);
  void UpdatePlot();
  void ClearPlot();
  void setMaxPoints(int nPoints);
  int  getMaxPoints();

signals:

public slots:

public:
  const int iline;
  const int ipoint;
  const int iplus;
  const int iper;
  const int istar;
  const int iuptriangle;
  const int idntriangle;
  const int icircle;

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
  void LinePlot(QPainter* painter, CDataStream2D *pData);
  void PointPlot(QPainter* painter, CDataStream2D* pData);
  void ScatterPlot(QPainter* painter, CDataStream2D* pData);
  void DrawLastPoint(QPainter* painter, CDataStream2D* pData);
  void ShowTitle(QPainter* painter, QFontMetrics fontMetrics, CDataStream2D* pData);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
//  void wheelEvent(QWheelEvent* event);

protected:
  QList<CDataStream2D*> dataSetList;
  QPen labelPen;
  QPen gridPen;
  QPen framePen;

  int maxDataPoints;
  bool bZooming;
  bool bShowMarker;
  double xMarker, yMarker;
  CAxisLimits Ax;
  CAxisFrame Pf;
  QString sTitle;
  QString sXCoord, sYCoord;
  double xfact, yfact;
  QPoint lastPos, zoomStart, zoomEnd;
};

#endif // PLOT2D_H
