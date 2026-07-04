#include <Arduino.h>
#include "buttons.h"
#include "HWCDC.h"
#include "pypilot_client.h"
#include "power.h"
#include "pilot_ui.h"

extern HWCDC USBSerial;
extern uint32_t lastActivity;

static bool lastState;
static uint32_t pressTime = 0;
static bool configOpened = false;

static bool autoSleepPending = false;
static uint32_t autoSleepTime = 0;

void buttons_begin()
{
    pinMode(9, INPUT_PULLUP);

    delay(10);

    lastState =
        digitalRead(9);
}

void buttons_update()
{
    if(autoSleepPending &&
       millis() >= autoSleepTime)
    {
        power_sleep();

        autoSleepPending = false;
    }

    bool state =
        digitalRead(9);

    //--------------------------------------------------
    // BUTTON PRESSED
    //--------------------------------------------------

    if(lastState == HIGH &&
       state == LOW)
    {
        power_activity();

        pressTime =
            millis();

        configOpened = false;
    }

    //--------------------------------------------------
    // LONG PRESS
    //--------------------------------------------------

    if(lastState == LOW &&
       state == LOW)
    {
        if(!configOpened &&
           millis() - pressTime > 2000)
        {
            if(brightnessVisible())
            {
                brightnessHide();
            }
            else if(configIsVisible())
            {
                brightnessShow();
            }
            else
            {
                configShow();
            }

            configOpened = true;
        }
    }

    //--------------------------------------------------
    // BUTTON RELEASED
    //--------------------------------------------------

    if(lastState == LOW &&
       state == HIGH)
    {
        if(!configOpened)
        {
            bool wasOff =
                !power_screenOn();

            if(wasOff)
                power_wake();

            watchCommand("AUTO");

            power_activity();

            if(wasOff)
            {
                autoSleepPending = true;

                autoSleepTime =
                    millis() + 1000;
            }
        }
    }

    lastState = state;
}