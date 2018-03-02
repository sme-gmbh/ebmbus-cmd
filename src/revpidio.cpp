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
    read(fd, $byte, 1);

    result = (byte & (1 << position));
    return result;
}

void RevPiDIO::setBit(int position, bool on)
{
    char byte;
    int offset = 70 + (position / 8);
    position %= 8;

    if(!file.isOpen())
        return;

    lseek(fd, offset, SEEK_SET);
    read(fd, $byte, 1);

    if (on)
        byte |= (1 << position);
    else
        byte &= ~(1 << position);

    lseek(fd, offset, SEEK_SET);
    write(fd, &byte, 1);
}
