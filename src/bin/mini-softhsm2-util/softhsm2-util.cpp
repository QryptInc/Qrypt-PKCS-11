/*
 * Copyright (c) 2010 .SE (The Internet Infrastructure Foundation)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
 softhsm2-util.cpp

 This program can be used for interacting with HSMs using PKCS#11.
 The default library is the libsofthsm2.so
 *****************************************************************************/

#include "softhsm2-util.h"
#include "findslot.h"
#include "getpw.h"
#include "library.h"
// #include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#include <io.h>
#endif
#include <iostream>
#include <fstream>

// Display the usage
void usage()
{
	printf("Support tool for PKCS#11\n");
	printf("Usage: softhsm2-util [ACTION] [OPTIONS]\n");
	printf("Action:\n");
	printf("  -h                Shows this help screen.\n");
	printf("  --help            Shows this help screen.\n");
	printf("  --init-token      Initialize the token at a given slot.\n");
	printf("                    Use with --slot or --token or --serial or --free,\n");
	printf("                    --label, --so-pin, and --pin.\n");
	printf("                    WARNING: Any content in token will be erased.\n");
	printf("  --show-slots      Display all the available slots.\n");
	printf("Options:\n");
	printf("  --free            Use the first free/uninitialized token.\n");
	printf("  --label <text>    Defines the label of the object or the token.\n");
	printf("  --module <path>   Define PKCS#11 library to use.\n");
	printf("  --pin <PIN>       The PIN for the normal user.\n");
	printf("  --serial <number> Will use the token with a matching serial number.\n");
	printf("  --slot <number>   The slot where the token is located.\n");
	printf("  --so-pin <PIN>    The PIN for the Security Officer (SO).\n");
	printf("  --token <label>   Will use the token with a matching token label.\n");
}

// Enumeration of the long options
enum {
	OPT_FREE = 0x100,
	OPT_HELP,
	OPT_INIT_TOKEN,
	OPT_LABEL,
	OPT_MODULE,
	OPT_PIN,
	OPT_SERIAL,
	OPT_SHOW_SLOTS,
	OPT_SLOT,
	OPT_SO_PIN,
	OPT_TOKEN
};

// Text representation of the long options
static const struct option long_options[] = {
	{ "free",            0, NULL, OPT_FREE },
	{ "help",            0, NULL, OPT_HELP },
	{ "init-token",      0, NULL, OPT_INIT_TOKEN },
	{ "label",           1, NULL, OPT_LABEL },
	{ "module",          1, NULL, OPT_MODULE },
	{ "pin",             1, NULL, OPT_PIN },
	{ "serial",          1, NULL, OPT_SERIAL },
	{ "show-slots",      0, NULL, OPT_SHOW_SLOTS },
	{ "slot",            1, NULL, OPT_SLOT },
	{ "so-pin",          1, NULL, OPT_SO_PIN },
	{ "token",           1, NULL, OPT_TOKEN },
	{ NULL,              0, NULL, 0 }
};

CK_FUNCTION_LIST_PTR p11;

