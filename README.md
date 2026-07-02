Nerea Watch

Nerea Watch is a standalone remote control in watch form for pypilot versions 0.56 and 0.70, specifically designed for use at sea.

It allows you to control an autopilot directly from an ESP32-C6-based smartwatch (https://www.waveshare.com/product/arduino/boards-kits/esp32-c6/esp32-c6-touch-amoled-2.06.htm)
without relying on a tablet or computer.

The project aims to provide a simple, fast, and legible interface that can be operated with one hand, even in challenging conditions.

Features
Autopilot engagement/disengagement.
Heading adjustment:
±1°
±10°
Autopilot mode selection:
Compass
nav
GPS
Wind
True Wind
Magnetic heading display.
Active mode display.
pypilot connection status.
Battery level display.
Configuration directly from the watch:
Wi-Fi SSID
Wi-Fi password
pypilot server IP address
TCP port (default: 23322)
Permanent configuration saving in ESP32-C6 memory.
Compatibility

Designed to work with pypilot.

Compatible with recent pypilot versions using the TCP interface (tested from the 0.56 branch up to version 0.70).

Hardware

The project is based on an ESP32 watch equipped with:

410×502 pixel color touchscreen
Built-in Wi-Fi
Rechargeable battery
Physical button
First Startup

Upon first startup:

no network settings are preconfigured;
only port 23322 is set by default.

Hold down the physical button for a few seconds to open the configuration menu, enter the Wi-Fi network settings and the pypilot server IP address, then save the configuration. Settings are retained after power-off or firmware updates.

Philosophy

Nerea Watch prioritizes:

a minimalist interface;
excellent readability;
low power consumption;
very low latency between the user and the autopilot.

The top button turns the pilot on or off, pressing it for more than 3 seconds lets you enter the connection settings. The bottom button puts it to sleep, and pressing it for more than 3 seconds turns the watch off.

The entire interface is developed using LVGL8, and the firmware is written in C++ using the Arduino environment for ESP32.
The audio implementation remains to be done, but I cannot find sufficient documentation to initialize the register settings within the Arduino environment.
Developed for sailors, by a sailor, using pypilot, and designed to provide a lightweight, standalone remote control that is always within easy reach.


License

Open Source project.
