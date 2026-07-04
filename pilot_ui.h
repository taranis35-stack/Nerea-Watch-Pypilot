#pragma once

void pilotUI_create();
void pilotUI_update();
void pilotUI_setHeading(int heading, char ref);
void pilotUI_setMode(const char *mode);
void pilotUI_setBattery(int percent, bool charging);
void pilotUI_setAuto(bool enabled);
void pilotUI_setConnected(bool connected);
typedef void (*PilotCommandCallback)(const char *);
void pilotUI_setCommandCallback(PilotCommandCallback cb);
void configShow();
void configHide();
bool configIsVisible();
void brightnessShow();
void brightnessHide();
bool brightnessVisible();