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
    restoreSettings();
}


void
plotPropertiesDlg::restoreSettings() {
    QSettings settings;
    labelColor        = settings.value("LabelColor", QColor(Qt::white)).toUInt();
    gridColor         = settings.value("GridColor", QColor(Qt::blue)).toUInt();
    frameColor        = settings.value("FrameColor", QColor(Qt::blue)).toUInt();
    painterBkColor    = settings.value("PainterBKColor", QColor(Qt::black)).toUInt();
    gridPenWidth      = settings.value("GridPenWidth", 1).toInt();
    maxDataPoints     = settings.value("MaxDataPoints", 100).toInt();
    painterFontName   = settings.value("PainterFontName", QString("Helvetica")).toString();
    painterFontSize   = settings.value("PainterFontSize", 16).toInt();
    painterFontWeight = QFont::Weight(settings.value("PainterFontWeight", QFont::Bold).toInt());
    painterFontItalic = settings.value("PainterFontItalic", false).toBool();
    painterFont       = QFont(painterFontName,
                              painterFontSize,
                              painterFontWeight,
                              painterFontItalic);
}


void
plotPropertiesDlg::saveSettings() {
    QSettings settings;
    settings.setValue("LabelColor", labelColor);
    settings.setValue("GridColor", gridColor);
    settings.setValue("FrameColor", frameColor);
    settings.setValue("PainterBKColor", painterBkColor);
    settings.setValue("GridPenWidth", gridPenWidth);
    settings.setValue("MaxDataPoints", maxDataPoints);
    settings.setValue("PainterFontName", painterFontName);
    settings.setValue("PainterFontSize", painterFontSize);
    settings.setValue("PainterFontWeight", QFont::Bold);
    settings.setValue("PainterFontItalic", painterFontItalic);
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
