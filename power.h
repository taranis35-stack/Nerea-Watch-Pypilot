#pragma once

void power_begin();
void power_update();

void power_sleep();
void power_wake();
void power_activity();

bool power_touchEnabled();
bool power_screenOn();
void power_setTouch(bool enabled);

int power_batteryPercent();
bool power_isCharging();

float power_batteryVoltage();
bool power_vbus();

void power_setBrightness(
  int percent);