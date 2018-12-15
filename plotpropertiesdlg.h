#pragma once
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

#include <QObject>
#include <QDialog>
#include <QSettings>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>

class plotPropertiesDlg : public QDialog
{
    Q_OBJECT
public:
    plotPropertiesDlg(QWidget *parent);

    QColor labelColor;
    QColor gridColor;
    QColor frameColor;
    QColor painterBkColor;

    int gridPenWidth;
    int maxDataPoints;
    QFont painterFont;

protected:
    void restoreSettings();
    void saveSettings();
    void initUI();
    void connectSignals();
    void setToolTips();

    QString painterFontName;
    int painterFontSize;
    QFont::Weight painterFontWeight;
    bool painterFontItalic;

    QPushButton BkColorButton;
    QPushButton frameColorButton;
    QPushButton gridColorButton;
    QPushButton labelColorButton;
    QLineEdit   gridPenWidthEdit;
};

