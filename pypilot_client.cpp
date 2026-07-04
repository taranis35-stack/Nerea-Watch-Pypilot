#include "pypilot_client.h"
#include <WiFiClient.h>
#include <WiFi.h>
#include "pilot_ui.h"
#include "HWCDC.h"
#include <math.h>
#include "config.h"
#include <stdlib.h>

extern HWCDC USBSerial;

static WiFiClient client;

static bool wifiConnected = false;
static bool pilotConnected = false;

static int heading = 0;
static int headingCommand = 0;
static bool autoMode = false;

static String mode = "COMPASS";

static uint32_t lastConnectTry = 0;
static String rxLine;

void pypilot_begin()
{

    if(strlen(cfgSSID)==0 || strlen(cfgHost)==0)
    {
        pilotUI_setConnected(false);
        return;
    }
    USBSerial.println();
    USBSerial.println("Connexion WiFi...");

    WiFi.begin(cfgSSID,cfgPassword);

    uint32_t start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start > 10000)
        {
            USBSerial.println();
            USBSerial.println("Echec WiFi");

            wifiConnected = false;
            pilotUI_setConnected(false);

            return;
        }

        delay(500);
        USBSerial.print(".");
    }

    USBSerial.println();
    USBSerial.println("WiFi connecté");

    wifiConnected = true;
}

static void parseLine(const String &line)
{
    int p = line.indexOf('=');

    if (p < 0)
        return;

    String key = line.substring(0, p);
    String value = line.substring(p + 1);

    key.trim();
    value.trim();

    //-------------------------------------------------
    // Heading
    //-------------------------------------------------

    if (key == "imu.heading_lowpass")
    {
        heading = (int)round(value.toFloat());

        pilotUI_setHeading(heading, 'M');
    }

    //-------------------------------------------------
    // Heading command
    //-------------------------------------------------

    else if (key == "ap.heading_command")
    {
        headingCommand = (int)round(value.toFloat());
    }

    //-------------------------------------------------
    // AUTO/STBY
    //-------------------------------------------------

    else if (key == "ap.enabled")
    {
        autoMode = value.equalsIgnoreCase("true");

        pilotUI_setAuto(autoMode);
    }

    //-------------------------------------------------
    // Current mode
    //-------------------------------------------------

    else if (key == "ap.mode")
    {
        USBSerial.print("RX MODE=");
        USBSerial.println(value);
       
        value.replace("\"", "");

        mode = value;

        mode.toUpperCase();

        pilotUI_setMode(mode.c_str());
    }

    //-------------------------------------------------
    // Available modes
    //-------------------------------------------------

    else if (key == "ap.modes")
    {
        USBSerial.print("Modes : ");
        USBSerial.println(value);
    }
}

void pypilot_update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        pilotConnected = false;
        return;
    }

    if (!client.connected())
    {
        pilotConnected = false;

        if (millis() - lastConnectTry < 2000)
            return;

        lastConnectTry = millis();

        USBSerial.println("Connexion Pypilot...");

        if(!client.connect(cfgHost,atoi(cfgPort)))
        {
            USBSerial.println("Echec");
            return;
        }

        USBSerial.println("Connecté");

        pilotConnected = true;

        client.print("watch={\"imu.heading_lowpass\":0}\n");
        client.print("watch={\"ap.heading\":0}\n");
        client.print("watch={\"ap.heading_command\":0}\n");
        client.print("watch={\"ap.enabled\":0}\n");
        client.print("watch={\"ap.mode\":0}\n");
        client.print("watch={\"ap.modes\":0}\n");
    }

    while (client.available())
    {
        char c = client.read();

        if (c == '\r')
            continue;

        if (c == '\n')
        {
            if (rxLine.length())
            {
                parseLine(rxLine);
                rxLine = "";
            }
        }
        else
        {
            rxLine += c;
        }
    }
}

void pypilot_send(const char *cmd)
{

    if(strlen(cfgHost)==0)
        return;

    WiFiClient tx;
    if(!tx.connect(cfgHost,atoi(cfgPort)))
        return;
    tx.print(cmd);
    tx.print('\n');
    tx.stop();
    USBSerial.print("TX : ");
    USBSerial.println(cmd);
}

bool pypilot_connected()
{
    return pilotConnected;
}

int pypilot_heading()
{
    return heading;
}

int pypilot_headingCommand()
{
    return headingCommand;
}

bool pypilot_enabled()
{
    return autoMode;
}

const char *pypilot_mode()
{
    return mode.c_str();
}


void watchCommand(const char *cmd)
{
USBSerial.print("CMD=");
USBSerial.println(cmd);

    //--------------------------------------------------
    // MODE
    //--------------------------------------------------

    if(!strncmp(cmd,"MODE:",5))
    {
        char txt[40];
        sprintf(txt,"ap.mode=\"%s\"",cmd+5);
        pypilot_send(txt);
        pypilot_send("watch={\"ap.mode\":true}");
        return;
    }
    //--------------------------------------------------
    // AUTO / STBY
    //--------------------------------------------------

    if (!strcmp(cmd, "AUTO"))
    {
        if (autoMode)
        {
            pypilot_send("ap.enabled=false");
        }
        else
        {
            char txt[40];

            sprintf(txt,
                    "ap.heading_command=%d",
                    heading);

            pypilot_send(txt);

            delay(100);

            pypilot_send("ap.enabled=true");
        }

        return;
    }

    //--------------------------------------------------
    // +1
    //--------------------------------------------------

     if(!strcmp(cmd,"+1"))
    {
        if(autoMode)
        {
            char txt[40];
            sprintf(txt,"ap.heading_command=%d",headingCommand+1);
            pypilot_send(txt);
        }
        else
            pypilot_send("servo.command=1");
        return;
    }

    //--------------------------------------------------
    // -1
    //--------------------------------------------------

    if(!strcmp(cmd,"-1"))
    {
        if(autoMode)
        {
            char txt[40];
            sprintf(txt,"ap.heading_command=%d",headingCommand-1);
            pypilot_send(txt);
        }
        else
            pypilot_send("servo.command=-1");
        return;
    }
    //--------------------------------------------------
    // +10
    //--------------------------------------------------

    if(!strcmp(cmd,"+10"))
    {
        if(autoMode)
        {
            char txt[40];
            sprintf(txt,"ap.heading_command=%d",headingCommand+10);
            pypilot_send(txt);
        }
        else
            pypilot_send("servo.command=1");
        return;
    }

    //--------------------------------------------------
    // -10
    //--------------------------------------------------

    if(!strcmp(cmd,"-10"))
    {
        if(autoMode)
        {
            char txt[40];
            sprintf(txt,"ap.heading_command=%d",headingCommand-10);
            pypilot_send(txt);
        }
        else
            pypilot_send("servo.command=-1");
        return;
    }
}