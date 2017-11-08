#include "AxisLimits.h"

CAxisLimits::CAxisLimits():
  XMin(0.0), XMax(3.0),	YMin(-10.0), YMax(10.0), 
  AutoX(false), AutoY(false), LogX(false), LogY(false){
}

CAxisLimits::~CAxisLimits(void) {
}
