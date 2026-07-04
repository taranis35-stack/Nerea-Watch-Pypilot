#pragma once

void pypilot_begin();
void pypilot_update();

void pypilot_send(const char *cmd);

void watchCommand(const char *cmd);

bool pypilot_connected();

int pypilot_heading();
int pypilot_headingCommand();

bool pypilot_enabled();

const char *pypilot_mode();