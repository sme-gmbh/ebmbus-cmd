#ifndef REVPIDIO_H
#define REVPIDIO_H

#include <QObject>
#include <QFile>

class RevPiDIO : public QObject
{
    Q_OBJECT

public:
    explicit RevPiDIO(QObject *parent = 0);

    bool getBit(int position);
    void setBit(int position, bool on);

private:
    QFile file;
};

#endif // REVPIDIO_H
