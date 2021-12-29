#include <Arduino.h>
#include "config.h"

uint32_t configVersion = 1515724450; //generated identifier to compare config with EEPROM

const configData defaults PROGMEM =
{
	"Too Button",
	"Octoprint API Key",
	"Octoprint IP Address",
	5000,
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
	"G91,G28 X0 Y0 Z0",
	"G91,G28 X0 Y0 Z0",
	true,
	"2.1.2.1"
};