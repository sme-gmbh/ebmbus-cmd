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

#include "revpidio.h"

RevPiDIO::RevPiDIO(QObject *parent) :
    QObject(parent)
{
    fd = open("/dev/piControl0", O_RDWR);
}

bool RevPiDIO::getBit(int position)
{
    bool result;
    char byte;
    int offset = 0 + (position / 8);
    position %= 8;

    lseek(fd, offset, SEEK_SET);
    read(fd, &byte, 1);

    result = (byte & (1 << position));
    return result;
}

void RevPiDIO::setBit(int position, bool on)
{
    char byte;
    int offset = 70 + (position / 8);
    position %= 8;

    lseek(fd, offset, SEEK_SET);
    read(fd, &byte, 1);

    if (on)
        byte |= (1 << position);
    else
        byte &= ~(1 << position);

    lseek(fd, offset, SEEK_SET);
    write(fd, &byte, 1);
}
