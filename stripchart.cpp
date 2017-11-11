#include <QPainter>
#include <QPaintEvent>
#include <QSettings>
#include <cmath>
#include <math.h>
#include <float.h>

#include "axesdialog.h"
#include "stripchart.h"


#define NINT(x) ((x) > 0.0 ? (int)floor((x)+0.5) : (int)ceil((x)-0.5))

const int StripChart::iline       = 0;
const int StripChart::ipoint      = 1;
const int StripChart::iplus       = 2;
const int StripChart::iper        = 3;
const int StripChart::istar       = 4;
const int StripChart::iuptriangle = 5;
const int StripChart::idntriangle = 6;
const int StripChart::icircle     = 7;


StripChart::StripChart(QWidget *parent, QString Title)
//  : QWidget(parent)
  : QDialog(parent)
  , sTitle(Title)
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
  setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
//  setAttribute(Qt::WA_AlwaysShowToolTips);
  QSettings settings;
  restoreGeometry(settings.value(sTitle+QString("Plot")).toByteArray());
  xMarker      = 0.0;
  yMarker      = 0.0;
  bShowMarker  = false;
  bZooming     = false;

  labelPen = QPen(Qt::white);
  gridPen  = QPen(Qt::blue);
  framePen = QPen(Qt::blue);

  gridPen.setWidth(1);

  sXCoord.sprintf("X=% 016.7g", 0.0);
  sYCoord.sprintf("Y=% 016.7g", 0.0);

  maxDataPoints = 100;

  setCursor(Qt::CrossCursor);
  setWindowTitle(Title);
}


StripChart::~StripChart() {
  QSettings settings;
  settings.setValue(sTitle+QString("Plot"), saveGeometry());
  while(!dataSetList.isEmpty()) {
    delete dataSetList.takeFirst();
  }
}


void
StripChart::setMaxPoints(int nPoints) {
  if(nPoints > 0) maxDataPoints = nPoints;
  for(int pos=0; pos<dataSetList.count(); pos++) {
    dataSetList.at(pos)->setMaxPoints(maxDataPoints);
  }
}


void
StripChart::NewPoint(int Id, double y) {
  if(std::isnan(y)) return;
  if(dataSetList.isEmpty())  return;
  CDataStream* pData = NULL;
  for(int pos=0; pos<dataSetList.count(); pos++) {
    if(dataSetList.at(pos)->GetId() == Id) {
      pData = dataSetList.at(pos);
      break;
    }
  }
  if(pData) {
    pData->AddPoint(y);
  }
}


QSize
StripChart::minimumSizeHint() const {
  return QSize(50, 50);
}


QSize
StripChart::sizeHint() const {
  return QSize(330, 330);
}



void
StripChart::closeEvent(QCloseEvent *event) {
  QSettings settings;
  settings.setValue(sTitle+QString("Plot"), saveGeometry());
  event->ignore();
}


