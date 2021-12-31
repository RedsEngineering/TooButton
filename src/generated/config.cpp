#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 3543615470; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"Too Button",
	"Octoprint API Key",
	"Octoprint IP Address",
	5000,
	"0.0.0.0",
	0,
	0,
	0,
	0,
	0,
	0,
	6,
	6,
	10,
	"M300,S340,P200",
	"M300,S340,P200",
	true,
	"2.1.2.1"
};