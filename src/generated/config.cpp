#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 2431764658; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"Too Button",
	"Octoprint API Key",
	"Octoprint IP Address",
	"TPLink IP Address",
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