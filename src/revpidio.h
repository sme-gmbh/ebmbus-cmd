#ifndef REVPIDIO_H
#define REVPIDIO_H

#include <QObject>
#include <QFile>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


class RevPiDIO : public QObject
{
    Q_OBJECT

public:
    explicit RevPiDIO(QObject *parent = 0);

    bool getBit(int position);
    void setBit(int position, bool on);

private:
    int fd;
};

#endif // REVPIDIO_H
