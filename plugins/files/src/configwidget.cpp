// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "configwidget.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QStandardPaths>
#include "mimetypechooser.h"

namespace Files{
/** ***************************************************************************/
ConfigWidget::ConfigWidget(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.pushButton_add, &QPushButton::clicked, this, &ConfigWidget::onButton_PathAdd);
    connect(ui.pushButton_remove, &QPushButton::clicked, this, &ConfigWidget::onButton_PathRemove);
    connect(ui.pushButton_advanced, &QPushButton::clicked, this, &ConfigWidget::onButton_Advanced);
    _vanishTimer.setInterval(5000);
    _vanishTimer.setSingleShot(true);
    connect(&_vanishTimer, &QTimer::timeout,  ui.label_info, &QLabel::clear);
}

/** ***************************************************************************/
ConfigWidget::~ConfigWidget()
{

}

/** ***************************************************************************/
void ConfigWidget::setVanishingInfo(const QString &t)
{
    ui.label_info->setText(t);
    _vanishTimer.start();
}

/** ***************************************************************************/
void ConfigWidget::onButton_PathAdd()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Choose path"),
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    if(path.isEmpty())
        return;

    emit requestAddPath(path);
}

/** ***************************************************************************/
void ConfigWidget::onButton_PathRemove()
{
    if (ui.listWidget_paths->currentItem() == nullptr)
        return;
    emit requestRemovePath(ui.listWidget_paths->currentItem()->text());
    // Remove works always
    delete ui.listWidget_paths->currentItem();
}

/** ***************************************************************************/
void ConfigWidget::onButton_Advanced()
{
    MimeTypeChooser *c = new MimeTypeChooser;
    c->show();
}
}