// The main function
int main(int argc, char* argv[])
{
	int option_index = 0;
	int opt;

	char* soPIN = NULL;
	char* userPIN = NULL;
	char* label = NULL;
	char* module = NULL;
	char* slot = NULL;
	char* serial = NULL;
	char* token = NULL;
	char* errMsg = NULL;
	bool freeToken = false;

	int doInitToken = 0;
	int doShowSlots = 0;
	int action = 0;
	int rv = 0;
	CK_SLOT_ID slotID = 0;

	moduleHandle = NULL;
	p11 = NULL;

	while ((opt = getopt_long(argc, argv, "hv", long_options, &option_index)) != -1)
	{
		switch (opt)
		{
			case OPT_SHOW_SLOTS:
				doShowSlots = 1;
				action++;
				break;
			case OPT_INIT_TOKEN:
				doInitToken = 1;
				action++;
				break;
			case OPT_SLOT:
				slot = optarg;
				break;
			case OPT_LABEL:
				label = optarg;
				break;
			case OPT_SERIAL:
				serial = optarg;
				break;
			case OPT_TOKEN:
				token = optarg;
				break;
			case OPT_MODULE:
				module = optarg;
				break;
			case OPT_SO_PIN:
				soPIN = optarg;
				break;
			case OPT_PIN:
				userPIN = optarg;
				break;
			case OPT_FREE:
				freeToken = true;
				break;
			case OPT_HELP:
			case 'h':
			default:
				usage();
				exit(0);
				break;
		}
	}

	// No action given, display the usage.
	if (action != 1)
	{
		usage();
		exit(1);
	}

	if (!module)
	{
		fprintf(stderr, "./softhsm2-util: missing required option '--module' \n");
		usage();
		exit(1);
	}

	// Get a pointer to the function list for PKCS#11 library
	CK_C_GetFunctionList pGetFunctionList = loadLibrary(module, &moduleHandle, &errMsg);
	if (!pGetFunctionList)
	{
		fprintf(stderr, "ERROR: Could not load the PKCS#11 library/module: %s\n", errMsg);
		fprintf(stderr, "ERROR: Please check log files for additional information.\n");
		exit(1);
	}

	// Load the function list
	(*pGetFunctionList)(&p11);

	// Initialize the library
	CK_RV p11rv = p11->C_Initialize(NULL_PTR);
	if (p11rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not initialize the PKCS#11 library/module: %s\n", module);
		fprintf(stderr, "ERROR: Please check log files for additional information.\n");
		exit(1);
	}

	// We should create the token.
	if (doInitToken)
	{
		// Get the slotID
		rv = findSlot(slot, serial, token, freeToken, slotID);
		if (!rv)
		{
			rv = initToken(slotID, label, soPIN, userPIN);
		}
	}

	// Show all available slots
	if (!rv && doShowSlots)
	{
		rv = showSlots();
	}

	p11->C_Finalize(NULL_PTR);
	unloadLibrary(moduleHandle);

	return rv;
}

// Initialize the token
int initToken(CK_SLOT_ID slotID, char* label, char* soPIN, char* userPIN)
{
	char so_pin_copy[MAX_PIN_LEN+1];
	char user_pin_copy[MAX_PIN_LEN+1];

	if (label == NULL)
	{
		fprintf(stderr, "ERROR: A label for the token must be supplied. "
				"Use --label <text>\n");
		return 1;
	}

	if (strlen(label) > 32)
	{
		fprintf(stderr, "ERROR: The token label must not have a length "
				"greater than 32 chars.\n");
		return 1;
	}

	// Get the passwords
	if (getPW(soPIN, so_pin_copy, CKU_SO) != 0)
	{
		fprintf(stderr, "ERROR: Could not get SO PIN\n");
		return 1;
	}
	if (getPW(userPIN, user_pin_copy, CKU_USER) != 0)
	{
		fprintf(stderr, "ERROR: Could not get user PIN\n");
		return 1;
	}

	// Load the variables
	CK_UTF8CHAR paddedLabel[32];
	memset(paddedLabel, ' ', sizeof(paddedLabel));
	memcpy(paddedLabel, label, strlen(label));

	CK_RV rv = p11->C_InitToken(slotID, (CK_UTF8CHAR_PTR)so_pin_copy, strlen(so_pin_copy), paddedLabel);

	switch (rv)
	{
		case CKR_OK:
			break;
		case CKR_SLOT_ID_INVALID:
			fprintf(stderr, "CKR_SLOT_ID_INVALID: Slot %lu does not exist.\n", slotID);
			return 1;
			break;
		case CKR_PIN_INCORRECT:
			fprintf(stderr, "CKR_PIN_INCORRECT: The given SO PIN does not match the "
					"one in the token. Needed when reinitializing the token.\n");
			return 1;
			break;
		case CKR_TOKEN_NOT_PRESENT:
			fprintf(stderr, "CKR_TOKEN_NOT_PRESENT: The token is not present. "
					"Please read the HSM manual for further assistance.\n");
			return 1;
			break;
		default:
			fprintf(stderr, "ERROR rv=0x%08X: Could not initialize the token.\n", (unsigned int)rv);
			fprintf(stderr, "Please check log files for additional information.\n");
			return 1;
			break;
	}

	CK_SESSION_HANDLE hSession;
	rv = p11->C_OpenSession(slotID, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL_PTR, NULL_PTR, &hSession);
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not open a session with the library.\n");
		return 1;
	}

	rv = p11->C_Login(hSession, CKU_SO, (CK_UTF8CHAR_PTR)so_pin_copy, strlen(so_pin_copy));
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not log in on the token.\n");
		return 1;
	}

	rv = p11->C_InitPIN(hSession, (CK_UTF8CHAR_PTR)user_pin_copy, strlen(user_pin_copy));
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not initialize the user PIN.\n");
		return 1;
	}

	// Get the token info
	CK_TOKEN_INFO tokenInfo;
	rv = p11->C_GetTokenInfo(slotID, &tokenInfo);
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not get info about the initialized token in slot %lu.\n", slotID);
		return 1;
	}

	// Reload the library
	p11->C_Finalize(NULL_PTR);
	rv = p11->C_Initialize(NULL_PTR);
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not initialize the library.\n");
		return 1;
	}

	// Get the slotID
	CK_SLOT_ID newSlotID;
	if (findSlot(tokenInfo, newSlotID))
	{
		return 1;
	}

	if (slotID == newSlotID)
	{
		printf("The token has been initialized on slot %lu\n", newSlotID);
	}
	else
	{
		printf("The token has been initialized and is reassigned to slot %lu\n", newSlotID);
	}

	return 0;
}

