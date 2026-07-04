#include "config.h"
#include <Preferences.h>

static Preferences prefs;
static int cfgBrightness = 80;

char cfgSSID[33] = "";
char cfgPassword[33] = "";
char cfgHost[32] = "";
char cfgPort[8] = "23322";

void configLoad() {
  prefs.begin("watch", true);

  prefs.getString("ssid", "").toCharArray(cfgSSID, sizeof(cfgSSID));
  prefs.getString("pass", "").toCharArray(cfgPassword, sizeof(cfgPassword));
  prefs.getString("host", "").toCharArray(cfgHost, sizeof(cfgHost));
  prefs.getString("port", "23322").toCharArray(cfgPort, sizeof(cfgPort));
  cfgBrightness =
    prefs.getInt(
      "bright",
      80);
  prefs.end();
}

void configSave() {
  prefs.begin("watch", false);

  prefs.putString("ssid", cfgSSID);
  prefs.putString("pass", cfgPassword);
  prefs.putString("host", cfgHost);
  prefs.putString("port", cfgPort);
  prefs.putInt(
    "bright",
    cfgBrightness);
  prefs.end();
}

int configGetBrightness()
{
  return cfgBrightness;
}

void configSetBrightness(
  int value)
{
  cfgBrightness =
    value;

  configSave();
}