void
StripChart::SetLimits (double XMin, double XMax, double YMin, double YMax,
                       bool AutoX, bool AutoY, bool LogX, bool LogY)
{
  Ax.XMin  = XMin;
  Ax.XMax  = XMax;
  Ax.YMin  = YMin;
  Ax.YMax  = YMax;
  Ax.AutoX = AutoX;
  Ax.AutoY = AutoY;
  Ax.LogX  = LogX;
  Ax.LogY  = LogY;

  if(!dataSetList.isEmpty()) {
    if(AutoX | AutoY) {
      bool EmptyData = true;
      if(Ax.AutoX) {
        if(Ax.LogX) {
          XMin = FLT_MAX;
          XMax = FLT_MIN;
        } else {
          XMin = FLT_MAX;
          XMax =-FLT_MAX;
        }
      }
      if(Ax.AutoY) {
        if(Ax.LogY) {
          YMin = FLT_MAX;
          YMax = FLT_MIN;
        } else {
          YMin = FLT_MAX;
          YMax =-FLT_MAX;
        }
      }
      CDataStream* pData;
      for(int pos=0; pos<dataSetList.count(); pos++) {
        pData = dataSetList.at(pos);
        if(pData->isShown) {
          if(pData->m_pointArray.count() != 0) {
            EmptyData = false;
            if(Ax.AutoX) {
              if(XMin > 0) {
                XMin = 0;
              }
              if(XMax < pData->m_pointArray.count()) {
                XMax = pData->m_pointArray.count();
              }
            }// if(Ax.AutoX)
            if(Ax.AutoY) {
              if(YMin > pData->miny) {
                YMin = pData->miny;
              }
              if(YMax < pData->maxy) {
                YMax = pData->maxy;
              }
            }// if(Ax.AutoY)
          }// if(pData->m_pointArray.GetSize() != 0)
        }// if(pData->isShowed)
      }// while (pos != NULL)
      if(EmptyData) {
        XMin = Ax.XMin;
        XMax = Ax.XMax;
        YMin = Ax.YMin;
        YMax = Ax.YMax;
      }
    }
  }
  if(XMin == XMax) {
    XMin  -= 0.05*(XMax+XMin)+FLT_MIN;
    XMax  += 0.05*(XMax+XMin)+FLT_MIN;
  }
  if(YMin == YMax) {
    YMin  -= 0.05*(YMax+YMin)+FLT_MIN;
    YMax  += 0.05*(YMax+YMin)+FLT_MIN;
  }
  if(XMin > XMax) {
    double tmp = XMin;
    XMin = XMax;
    XMax = tmp;
  }
  if(YMin > YMax) {
    double tmp = YMin;
    YMin = YMax;
    YMax = tmp;
  }
  if(LogX) {
    if(XMin <= 0.0) XMin = FLT_MIN;
    if(XMax <= 0.0) XMax = 2.0*FLT_MIN;
  }
  if(LogY) {
    if(YMin <= 0.0) YMin = FLT_MIN;
    if(YMax <= 0.0) YMax = 2.0*FLT_MIN;
  }
  Ax.XMin  = XMin;
  Ax.XMax  = XMax;
  Ax.YMin  = YMin;
  Ax.YMax  = YMax;
}


void
StripChart::UpdateChart() {
  update();
}


void
StripChart::ClearChart() {
  while(!dataSetList.isEmpty()) {
    delete dataSetList.takeFirst();
  }
  update();
}


bool
StripChart::DelDataSet(int Id) {
  bool result = false;
  if(dataSetList.isEmpty()) return result;
  CDataStream* pData;
  for(int i=0; i<dataSetList.count(); i++) {
    pData = dataSetList.at(i);
    if(pData->GetId() == Id) {
      dataSetList.removeAt(i);
      result = true;
      break;
    }
  }
  return result;
}


void
StripChart::paintEvent(QPaintEvent *event) {
  QPainter painter;
  painter.begin(this);
  //painter.setRenderHint(QPainter::Antialiasing);
  DrawPlot(&painter, event);
  painter.end();
}


void
StripChart::XTicLin(QPainter* painter, QFontMetrics fontMetrics) {
  double xmax, xmin;
  double dx, dxx, b, fmant;
  int isx, ic, iesp, jy, isig, ix, ix0, iy0;
  QString Label;

  if (Ax.XMax <= 0.0) {
    xmax =-Ax.XMin;	xmin=-Ax.XMax; isx= -1;
  } else {
    xmax = Ax.XMax; xmin= Ax.XMin; isx= 1;
  }
  dx = xmax - xmin;
  b = log10(dx);
  ic = NINT(b) - 2;
  dx = (double)NINT(pow(10.0, (b-ic-1.0)));

  if(dx < 11.0) dx = 10.0;
  else if(dx < 28.0) dx = 20.0;
  else if(dx < 70.0) dx = 50.0;
  else dx = 100.0;

  dx = dx * pow(10.0, (double)ic);
  xfact = (Pf.r-Pf.l) / (xmax-xmin);
  dxx = (xmax+dx) / dx;
  dxx = floor(dxx) * dx;
  iy0 = int(Pf.b + fontMetrics.height()+5);
  iesp = int(floor(log10(dxx)));
  if (dxx > xmax) dxx = dxx - dx;
  do {
    if(isx == -1)
      ix = int(Pf.r-(dxx-xmin) * xfact);
    else
      ix = int((dxx-xmin) * xfact + Pf.l);
    jy = int(Pf.b + 5);// Perche' 5 ?
    painter->setPen(gridPen);
    painter->drawLine(QLine(ix, int(Pf.t), ix, jy));
    isig = 0;
    if(dxx == 0.0)
      fmant= 0.0;
    else {
      isig = int(dxx/fabs(dxx));
      dxx = fabs(dxx);
      fmant = log10(dxx) - double(iesp);
      fmant = pow(10.0, fmant)*10000.0 + 0.5;
      fmant = floor(fmant)/10000.0;
      fmant = isig * fmant;
    }
    if((double)isx*fmant <= -10.0)
      Label.sprintf("% 6.2f", (double)isx*fmant);
    else
      Label.sprintf("% 6.3f", (double)isx*fmant);
    ix0 = ix - fontMetrics.width(Label)/2;
    painter->setPen(labelPen);
    painter->drawText(QPoint(ix0, iy0), Label);
    dxx = isig*dxx - dx;
  }	while(dxx >= xmin);
  painter->setPen(labelPen);
  painter->drawText(QPoint(int(Pf.r + 2),	int(Pf.b - 0.5*fontMetrics.height())), "x10");
  int icx = fontMetrics.width("x10 ");
  Label.sprintf("%-3i", iesp);
  painter->setPen(labelPen);
  painter->drawText(QPoint(int(Pf.r+icx),	int(Pf.b - fontMetrics.height())), Label);
}


