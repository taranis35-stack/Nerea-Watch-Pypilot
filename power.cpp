#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"
#include "HWCDC.h"
#include "Arduino_GFX_Library.h"
#include "splash.h"

extern Arduino_CO5300 *gfx;
extern HWCDC USBSerial;

#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>
#include "power.h"

static XPowersPMU PMU;

static bool screenOn=true;
static bool buttonPressed=false;
static bool longDone=false;
static uint32_t pressTime=0;
static uint32_t lastActivity=0;

void power_activity()
{
  lastActivity=millis();
}


void power_sleep()
{
  if(!screenOn)
    return;

  screenOn=false;
  lastActivity=millis();
  gfx->displayOff();
  USBSerial.println("SCREEN OFF");
}

void power_wake()
{
  if(screenOn)
    return;

  screenOn=true;
  lastActivity=millis();
  gfx->displayOn();
  USBSerial.println("SCREEN ON");
}

void power_begin()
{
  if(!PMU.init(Wire,IIC_SDA,IIC_SCL))
  {
    USBSerial.println("PMU ERROR");
    return;
  }

  PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
  PMU.clearIrqStatus();

  PMU.enableIRQ(
    XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ |
    XPOWERS_AXP2101_PKEY_POSITIVE_IRQ);

  lastActivity=millis();

  USBSerial.println("PMU READY");
}

void power_update()
{
  PMU.getIrqStatus();

  if(PMU.isPekeyNegativeIrq())
  {
    buttonPressed=true;
    longDone=false;
    pressTime=millis();
  }

  if(PMU.isPekeyPositiveIrq())
  {
    buttonPressed=false;

    if(!longDone)
    {
      if(screenOn)
        power_sleep();
      else
        power_wake();
    }
  }

  if(buttonPressed &&
     !longDone &&
     millis()-pressTime>=2000)
  {
    longDone=true;
    splashShutdown();
    PMU.shutdown();
    while(true);
  }
  if(screenOn && millis()-lastActivity>=15000)
    power_sleep();

  PMU.clearIrqStatus();
}

bool power_screenOn()
{
  return screenOn;
}

bool power_touchEnabled()
{
  return screenOn;
}

void power_setTouch(bool enabled)
{
  if(enabled)
    power_wake();
  else
    power_sleep();
}

int power_batteryPercent()
{
    return PMU.getBatteryPercent();
}

bool power_isCharging()
{
    return PMU.isVbusIn();
}

float power_batteryVoltage()
{
    return PMU.getBattVoltage() / 1000.0;
}

bool power_vbus()
{
    return PMU.isVbusIn();
}

void power_setBrightness(
  int percent)
{
  if(percent < 10)
    percent = 10;

  if(percent > 100)
    percent = 100;

  gfx->setBrightness(
    map(
      percent,
      10,
      100,
      0,
      255));
}