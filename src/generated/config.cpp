#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 2069190015; //generated identifier to compare config with EEPROM

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
	true,
	"2.1.2.1"
};