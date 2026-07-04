#include <Wire.h>
#include <Arduino.h>
#include "pin_config.h"
#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "lv_conf.h"
#include "pilot_ui.h"
#include "HWCDC.h"
#include "buttons.h"
#include "power.h"
#include "splash.h"
#include "pypilot_client.h"
#include "config.h"





HWCDC USBSerial;

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;


lv_display_t *disp;
lv_color_t *disp_draw_buf;

//#define DIRECT_RENDER_MODE

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS,
  LCD_SCLK,
  LCD_SDIO0,
  LCD_SDIO1,
  LCD_SDIO2,
  LCD_SDIO3);

Arduino_CO5300 *gfx = new Arduino_CO5300(
  bus,
  LCD_RESET,
  0,
  LCD_WIDTH,
  LCD_HEIGHT,
  22,
  0,
  0,
  0);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(
  new Arduino_FT3x68(
    IIC_Bus,
    FT3168_DEVICE_ADDRESS,
    DRIVEBUS_DEFAULT_VALUE,
    TP_INT,
    Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt() {
  FT3168->IIC_Interrupt_Flag = true;
}

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  USBSerial.println(buf);
}
#endif

uint32_t millis_cb() {
  return millis();
}

void my_disp_flush(lv_display_t *disp,
                   const lv_area_t *area,
                   uint8_t *px_map) {
#ifndef DIRECT_RENDER_MODE

  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(
    area->x1,
    area->y1,
    (uint16_t *)px_map,
    w,
    h);

#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev,
                      lv_indev_data_t *data) {

  if (!power_touchEnabled()) {
    FT3168->IIC_Read_Device_Value(
      FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);

    FT3168->IIC_Read_Device_Value(
      FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
    FT3168->IIC_Interrupt_Flag = false;
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  int32_t x =
    FT3168->IIC_Read_Device_Value(
      FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);

  int32_t y =
    FT3168->IIC_Read_Device_Value(
      FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (FT3168->IIC_Interrupt_Flag) {
    power_activity();
    FT3168->IIC_Interrupt_Flag = false;

    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void rounder_event_cb(lv_event_t *e) {
  lv_area_t *area =
    (lv_area_t *)lv_event_get_param(e);

  area->x1 = (area->x1 >> 1) << 1;
  area->y1 = (area->y1 >> 1) << 1;

  area->x2 = ((area->x2 >> 1) << 1) + 1;
  area->y2 = ((area->y2 >> 1) << 1) + 1;
}



void setup() {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  USBSerial.begin(115200);

  if (!gfx->begin()) {
    USBSerial.println("Display init failed");
    while (1)
      ;
  }

//  gfx->Display_Brightness(64);

  gfx->fillScreen(RGB565_BLACK);
  splashBoot();

  Wire.begin(IIC_SDA, IIC_SCL);
  power_begin();
  while (!FT3168->begin()) {
    USBSerial.println("Touch init...");
    delay(1000);
  }

  FT3168->IIC_Write_Device_State(
    FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
    FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

  lv_init();

  lv_tick_set_cb(millis_cb);

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();

#ifdef DIRECT_RENDER_MODE
  bufSize = screenWidth * screenHeight;
#else
  bufSize = screenWidth * 50;
#endif

#ifdef ESP32

#if defined(DIRECT_RENDER_MODE)

  disp_draw_buf =
    (lv_color_t *)gfx->getFramebuffer();

#else

  disp_draw_buf =
    (lv_color_t *)heap_caps_malloc(
      bufSize * 2,
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  if (!disp_draw_buf)
    disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);

#endif

#else

  disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);

#endif

  disp = lv_display_create(
    screenWidth,
    screenHeight);

  lv_display_set_flush_cb(
    disp,
    my_disp_flush);

#ifdef DIRECT_RENDER_MODE

  lv_display_set_buffers(
    disp,
    disp_draw_buf,
    NULL,
    bufSize * 2,
    LV_DISPLAY_RENDER_MODE_DIRECT);

#else

  lv_display_set_buffers(
    disp,
    disp_draw_buf,
    NULL,
    bufSize * 2,
    LV_DISPLAY_RENDER_MODE_PARTIAL);

#endif

  lv_indev_t *indev = lv_indev_create();

  lv_indev_set_type(
    indev,
    LV_INDEV_TYPE_POINTER);

  lv_indev_set_read_cb(
    indev,
    my_touchpad_read);

  lv_display_add_event_cb(
    disp,
    rounder_event_cb,
    LV_EVENT_INVALIDATE_AREA,
    NULL);

  lv_obj_set_style_bg_color(
    lv_scr_act(),
    lv_color_black(),
    0);

  // ==========================
  // NEREA UI
  // ==========================
  configLoad();
  pilotUI_create();
  pilotUI_setCommandCallback(watchCommand);
  pypilot_begin();
  buttons_begin();
  

  USBSerial.println("Ready");
  // Test provisoire
}

void loop()
{
  power_update();
  buttons_update();
  lv_task_handler();
  pypilot_update();


  pilotUI_setBattery(
    power_batteryPercent(),
    power_isCharging());


  pilotUI_update();

#ifdef DIRECT_RENDER_MODE
  gfx->flush();
#else
#ifdef CANVAS
  gfx->flush();
#endif
#endif

  delay(20);
}