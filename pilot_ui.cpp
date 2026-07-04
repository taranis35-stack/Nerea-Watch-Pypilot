#include "pilot_ui.h"
#include <lvgl.h>
#include <stdio.h>
#include <cstring>
#include "config.h"
#define SAFE_MARGIN 38
#include "power.h"
static bool brightnessOpen = false;
static lv_obj_t *brightnessPanel;
static lv_obj_t *brightnessSlider;
static lv_obj_t *brightnessValue;
static void brightness_event(lv_event_t *e);
static lv_style_t styleBtn;
static lv_style_t styleBtnPressed;
static lv_style_t styleAuto;
static lv_style_t styleAutoPressed;
static lv_style_t styleAutoOff;
static lv_obj_t *lblStatus;
static lv_obj_t *lblBattery;
static lv_obj_t *lblCharge;
static bool batteryCharging = false;
static lv_obj_t *lblHeading;
static lv_obj_t *btnMode;
static lv_obj_t *lblMode;
static lv_obj_t *btnAuto;
static lv_obj_t *lblAuto;
static lv_obj_t *btnM1;
static lv_obj_t *btnP1;
static lv_obj_t *btnM10;
static lv_obj_t *btnP10;
static PilotCommandCallback commandCallback = nullptr;
static lv_obj_t *modeWindow;
static lv_obj_t *modeButtons[5];
static lv_obj_t *configWindow;
static bool configVisible = false;
static lv_obj_t *configBox;
static lv_obj_t *txtField;
static lv_obj_t *kbd;
static int editField = -1;
static lv_obj_t *configButtons[6];
static lv_obj_t *configLabelObj[6];

void configShow() {
  configVisible = true;
  lv_obj_clear_flag(configWindow, LV_OBJ_FLAG_HIDDEN);
}