void
StripChart::YTicLin(QPainter* painter, QFontMetrics fontMetrics) {
  double ymax, ymin;
  double dy, dyy, b, fmant;
  int isy, icc, iesp, jx, isig, iy, ix0, iy0;
  QString Label;

  if (Ax.YMax <= 0.0) {
    ymax = -Ax.YMin; ymin= -Ax.YMax; isy= -1;
  } else {
    ymax = Ax.YMax; ymin= Ax.YMin; isy= 1;
  }
  dy = ymax - ymin;
  b = log10(dy);
  icc = NINT(b) - 2;
  dy = (double)NINT(pow(10.0, (b-icc-1.0)));

  if(dy < 11.0) dy = 10.0;
  else if(dy < 28.0) dy = 20.0;
  else if(dy < 70.0) dy = 50.0;
  else dy = 100.0;

  dy = dy * pow(10.0, (double)icc);
  yfact = (Pf.t-Pf.b) / (ymax-ymin);
  dyy = (ymax+dy) / dy;
  dyy = floor(dyy) * dy;
  iesp = int(floor(log10(dyy)));
  if(dyy > ymax) dyy = dyy - dy;
  do {
    if(isy == -1)
      iy = int(Pf.t - (dyy-ymin) * yfact);
    else
      iy = int((dyy-ymin) * yfact + Pf.b);
    jx = int(Pf.r);
    painter->setPen(gridPen);
    painter->drawLine(QLine(int(Pf.l-5), iy, jx, iy));
    isig = 0;
    if(dyy == 0.0)
      fmant = 0.0;
    else{
      isig = int(dyy/fabs(dyy));
      dyy = fabs(dyy);
      fmant = log10(dyy) - double(iesp);
      fmant = pow(10.0, fmant)*10000.0 + 0.5;
      fmant = floor(fmant)/10000.0;
      fmant = isig * fmant;
    }
    if((double)isy*fmant <= -10.0)
      Label.sprintf("% 7.3f", (double)isy*fmant);
    else
      Label.sprintf("% 7.4f", (double)isy*fmant);
    ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
    iy0 = iy + fontMetrics.height()/2;
    painter->setPen(labelPen);
    painter->drawText(QPoint(ix0, iy0), Label);
    dyy = isig*dyy - dy;
  }	while (dyy >= ymin);
  QPoint point(int(Pf.l), int(Pf.t-0.5*fontMetrics.height()));
  painter->setPen(labelPen);
  painter->drawText(point, "x10");
  int icx = fontMetrics.width("x10 ");
  Label.sprintf("%-3i", iesp);
  painter->setPen(labelPen);
  painter->drawText(QPoint(int(int(Pf.l)+icx),int(Pf.t-fontMetrics.height())),Label);
}


