#ifndef LIGHTBUTTON_H
#define LIGHTBUTTON_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "revpidio.h"

class LightButton : public QObject
{
    Q_OBJECT
public:
    explicit LightButton(QObject *parent, RevPiDIO* io, int address_light, int address_button);

    // pressed_milliseconds indicated the number of milliseconds since a button has been pressed
    // it returns 0 as soon as the button is released
    int pressed_milliseconds();

    // clickTime defines the maximum time between a button press and release in order to fire the
    // signal "clicked"
    void setClickTime(int milliseconds);

    void setBlinkFrequency(int milliHetz);

    typedef enum {
        LED_OFF,
        LED_ON,
        LED_BLINK
    } LED_Status;

private:
    QTimer m_timer;
    RevPiDIO* m_io;
    int m_address_light;
    int m_address_button;
    LED_Status m_LEDstatus;
    quint64 m_blinkFrequency;
    bool m_old_buttonState;
    bool m_button_pressed;
    QDateTime m_dateTime_lastPress;
    int m_clickTime;


signals:
    void signal_button_pressed();
    void signal_button_released();
    void signal_button_clicked();

public slots:
    void slot_setLight(LED_Status status);

private slots:
    void slot_timer_fired();
};

#endif // LIGHTBUTTON_H
