#include <stdio.h>
#include "maincontroller.h"


MainController::MainController(QObject *parent) :
    QObject(parent)
{
    printf("ebmBus main controller startup...\n");

    m_loghandler = new Loghandler(this);
    connect(m_loghandler, SIGNAL(signal_newError()), this, SLOT(slot_newError()));
    connect(m_loghandler, SIGNAL(signal_allErrorsQuit()), this, SLOT(slot_allErrorsQuit()));
    connect(m_loghandler, SIGNAL(signal_allErrorsGone()), this, SLOT(slot_allErrorsGone()));

    m_speed = -1;

    m_ebmbusSystem = new EbmBusSystem(this, &m_io);
    m_ebmModbusSystem = new EbmModbusSystem(this, m_loghandler);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);

    connect(&m_manualSpeedUpdateTimer, SIGNAL(timeout()), this, SLOT(slot_manualSpeedUpdateTimer_fired()));
    m_manualSpeedUpdateTimer.start(300000); // Every 300 seconds (5 minutes)

    m_lightbutton_operation = new LightButton(this, &m_io, 4, 4);
    m_lightbutton_error = new LightButton(this, &m_io, 5, 5);
    m_lightbutton_speed_0 = new LightButton(this, &m_io, 6, 6);
    m_lightbutton_speed_50 = new LightButton(this, &m_io, 7, 7);
    m_lightbutton_speed_100 = new LightButton(this, &m_io, 8, 8);

    connect(m_lightbutton_operation, SIGNAL(signal_button_clicked()), this, SLOT(slot_button_operation_clicked()));
    connect(m_lightbutton_error, SIGNAL(signal_button_clicked()), this, SLOT(slot_button_error_clicked()));
    connect(m_lightbutton_speed_0, SIGNAL(signal_button_clicked()), this, SLOT(slot_button_speed_0_clicked()));
    connect(m_lightbutton_speed_50, SIGNAL(signal_button_clicked()), this, SLOT(slot_button_speed_50_clicked()));
    connect(m_lightbutton_speed_100, SIGNAL(signal_button_clicked()), this, SLOT(slot_button_speed_100_clicked()));

    m_ups = new UninterruptiblePowerSupply(this, &m_io, 9, m_loghandler);
    m_osControl = new OperatingSystemControl(this);
    connect(m_ups, SIGNAL(signal_mainswitchOff()), this, SLOT(slot_shutdownNOW()));
    connect(m_ups, SIGNAL(signal_mainswitchOff()), m_osControl, SLOT(slot_shutdownNOW()));
    connect(m_ups, SIGNAL(signal_shutdownDueToPowerloss()), this, SLOT(slot_shutdownNOW()));
    connect(m_ups, SIGNAL(signal_shutdownDueToPowerloss()), m_osControl, SLOT(slot_shutdownNOW()));
    connect(m_ups, SIGNAL(signal_powerGoodAgain()), this, SLOT(slot_mainsPowerRestored()));

    m_ffudatabase = new FFUdatabase(this, m_ebmbusSystem, m_loghandler);
    m_ffudatabase->loadFromHdd();

    m_auxfandatabase = new AuxFanDatabase(this, m_ebmModbusSystem, m_loghandler);
    m_auxfandatabase->loadFromHdd();

    m_remotecontroller = new RemoteController(this, m_ffudatabase, m_auxfandatabase, m_loghandler);
    connect(m_remotecontroller, SIGNAL(signal_activated()), this, SLOT(slot_remoteControlActivated()));
    connect(m_remotecontroller, SIGNAL(signal_activated()), m_ffudatabase, SLOT(slot_remoteControlActivated()));
    connect(m_remotecontroller, SIGNAL(signal_activated()), m_auxfandatabase, SLOT(slot_remoteControlActivated()));
    connect(m_remotecontroller, SIGNAL(signal_deactivated()), this, SLOT(slot_remoteControlDeactivated()));
    connect(m_remotecontroller, SIGNAL(signal_deactivated()), m_ffudatabase, SLOT(slot_remoteControlDeactivated()));
    connect(m_remotecontroller, SIGNAL(signal_deactivated()), m_auxfandatabase, SLOT(slot_remoteControlDeactivated()));
    connect(m_remotecontroller, SIGNAL(signal_connected()), this, SLOT(slot_remoteControlConnected()));
    connect(m_remotecontroller, SIGNAL(signal_disconnected()), this, SLOT(slot_remoteControlDisconnected()));

    connect(m_remotecontroller, SIGNAL(signal_buttonSimulated_operation_clicked()), this, SLOT(slot_button_operation_clicked()));
    connect(m_remotecontroller, SIGNAL(signal_buttonSimulated_error_clicked()), this, SLOT(slot_button_error_clicked()));
    connect(m_remotecontroller, SIGNAL(signal_buttonSimulated_speed_0_clicked()), this, SLOT(slot_button_speed_0_clicked()));
    connect(m_remotecontroller, SIGNAL(signal_buttonSimulated_speed_50_clicked()), this, SLOT(slot_button_speed_50_clicked()));
    connect(m_remotecontroller, SIGNAL(signal_buttonSimulated_speed_100_clicked()), this, SLOT(slot_button_speed_100_clicked()));


    m_lightbutton_operation->slot_setLight(LightButton::LED_BLINK);

    // Temporarily broadcast speed at startup - remove this later
    // m_ebmbusSystem->broadcastSpeed(170);
}

MainController::~MainController()
{
}

