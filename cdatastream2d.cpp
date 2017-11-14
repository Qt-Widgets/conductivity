#include "cdatastream2d.h"
#include <float.h>

CDataStream2D::CDataStream2D(int Id, int PenWidth, QColor Color, int Symbol, QString Title)
  : CDataStream(Id, PenWidth, Color, Symbol, Title)
{
  Properties.SetId(Id);
  Properties.Color    = Color;
  Properties.PenWidth = PenWidth;
  Properties.Symbol   = Symbol;
  if(Title != QString())
    Properties.Title = Title;
  else
    Properties.Title.sprintf("Data Set %d", Properties.GetId());
  isShown         = false;
  bShowCurveTitle = false;
  maxPoints = 100;
}


CDataStream2D::CDataStream2D(CDataSetProperties myProperties)
  : CDataStream(myProperties)
{
  Properties = myProperties;
  if(myProperties.Title == QString())
    Properties.Title.sprintf("Data Set %d", Properties.GetId());
  isShown         = false;
  bShowCurveTitle = false;
  maxPoints = 100;
}


CDataStream2D::~CDataStream2D() {
}


void
CDataStream2D::SetShow(bool show) {
  isShown = show;
}


void
CDataStream2D::AddPoint(double x, double y) {
  m_pointArrayX.append(x);
  m_pointArrayY.append(y);
  if(m_pointArrayX.count() == 1) {
    minx = x-FLT_MIN;
    maxx = x+FLT_MIN;
    miny = y-FLT_MIN;
    maxy = y+FLT_MIN;
  }
  if(m_pointArrayX.count() > maxPoints) {
    m_pointArrayX.removeFirst();
    m_pointArrayX.removeFirst();
    minx = x-FLT_MIN;
    maxx = x+FLT_MIN;
    miny = y-FLT_MIN;
    maxy = y+FLT_MIN;
    for(int i=0; i< m_pointArray.count(); i++) {
      if(m_pointArrayX.at(i) < minx) minx = m_pointArrayX.at(i);
      if(m_pointArrayX.at(i) > maxx) maxx = m_pointArrayX.at(i);
      if(m_pointArrayY.at(i) < miny) miny = m_pointArrayY.at(i);
      if(m_pointArrayY.at(i) > maxy) maxy = m_pointArrayY.at(i);
    }
  }
  else {
    minx = minx < x ? minx : x-FLT_MIN;
    maxx = maxx > x ? maxx : x+FLT_MIN;
    miny = miny < y ? miny : y-FLT_MIN;
    maxy = maxy > y ? maxy : y+FLT_MIN;
  }
}


void
CDataStream2D::SetColor(QColor Color) {
  CDataStream::SetColor(Color);
}


int
CDataStream2D::GetId() {
  return CDataStream::GetId();
}


void
CDataStream2D::RemoveAllPoints() {
  m_pointArrayX.clear();
  m_pointArrayY.clear();
}


void
CDataStream2D::SetTitle(QString myTitle) {
  CDataStream::SetTitle(myTitle);
}


void
CDataStream2D::SetShowTitle(bool show) {
  CDataStream::SetShowTitle(show);
}


CDataSetProperties
CDataStream2D::GetProperties() {
  return CDataStream::GetProperties();
}


void
CDataStream2D::SetProperties(CDataSetProperties newProperties) {
  CDataStream::SetProperties(newProperties);
}



QString
CDataStream2D::GetTitle() {
  return CDataStream::GetTitle();
}


void
CDataStream2D::setMaxPoints(int nPoints) {
  CDataStream::setMaxPoints(nPoints);
}


int
CDataStream2D::getMaxPoints() {
  return CDataStream::getMaxPoints();
}