void configHide() {
  configVisible = false;

  lv_obj_add_flag(configWindow, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(txtField, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(btnMode, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btnAuto, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btnM1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btnP1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btnM10, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(btnP10, LV_OBJ_FLAG_HIDDEN);
}

static void editConfig(int field) {
  editField = field;
  configHide();
  lv_obj_add_flag(btnMode, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnAuto, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnM1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnP1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnM10, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnP10, LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(txtField, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(kbd, LV_OBJ_FLAG_HIDDEN);

  switch (field) {
    case 0:
      lv_textarea_set_placeholder_text(txtField, "WiFi SSID");
      lv_textarea_set_text(txtField, cfgSSID);
      lv_keyboard_set_mode(kbd, LV_KEYBOARD_MODE_TEXT_LOWER);
      break;

    case 1:
      lv_textarea_set_placeholder_text(txtField, "Password");
      lv_textarea_set_text(txtField, cfgPassword);
      lv_keyboard_set_mode(kbd, LV_KEYBOARD_MODE_TEXT_LOWER);
      break;

    case 2:
      lv_textarea_set_placeholder_text(txtField, "Pypilot IP");
      lv_textarea_set_text(txtField, cfgHost);
      lv_keyboard_set_mode(kbd, LV_KEYBOARD_MODE_TEXT_LOWER);
      break;

    case 3:
      lv_textarea_set_placeholder_text(txtField, "Port");
      lv_textarea_set_text(txtField, cfgPort);
      lv_keyboard_set_mode(kbd, LV_KEYBOARD_MODE_NUMBER);
      break;
  }

  lv_obj_move_foreground(txtField);
  lv_obj_move_foreground(kbd);
  lv_textarea_set_cursor_click_pos(txtField, true);
  lv_obj_add_state(txtField, LV_STATE_FOCUSED);
}

bool configIsVisible() {
  return configVisible;
}
static const char *modeLabel[5] = {
  "COMPASS",
  "GPS",
  "NAV",
  "WIND",
  "TRUE WIND"
};

static const char *modeCmd[5] = {
  "MODE:compass",
  "MODE:gps",
  "MODE:nav",
  "MODE:wind",
  "MODE:truewind"
};

static const char *configLabel[5] = {
  "WiFi SSID",
  "Password",
  "Pypilot IP",
  "Port",
  "SAVE"
};

static void mode_event(lv_event_t *e) {
  if (lv_obj_has_flag(modeWindow, LV_OBJ_FLAG_HIDDEN))
    lv_obj_clear_flag(modeWindow, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(modeWindow, LV_OBJ_FLAG_HIDDEN);
}

static void mode_select_event(lv_event_t *e) {
  const char *mode = (const char *)lv_event_get_user_data(e);
  if (commandCallback)
    commandCallback(mode);
  lv_obj_add_flag(modeWindow, LV_OBJ_FLAG_HIDDEN);
}

static void config_event(lv_event_t *e) {
  configHide();
}

static void config_select_event(
  lv_event_t *e) {
  int id =
    (int)(intptr_t)
      lv_event_get_user_data(e);

  switch (id) {
    case 0:
    case 1:
    case 2:
    case 3:

      editConfig(
        id);

      break;

    case 4:

      brightnessShow();

      break;

    case 5:

      configSave();

      configHide();

      break;
  }
}

static void keyboard_event(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code != LV_EVENT_READY && code != LV_EVENT_CANCEL)
    return;

  if (code == LV_EVENT_READY) {
    const char *txt = lv_textarea_get_text(txtField);

    switch (editField) {
      case 0: strncpy(cfgSSID, txt, sizeof(cfgSSID) - 1); break;
      case 1: strncpy(cfgPassword, txt, sizeof(cfgPassword) - 1); break;
      case 2: strncpy(cfgHost, txt, sizeof(cfgHost) - 1); break;
      case 3: strncpy(cfgPort, txt, sizeof(cfgPort) - 1); break;
    }
  }

  lv_obj_add_flag(txtField, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(kbd, LV_OBJ_FLAG_HIDDEN);

  configShow();
}

static void auto_event(lv_event_t *e) {
  if (commandCallback)
    commandCallback("AUTO");
}
static void minus1_event(lv_event_t *e) {
  if (commandCallback)
    commandCallback("-1");
}
static void plus1_event(lv_event_t *e) {
  if (commandCallback)
    commandCallback("+1");
}
static void minus10_event(lv_event_t *e) {
  if (commandCallback)
    commandCallback("-10");
}
static void plus10_event(lv_event_t *e) {
  if (commandCallback)
    commandCallback("+10");
}

static void initStyles() {
  lv_style_init(&styleBtn);

  lv_style_set_bg_color(
    &styleBtn,
    lv_color_hex(0x181818));

  lv_style_set_border_color(
    &styleBtn,
    lv_color_hex(0x707070));

  lv_style_set_border_width(
    &styleBtn,
    3);

  lv_style_set_radius(
    &styleBtn,
    14);

  lv_style_set_text_color(
    &styleBtn,
    lv_color_white());

  lv_style_set_pad_all(
    &styleBtn,
    0);

  lv_style_init(&styleBtnPressed);

  lv_style_set_bg_color(
    &styleBtnPressed,
    lv_color_hex(0x003D73));

  lv_style_set_border_color(
    &styleBtnPressed,
    lv_color_hex(0x00D8FF));

  lv_style_set_border_width(
    &styleBtnPressed,
    3);

  lv_style_set_radius(
    &styleBtnPressed,
    14);

  lv_style_set_translate_y(
    &styleBtnPressed,
    2);

  lv_style_init(&styleAuto);

  lv_style_set_bg_color(
    &styleAuto,
    lv_palette_main(LV_PALETTE_GREEN));

  lv_style_set_border_color(
    &styleAuto,
    lv_palette_lighten(LV_PALETTE_GREEN, 2));

  lv_style_set_border_width(
    &styleAuto,
    3);

  lv_style_set_radius(
    &styleAuto,
    14);

  lv_style_init(&styleAutoPressed);

  lv_style_set_bg_color(
    &styleAutoPressed,
    lv_palette_lighten(LV_PALETTE_GREEN, 1));

  lv_style_set_border_color(
    &styleAutoPressed,
    lv_color_hex(0x00FFFF));

  lv_style_set_border_width(
    &styleAutoPressed,
    3);

  lv_style_set_radius(
    &styleAutoPressed,
    14);

  lv_style_set_translate_y(
    &styleAutoPressed,
    2);

  lv_style_init(&styleAutoOff);

  lv_style_set_bg_color(
    &styleAutoOff,
    lv_palette_main(LV_PALETTE_RED));

  lv_style_set_border_color(
    &styleAutoOff,
    lv_palette_lighten(LV_PALETTE_RED, 2));

  lv_style_set_border_width(
    &styleAutoOff,
    3);

  lv_style_set_radius(
    &styleAutoOff,
    14);
}

void pilotUI_setCommandCallback(PilotCommandCallback cb) {
  commandCallback = cb;
}

static void brightness_event(
  lv_event_t *e) {
  int value =
    lv_slider_get_value(
      brightnessSlider);

  char txt[10];

  sprintf(
    txt,
    "%d%%",
    value);

  lv_label_set_text(
    brightnessValue,
    txt);

  configSetBrightness(
    value);

  power_setBrightness(
    value);
}

void pilotUI_create() {
  initStyles();
  lv_obj_t *lab;

  lv_obj_set_style_bg_color(
    lv_scr_act(),
    lv_color_black(),
    0);

  //--------------------------------------------------
  // NEREA
  //--------------------------------------------------

  lblStatus = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_font(
    lblStatus,
    &lv_font_montserrat_24,
    0);

  lv_obj_set_style_text_color(
    lblStatus,
    lv_palette_main(LV_PALETTE_GREEN),
    0);

  lv_label_set_text(
    lblStatus,
    LV_SYMBOL_OK " PYPILOT");

  lv_obj_align(
    lblStatus,
    LV_ALIGN_TOP_LEFT,
    SAFE_MARGIN,
    16);


  //--------------------------------------------------
  // Charge
  //--------------------------------------------------

  lblCharge = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_font(
    lblCharge,
    &lv_font_montserrat_24,
    0);

  lv_obj_set_style_text_color(
    lblCharge,
    lv_palette_main(LV_PALETTE_GREEN),
    0);

  lv_label_set_text(
    lblCharge,
    LV_SYMBOL_CHARGE);

  lv_obj_align(
    lblCharge,
    LV_ALIGN_TOP_RIGHT,
    -110,
    16);

  lv_obj_add_flag(
    lblCharge,
    LV_OBJ_FLAG_HIDDEN);

  //--------------------------------------------------
  // Batterie
  //--------------------------------------------------

  lblBattery = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_font(
    lblBattery,
    &lv_font_montserrat_24,
    0);

  lv_obj_set_style_text_color(
    lblBattery,
    lv_color_white(),
    0);

  lv_label_set_text(
    lblBattery,
    "82%");

  lv_obj_align(
    lblBattery,
    LV_ALIGN_TOP_RIGHT,
    -SAFE_MARGIN,
    16);

  //--------------------------------------------------
  // Trait
  //--------------------------------------------------

  lv_obj_t *line = lv_obj_create(lv_scr_act());

  lv_obj_remove_style_all(line);

  lv_obj_set_size(
    line,
    330,
    2);

  lv_obj_set_style_bg_color(
    line,
    lv_color_hex(0x404040),
    0);

  lv_obj_align(
    line,
    LV_ALIGN_TOP_MID,
    0,
    52);

  //--------------------------------------------------
  // Cap
  //--------------------------------------------------

  lblHeading = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_font(
    lblHeading,
    &lv_font_montserrat_48,
    0);

  lv_obj_set_style_text_color(
    lblHeading,
    lv_color_white(),
    0);

  lv_label_set_text(
    lblHeading,
    "182° M");

  lv_obj_align(
    lblHeading,
    LV_ALIGN_TOP_LEFT,
    28,
    70);

  //--------------------------------------------------
  // COMPASS
  //--------------------------------------------------

  btnMode = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnMode,
    &styleBtn,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnMode,
    &styleBtnPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnMode,
    155,
    56);

  lv_obj_align(
    btnMode,
    LV_ALIGN_TOP_RIGHT,
    -22,
    66);

  lv_obj_add_event_cb(
    btnMode,
    mode_event,
    LV_EVENT_CLICKED,
    NULL);

  lblMode = lv_label_create(btnMode);

  lv_obj_set_style_text_font(
    lblMode,
    &lv_font_montserrat_20,
    0);

  lv_label_set_text(
    lblMode,
    "COMPASS " LV_SYMBOL_DOWN);

  lv_obj_center(lblMode);

  //--------------------------------------------------
  // -1
  //--------------------------------------------------

  btnM1 = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnM1,
    &styleBtn,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnM1,
    &styleBtnPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnM1,
    170,
    90);

  lv_obj_align(
    btnM1,
    LV_ALIGN_TOP_LEFT,
    18,
    165);

  lv_obj_add_event_cb(
    btnM1,
    minus1_event,
    LV_EVENT_CLICKED,
    NULL);

  lab = lv_label_create(btnM1);

  lv_obj_set_style_text_font(
    lab,
    &lv_font_montserrat_38,
    0);

  lv_label_set_text(
    lab,
    "-1");

  lv_obj_center(lab);

  //--------------------------------------------------
  // +1
  //--------------------------------------------------

  btnP1 = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnP1,
    &styleBtn,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnP1,
    &styleBtnPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnP1,
    170,
    90);

  lv_obj_align(
    btnP1,
    LV_ALIGN_TOP_RIGHT,
    -18,
    165);

  lv_obj_add_event_cb(
    btnP1,
    plus1_event,
    LV_EVENT_CLICKED,
    NULL);

  lab = lv_label_create(btnP1);

  lv_obj_set_style_text_font(
    lab,
    &lv_font_montserrat_38,
    0);

  lv_label_set_text(
    lab,
    "+1");

  lv_obj_center(lab);

  //--------------------------------------------------
  // -10
  //--------------------------------------------------

  btnM10 = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnM10,
    &styleBtn,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnM10,
    &styleBtnPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnM10,
    170,
    90);

  lv_obj_align(
    btnM10,
    LV_ALIGN_TOP_LEFT,
    18,
    272);

  lv_obj_add_event_cb(
    btnM10,
    minus10_event,
    LV_EVENT_CLICKED,
    NULL);

  lab = lv_label_create(btnM10);

  lv_obj_set_style_text_font(
    lab,
    &lv_font_montserrat_38,
    0);

  lv_label_set_text(
    lab,
    "-10");

  lv_obj_center(lab);

  //--------------------------------------------------
  // +10
  //--------------------------------------------------

  btnP10 = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnP10,
    &styleBtn,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnP10,
    &styleBtnPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnP10,
    170,
    90);

  lv_obj_align(
    btnP10,
    LV_ALIGN_TOP_RIGHT,
    -18,
    272);

  lv_obj_add_event_cb(
    btnP10,
    plus10_event,
    LV_EVENT_CLICKED,
    NULL);

  lab = lv_label_create(btnP10);

  lv_obj_set_style_text_font(
    lab,
    &lv_font_montserrat_38,
    0);

  lv_label_set_text(
    lab,
    "+10");

  lv_obj_center(lab);

  //--------------------------------------------------
  // AUTO
  //--------------------------------------------------

  btnAuto = lv_btn_create(lv_scr_act());

  lv_obj_add_style(
    btnAuto,
    &styleAuto,
    LV_PART_MAIN);

  lv_obj_add_style(
    btnAuto,
    &styleAutoPressed,
    LV_PART_MAIN | LV_STATE_PRESSED);

  lv_obj_set_size(
    btnAuto,
    340,
    90);

  lv_obj_align(
    btnAuto,
    LV_ALIGN_BOTTOM_MID,
    0,
    -20);

  lv_obj_add_event_cb(
    btnAuto,
    auto_event,
    LV_EVENT_CLICKED,
    NULL);

  lblAuto = lv_label_create(btnAuto);

  lv_obj_set_style_text_font(
    lblAuto,
    &lv_font_montserrat_40,
    0);

  lv_label_set_text(
    lblAuto,
    "AUTO");

  lv_obj_center(lblAuto);

  modeWindow = lv_obj_create(lv_scr_act());

  lv_obj_set_size(
    modeWindow,
    280,
    360);

  lv_obj_center(modeWindow);

  lv_obj_add_flag(
    modeWindow,
    LV_OBJ_FLAG_HIDDEN);

  for (int i = 0; i < 5; i++) {
    modeButtons[i] = lv_btn_create(modeWindow);

    lv_obj_set_width(
      modeButtons[i],
      250);

    lv_obj_align(
      modeButtons[i],
      LV_ALIGN_TOP_MID,
      0,
      20 + i * 60);

    lv_obj_add_event_cb(
      modeButtons[i],
      mode_select_event,
      LV_EVENT_CLICKED,
      (void *)modeCmd[i]);

    lab = lv_label_create(modeButtons[i]);
    lv_obj_set_style_text_font(
      lab,
      &lv_font_montserrat_28,
      0);

    lv_label_set_text(
      lab,
      modeLabel[i]);

    lv_obj_center(lab);
  }

  //--------------------------------------------------
  // CONFIGURATION
  //--------------------------------------------------

  configWindow = lv_obj_create(lv_scr_act());

  lv_obj_remove_style_all(configWindow);

  lv_obj_set_size(
    configWindow,
    LV_PCT(100),
    LV_PCT(100));

  lv_obj_set_style_bg_opa(
    configWindow,
    LV_OPA_TRANSP,
    0);

  lv_obj_add_event_cb(
    configWindow,
    config_event,
    LV_EVENT_CLICKED,
    NULL);

  lv_obj_add_flag(
    configWindow,
    LV_OBJ_FLAG_HIDDEN);

  configBox = lv_obj_create(configWindow);

  lv_obj_set_size(
    configBox,
    280,
    420);

  lv_obj_center(configBox);

  txtField = lv_textarea_create(lv_scr_act());

  lv_obj_set_size(
    txtField,
    390,
    90);

  lv_obj_align(
    txtField,
    LV_ALIGN_TOP_MID,
    0,
    10);

  lv_obj_set_style_text_font(
    txtField,
    &lv_font_montserrat_32,
    LV_PART_MAIN);

  lv_obj_add_flag(
    txtField,
    LV_OBJ_FLAG_HIDDEN);

  kbd = lv_keyboard_create(lv_scr_act());

  lv_obj_set_size(
    kbd,
    400,
    390);

  lv_obj_align(
    kbd,
    LV_ALIGN_BOTTOM_MID,
    0,
    0);

  lv_keyboard_set_textarea(
    kbd,
    txtField);

  lv_obj_add_event_cb(
    kbd,
    keyboard_event,
    LV_EVENT_ALL,
    NULL);

  lv_obj_add_flag(
    kbd,
    LV_OBJ_FLAG_HIDDEN);

  for (int i = 0; i < 6; i++) {

    configButtons[i] =
      lv_btn_create(
        configBox);

    lv_obj_add_event_cb(
      configButtons[i],
      config_select_event,
      LV_EVENT_CLICKED,
      (void *)(intptr_t)i);

    lv_obj_set_size(
      configButtons[i],
      250,
      52);

    lv_obj_align(
      configButtons[i],
      LV_ALIGN_TOP_MID,
      0,
      18 + i * 58);

    lab =
      lv_label_create(
        configButtons[i]);

    configLabelObj[i] =
      lab;

    lv_obj_set_style_text_font(
      lab,
      &lv_font_montserrat_24,
      0);

    lv_obj_align(
      lab,
      LV_ALIGN_LEFT_MID,
      12,
      0);

    switch (i) {

      case 0:

        lv_label_set_text(
          lab,
          "WiFi SSID >");

        break;

      case 1:

        lv_label_set_text(
          lab,
          "Password >");

        break;

      case 2:

        lv_label_set_text(
          lab,
          "Pypilot IP >");

        break;

      case 3:

        lv_label_set_text(
          lab,
          "Port >");

        break;

      case 4:

        lv_label_set_text(
          lab,
          "Brightness >");

        break;

      case 5:

        lv_obj_set_height(
          configButtons[i],
          60);

        lv_label_set_text(
          lab,
          "SAVE");

        lv_obj_center(
          lab);

        break;
    }
  }
  //--------------------------------------------------
  // BRIGHTNESS PANEL
  //--------------------------------------------------

  brightnessPanel =
    lv_obj_create(
      lv_scr_act());

  lv_obj_set_size(
    brightnessPanel,
    LV_PCT(100),
    LV_PCT(100));

  lv_obj_set_style_bg_color(
    brightnessPanel,
    lv_color_black(),
    0);

  lv_obj_add_flag(
    brightnessPanel,
    LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *lbl =
    lv_label_create(
      brightnessPanel);

  lv_label_set_text(
    lbl,
    "Brightness");

  lv_obj_set_style_text_font(
    lbl,
    &lv_font_montserrat_30,
    0);

  lv_obj_align(
    lbl,
    LV_ALIGN_TOP_MID,
    0,
    40);

  brightnessValue =
    lv_label_create(
      brightnessPanel);

  lv_obj_set_style_text_font(
    brightnessValue,
    &lv_font_montserrat_40,
    0);

  lv_obj_align(
    brightnessValue,
    LV_ALIGN_TOP_MID,
    0,
    100);

  brightnessSlider =
    lv_slider_create(
      brightnessPanel);

  lv_obj_set_size(
    brightnessSlider,
    300,
    18);

  lv_obj_align(
    brightnessSlider,
    LV_ALIGN_CENTER,
    0,
    0);

  lv_slider_set_range(
    brightnessSlider,
    10,
    100);

  lv_obj_add_event_cb(
    brightnessSlider,
    brightness_event,
    LV_EVENT_VALUE_CHANGED,
    NULL);

  lv_slider_set_value(
    brightnessSlider,
    configGetBrightness(),
    LV_ANIM_OFF);

  char txt[10];

  sprintf(
    txt,
    "%d%%",
    configGetBrightness());

  lv_label_set_text(
    brightnessValue,
    txt);

  lbl =
    lv_label_create(
      brightnessPanel);

  lv_label_set_text(
    lbl,
    "Hold TOP\n"
    "to close");

  lv_obj_set_style_text_align(
    lbl,
    LV_TEXT_ALIGN_CENTER,
    0);

  lv_obj_align(
    lbl,
    LV_ALIGN_BOTTOM_MID,
    0,
    -40);

  pilotUI_setAuto(
    false);

}


void pilotUI_update() {
}

void pilotUI_setHeading(
  int heading,
  char ref) {
  char txt[20];

  sprintf(
    txt,
    "%03d° %c",
    heading,
    ref);

  lv_label_set_text(
    lblHeading,
    txt);
}

void pilotUI_setMode(
  const char *mode) {
  lv_label_set_text(
    lblMode,
    mode);
}

void pilotUI_setBattery(
  int percent,
  bool charging) {
  char txt[20];

  batteryCharging =
    charging;

  sprintf(
    txt,
    "%d%%",
    percent);

  if (charging)
    lv_obj_clear_flag(
      lblCharge,
      LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(
      lblCharge,
      LV_OBJ_FLAG_HIDDEN);

  lv_label_set_text(
    lblBattery,
    txt);

  if (percent > 40)
    lv_obj_set_style_text_color(
      lblBattery,
      lv_color_white(),
      0);

  else if (percent > 20)
    lv_obj_set_style_text_color(
      lblBattery,
      lv_palette_main(
        LV_PALETTE_ORANGE),
      0);

  else
    lv_obj_set_style_text_color(
      lblBattery,
      lv_palette_main(
        LV_PALETTE_RED),
      0);
}

void pilotUI_setAuto(
  bool enabled) {
  lv_obj_remove_style(
    btnAuto,
    &styleAuto,
    LV_PART_MAIN);

  lv_obj_remove_style(
    btnAuto,
    &styleAutoOff,
    LV_PART_MAIN);

  if (enabled) {
    lv_obj_add_style(
      btnAuto,
      &styleAuto,
      LV_PART_MAIN);

    lv_label_set_text(
      lblAuto,
      "AUTO");
  } else {
    lv_obj_add_style(
      btnAuto,
      &styleAutoOff,
      LV_PART_MAIN);

    lv_label_set_text(
      lblAuto,
      "STBY");
  }
}

void pilotUI_setConnected(
  bool connected) {
  if (connected) {
    lv_label_set_text(
      lblStatus,
      LV_SYMBOL_OK " PYPILOT");

    lv_obj_set_style_text_color(
      lblStatus,
      lv_palette_main(
        LV_PALETTE_GREEN),
      0);
  } else {
    lv_label_set_text(
      lblStatus,
      LV_SYMBOL_CLOSE " PYPILOT");

    lv_obj_set_style_text_color(
      lblStatus,
      lv_palette_main(
        LV_PALETTE_RED),
      0);
  }
}

void brightnessShow() {
  brightnessOpen = true;

  lv_obj_add_flag(
    configWindow,
    LV_OBJ_FLAG_HIDDEN);

  int value =
    configGetBrightness();

  lv_slider_set_value(
    brightnessSlider,
    value,
    LV_ANIM_OFF);

  char txt[10];

  sprintf(
    txt,
    "%d%%",
    value);

  lv_label_set_text(
    brightnessValue,
    txt);

  lv_obj_add_flag(
    configBox,
    LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(
    brightnessPanel,
    LV_OBJ_FLAG_HIDDEN);
}

void brightnessHide() {
  brightnessOpen = false;

  lv_obj_add_flag(
    brightnessPanel,
    LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(
    configBox,
    LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(
    configWindow,
    LV_OBJ_FLAG_HIDDEN);
}

bool brightnessVisible() {
  return brightnessOpen;
}
