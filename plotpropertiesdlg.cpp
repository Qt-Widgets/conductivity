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
#include "plotpropertiesdlg.h"

plotPropertiesDlg::plotPropertiesDlg(QWidget *parent)
    : QDialog(parent)
{
    labelColor        = Qt::white;
    gridColor         = Qt::blue;
    frameColor        = Qt::blue;
    painterBkColor    = Qt::black;
    gridPenWidth      = 1;
    maxDataPoints     = 100;
    painterFontName   = QString("Helvetica");
    painterFontSize   = 16;
    painterFontWeight = QFont::Bold;
    painterFontItalic = false;
    painterFont       = QFont(painterFontName,
                              painterFontSize,
                              painterFontWeight,
                              painterFontItalic);
}


void
plotPropertiesDlg::restoreSettings() {

}


void
plotPropertiesDlg::saveSettings() {

}


void
plotPropertiesDlg::setToolTips() {

}


void
plotPropertiesDlg::initUI() {

}


void
plotPropertiesDlg::connectSignals() {

}
