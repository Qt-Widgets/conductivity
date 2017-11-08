// DataSetProperties.cpp : file di implementazione
//

#include "DataSetProperties.h"


CDataSetProperties::CDataSetProperties() {
}


CDataSetProperties::CDataSetProperties(int myId, int myPenWidth, QColor myColor, int mySymbol, QString myTitle) {
	Id       = myId;
	PenWidth = myPenWidth;
	Color    = myColor;
	Symbol   = mySymbol;
	Title    = myTitle;
}


CDataSetProperties::~CDataSetProperties() {
}


int 
CDataSetProperties::GetId() {
  return Id;
}

void 
CDataSetProperties::SetId(int myId) {
  Id = myId;
}
