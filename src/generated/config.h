#ifndef CONFIG_H
#define CONFIG_H

struct configData
{
	char projectName[32];
	char octoprintapikey[40];
	char octoprintip[40];
	uint16_t octoprintport;
	char tplinktip[40];
	uint16_t button1_click;
	uint16_t button1_doubleclick;
	uint16_t button1_hold;
	uint16_t button2_click;
	uint16_t button2_doubleclick;
	uint16_t button2_hold;
	uint16_t led1_color;
	uint16_t led2_color;
	uint16_t led_brightness;
	bool enableledflash;
	char projectVersion[20];
};

extern uint32_t configVersion;
extern const configData defaults;

#endif