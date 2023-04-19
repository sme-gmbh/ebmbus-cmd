/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2018 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "operatingsystemcontrol.h"

OperatingSystemControl::OperatingSystemControl(QObject *parent) : QObject(parent)
{

}

void OperatingSystemControl::slot_shutdownNOW()
{
    QProcess p;
    QStringList args;
    args.append("poweroff");

//    p.startDetached("sudo poweroff");
    p.startDetached("sudo", args);
}
