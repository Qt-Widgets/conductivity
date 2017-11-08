#pragma once

class CAxisLimits {
public:
  CAxisLimits(void);
  virtual ~CAxisLimits(void);

	double XMin, XMax, YMin, YMax;
  bool AutoX, AutoY;
  bool LogX, LogY;
};
