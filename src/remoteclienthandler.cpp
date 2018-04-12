#include "remoteclienthandler.h"

RemoteClientHandler::RemoteClientHandler(QObject *parent, QTcpSocket *socket) : QObject(parent)
{
    this->socket = socket;
    //this->lv_database = lv_database;

#ifdef DEBUG
    QString debugStr;

    debugStr += "New connection ";

    if (socket->socketType() == QAbstractSocket::TcpSocket)
        debugStr += "tcp from ";
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
        debugStr += "udp from ";
    else if (socket->socketType() == QAbstractSocket::UnknownSocketType)
        debugStr += " - unknown socket type - ";

    debugStr += socket->peerAddress().toString() + ":" + QString().setNum(socket->peerPort()) + " \r\n";

    printf("%s", debugStr.toLatin1().data());
#endif
    socket->write("Hello\r\n");

    connect(socket, SIGNAL(readyRead()), this, SLOT(slot_read_ready()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
}

void RemoteClientHandler::slot_read_ready()
{
    while (socket->canReadLine())
    {
        // Data format:
        // COMMAND id [$parentId] [key]>[value];[key]>[value];...
        QString line = QString::fromUtf8(this->socket->readLine());
        line.remove(QRegExp("[\\r\\n]"));      // Strip newlines at the beginning and at the end
        int commandLength = line.indexOf(' ');
        if (commandLength == -1) commandLength = line.length();
        QString command = line.left(commandLength);
        line.remove(0, commandLength + 1);  // Remove command and optional space
        //line = line.trimmed();      // Strip paces and newlines at the beginning and at the end
        int idLength = line.indexOf(' ');
        if (idLength == -1)
            idLength = line.length();
#ifdef DEBUG
        printf("Received data: \r\n");
#endif

        quint64 id = 0;
        bool idValid = true;

        id = line.left(idLength).toULongLong(&idValid);

        line.remove(0, idLength + 1);       // Remove id and optional space

        // Parse key/value pairs now
        QStringList commandChunks = line.split(';', QString::SkipEmptyParts);
        QMap<QString, QString> data;

        foreach(QString commandChunk, commandChunks)
        {
#ifdef DEBUG
            printf("Decoding chunk: %s\r\n", commandChunk.toLatin1().data());
#endif
            QStringList key_value_pair = commandChunk.split('>');
            if (key_value_pair.length() != 2)
            {
                socket->write("ERROR: key_value_pair length invalid\r\n");
                continue;
            }
            // key and value are base64 encoded.
            QString key = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(0).toUtf8()));
            QString value = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(1).toUtf8()));
            data.insert(key, value);
        }

        // The following code is taken from the sme lv_server and must be changed accordingly for the openFFUcontrol remote protocol.

        // message is distributed to other clients in this way
        //emit signal_broadcast(this->socket, line);

//        if (command == "GET") // Get a set of data
//        {
//            if (!idValid)
//            {
//                socket->write("ERROR: unable to decode id\r\n");
//                continue;
//            }
//            socket->write(lv_database->get(id));    // This is the only function returning QByteArray instead of QString
//            continue;
//        }

//        if (command == "CREATE") // Creates a new set of data in the parent identified by id
//        {
//            QByteArray response = lv_database->create(data).toUtf8();
//            socket->write(response);
//            if (!response.startsWith("ERROR"))
//                emit signal_broadcast(this->socket, response);
//            continue;
//        }

//        if (command == "SET") // Set a set of data, overwrites all keys
//        {
//            if (!idValid)
//            {
//                socket->write("ERROR: unable to decode id\r\n");
//                continue;
//            }
//            socket->write(lv_database->set(id, data).toUtf8());
//            continue;
//        }

//        if (command == "CHANGE") // Change a set of data, overwrites the given keys
//        {
//            if (!idValid)
//            {
//                socket->write("ERROR: unable to decode id\r\n");
//                continue;
//            }
//            QByteArray response = lv_database->change(id, data).toUtf8();
//            socket->write(response);
//            if (!response.startsWith("ERROR"))
//                emit signal_broadcast(this->socket, response);
//            continue;
//        }

        //    if (command == "REMOVE") // Delete a set of data, actually no deletion, but removes this node from the parent subnode list (data is kept on hdd)
        //    {
        //        socket->write(lv_database->remove(id).toUtf8());
        //        continue;
        //    }

        // If control reaches this point, we have an unsupported command
        socket->write("ERROR: Command not supported: " + command.toUtf8() + "\r\n");
    }
}

void RemoteClientHandler::slot_disconnected()
{
#ifdef DEBUG
    QString debugStr;

    debugStr += "Closed connection ";

    if (socket->socketType() == QAbstractSocket::TcpSocket)
        debugStr += "tcp from ";
    else if (socket->socketType() == QAbstractSocket::UdpSocket)
        debugStr += "udp from ";
    else if (socket->socketType() == QAbstractSocket::UnknownSocketType)
        debugStr += " - unknown socket type - ";

    debugStr += socket->peerAddress().toString() + ":" + QString().setNum(socket->peerPort()) + " \r\n";

    printf("%s", debugStr.toLatin1().data());
#endif
    emit signal_connectionClosed(this->socket, this);
}