void
StripChart::XTicLog(QPainter* painter, QFontMetrics fontMetrics) {
  int i, ix, ix0, iy0, jy, j;
  double dx;
  QString Label;

  jy = int(Pf.b + 5);// Perche' 5 ?
  iy0 = int(Pf.b + fontMetrics.height()+5);

  if(Ax.XMin < FLT_MIN) Ax.XMin = FLT_MIN;
  if(Ax.XMax < FLT_MIN) Ax.XMax = 10.0*FLT_MIN;

  double xlmin = log10(Ax.XMin);
  int minx = int(xlmin);
  if((xlmin < 0.0) && (xlmin != minx)) minx= minx - 1;

  double xlmax = log10(Ax.XMax);
  int maxx = int(xlmax);
  if((xlmax > 0.0) && (xlmax != maxx)) maxx= maxx + 1;

  xfact = (Pf.r-Pf.l) / ((xlmax-xlmin)+FLT_MIN);

  bool init = true;
  int decades = maxx - minx;
  double x = pow(10.0, minx);
  if(decades < 6) {
    for(i=0; i<decades; i++) {
      dx = pow(10.0, (minx + i));
      if(x >= Ax.XMin) {
        ix = int(Pf.l + (log10(x)-xlmin)*xfact);
        Label.sprintf("%7.0e", x);
        ix0 = ix - fontMetrics.width(Label)/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
        init = false;
      }
      for(j=1; j<10; j++){
        x = x + dx;
        if((x >= Ax.XMin) && (x <= Ax.XMax)) {
          ix = int(Pf.l + (log10(x)-xlmin)*xfact);
          painter->setPen(gridPen);
          painter->drawLine(QLine(ix, int(Pf.t), ix, jy));
          Label.sprintf("%7.0e", x);
          if(init || (j == 9 && decades == 1)) {
            ix0 = ix - fontMetrics.width(Label)/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
            init = false;
          } else if (decades == 1) {
            Label = Label.left(2);
            ix0 = ix - fontMetrics.width(Label)/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
          }
        }
      }
    }// for(i=0; i<decades; i++)
    if((decades != 1) && (x <= Ax.XMax)) {
      Label.sprintf("%7.0e", x);
      ix = int(Pf.l + (log10(x)-xlmin)*xfact);
      ix0 = ix - fontMetrics.width(Label)/2;
      painter->setPen(labelPen);
      painter->drawText(QPoint(ix0, iy0), Label);
    }
  } else {// decades > 5
    for(i=1; i<=decades; i++) {
      x = pow(10.0, minx + i);
      if((x >= Ax.XMin) && (x <= Ax.XMax)) {
        ix = int(Pf.l + (log10(x)-xlmin)*xfact);
        painter->setPen(gridPen);
        painter->drawLine(QLine(ix, int(Pf.t),ix, jy));
        Label.sprintf("%7.0e", x);
        ix0 = ix - fontMetrics.width(Label)/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
      }
    }
  }//if(decades < 6)
}


void
StripChart::YTicLog(QPainter* painter, QFontMetrics fontMetrics) {
  int i, iy, ix0, iy0, j;
  double dy;
  QString Label;

  if(Ax.YMin < FLT_MIN) Ax.YMin = FLT_MIN;
  if(Ax.YMax < FLT_MIN) Ax.YMax = 10.0*FLT_MIN;

  double ylmin = log10(Ax.YMin);
  int miny = int(ylmin);
  if((ylmin < 0.0) && (ylmin != miny)) miny= miny - 1;

  double ylmax = log10(Ax.YMax);
  int maxy = int(ylmax);
  if((ylmax > 0.0) && (ylmax != maxy)) maxy= maxy + 1;

  yfact = (Pf.t-Pf.b) / ((ylmax-ylmin)+FLT_MIN);

  bool init = true;
  int decades = maxy - miny;
  double y = pow(10.0, miny);
  if(decades < 6) {
    for(i=0; i<decades; i++) {
      dy = pow(10.0, (miny + i));
      if(y >= Ax.YMin) {
        iy = int(Pf.b + (log10(y)-ylmin)*yfact);
        Label.sprintf("%7.0e", y);
        ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
        iy0 = iy + fontMetrics.height()/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
        init = false;
      }
      for(j=1; j<10; j++){
        y = y + dy;
        if((y >= Ax.YMin) && (y <= Ax.YMax)) {
          iy = int(Pf.b + (log10(y)-ylmin)*yfact);
          painter->setPen(gridPen);
          painter->drawLine(QLine(int(Pf.l-5), iy, int(Pf.r), iy));
          Label.sprintf("%7.0e", y);
          if(init || (j == 9 && decades == 1)) {
            ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
            iy0 = iy + fontMetrics.height()/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
            init = false;
          } else if (decades == 1) {
            Label = Label.left(2);
            ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
            iy0 = iy + fontMetrics.height()/2;
            painter->setPen(labelPen);
            painter->drawText(QPoint(ix0, iy0), Label);
          }
        }
      }
    }// for(i=0; i<decades; i++)
    if((decades != 1) && (y <= Ax.YMax)) {
      Label.sprintf("%7.0e", y);
      iy = int(Pf.b - (log10(y)-ylmin)*yfact);
      ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
      iy0 = iy + fontMetrics.height()/2;
      painter->setPen(labelPen);
      painter->drawText(QPoint(ix0, iy0), Label);
    }
  } else {// decades > 5
    for(i=1; i<=decades; i++) {
      y = pow(10.0, miny + i);
      if((y >= Ax.YMin) && (y <= Ax.YMax)) {
        iy = int(Pf.b + (log10(y)-ylmin)*yfact);
        painter->setPen(gridPen);
        painter->drawLine(QLine(int(Pf.l-5), iy, int(Pf.r), iy));
        Label.sprintf("%7.0e", y);
        ix0 = int(Pf.l - fontMetrics.width(Label) - 5);
        iy0 = iy + fontMetrics.height()/2;
        painter->setPen(labelPen);
        painter->drawText(QPoint(ix0, iy0), Label);
      }
    }
  }//if(decades < 6)
}


