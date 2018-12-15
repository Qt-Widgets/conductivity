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

#include <QGridLayout>
#include <QLabel>
#include <QColorDialog>
#include <QFontDialog>


plotPropertiesDlg::plotPropertiesDlg(QWidget *parent)
    : QDialog(parent)
{
    restoreSettings();
    initUI();

    // Create the Dialog Layout
    QGridLayout* pLayout = new QGridLayout();

    pLayout->addWidget(&BkColorButton,    0, 0, 1, 1);
    pLayout->addWidget(&frameColorButton, 0, 1, 1, 1);
    pLayout->addWidget(&gridColorButton,  1, 0, 1, 1);
    pLayout->addWidget(&labelColorButton, 1, 1, 1, 1);
    pLayout->addWidget(&labelFontButton,  2, 0, 1, 1);

    pLayout->addWidget(new QLabel("Grid Lines Width"),  2, 0, 1, 1);
    pLayout->addWidget(&gridPenWidthEdit,               2, 1, 1, 1);
    pLayout->addWidget(new QLabel("Max Data Points"),   3, 0, 1, 1);
    pLayout->addWidget(&maxDataPointsEdit,              3, 1, 1, 1);

    pLayout->addWidget(pButtonBox, 4, 0, 1, 2);

    // Set the Layout
    setLayout(pLayout);
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
    emit configChanged();
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
    BkColorButton.setText("Bkg Color");
    frameColorButton.setText("Frame Color");
    gridColorButton.setText("Grid Color");
    labelColorButton.setText("Labels Color");
    labelFontButton.setText("Label Font");

    gridPenWidthEdit.setText(QString("%1").arg(gridPenWidth));
    maxDataPointsEdit.setText(QString("%1").arg(maxDataPoints));

    pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                      QDialogButtonBox::Cancel);
    setToolTips();
}


void
plotPropertiesDlg::connectSignals() {
    connect(&BkColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeBkColor()));
    connect(&frameColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeFrameColor()));
    connect(&gridColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeGridColor()));
    connect(&labelColorButton, SIGNAL(clicked()),
            this, SLOT(onChangeLabelsColor()));
    connect(&labelFontButton, SIGNAL(clicked()),
            this, SLOT(onChangeLabelsFont()));
    // Line Edit
    connect(&gridPenWidthEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onChangeGridPenWidth(const QString)));
    connect(&maxDataPointsEdit, SIGNAL(textChanged(const QString)),
            this, SLOT(onChangeMaxDataPoints(const QString)));

    connect(pButtonBox, SIGNAL(accepted()),
            this, SLOT(onOk()));
    connect(pButtonBox, SIGNAL(rejected()),
            this, SLOT(onCancel()));
}


void
plotPropertiesDlg::onCancel() {
    restoreSettings();
    emit configChanged();
    reject();
}


void
plotPropertiesDlg::onOk() {
    saveSettings();
    accept();
}


void
plotPropertiesDlg::onChangeBkColor() {
    QColorDialog colorDialog(painterBkColor);
    if(colorDialog.exec()==QDialog::Accepted) {
        painterBkColor = colorDialog.getColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeFrameColor() {
    QColorDialog colorDialog(frameColor);
    if(colorDialog.exec()==QDialog::Accepted) {
        frameColor = colorDialog.getColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeGridColor() {
    QColorDialog colorDialog(gridColor);
    if(colorDialog.exec()==QDialog::Accepted) {
        gridColor = colorDialog.getColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeLabelsColor() {
    QColorDialog colorDialog(labelColor);
    if(colorDialog.exec()==QDialog::Accepted) {
        labelColor = colorDialog.getColor();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeLabelsFont() {
    QFontDialog fontDialog(painterFont);
    if(fontDialog.exec()==QDialog::Accepted) {
        painterFont = fontDialog.font();
        painterFontName   = painterFont.family();
        painterFontSize   = painterFont.pointSize();
        painterFontWeight = QFont::Weight(painterFont.weight());
        painterFontItalic = painterFont.italic();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeGridPenWidth(const QString sNewVal) {
    if((sNewVal.toInt() > 0) &&
       (sNewVal.toInt() < 10))
    {
        gridPenWidth = sNewVal.toInt();
        emit configChanged();
    }
}


void
plotPropertiesDlg::onChangeMaxDataPoints(const QString sNewVal) {
    if((sNewVal.toInt() > 0) &&
       (sNewVal.toInt() < 2000))
    {
        maxDataPoints = sNewVal.toInt();
        emit configChanged();
    }
}


