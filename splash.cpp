#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "splash.h"
#include "images/splash_logo.h"

extern Arduino_GFX *gfx;

void splashBoot()
{
    gfx->fillScreen(RGB565_BLACK);
    gfx->draw16bitRGBBitmap(
        (410-208)/2,
        (502-277)/2,
        splash_logo,
        208,
        277);
    delay(500);
}

void splashShutdown()
{
    gfx->fillScreen(RGB565_BLACK);
    gfx->draw16bitRGBBitmap(
        (410-208)/2,
        (502-277)/2,
        splash_logo,
        208,
        277);
    delay(500);
}