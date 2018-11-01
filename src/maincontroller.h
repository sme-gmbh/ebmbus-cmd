#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <libebmbus/ebmbus.h>
#include "revpidio.h"
#include "ebmbussystem.h"
#include "ebmmodbussystem.h"
#include "lightbutton.h"
#include "uninterruptiblepowersupply.h"
#include "operatingsystemcontrol.h"
#include "remotecontroller.h"
#include "ffudatabase.h"
#include "auxfandatabase.h"
#include "loghandler.h"

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
    ~MainController();

private:
    Loghandler* m_loghandler;

    RevPiDIO m_io;

    LightButton* m_lightbutton_operation;
    LightButton* m_lightbutton_error;
    LightButton* m_lightbutton_speed_0;
    LightButton* m_lightbutton_speed_50;
    LightButton* m_lightbutton_speed_100;

    EbmBusSystem* m_ebmbusSystem;
    EbmModbusSystem* m_ebmModbusSystem;

    UninterruptiblePowerSupply* m_ups;
    OperatingSystemControl* m_osControl;

    FFUdatabase* m_ffudatabase;
    AuxFanDatabase* m_auxfandatabase;

    RemoteController* m_remotecontroller;

    QTimer m_timer;
    QTimer m_manualSpeedUpdateTimer;

    int m_speed;

private slots:
    void slot_timer_fired();
    void slot_manualSpeedUpdateTimer_fired();

    void slot_button_operation_clicked();
    void slot_button_error_clicked();
    void slot_button_speed_0_clicked();
    void slot_button_speed_50_clicked();
    void slot_button_speed_100_clicked();

    void slot_remoteControlActivated();
    void slot_remoteControlDeactivated();
    void slot_remoteControlConnected();
    void slot_remoteControlDisconnected();

    void slot_shutdownNOW();
    void slot_mainsPowerRestored();

    void slot_newError();
    void slot_allErrorsQuit();
    void slot_allErrorsGone();
};

#endif // MAINCONTROLLER_H