void
StripChart::DrawFrame(QPainter* painter, QFontMetrics fontMetrics) {
  if(Ax.LogX) XTicLog(painter, fontMetrics); else XTicLin(painter, fontMetrics);
  if(Ax.LogY) YTicLog(painter, fontMetrics); else YTicLin(painter, fontMetrics);

  painter->setPen(framePen);
  painter->drawLine(QLine(int(Pf.l), int(Pf.b), int(Pf.r), int(Pf.b)));
  painter->drawLine(QLine(int(Pf.r), int(Pf.b), int(Pf.r), int(Pf.t)));
  painter->drawLine(QLine(int(Pf.r), int(Pf.t), int(Pf.l), int(Pf.t)));
  painter->drawLine(QLine(int(Pf.l), int(Pf.t), int(Pf.l), int(Pf.b)));

  painter->setPen(labelPen);
  int icx = fontMetrics.width((sTitle));
  painter->drawText(QPoint(int((width()-icx)/2), int(fontMetrics.height())), sTitle);
}


void
StripChart::DrawData(QPainter* painter, QFontMetrics fontMetrics) {
  if(dataSetList.isEmpty()) return;
  CDataStream* pData;
  for(int pos=0; pos<dataSetList.count(); pos++) {
    pData = dataSetList.at(pos);
    if(pData->isShown) {
      if(pData->GetProperties().Symbol == iline) {
        LinePlot(painter, pData);
      } else if(pData->GetProperties().Symbol == ipoint) {
        PointPlot(painter, pData);
      } else {
        ScatterPlot(painter, pData);
      }
      if(pData->bShowCurveTitle) ShowTitle(painter, fontMetrics, pData);
    }
  }
}


void
StripChart::LinePlot(QPainter* painter, CDataStream* pData) {
  if(!pData->isShown) return;
  int iMax = int(pData->m_pointArray.count());
  if(iMax == 0) return;
  QPen dataPen = QPen(pData->GetProperties().Color);
  dataPen.setWidth(pData->GetProperties().PenWidth);
  painter->setPen(dataPen);
  int ix0, iy0, ix1, iy1;
  double xlmin, ylmin;
  if(Ax.XMin > 0.0)
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  if(Ax.LogX) {
    ix0 = int(((log10(1.0) - xlmin)*xfact) + Pf.l);
  } else
    ix0 = int(((1.0 - Ax.XMin)*xfact) + Pf.l);
  if(Ax.LogY) {
    if(pData->m_pointArray[0] > 0.0)
      iy0 = int((Pf.b + (log10(pData->m_pointArray[0]) - ylmin)*yfact));
    else
      iy0 =-INT_MAX; // Solo per escludere il punto
  } else
    iy0 = int((Pf.b + (pData->m_pointArray[0] - Ax.YMin)*yfact));

  for(int i=1; i<iMax; i++) {
    if(Ax.LogX)
      ix1 = int(((log10(i+1) - xlmin)*xfact) + Pf.l);
    else
      ix1 = int(((i+1 - Ax.XMin)*xfact) + Pf.l);
    if(Ax.LogY)
      if(pData->m_pointArray[i] > 0.0)
        iy1 = int((Pf.b + (log10(pData->m_pointArray[i]) - ylmin)*yfact));
      else
        iy1 =-INT_MAX; // Solo per escludere il punto
    else
      iy1 = int((Pf.b + (pData->m_pointArray[i] - Ax.YMin)*yfact));

    if(!(ix1<Pf.l || iy1<Pf.t || iy1>Pf.b)) {
      painter->drawLine(ix0, iy0, ix1, iy1);
    }
    ix0 = ix1;
    iy0 = iy1;
    if(ix1 > Pf.r) {
      break;
    }
  }
  DrawLastPoint(painter, pData);
}


