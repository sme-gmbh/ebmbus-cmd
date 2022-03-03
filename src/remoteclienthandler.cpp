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

#include "remoteclienthandler.h"

RemoteClientHandler::RemoteClientHandler(QObject *parent, QTcpSocket *socket, FFUdatabase *ffuDB, AuxFanDatabase *auxFanDB, OCUdatabase *ocuDB, Loghandler* loghandler) : QObject(parent)
{
    this->socket = socket;
    m_ffuDB = ffuDB;
    m_auxFanDB = auxFanDB;
    m_ocuDB = ocuDB;
    m_loghandler = loghandler;

    m_livemode = false;

#ifdef QT_DEBUG
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
    connect(m_ffuDB, SIGNAL(signal_DCIaddressingFinished(int)), this, SLOT(slot_DCIaddressingFinished(int)));
    connect(m_ffuDB, SIGNAL(signal_DCIaddressingGotSerialNumber(int,quint8,quint8,quint8,quint32)), this, SLOT(slot_DCIaddressingGotSerialNumber(int,quint8,quint8,quint8,quint32)));
    connect(m_ffuDB, SIGNAL(signal_FFUactualDataHasChanged(int)), this, SLOT(slot_FFUactualDataHasChanged(int)));
    connect(m_auxFanDB, &AuxFanDatabase::signal_AuxFanActualDataHasChanged, this, &RemoteClientHandler::slot_AuxFanActualDataHasChanged);
    connect(m_ocuDB, &OCUdatabase::signal_OCUActualDataHasChanged, this, &RemoteClientHandler::slot_OCUActualDataHasChanged);
}

