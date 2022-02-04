#ifndef DASH_H
#define DASH_H

struct dashboardData
{
	char HostName[32];
	char MAC[32];
	char SSID[32];
	char IPAddress[16];
	char Netmask[16];
	char Default_GW[16];
	char DNS_IP[16];
};

#endif