void
StripChart::DrawLastPoint(QPainter* painter, CDataStream* pData) {
  if(!pData->isShown) return;
  int ix, iy, i;
  i = int(pData->m_pointArray.count()-1);

  double xlmin, ylmin;
  if(Ax.XMin > 0.0)
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  if(Ax.LogX) {
    ix = int(((log10(pData->m_pointArray.count()) - xlmin)*xfact) + Pf.l);
  } else
    ix = int(((pData->m_pointArray.count() - Ax.XMin)*xfact) + Pf.l);
  if(Ax.LogY) {
    if(pData->m_pointArray[i] > 0.0)
      iy = int((Pf.b + (log10(pData->m_pointArray[i]) - ylmin)*yfact));
    else
      return;
  } else
    iy = int((Pf.b + (pData->m_pointArray[i] - Ax.YMin)*yfact));
    if(ix<=Pf.r && ix>=Pf.l && iy>=Pf.t && iy<=Pf.b)
      painter->drawPoint(ix, iy);
  return;
}


void
StripChart::PointPlot(QPainter* painter, CDataStream* pData) {
  int iMax = int(pData->m_pointArray.count());
  if(iMax == 0) return;
  QPen dataPen = QPen(pData->GetProperties().Color);
  dataPen.setWidth(pData->GetProperties().PenWidth);
  painter->setPen(dataPen);
  int ix, iy;
  double xlmin, ylmin;
  if(Ax.XMin > 0.0)
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  for (int i=0; i < iMax; i++) {
    if(!(pData->m_pointArray[i] < Ax.YMin || pData->m_pointArray[i] > Ax.YMax )) {
      if(Ax.LogX) {
        ix = int(((log10(i+1) - xlmin)*xfact) + Pf.l);
      } else
        ix = int(((i+1 - Ax.XMin)*xfact) + Pf.l);
      if(Ax.LogY) {
        if(pData->m_pointArray[i] > 0.0)
          iy = int((Pf.b + (log10(pData->m_pointArray[i]) - ylmin)*yfact));
        else
          iy =-INT_MAX; // Solo per escludere il punto
      } else
        iy = int((Pf.b + (pData->m_pointArray[i] - Ax.YMin)*yfact));
      painter->drawPoint(ix, iy);
    }
  }//for (int i=0; i <= iMax; i++)

}


void
StripChart::ScatterPlot(QPainter* painter, CDataStream* pData) {
  int iMax = int(pData->m_pointArray.count());
  if(iMax == 0) return;
  QPen dataPen = QPen(pData->GetProperties().Color);
  dataPen.setWidth(pData->GetProperties().PenWidth);
  painter->setPen(dataPen);
  int ix, iy;

  double xlmin, ylmin;
  if(Ax.XMin > 0.0)
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  int SYMBOLS_DIM = 8;
  QSize Size(SYMBOLS_DIM, SYMBOLS_DIM);

  for (int i=0; i < iMax; i++) {
    if(pData->m_pointArray[i] >= Ax.YMin &&
       pData->m_pointArray[i] <= Ax.YMax) {
      if(Ax.LogX)
        ix = int(((log10(i+1) - xlmin)*xfact) + Pf.l);
      else//Asse X Lineare
        ix= int(((i+1 - Ax.XMin)*xfact) + Pf.l);
      if(Ax.LogY) {
        if(pData->m_pointArray[i] > 0.0)
          iy = int(((log10(pData->m_pointArray[i]) - ylmin)*yfact) + Pf.b);
        else
          iy =-INT_MAX; // Solo per escludere il punto
      } else
        iy = int(((pData->m_pointArray[i] - Ax.YMin)*yfact) + Pf.b);

      if(pData->GetProperties().Symbol == iplus) {
        painter->drawLine(ix, iy-Size.height()/2, ix, iy+Size.height()/2+1);
        painter->drawLine(ix-Size.width()/2, iy, ix+Size.width()/2+1, iy);
      } else if(pData->GetProperties().Symbol == iper) {
        painter->drawLine(ix-Size.width()/2+1, iy+Size.height()/2-1, ix+Size.width()/2-1, iy-Size.height()/2);
        painter->drawLine(ix+Size.width()/2-1, iy+Size.height()/2-1, ix-Size.width()/2+1, iy-Size.height()/2);
      } else if(pData->GetProperties().Symbol == istar) {
        painter->drawLine(ix, iy-Size.height()/2, ix, iy+Size.height()/2+1);
        painter->drawLine(ix-Size.width()/2, iy, ix+Size.width()/2+1, iy);
        painter->drawLine(ix-Size.width()/2+1, iy+Size.height()/2-1, ix+Size.width()/2-1, iy-Size.height()/2);
        painter->drawLine(ix+Size.width()/2-1, iy+Size.height()/2-1, ix-Size.width()/2+1, iy-Size.height()/2);
      } else if(pData->GetProperties().Symbol == iuptriangle) {
        painter->drawLine(ix, iy-Size.height()/2, ix+Size.width()/2, iy+Size.height()/2);
        painter->drawLine(ix+Size.width()/2, iy+Size.height()/2, ix-Size.width()/2, iy+Size.height()/2);
        painter->drawLine(ix-Size.width()/2, iy+Size.height()/2, ix, iy-Size.height()/2);
      } else if(pData->GetProperties().Symbol == idntriangle) {
        painter->drawLine(ix, iy+Size.height()/2, ix+Size.width()/2, iy-Size.height()/2);
        painter->drawLine(ix+Size.width()/2, iy-Size.height()/2, ix-Size.width()/2, iy-Size.height()/2);
        painter->drawLine(ix-Size.width()/2, iy-Size.height()/2, ix, iy+Size.height()/2);
      } else if(pData->GetProperties().Symbol == icircle) {
        painter->drawEllipse(QRect(ix-Size.width()/2, iy-Size.height()/2, Size.width(), Size.height()));
      } else {
        painter->drawLine(ix-Size.width()/2, iy, ix-Size.width()/2, iy-Size.height());
        painter->drawLine(ix, iy-Size.height()/2, ix-Size.width(), iy-Size.height()/2);
      }
    }
  }
}


