#include "revpidio.h"

RevPiDIO::RevPiDIO(QObject *parent) :
    QObject(parent)
{
    file.setFileName("/dev/piControl0");
    file.open(QIODevice::ReadWrite);
}

bool RevPiDIO::getBit(int position)
{
    bool result;
    char byte;
    int offset = 0 + (position / 8);
    position %= 8;

    if(!file.isOpen())
        return false;

    file.seek(offset);
    file.read(&byte, 1);

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

    file.seek(offset);
    file.read(&byte, 1);

    if (on)
        byte |= (1 << position);
    else
        byte &= ~(1 << position);

    file.seek(offset);
    file.write(&byte, 1);
}
