#include <float.h>
#include "cdatastream.h"

CDataStream::CDataStream(int Id, int PenWidth, QColor Color, int Symbol, QString Title) {
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


CDataStream::CDataStream(CDataSetProperties myProperties) {
  Properties = myProperties;
  if(myProperties.Title == QString())
    Properties.Title.sprintf("Data Set %d", Properties.GetId());
  isShown         = false;
  bShowCurveTitle = false;
}

CDataStream::~CDataStream(void) {
}


void
CDataStream::SetShow(bool show) {
  isShown = show;
}


void
CDataStream::AddPoint(double point) {
  m_pointArray.append(point);
  if(m_pointArray.count() > maxPoints)
    m_pointArray.removeFirst();
  miny = point-FLT_MIN;
  maxy = point+FLT_MIN;
  for(int i=0; i< m_pointArray.count(); i++) {
    if(m_pointArray.at(i) < miny) miny = m_pointArray.at(i);
    if(m_pointArray.at(i) > maxy) maxy = m_pointArray.at(i);
  }
}


void
CDataStream::SetColor(QColor Color) {
  Properties.Color = Color;
}


int
CDataStream::GetId() {
  return Properties.GetId();
}


void
CDataStream::RemoveAllPoints() {
  m_pointArray.clear();
}


void
CDataStream::SetTitle(QString myTitle) {
  Properties.Title = myTitle;
}


void
CDataStream::SetShowTitle(bool show) {
  bShowCurveTitle = show;
}


CDataSetProperties
CDataStream::GetProperties() {
  return Properties;
}



void
CDataStream::SetProperties(CDataSetProperties newProperties) {
  Properties = newProperties;
}



QString
CDataStream::GetTitle() {
  return Properties.Title;
}


void
CDataStream::setMaxPoints(int nPoints) {
  maxPoints = nPoints;
}


int
CDataStream::getMaxPoints() {
  return maxPoints;
}