void
StripChart::ShowTitle(QPainter* painter, QFontMetrics fontMetrics, CDataStream *pData) {
  QPen titlePen = QPen(pData->GetProperties().Color);
  painter->setPen(titlePen);
  painter->drawText(int(Pf.r+4), int(Pf.t+fontMetrics.height()*(pData->GetId())), pData->GetTitle());
}


void
StripChart::SetShowDataSet(int Id, bool Show) {
  if(!dataSetList.isEmpty()) {
    for(int pos=0; pos<dataSetList.count(); pos++) {
      CDataStream* pData = dataSetList.at(pos);
      if(pData->GetId() == Id) {
        pData->SetShow(Show);
        break;
      }
    }
  }
}


void
StripChart::SetShowTitle(int Id, bool show) {
  if(dataSetList.isEmpty()) return;
  CDataStream* pData;
  for(int pos=0; pos<dataSetList.count(); pos++) {
    pData = dataSetList.at(pos);
    if(pData->GetId() == Id) {
      pData->SetShowTitle(show);
      return;
    }
  }
}


CDataStream*
StripChart::NewDataSet(int Id, int PenWidth, QColor Color, int Symbol, QString Title) {
  CDataStream* pDataItem = new CDataStream(Id, PenWidth, Color, Symbol, Title);
  pDataItem->setMaxPoints(maxDataPoints);
  dataSetList.append(pDataItem);
  return pDataItem;
}


void
StripChart::DrawPlot(QPainter* painter, QPaintEvent *event) {
  if(Ax.AutoX || Ax.AutoY) {
    SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
  }
  painter->fillRect(event->rect(), QBrush(QColor(0, 0, 0)));
  painter->setFont(QFont("Helvetica", 8, QFont::Normal));
  QFontMetrics fontMetrics = painter->fontMetrics();

  Pf.l = fontMetrics.width("-0.00000") + 2.0;
  Pf.r = width() - fontMetrics.width("x10-999") - 5.0;
  Pf.t = 2.0 * fontMetrics.height();
  Pf.b = height() - 3.0*fontMetrics.height();

  DrawFrame(painter, fontMetrics);
  DrawData(painter, fontMetrics);
  if(bZooming) {
    QPen zoomPen(Qt::yellow);
    painter->setPen(zoomPen);
    int ix0 = zoomStart.rx() < zoomEnd.rx() ? zoomStart.rx() : zoomEnd.rx();
    int iy0 = zoomStart.ry() < zoomEnd.ry() ? zoomStart.ry() : zoomEnd.ry();
    painter->drawRect(ix0, iy0, abs(zoomStart.rx()-zoomEnd.rx()), abs(zoomStart.ry()-zoomEnd.ry()));
  }
}


