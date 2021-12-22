#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 1819746818; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"Too Button",
	"Octoprint API Key",
	"Octoprint IP Address",
	"0.0.0.0",
	1,
	0,
	0,
	0,
	0,
	0,
	6,
	6,
	10,
	true
};