void RemoteClientHandler::slot_read_ready()
{
    while (socket->canReadLine())
    {
        // Data format:
        // COMMAND [--key][=value] [--key][=value]...
        QString line = QString::fromUtf8(this->socket->readLine());
        line.remove(QRegExp("[\\r\\n]"));      // Strip newlines at the beginning and at the end
        int commandLength = line.indexOf(' ');
        if (commandLength == -1) commandLength = line.length();
        QString command = line.left(commandLength);
        line.remove(0, commandLength + 1);  // Remove command and optional space

#ifdef QT_DEBUG
        printf("Received data: \r\n");
#endif

        // Parse key/value pairs now
        QStringList commandChunks = line.split(' ', QString::SkipEmptyParts);
        QMap<QString, QString> data;

        foreach(QString commandChunk, commandChunks)
        {
#ifdef QT_DEBUG
            printf("Decoding chunk: %s\r\n", commandChunk.toUtf8().data());
#endif
            QStringList key_value_pair = commandChunk.split('=');
            if ((key_value_pair.length() > 2) || (key_value_pair.length() < 1))
            {
                socket->write("ERROR: key_value_pair length invalid\r\n");
                continue;
            }
//            // key and value are base64 encoded.
//            QString key = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(0).toUtf8()));
//            QString value = QString::fromUtf8(QByteArray::fromBase64(key_value_pair.at(1).toUtf8()));
            QString key = key_value_pair.at(0);
            if (key.startsWith("--"))
            {
                key.remove(0, 2);   // Remove leading "--"
                if (key_value_pair.length() == 2)
                {
                    QString value = key_value_pair.at(1);
                    data.insert(key, value);
                }
                else
                {
                    data.insert(key, "query");
                }
            }
        }

        // message is distributed to other clients in this way
        //emit signal_broadcast(QByteArray);

        if (command == "help")
        {
            socket->write("This is the commandset of the openFFUcontrol remote unit:\r\n"
                          "\r\n"
                          "<COMMAND> [--key[=value]]\r\n"
                          "\r\n"
                          "COMMANDS:\r\n"
                          "    hostname\r\n"
                          "        Show the hostname of the controller.\r\n"
                          "    startlive\r\n"
                          "        Show data of ffus in realtime. Can be stopped with stoplive\r\n"
                          "    stoplive\r\n"
                          "        Stop live showing of ffu data.\r\n"
                          "    list\r\n"
                          "        Show the list of currently configured ffus from the controller database.\r\n"
                          "    list-auxfans\r\n"
                          "        Show the list of currently configured auxiliary fans from the controller database.\r\n"
                          "    list-ocus\r\n"
                          "        Show the list of currently configured OCUs from the controller database.\r\n"
                          "    log\r\n"
                          "        Show the log consisting of infos, warnings and errors.\r\n"
                          "\r\n"
                          "    buffers\r\n"
                          "        Show buffer levels.\r\n"
                          "\r\n"
                          "    button --button=BUTTONNAME\r\n"
                          "        Simulate a button click.\r\n"
                          "        Possible BUTTONNAMEs: operation, error, speed0, speed50, speed100.r\n"
                          "\r\n"
                          "    button-leds\r\n"
                          "        Get shown status of button leds.\r\n"
                          "\r\n"
                          "    add-ffu --bus=BUSNR --id=ID --unit=UNIT\r\n"
                          "        Add a new ffu with ID to the controller database at BUSNR at position UNIT from start of bus.\r\n"
                          "\r\n"
                          "    delete-ffu --id=ID --bus=BUSNR\r\n"
                          "        Delete ffu with ID from the controller database.\r\n"
                          "        Note that you can delete all ffus of a certain bus by using BUSNR only.\r\n"
                          "\r\n"
                          "    add-auxfan --bus=BUSNR --id=ID --fanAddress=ADR\r\n"
                          "        Add a new auxiliary fan with ID to the controller database at BUSNR with modbus address ADR.\r\n"
                          "\r\n"
                          "    delete-auxfan --id=ID --bus=BUSNR\r\n"
                          "        Delete auxiliary fan with ID from the controller database.\r\n"
                          "        Note that you can delete all auxiliary fans of a certain bus by using BUSNR only.\r\n"
                          "\r\n"
                          "    add-ocu --bus=BUSNR --unitAddress=ARD --id=ID\r\n"
                          "        Add a new OCU with ID to the controller database at BUSNR with modbus address ADR.\r\n"
                          "\r\n"
                          "    delete-ocu --id=ID --bus=BUSNR\r\n"
                          "        Delete OCU with ID from the controller database.\r\n"
                          "        Note that you can delete all OCUs of a certain bus by using BUSNR only.\r\n"
                          "\r\n"
                          "    broadcast --bus=BUSNR\r\n"
                          "        Broadcast data to all buses and all units.\r\n"
                          "        Possible keys: rawspeed, ...tbd.r\n"
                          "\r\n"
                          "    dci-address --bus=BUSNR --startAdr=ADR --ids=IDS\r\n"
                          "        Start daisy-chain addressing of bus-line BUSNR beginning at ADR.\r\n"
                          "        If IDS is set, rbu will automatically insert new ffus with the given ids.\r\n"
                          "        IDS are given in comma separated format or as just one id, in that case it is autoincremented for each unit.\r\n"
                          "\r\n"
                          "    set --parameter=VALUE\r\n"
                          "\r\n"
                          "    get --parameter\r\n"
                          "        parameter 'actual' lists all actual values of the selected ffu.\r\n"
                          "\r\n"
                          "    raw-set --bus=BUSNR --KEY=VALUE\r\n"
                          "\r\n"
                          "    raw-get --bus=BUSNR --KEY1 [--KEY2 ...]\r\n");
        }
        // ************************************************** hostname **************************************************
        else if (command == "hostname")
        {
            QString line;
            line = "Hostname=" + QHostInfo::localHostName() + "\n";
            socket->write(line.toUtf8());
        }
        // ************************************************** startlive **************************************************
        else if (command == "startlive")
        {
            QString line;
            line = "Liveshow=on\n";
            socket->write(line.toUtf8());
            m_livemode = true;
        }
        // ************************************************** stoplive **************************************************
        else if (command == "stoplive")
        {
            QString line;
            line = "Liveshow=off\n";
            socket->write(line.toUtf8());
            m_livemode = false;
        }
        // ************************************************** list **************************************************
        else if (command == "list")
        {
            QList<FFU*> ffus = m_ffuDB->getFFUs();
            foreach(FFU* ffu, ffus)
            {
                QString line;

                line.sprintf("FFU id=%i busID=%i unit=%i fanAddress=%i fanGroup=%i nSet=%i\r\n", ffu->getId(), ffu->getBusID(), ffu->getUnit(), ffu->getFanAddress(), ffu->getFanGroup(), ffu->getSpeedSetpoint());

                socket->write(line.toUtf8());
            }
        }
        // ************************************************** list-auxfans **************************************************
        else if (command == "list-auxfans")
        {
            QList<AuxFan*> auxFans = m_auxFanDB->getAuxFans();
            foreach(AuxFan* auxFan, auxFans)
            {
                QString line;

                line.sprintf("AuxFan id=%i busID=%i fanAddress=%i nSet=%i\r\n", auxFan->getId(), auxFan->getBusID(), auxFan->getFanAddress(), auxFan->getSpeedSetpoint());

                socket->write(line.toUtf8());
            }
        }
        // ************************************************** list-ocus **************************************************
        else if (command == "list-ocus")
        {
            QList<OCU*> ocus = m_ocuDB->getOCUs();
            foreach(OCU* ocu, ocus)
            {
                QString line;

                line.sprintf("OCU id=%i busID=%i\r\n", ocu->getId(), ocu->getBusID());

                socket->write(line.toUtf8());
            }
        }
        // ************************************************** log **************************************************
        else if (command == "log")
        {
            socket->write(m_loghandler->toString(LogEntry::Info).toUtf8() + "\n");
            socket->write(m_loghandler->toString(LogEntry::Warning).toUtf8() + "\n");
            socket->write(m_loghandler->toString(LogEntry::Error).toUtf8() + "\n");
        }
        // ************************************************** buffers **************************************************
        else if (command == "buffers")
        {
            int i = 0;
            foreach(EbmBus* bus, *m_ffuDB->getBusList())
            {
                int telegramQueueLevel_standardPriority = bus->getSizeOfTelegramQueue(false);
                int telegramQueueLevel_highPriority = bus->getSizeOfTelegramQueue(true);
                QString line;
                line.sprintf("EbmBus line %i: TelegramQueueLevel_standardPriority=%i TelegramQueueLevel_highPriority=%i\r\n",
                             i, telegramQueueLevel_standardPriority, telegramQueueLevel_highPriority);
                socket->write(line.toUtf8());
                i++;
            }
        }
        // ************************************************** button **************************************************
        else if (command == "button")
        {
            QString button = data.value("button");
            if (button.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"button\" not specified. Abort.\r\n");
                continue;
            }

            if (button == "operation")
                emit signal_buttonSimulated_operation_clicked();
            else if (button == "error")
                emit signal_buttonSimulated_error_clicked();
            else if (button == "speed0")
                emit signal_buttonSimulated_speed_0_clicked();
            else if (button == "speed50")
                emit signal_buttonSimulated_speed_50_clicked();
            else if (button == "speed100")
                emit signal_buttonSimulated_speed_100_clicked();
        }
        // ************************************************** button-leds **************************************************
        else if (command == "button-leds")
        {
            QString response;
            response.sprintf("Button-LED[operation]=.\r\n");
            response.sprintf("Button-LED[error]=.\r\n");
            response.sprintf("Button-LED[speed0]=.\r\n");
            response.sprintf("Button-LED[speed50]=.\r\n");
            response.sprintf("Button-LED[speed100]=.\r\n");

            response = "Not implemented yet.\r\n";  // TBD. Implementation

            socket->write(response.toUtf8());
        }
        // ************************************************** add-ffu **************************************************
        else if ((command == "add-ffu") || (command == "add-auxfan") || (command == "add-ocu"))
        {
            bool ok;

            QString busString = data.value("bus");
            int bus = busString.toInt(&ok);
            if (busString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified or bus cannot be parsed. Abort.\r\n");
                continue;
            }

            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

            QString unitString = data.value("unit");
            int unit = unitString.toInt(&ok);
            if ((unitString.isEmpty() || !ok) && ((command == "add-ffu") || (command == "add-ocu")))
            {
                socket->write("Error[Commandparser]: parameter \"unit\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

            QString addressString = data.value("fanAddress");
            int fanAddress = addressString.toInt(&ok);
            if ((addressString.isEmpty() || !ok) && (command == "add-auxfan"))
            {
                socket->write("Error[Commandparser]: parameter \"fanAddress\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("add-ffu bus=" + QString().setNum(bus).toUtf8() + " id=" + QString().setNum(id).toUtf8() + " unit=" + QString().setNum(unit).toUtf8() + "\r\n");
#endif
            QString response;
            if (command == "add-ffu")
                response = m_ffuDB->addFFU(id, bus, unit);
            else if (command == "add-auxfan")
                response = m_auxFanDB->addAuxFan(id, bus, fanAddress);
            else if (command == "add-ocu")
                response = m_ocuDB->addOCU(id, bus, unit);
            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** delete-ffu **************************************************
        else if (command == "delete-ffu")
        {
            bool ok;
            QString response;
            bool noID = false;
            bool noBus = false;

            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                noID = true;
            }
            else
            {
                response += m_ffuDB->deleteFFU(id) + "\n";
            }

            QString busString = data.value("bus");
            int bus = busString.toInt(&ok);
            if (busString.isEmpty() || !ok)
            {
                noBus = true;
            }
            else
            {
                foreach (FFU* ffu, m_ffuDB->getFFUs(bus))
                {
                    response += m_ffuDB->deleteFFU(ffu->getId()) + "\n";
                }
            }

            if (noID && noBus)
                response = "Error[Commandparser]: Neither parameter \"id\" nor parameter \"bus\" specified. Abort.\r\n";


#ifdef DEBUG
            socket->write("delete-ffu id=" + QString().setNum(id).toUtf8() + "\r\n");
#endif


            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** delete-auxfan **************************************************
        else if (command == "delete-auxfan")
        {
            bool ok;
            QString response;
            bool noID = false;
            bool noBus = false;

            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                noID = true;
            }
            else
            {
                response += m_ffuDB->deleteFFU(id) + "\n";
            }

            QString busString = data.value("bus");
            int bus = busString.toInt(&ok);
            if (busString.isEmpty() || !ok)
            {
                noBus = true;
            }
            else
            {
                foreach (AuxFan* auxFan, m_auxFanDB->getAuxFans(bus))
                {
                    response += m_auxFanDB->deleteAuxFan(auxFan->getId()) + "\n";
                }
            }

            if (noID && noBus)
                response = "Error[Commandparser]: Neither parameter \"id\" nor parameter \"bus\" specified. Abort.\r\n";


#ifdef DEBUG
            socket->write("delete-auxfan id=" + QString().setNum(id).toUtf8() + "\r\n");
#endif


            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** broadcast **************************************************
        else if (command == "broadcast")
        {
            bool ok;

            QString busString = data.value("bus");
            int bus = busString.toInt(&ok);
            if (busString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified or bus cannot be parsed. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("broadcast bus=" + QString().setNum(bus).toUtf8() + " speed=" + speed.toUtf8() + "\r\n");
#endif

            data.remove("bus"); // busNr should no be passed to broadcast, so we remove it here.
            QString response = m_ffuDB->broadcast(bus, data);
            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** dci-address **************************************************
        else if (command == "dci-address")
        {
            bool ok;

            QString busString = data.value("bus");
            int bus = busString.toInt(&ok);
            if (busString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified or bus cannot be parsed. Abort.\r\n");
                continue;
            }

            QString startAdr = data.value("startAdr");
            if (startAdr.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"startAdr\" not specified. Abort.\r\n");
                continue;
            }

            QString idsString = data.value("ids");

#ifdef DEBUG
            socket->write("dci-address bus=" + QString().setNum(bus).toUtf8() + " startAdr=" + startAdr.toUtf8() + "\r\n");
#endif

            QString response = m_ffuDB->startDCIaddressing(bus, "tbd.", idsString);
            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** raw-set **************************************************
        else if (command == "raw-set")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("raw-set bus=" + bus.toUtf8() + "\r\n");
#endif
        }
        // ************************************************** raw-get **************************************************
        else if (command == "raw-get")
        {
            socket->write("Not implemented yet. Running in echo mode.\r\n");

            QString bus = data.value("bus");
            if (bus.isEmpty())
            {
                socket->write("Error[Commandparser]: parameter \"bus\" not specified. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("raw-get bus=" + bus.toUtf8() + "\r\n");
#endif
        }
        // ************************************************** set **************************************************
        else if (command == "set")
        {
            bool ok;
            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("set id=" + QString().setNum(id).toUtf8() + "\r\n");
#endif
            QString response;
            if (m_ffuDB->getFFUbyID(id) != nullptr)
                response = m_ffuDB->setFFUdata(id, data);
            else if (m_auxFanDB->getAuxFanByID(id) != nullptr)
                response = m_auxFanDB->setAuxFanData(id, data);
            socket->write(response.toUtf8() + "\r\n");
        }
        // ************************************************** get **************************************************
        else if (command == "get")
        {
            bool ok;
            QString idString = data.value("id");
            int id = idString.toInt(&ok);
            if (idString.isEmpty() || !ok)
            {
                socket->write("Error[Commandparser]: parameter \"id\" not specified or id can not be parsed. Abort.\r\n");
                continue;
            }

#ifdef DEBUG
            socket->write("get id=" + id.toUtf8() + "\r\n");
#endif
            QMap<QString,QString> responseData;

            if (m_ffuDB->getFFUbyID(id) != nullptr)
                responseData = m_ffuDB->getFFUdata(id, data.keys("query"));
            else if (m_auxFanDB->getAuxFanByID(id) != nullptr)
                responseData = m_auxFanDB->getAuxFanData(id, data.keys("query"));
            if (responseData.value("actualData").toInt() == 1)
            {
                socket->write("ActualData from id=" + QString().setNum(id).toUtf8());
                responseData.remove("actualData");  // Remove special treatment marker
            }
            else
                socket->write("Data from id=" + QString().setNum(id).toUtf8());
            QString errors;
            foreach(QString key, responseData.keys())
            {
                QString response = responseData.value(key);
                if (!response.startsWith("Error[FFU]:"))
                    socket->write(" " + key.toUtf8() + "=" + response.toUtf8());
                else
                    errors.append(response + "\r\n");
            }
            socket->write("\r\n");
            if (!errors.isEmpty())
            {
                socket->write(errors.toUtf8());
            }
        }
        // ************************************************** UNSUPPORTED COMMAND **************************************************
        else
        {
            // If control reaches this point, we have an unsupported command
            socket->write("ERROR: Command not supported: " + command.toUtf8() + "\r\n");
        }
    }
}

void RemoteClientHandler::slot_disconnected()
{
#ifdef QT_DEBUG
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

void RemoteClientHandler::slot_DCIaddressingFinished(int busID)
{
    socket->write("dci-address successful on bus=" + QByteArray().setNum(busID) + "\r\n");
}

void RemoteClientHandler::slot_DCIaddressingGotSerialNumber(int busID, quint8 unit, quint8 fanAddress, quint8 fanGroup, quint32 serialNumber)
{
    QString response;
    response.sprintf("dci-address bus=%i unit=%i serial=%i fanAddress=%i fanGroup=%i\r\n", busID, unit, serialNumber, fanAddress, fanGroup);
    socket->write(response.toUtf8());
}

void RemoteClientHandler::slot_FFUactualDataHasChanged(int id)
{
    if (m_livemode)
    {
        socket->write("ActualData from id=" + QString().setNum(id).toUtf8());
        QMap<QString,QString> responseData = m_ffuDB->getFFUdata(id, QStringList() << "actual");
        responseData.remove("actualData");  // Remove special treatment marker
        foreach(QString key, responseData.keys())
        {
            QString response = responseData.value(key);
            if (!response.startsWith("Error[FFU]:"))
                socket->write(" " + key.toUtf8() + "=" + response.toUtf8());
        }
        socket->write("\r\n");
    }
}

void RemoteClientHandler::slot_AuxFanActualDataHasChanged(int id)
{
    if (m_livemode)
    {
        socket->write("ActualData from id=" + QString().setNum(id).toUtf8());
        QMap<QString,QString> responseData = m_auxFanDB->getAuxFanData(id, QStringList() << "actual");
        responseData.remove("actualData");  // Remove special treatment marker
        foreach(QString key, responseData.keys())
        {
            QString response = responseData.value(key);
            if (!response.startsWith("Error[AuxFan]:"))
                socket->write(" " + key.toUtf8() + "=" + response.toUtf8());
        }
        socket->write("\r\n");
    }
}

void RemoteClientHandler::slot_OCUActualDataHasChanged(int id)
{
    if (m_livemode)
    {
        socket->write("ActualData from id=" + QString().setNum(id).toUtf8());
        QMap<QString,QString> responseData = m_auxFanDB->getAuxFanData(id, QStringList() << "actual");
        responseData.remove("actualData");  // Remove special treatment marker
        foreach(QString key, responseData.keys())
        {
            QString response = responseData.value(key);
            if (!response.startsWith("Error[OCU]:"))
                socket->write(" " + key.toUtf8() + "=" + response.toUtf8());
        }
        socket->write("\r\n");
    }
}
