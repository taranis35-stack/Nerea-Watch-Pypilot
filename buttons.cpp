#include <Arduino.h>
#include "buttons.h"
#include "HWCDC.h"
#include "pypilot_client.h"
#include "power.h"
#include "pilot_ui.h"

extern HWCDC USBSerial;
extern uint32_t lastActivity;

static bool lastState;
static uint32_t pressTime=0;
static bool configOpened=false;

void buttons_begin()
{
    pinMode(9,INPUT_PULLUP);

    delay(10);
    lastState=digitalRead(9);
}

void buttons_update()
{
    bool state =
      digitalRead(9);

    if(lastState==HIGH &&
       state==LOW)
    {
        power_activity();

        pressTime =
          millis();

        configOpened =
          false;
    }

    if(lastState==LOW &&
       state==LOW)
    {
        if(!configOpened &&
           millis()-pressTime>2000)
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

    if(lastState==LOW &&
       state==HIGH)
    {
        if(!configOpened)
            watchCommand("AUTO");

        power_activity();
    }

    lastState = state;
}