// Show what slots are available
int showSlots()
{
	CK_ULONG ulSlotCount;
	CK_RV rv = p11->C_GetSlotList(CK_FALSE, NULL_PTR, &ulSlotCount);
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not get the number of slots.\n");
		return 1;
	}

	CK_SLOT_ID_PTR pSlotList = (CK_SLOT_ID_PTR) malloc(ulSlotCount*sizeof(CK_SLOT_ID));
	if (!pSlotList)
	{
		fprintf(stderr, "ERROR: Could not allocate memory.\n");
		return 1;
	}

	rv = p11->C_GetSlotList(CK_FALSE, pSlotList, &ulSlotCount);
	if (rv != CKR_OK)
	{
		fprintf(stderr, "ERROR: Could not get the slot list.\n");
		free(pSlotList);
		return 1;
	}

	printf("Available slots:\n");

	for (CK_ULONG i = 0; i < ulSlotCount; i++)
	{
		CK_SLOT_INFO slotInfo;
		CK_TOKEN_INFO tokenInfo;

		rv = p11->C_GetSlotInfo(pSlotList[i], &slotInfo);
		if (rv != CKR_OK)
		{
			fprintf(stderr, "ERROR: Could not get info about slot %lu.\n", pSlotList[i]);
			continue;
		}

		printf("Slot %lu\n", pSlotList[i]);
		printf("    Slot info:\n");
		printf("        Description:      %.*s\n", 64, slotInfo.slotDescription);
		printf("        Manufacturer ID:  %.*s\n", 32, slotInfo.manufacturerID);
		printf("        Hardware version: %i.%i\n", slotInfo.hardwareVersion.major,
							    slotInfo.hardwareVersion.minor);
		printf("        Firmware version: %i.%i\n", slotInfo.firmwareVersion.major,
							    slotInfo.firmwareVersion.minor);
		printf("        Token present:    ");
		if ((slotInfo.flags & CKF_TOKEN_PRESENT) == 0)
		{
			printf("no\n");
			continue;
		}

		printf("yes\n");
		printf("    Token info:\n");

		rv = p11->C_GetTokenInfo(pSlotList[i], &tokenInfo);
		if (rv != CKR_OK)
		{
			fprintf(stderr, "ERROR: Could not get info about the token in slot %lu.\n",
				pSlotList[i]);
			continue;
		}

		printf("        Manufacturer ID:  %.*s\n", 32, tokenInfo.manufacturerID);
		printf("        Model:            %.*s\n", 16, tokenInfo.model);
		printf("        Hardware version: %i.%i\n", tokenInfo.hardwareVersion.major,
							    tokenInfo.hardwareVersion.minor);
		printf("        Firmware version: %i.%i\n", tokenInfo.firmwareVersion.major,
							    tokenInfo.firmwareVersion.minor);
		printf("        Serial number:    %.*s\n", 16, tokenInfo.serialNumber);
		printf("        Initialized:      ");
		if ((tokenInfo.flags & CKF_TOKEN_INITIALIZED) == 0)
		{
			printf("no\n");
		}
		else
		{
			printf("yes\n");
		}

		printf("        User PIN init.:   ");
		if ((tokenInfo.flags & CKF_USER_PIN_INITIALIZED) == 0)
		{
			printf("no\n");
		}
		else
		{
			printf("yes\n");
		}

		printf("        Label:            %.*s\n", 32, tokenInfo.label);

	}

	free(pSlotList);

	return 0;
}