void
StripChart::mousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::RightButton) {


  } else if (event->buttons() & Qt::LeftButton) {
    if(event->modifiers() & Qt::ShiftModifier) {
      setCursor(Qt::SizeAllCursor);
      zoomStart = event->pos();
      bZooming = true;
    } else {
      setCursor(Qt::OpenHandCursor);
      lastPos = event->pos();
    }
  }
  event->accept();
}


void
StripChart::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() & Qt::RightButton) {

    event->accept();
  } else if (event->button() & Qt::LeftButton) {
    if(bZooming) {
      bZooming = false;
      QPoint distance = zoomStart-zoomEnd;
      if(abs(distance.rx()) < 10 || abs(distance.ry()) < 10) return;
      double x1, x2, y1, y2, tmp;
      if(Ax.LogX) {
        x1 = pow(10.0, log10(Ax.XMin)+(zoomEnd.rx()-Pf.l)/xfact);
        x2 = pow(10.0, log10(Ax.XMin)+(zoomStart.rx()-Pf.l)/xfact);
      } else {
        x1 = (zoomEnd.rx()-Pf.l)/xfact + Ax.XMin;
        x2 = (zoomStart.rx()-Pf.l)/xfact + Ax.XMin;
      }
      if(Ax.LogY) {
        y1 = pow(10.0, log10(Ax.YMin)+(zoomEnd.ry()-Pf.b)/yfact);
        y2 = pow(10.0, log10(Ax.YMin)+(zoomStart.ry()-Pf.b)/yfact);
      } else {
        y1 = (zoomEnd.ry()-Pf.b) / yfact + Ax.YMin;
        y2 = (zoomStart.ry()-Pf.b) / yfact + Ax.YMin;
      }
      if(x2<x1) {
        tmp = x2;
        x2 = x1;
        x1 = tmp;
      }
      if(y2<y1) {
        tmp = y2;
        y2 = y1;
        y1 = tmp;
      }
      SetLimits(x1, x2, y1, y2, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    }
    event->accept();
  }
  UpdateChart();
  setCursor(Qt::CrossCursor);
}


void
StripChart::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    if(!bZooming) {
      double xmin, xmax, ymin, ymax;
      double dxPix = event->pos().rx() - lastPos.rx();
      double dyPix = event->pos().ry() - lastPos.ry();
      double dx    = dxPix / xfact;
      double dy    = dyPix / yfact;
      if(Ax.LogX) {
        xmin = pow(10.0, log10(Ax.XMin)-dx);
        xmax = pow(10.0, log10(Ax.XMax)-dx);
      } else {
        xmin = Ax.XMin - dx;
        xmax = Ax.XMax - dx;
      }
      if(Ax.LogY) {
        ymin = pow(10.0, log10(Ax.YMin)-dy);
        ymax = pow(10.0, log10(Ax.YMax)-dy);
      } else {
        ymin = Ax.YMin - dy;
        ymax = Ax.YMax - dy;
      }
      lastPos = event->pos();
      SetLimits (xmin, xmax, ymin, ymax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
      UpdateChart();
    } else {// is Zooming
      zoomEnd = event->pos();
      UpdateChart();
    }
  } else {
    double xval, yval;
    if(Ax.LogX) {
      xval = pow(10.0, log10(Ax.XMin)+(event->pos().rx()-Pf.l)/xfact);
    }else {
      xval =Ax.XMin + (event->pos().rx()-Pf.l) / xfact;
    }
    if(Ax.LogY) {
      yval = pow(10.0, log10(Ax.YMin)+(event->pos().ry()-Pf.b)/yfact);
    } else {
      yval =Ax.YMin + (event->pos().ry()-Pf.b) / yfact;
    }
    sXCoord.sprintf("X=% -10.7g", xval);
    sYCoord.sprintf("Y=% -10.7g", yval);
    //>>>>>>>>>>>>>SetTimer(MOUSETIMER, CURSORUPDATETIME, NULL);
  }
//  setToolTip(sXCoord + " " + sYCoord);
  event->accept();
}


void
StripChart::mouseDoubleClickEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  AxesDialog axesDialog(this);
  axesDialog.initDialog(Ax);
  int iRes = axesDialog.exec();
  if(iRes==QDialog::Accepted) {
    Ax = axesDialog.newLimits;
    SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    UpdateChart();
  }
}