// This is a periodic timer function for visualisation and operation
void MainController::slot_timer_fired()
{
    static int currentManualSpeed = -1;

    if (!m_remotecontroller->isEnabled())
    {

        if ((m_lightbutton_speed_100->pressed_milliseconds() > 1000) && (m_speed < 255))
        {
            if (m_speed != -1)
                m_speed++;
            else
                m_speed = 127;
        }

        if ((m_lightbutton_speed_50->pressed_milliseconds() > 1000) && (m_speed > 0))
        {
            if (m_speed != -1)
                m_speed--;
            else m_speed = 127;
        }

        if ((m_lightbutton_speed_0->pressed_milliseconds() > 5000) && (m_speed != 0))
        {
            if (m_speed != -1)
                m_speed = 0;
            else m_speed = 127;
        }

        if ((m_speed != currentManualSpeed) && (m_speed != -1))
        {
            if (currentManualSpeed == -1)
                m_ebmbusSystem->broadcastSpeed(m_speed, true);
            else
                m_ebmbusSystem->broadcastSpeed(m_speed, false);
            currentManualSpeed = m_speed;
        }

        if (m_speed == -1)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_BLINK);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_BLINK);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_BLINK);
        }
        else if (m_speed == 0)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_ON);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_OFF);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_OFF);
        }
        else if (m_speed < 127)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_ON);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_ON);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_OFF);
        }
        else if (m_speed == 127)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_OFF);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_ON);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_OFF);
        }
        else if (m_speed < 255)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_OFF);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_ON);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_ON);
        }
        else if (m_speed == 255)
        {
            m_lightbutton_speed_0->slot_setLight(LightButton::LED_OFF);
            m_lightbutton_speed_50->slot_setLight(LightButton::LED_OFF);
            m_lightbutton_speed_100->slot_setLight(LightButton::LED_ON);
        }
    }
    else
    {
        m_speed = -1;
        currentManualSpeed = m_speed;
        m_lightbutton_speed_0->slot_setLight(LightButton::LED_OFF);
        m_lightbutton_speed_50->slot_setLight(LightButton::LED_OFF);
        m_lightbutton_speed_100->slot_setLight(LightButton::LED_OFF);
    }
}

// This timer broadcasts manual speed on a regular basis to catch all ffus that had a powerfailure in the meantime
void MainController::slot_manualSpeedUpdateTimer_fired()
{
    if (!m_remotecontroller->isEnabled() && (m_speed != -1))
    {
        m_ebmbusSystem->broadcastSpeed(m_speed);
    }
}

// This slot is called if the operator clicked (=== pressed for less than 1 second) the operation mode button locally
void MainController::slot_button_operation_clicked()
{
    if (m_remotecontroller->isEnabled())
        m_remotecontroller->slot_deactivate();
    else
        m_remotecontroller->slot_activate();
}

// This slot is called if the operator clicked (=== pressed for less than 1 second) the quit errors button locally
void MainController::slot_button_error_clicked()
{
    m_loghandler->slot_quitErrors();
}

// This slot is called if the operator clicked (=== pressed for less than 1 second) the 0 % speed button locally
void MainController::slot_button_speed_0_clicked()
{

}

// This slot is called if the operator clicked (=== pressed for less than 1 second) the 50 % speed button locally
void MainController::slot_button_speed_50_clicked()
{
    if (!m_remotecontroller->isEnabled())
    {
        m_speed = 127;
        m_ebmbusSystem->broadcastSpeed(m_speed);
    }
}

// This slot is called if the operator clicked (=== pressed for less than 1 second) the 100 % speed button locally
void MainController::slot_button_speed_100_clicked()
{
    if (!m_remotecontroller->isEnabled())
    {
        m_speed = 255;
        m_ebmbusSystem->broadcastSpeed(m_speed);
    }
}

// This slot is called if remote control by a server is possible and intentionally activated by an operator
void MainController::slot_remoteControlActivated()
{
    m_lightbutton_operation->slot_setLight(LightButton::LED_ON);
}

// This slot is called if remote controlling is deactivated intentionally by an operator
void MainController::slot_remoteControlDeactivated()
{
    m_lightbutton_operation->slot_setLight(LightButton::LED_BLINK);
}

// This slot is called as soon as the first server connects to the remotecontroller
void MainController::slot_remoteControlConnected()
{
    if (m_remotecontroller->isActive())
        m_lightbutton_operation->slot_setLight(LightButton::LED_ON);
}

// This slot is called if the remotecontroller is not connected to at least ONE server
void MainController::slot_remoteControlDisconnected()
{
    m_lightbutton_operation->slot_setLight(LightButton::LED_BLINK);
}

// This slot is called if the errorhandler gets a new error, that we want to show the operator by a blinking red led
void MainController::slot_newError()
{
    m_lightbutton_error->slot_setLight(LightButton::LED_BLINK);
}

// This slot is called if the system is going down for poweroff
void MainController::slot_shutdownNOW()
{
    m_lightbutton_operation->slot_setLight(LightButton::LED_OFF);
    m_lightbutton_error->slot_setLight(LightButton::LED_ON);
}

void MainController::slot_mainsPowerRestored()
{
    // Temporarily broadcast speed at startup - remove this later
    m_ebmbusSystem->broadcastSpeed(170);
}

void MainController::slot_allErrorsQuit()
{
    m_lightbutton_error->slot_setLight(LightButton::LED_ON);
}

void MainController::slot_allErrorsGone()
{
    m_lightbutton_error->slot_setLight(LightButton::LED_OFF);
}
