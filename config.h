#pragma once

extern char cfgSSID[33];
extern char cfgPassword[33];
extern char cfgHost[32];
extern char cfgPort[8];

void configLoad();
void configSave();
int configGetBrightness();
void configSetBrightness(int value);