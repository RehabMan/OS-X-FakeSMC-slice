/*
 *  fakesmc.h
 *  FakeSMCIntelThermal
 *
 *  Created by mozodojo on 18/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

struct AppleSMCData;

typedef struct AppleSMCData* SMCData;
typedef void (*PluginCallback)(SMCData);

struct AppleSMCData {
	uint8_t len;
	char *key;
	char *data;
	PluginCallback callback;
	SMCData next;
};

extern void FakeSMCAddKey (const char*, uint8_t, char*, PluginCallback);