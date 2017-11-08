#pragma once
#include <QColor>

class CDataSetProperties {

public:
  CDataSetProperties(void);
  CDataSetProperties(int myId, int myPenWidth, QColor myColor, int mySymbol, QString myTitle);
	virtual ~CDataSetProperties(void);
	void SetId(int Id);
	int GetId();
  QString Title;
	int PenWidth;
	int Symbol;
  QColor Color;

private:
	int Id;
};
