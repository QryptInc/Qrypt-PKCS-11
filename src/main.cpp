/*
 * Copyright (c)2010 SURFnet bv
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
 * INTERRUPTION)HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************
 main.cpp

 This file contains the main entry point to the PKCS #11 library. All it does
 is dispatch calls to the actual implementation and check for fatal exceptions
 on the boundary of the library.

 Everything will crash and burn if multiple things try to use this at once.
 *****************************************************************************/

// The functions are exported library/DLL entry points
#define CRYPTOKI_EXPORTS

#include <stdlib.h>
#include <cstring>         // strncpy, memset

#include "cryptoki.h"                    // PKCS#11 types
#include "qryptoki_pkcs11_vendor_defs.h" // CKR_QRYPT_*
#include "log.h"                         // logging macros
#include "GlobalData.h"                  // GlobalData

#if defined(__GNUC__) && \
	(__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) || \
	defined(__SUNPRO_C) && __SUNPRO_C >= 0x590
#define PKCS_API __attribute__ ((visibility("default")))
#else
#define PKCS_API
#endif

// PKCS #11 function list
static CK_FUNCTION_LIST functionList =
{
	// Version information
	{ CRYPTOKI_VERSION_MAJOR, CRYPTOKI_VERSION_MINOR },
	// Function pointers
	C_Initialize,
	C_Finalize,
	C_GetInfo,
	C_GetFunctionList,
	C_GetSlotList,
	C_GetSlotInfo,
	C_GetTokenInfo,
	C_GetMechanismList,
	C_GetMechanismInfo,
	C_InitToken,
	C_InitPIN,
	C_SetPIN,
	C_OpenSession,
	C_CloseSession,
	C_CloseAllSessions,
	C_GetSessionInfo,
	C_GetOperationState,
	C_SetOperationState,
	C_Login,
	C_Logout,
	C_CreateObject,
	C_CopyObject,
	C_DestroyObject,
	C_GetObjectSize,
	C_GetAttributeValue,
	C_SetAttributeValue,
	C_FindObjectsInit,
	C_FindObjects,
	C_FindObjectsFinal,
	C_EncryptInit,
	C_Encrypt,
	C_EncryptUpdate,
	C_EncryptFinal,
	C_DecryptInit,
	C_Decrypt,
	C_DecryptUpdate,
	C_DecryptFinal,
	C_DigestInit,
	C_Digest,
	C_DigestUpdate,
	C_DigestKey,
	C_DigestFinal,
	C_SignInit,
	C_Sign,
	C_SignUpdate,
	C_SignFinal,
	C_SignRecoverInit,
	C_SignRecover,
	C_VerifyInit,
	C_Verify,
	C_VerifyUpdate,
	C_VerifyFinal,
	C_VerifyRecoverInit,
	C_VerifyRecover,
	C_DigestEncryptUpdate,
	C_DecryptDigestUpdate,
	C_SignEncryptUpdate,
	C_DecryptVerifyUpdate,
	C_GenerateKey,
	C_GenerateKeyPair,
	C_WrapKey,
	C_UnwrapKey,
	C_DeriveKey,
	C_SeedRandom,
	C_GenerateRandom,
	C_GetFunctionStatus,
	C_CancelFunction,
	C_WaitForSlotEvent
};

// General-purpose functions
PKCS_API CK_RV C_Initialize(CK_VOID_PTR pInitArgs)
{
	try {
		if(GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_ALREADY_INITIALIZED;

		// Initialize global
		CK_RV rv = GlobalData::getInstance().initialize((CK_C_INITIALIZE_ARGS_PTR)pInitArgs);
		if(rv != CKR_OK) {
			GlobalData::getInstance().finalize();
			return rv;
		}

		// Initialize base HSM
		CK_C_Initialize Base_C_Initialize = (CK_C_Initialize)GlobalData::getInstance().getBaseFunction("C_Initialize");
		if(Base_C_Initialize == NULL) {
			GlobalData::getInstance().finalize();
			return CKR_GENERAL_ERROR;
		}

		rv = (*Base_C_Initialize)(pInitArgs);
		if(rv != CKR_OK) GlobalData::getInstance().finalize();

		return rv;
	} catch (std::bad_alloc &ex) {
		return CKR_HOST_MEMORY;
	} catch (...) {
		return CKR_GENERAL_ERROR;
	}
}

PKCS_API CK_RV C_Finalize(CK_VOID_PTR pReserved)
{
	try {
		if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;

		CK_C_Finalize Base_C_Finalize = (CK_C_Finalize)GlobalData::getInstance().getBaseFunction("C_Finalize");
		if(Base_C_Finalize == NULL) return CKR_GENERAL_ERROR;

		CK_RV rv = (*Base_C_Finalize)(pReserved);

		if(rv != CKR_OK) return rv;
		
		return GlobalData::getInstance().finalize();
	} catch (std::bad_alloc &ex) {
		return CKR_HOST_MEMORY;
	} catch (...) {
		return CKR_GENERAL_ERROR;
	}
}

PKCS_API CK_RV C_GetInfo(CK_INFO_PTR pInfo)
{
	try {
		if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;

		CK_C_GetInfo Base_C_GetInfo = (CK_C_GetInfo)GlobalData::getInstance().getBaseFunction("C_GetInfo");
		if(Base_C_GetInfo == NULL) return CKR_GENERAL_ERROR;

		CK_RV rv = (*Base_C_GetInfo)(pInfo);

		if(rv == CKR_OK) {
			// Write over manufacturer

			// Set all 32 to blank characters
			memset(pInfo->manufacturerID, ' ', 32);

			// Set manufacturer to Qrypt
			const char *manufacturer = "Qrypt, Inc.";
			size_t manufacturer_len = strlen(manufacturer);

			memcpy(pInfo->manufacturerID, manufacturer, manufacturer_len);

			// Prepend "Wrap of " to description

			const char *description_prefix = "Wrap of ";
			size_t prefix_len = strlen(description_prefix);

			// Temporarily store the original description
			std::unique_ptr<char[]> oldDescription = std::make_unique<char[]>(32);
			memcpy(oldDescription.get(), pInfo->libraryDescription, 32);

			// Start description with "Wrap of "
			memcpy(pInfo->libraryDescription, description_prefix, prefix_len);

			// Finish description with original
			memcpy(&pInfo->libraryDescription[prefix_len], oldDescription.get(), 32 - prefix_len);

			// Write over libraryVersion
			pInfo->libraryVersion.major = 0;
			pInfo->libraryVersion.minor = 1;
		}

		return rv;
	} catch (std::bad_alloc &ex) {
		return CKR_HOST_MEMORY;
	} catch (...) {
		return CKR_GENERAL_ERROR;
	}
}

PKCS_API CK_RV C_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR ppFunctionList)
{
    if (ppFunctionList == NULL_PTR) return CKR_ARGUMENTS_BAD;

    *ppFunctionList = &functionList;

    return CKR_OK;
}

// RNG functions
PKCS_API CK_RV C_SeedRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSeed, CK_ULONG ulSeedLen)
{
	try {
		if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;

		CK_C_SeedRandom Base_C_SeedRandom = (CK_C_SeedRandom)GlobalData::getInstance().getBaseFunction("C_SeedRandom");
		if(Base_C_SeedRandom == NULL) return CKR_GENERAL_ERROR;

		CK_RV rv = (*Base_C_SeedRandom)(hSession, pSeed, 0);
		switch (rv) {
			case CKR_DEVICE_ERROR:
			case CKR_DEVICE_MEMORY:
			case CKR_DEVICE_REMOVED:
			case CKR_FUNCTION_FAILED:
			case CKR_HOST_MEMORY:
			case CKR_OK:
			case CKR_RANDOM_NO_RNG:
			case CKR_RANDOM_SEED_NOT_SUPPORTED:
				break;
			case CKR_ARGUMENTS_BAD:
			case CKR_FUNCTION_CANCELED:
			case CKR_OPERATION_ACTIVE:
			case CKR_SESSION_CLOSED:
			case CKR_SESSION_HANDLE_INVALID:
			case CKR_USER_NOT_LOGGED_IN:
			case CKR_GENERAL_ERROR:
				return rv;
			default:
				DEBUG_MSG("Base HSM C_GenerateRandom returned weird rv = %lu", rv);
				return rv;
		}

		return CKR_RANDOM_SEED_NOT_SUPPORTED;
	} catch (std::bad_alloc &ex) {
		return CKR_HOST_MEMORY;
	} catch (...) {
		return CKR_GENERAL_ERROR;
	}
}

PKCS_API CK_RV C_GenerateRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pRandomData, CK_ULONG ulRandomLen)
{
	try {
		if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;

		CK_C_GenerateRandom Base_C_GenerateRandom = (CK_C_GenerateRandom)GlobalData::getInstance().getBaseFunction("C_GenerateRandom");
		if(Base_C_GenerateRandom == NULL) return CKR_GENERAL_ERROR;

		CK_RV rv = (*Base_C_GenerateRandom)(hSession, pRandomData, 0);
		switch (rv) {
			case CKR_DEVICE_ERROR:
			case CKR_DEVICE_MEMORY:
			case CKR_DEVICE_REMOVED:
			case CKR_FUNCTION_FAILED:
			case CKR_HOST_MEMORY:
			case CKR_OK:
			case CKR_RANDOM_NO_RNG:
				break;
			case CKR_ARGUMENTS_BAD:
			case CKR_FUNCTION_CANCELED:
			case CKR_OPERATION_ACTIVE:
			case CKR_SESSION_CLOSED:
			case CKR_SESSION_HANDLE_INVALID:
			case CKR_USER_NOT_LOGGED_IN:
			case CKR_GENERAL_ERROR:
				return rv;
			default:
				DEBUG_MSG("Base HSM C_GenerateRandom returned weird rv = %lu", rv);
				return rv;
		}

		rv = GlobalData::getInstance().lockRandomBufferMutex();
		if(rv != CKR_OK) return rv;

		// Get Qrypt random
		rv = GlobalData::getInstance().getRandom(pRandomData, ulRandomLen);
		
		const char *errorMsg;
		if(rv == CKR_QRYPT_TOKEN_EMPTY)
			errorMsg = "Environment variable QRYPT_EAAS_TOKEN empty.";
		else if(rv == CKR_QRYPT_TOKEN_INVALID)
			errorMsg = "Could not fulfill random request, is QRYPT_EAAS_TOKEN a valid Qrypt entropy token?";
		else if(rv == CKR_QRYPT_CA_CERT_FAILURE)
			errorMsg = "CA certificate error, consider editing QRYPT_CA_CERT_PATH.";
		else if(rv != CKR_OK)
			errorMsg = "Unexpected error, likely will return CKR_GENERAL_ERROR.";

		if(rv != CKR_OK) {
			ERROR_MSG(errorMsg);
			GlobalData::getInstance().unlockRandomBufferMutex();
			return rv;
		}

		// Unlock RandomBuffer mutex;
		rv = GlobalData::getInstance().unlockRandomBufferMutex();
		if(rv != CKR_OK) return rv;

		INFO_MSG("Retrieved %lu bytes from Qrypt Entropy API.", ulRandomLen);
		return CKR_OK;
	} catch (std::bad_alloc &ex) {
		return CKR_HOST_MEMORY;
	} catch (...) {
		return CKR_GENERAL_ERROR;
	}
}

// Other functions (these all look the same)
PKCS_API CK_RV C_GetSlotList(CK_BBOOL tokenPresent, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetSlotList Base_C_GetSlotList = (CK_C_GetSlotList)GlobalData::getInstance().getBaseFunction("C_GetSlotList");
	if(Base_C_GetSlotList == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetSlotList)(tokenPresent, pSlotList, pulCount);
}

PKCS_API CK_RV C_GetSlotInfo(CK_SLOT_ID slotID, CK_SLOT_INFO_PTR pInfo)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetSlotInfo Base_C_GetSlotInfo = (CK_C_GetSlotInfo)GlobalData::getInstance().getBaseFunction("C_GetSlotInfo");
	if(Base_C_GetSlotInfo == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetSlotInfo)(slotID, pInfo);
}

PKCS_API CK_RV C_GetTokenInfo(CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR pInfo)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetTokenInfo Base_C_GetTokenInfo = (CK_C_GetTokenInfo)GlobalData::getInstance().getBaseFunction("C_GetTokenInfo");
	if(Base_C_GetTokenInfo == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetTokenInfo)(slotID, pInfo);
}

PKCS_API CK_RV C_GetMechanismList(CK_SLOT_ID slotID, CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetMechanismList Base_C_GetMechanismList = (CK_C_GetMechanismList)GlobalData::getInstance().getBaseFunction("C_GetMechanismList");
	if(Base_C_GetMechanismList == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetMechanismList)(slotID, pMechanismList, pulCount);
}

PKCS_API CK_RV C_GetMechanismInfo(CK_SLOT_ID slotID, CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetMechanismInfo Base_C_GetMechanismInfo = (CK_C_GetMechanismInfo)GlobalData::getInstance().getBaseFunction("C_GetMechanismInfo");
	if(Base_C_GetMechanismInfo == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetMechanismInfo)(slotID, type, pInfo);
}

PKCS_API CK_RV C_InitToken(CK_SLOT_ID slotID, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen, CK_UTF8CHAR_PTR pLabel)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_InitToken Base_C_InitToken = (CK_C_InitToken)GlobalData::getInstance().getBaseFunction("C_InitToken");
	if(Base_C_InitToken == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_InitToken)(slotID, pPin, ulPinLen, pLabel);
}

PKCS_API CK_RV C_InitPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_InitPIN Base_C_InitPIN = (CK_C_InitPIN)GlobalData::getInstance().getBaseFunction("C_InitPIN");
	if(Base_C_InitPIN == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_InitPIN)(hSession, pPin, ulPinLen);
}

PKCS_API CK_RV C_SetPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pOldPin, CK_ULONG ulOldLen, CK_UTF8CHAR_PTR pNewPin, CK_ULONG ulNewLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SetPIN Base_C_SetPIN = (CK_C_SetPIN)GlobalData::getInstance().getBaseFunction("C_SetPIN");
	if(Base_C_SetPIN == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SetPIN)(hSession, pOldPin, ulOldLen, pNewPin, ulNewLen);
}

PKCS_API CK_RV C_OpenSession(CK_SLOT_ID slotID, CK_FLAGS flags, CK_VOID_PTR pApplication, CK_NOTIFY notify, CK_SESSION_HANDLE_PTR phSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_OpenSession Base_C_OpenSession = (CK_C_OpenSession)GlobalData::getInstance().getBaseFunction("C_OpenSession");
	if(Base_C_OpenSession == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_OpenSession)(slotID, flags, pApplication, notify, phSession);
}

PKCS_API CK_RV C_CloseSession(CK_SESSION_HANDLE hSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_CloseSession Base_C_CloseSession = (CK_C_CloseSession)GlobalData::getInstance().getBaseFunction("C_CloseSession");
	if(Base_C_CloseSession == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_CloseSession)(hSession);
}

PKCS_API CK_RV C_CloseAllSessions(CK_SLOT_ID slotID)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_CloseAllSessions Base_C_CloseAllSessions = (CK_C_CloseAllSessions)GlobalData::getInstance().getBaseFunction("C_CloseAllSessions");
	if(Base_C_CloseAllSessions == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_CloseAllSessions)(slotID);
}

PKCS_API CK_RV C_GetSessionInfo(CK_SESSION_HANDLE hSession, CK_SESSION_INFO_PTR pInfo)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetSessionInfo Base_C_GetSessionInfo = (CK_C_GetSessionInfo)GlobalData::getInstance().getBaseFunction("C_GetSessionInfo");
	if(Base_C_GetSessionInfo == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetSessionInfo)(hSession, pInfo);
}

PKCS_API CK_RV C_GetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState, CK_ULONG_PTR pulOperationStateLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetOperationState Base_C_GetOperationState = (CK_C_GetOperationState)GlobalData::getInstance().getBaseFunction("C_GetOperationState");
	if(Base_C_GetOperationState == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetOperationState)(hSession, pOperationState, pulOperationStateLen);
}

PKCS_API CK_RV C_SetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState, CK_ULONG ulOperationStateLen, CK_OBJECT_HANDLE hEncryptionKey, CK_OBJECT_HANDLE hAuthenticationKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SetOperationState Base_C_SetOperationState = (CK_C_SetOperationState)GlobalData::getInstance().getBaseFunction("C_SetOperationState");
	if(Base_C_SetOperationState == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SetOperationState)(hSession, pOperationState, ulOperationStateLen, hEncryptionKey, hAuthenticationKey);
}

PKCS_API CK_RV C_Login(CK_SESSION_HANDLE hSession, CK_USER_TYPE userType, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Login Base_C_Login = (CK_C_Login)GlobalData::getInstance().getBaseFunction("C_Login");
	if(Base_C_Login == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Login)(hSession, userType, pPin, ulPinLen);
}

PKCS_API CK_RV C_Logout(CK_SESSION_HANDLE hSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Logout Base_C_Logout = (CK_C_Logout)GlobalData::getInstance().getBaseFunction("C_Logout");
	if(Base_C_Logout == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Logout)(hSession);
}

PKCS_API CK_RV C_CreateObject(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_CreateObject Base_C_CreateObject = (CK_C_CreateObject)GlobalData::getInstance().getBaseFunction("C_CreateObject");
	if(Base_C_CreateObject == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_CreateObject)(hSession, pTemplate, ulCount, phObject);
}

PKCS_API CK_RV C_CopyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phNewObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_CopyObject Base_C_CopyObject = (CK_C_CopyObject)GlobalData::getInstance().getBaseFunction("C_CopyObject");
	if(Base_C_CopyObject == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_CopyObject)(hSession, hObject, pTemplate, ulCount, phNewObject);
}

PKCS_API CK_RV C_DestroyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DestroyObject Base_C_DestroyObject = (CK_C_DestroyObject)GlobalData::getInstance().getBaseFunction("C_DestroyObject");
	if(Base_C_DestroyObject == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DestroyObject)(hSession, hObject);
}

PKCS_API CK_RV C_GetObjectSize(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ULONG_PTR pulSize)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetObjectSize Base_C_GetObjectSize = (CK_C_GetObjectSize)GlobalData::getInstance().getBaseFunction("C_GetObjectSize");
	if(Base_C_GetObjectSize == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetObjectSize)(hSession, hObject, pulSize);
}

PKCS_API CK_RV C_GetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetAttributeValue Base_C_GetAttributeValue = (CK_C_GetAttributeValue)GlobalData::getInstance().getBaseFunction("C_GetAttributeValue");
	if(Base_C_GetAttributeValue == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetAttributeValue)(hSession, hObject, pTemplate, ulCount);
}

PKCS_API CK_RV C_SetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SetAttributeValue Base_C_SetAttributeValue = (CK_C_SetAttributeValue)GlobalData::getInstance().getBaseFunction("C_SetAttributeValue");
	if(Base_C_SetAttributeValue == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SetAttributeValue)(hSession, hObject, pTemplate, ulCount);
}

PKCS_API CK_RV C_FindObjectsInit(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_FindObjectsInit Base_C_FindObjectsInit = (CK_C_FindObjectsInit)GlobalData::getInstance().getBaseFunction("C_FindObjectsInit");
	if(Base_C_FindObjectsInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_FindObjectsInit)(hSession, pTemplate, ulCount);
}

PKCS_API CK_RV C_FindObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_FindObjects Base_C_FindObjects = (CK_C_FindObjects)GlobalData::getInstance().getBaseFunction("C_FindObjects");
	if(Base_C_FindObjects == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_FindObjects)(hSession, phObject, ulMaxObjectCount, pulObjectCount);
}

PKCS_API CK_RV C_FindObjectsFinal(CK_SESSION_HANDLE hSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_FindObjectsFinal Base_C_FindObjectsFinal = (CK_C_FindObjectsFinal)GlobalData::getInstance().getBaseFunction("C_FindObjectsFinal");
	if(Base_C_FindObjectsFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_FindObjectsFinal)(hSession);
}

PKCS_API CK_RV C_EncryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_EncryptInit Base_C_EncryptInit = (CK_C_EncryptInit)GlobalData::getInstance().getBaseFunction("C_EncryptInit");
	if(Base_C_EncryptInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_EncryptInit)(hSession, pMechanism, hObject);
}

PKCS_API CK_RV C_Encrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pEncryptedData, CK_ULONG_PTR pulEncryptedDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Encrypt Base_C_Encrypt = (CK_C_Encrypt)GlobalData::getInstance().getBaseFunction("C_Encrypt");
	if(Base_C_Encrypt == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Encrypt)(hSession, pData, ulDataLen, pEncryptedData, pulEncryptedDataLen);
}

PKCS_API CK_RV C_EncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pEncryptedData, CK_ULONG_PTR pulEncryptedDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_EncryptUpdate Base_C_EncryptUpdate = (CK_C_EncryptUpdate)GlobalData::getInstance().getBaseFunction("C_EncryptUpdate");
	if(Base_C_EncryptUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_EncryptUpdate)(hSession, pData, ulDataLen, pEncryptedData, pulEncryptedDataLen);
}

PKCS_API CK_RV C_EncryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedData, CK_ULONG_PTR pulEncryptedDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_EncryptFinal Base_C_EncryptFinal = (CK_C_EncryptFinal)GlobalData::getInstance().getBaseFunction("C_EncryptFinal");
	if(Base_C_EncryptFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_EncryptFinal)(hSession, pEncryptedData, pulEncryptedDataLen);
}

PKCS_API CK_RV C_DecryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DecryptInit Base_C_DecryptInit = (CK_C_DecryptInit)GlobalData::getInstance().getBaseFunction("C_DecryptInit");
	if(Base_C_DecryptInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DecryptInit)(hSession, pMechanism, hObject);
}

PKCS_API CK_RV C_Decrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen, CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Decrypt Base_C_Decrypt = (CK_C_Decrypt)GlobalData::getInstance().getBaseFunction("C_Decrypt");
	if(Base_C_Decrypt == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Decrypt)(hSession, pEncryptedData, ulEncryptedDataLen, pData, pulDataLen);
}

PKCS_API CK_RV C_DecryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen, CK_BYTE_PTR pData, CK_ULONG_PTR pDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DecryptUpdate Base_C_DecryptUpdate = (CK_C_DecryptUpdate)GlobalData::getInstance().getBaseFunction("C_DecryptUpdate");
	if(Base_C_DecryptUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DecryptUpdate)(hSession, pEncryptedData, ulEncryptedDataLen, pData, pDataLen);
}

PKCS_API CK_RV C_DecryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG_PTR pDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DecryptFinal Base_C_DecryptFinal = (CK_C_DecryptFinal)GlobalData::getInstance().getBaseFunction("C_DecryptFinal");
	if(Base_C_DecryptFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DecryptFinal)(hSession, pData, pDataLen);
}

PKCS_API CK_RV C_DigestInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DigestInit Base_C_DigestInit = (CK_C_DigestInit)GlobalData::getInstance().getBaseFunction("C_DigestInit");
	if(Base_C_DigestInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DigestInit)(hSession, pMechanism);
}

PKCS_API CK_RV C_Digest(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pDigest, CK_ULONG_PTR pulDigestLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Digest Base_C_Digest = (CK_C_Digest)GlobalData::getInstance().getBaseFunction("C_Digest");
	if(Base_C_Digest == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Digest)(hSession, pData, ulDataLen, pDigest, pulDigestLen);
}

PKCS_API CK_RV C_DigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DigestUpdate Base_C_DigestUpdate = (CK_C_DigestUpdate)GlobalData::getInstance().getBaseFunction("C_DigestUpdate");
	if(Base_C_DigestUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DigestUpdate)(hSession, pPart, ulPartLen);
}

PKCS_API CK_RV C_DigestKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DigestKey Base_C_DigestKey = (CK_C_DigestKey)GlobalData::getInstance().getBaseFunction("C_DigestKey");
	if(Base_C_DigestKey == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DigestKey)(hSession, hObject);
}

PKCS_API CK_RV C_DigestFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pDigest, CK_ULONG_PTR pulDigestLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DigestFinal Base_C_DigestFinal = (CK_C_DigestFinal)GlobalData::getInstance().getBaseFunction("C_DigestFinal");
	if(Base_C_DigestFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DigestFinal)(hSession, pDigest, pulDigestLen);
}

PKCS_API CK_RV C_SignInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignInit Base_C_SignInit = (CK_C_SignInit)GlobalData::getInstance().getBaseFunction("C_SignInit");
	if(Base_C_SignInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignInit)(hSession, pMechanism, hKey);
}

PKCS_API CK_RV C_Sign(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Sign Base_C_Sign = (CK_C_Sign)GlobalData::getInstance().getBaseFunction("C_Sign");
	if(Base_C_Sign == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Sign)(hSession, pData, ulDataLen, pSignature, pulSignatureLen);
}

PKCS_API CK_RV C_SignUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignUpdate Base_C_SignUpdate = (CK_C_SignUpdate)GlobalData::getInstance().getBaseFunction("C_SignUpdate");
	if(Base_C_SignUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignUpdate)(hSession, pPart, ulPartLen);
}

PKCS_API CK_RV C_SignFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignFinal Base_C_SignFinal = (CK_C_SignFinal)GlobalData::getInstance().getBaseFunction("C_SignFinal");
	if(Base_C_SignFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignFinal)(hSession, pSignature, pulSignatureLen);
}

PKCS_API CK_RV C_SignRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignRecoverInit Base_C_SignRecoverInit = (CK_C_SignRecoverInit)GlobalData::getInstance().getBaseFunction("C_SignRecoverInit");
	if(Base_C_SignRecoverInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignRecoverInit)(hSession, pMechanism, hKey);
}

PKCS_API CK_RV C_SignRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignRecover Base_C_SignRecover = (CK_C_SignRecover)GlobalData::getInstance().getBaseFunction("C_SignRecover");
	if(Base_C_SignRecover == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignRecover)(hSession, pData, ulDataLen, pSignature, pulSignatureLen);
}

PKCS_API CK_RV C_VerifyInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_VerifyInit Base_C_VerifyInit = (CK_C_VerifyInit)GlobalData::getInstance().getBaseFunction("C_VerifyInit");
	if(Base_C_VerifyInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_VerifyInit)(hSession, pMechanism, hKey);
}

PKCS_API CK_RV C_Verify(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_Verify Base_C_Verify = (CK_C_Verify)GlobalData::getInstance().getBaseFunction("C_Verify");
	if(Base_C_Verify == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_Verify)(hSession, pData, ulDataLen, pSignature, ulSignatureLen);
}

PKCS_API CK_RV C_VerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_VerifyUpdate Base_C_VerifyUpdate = (CK_C_VerifyUpdate)GlobalData::getInstance().getBaseFunction("C_VerifyUpdate");
	if(Base_C_VerifyUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_VerifyUpdate)(hSession, pPart, ulPartLen);
}

PKCS_API CK_RV C_VerifyFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_VerifyFinal Base_C_VerifyFinal = (CK_C_VerifyFinal)GlobalData::getInstance().getBaseFunction("C_VerifyFinal");
	if(Base_C_VerifyFinal == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_VerifyFinal)(hSession, pSignature, ulSignatureLen);
}

PKCS_API CK_RV C_VerifyRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_VerifyRecoverInit Base_C_VerifyRecoverInit = (CK_C_VerifyRecoverInit)GlobalData::getInstance().getBaseFunction("C_VerifyRecoverInit");
	if(Base_C_VerifyRecoverInit == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_VerifyRecoverInit)(hSession, pMechanism, hKey);
}

PKCS_API CK_RV C_VerifyRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen, CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_VerifyRecover Base_C_VerifyRecover = (CK_C_VerifyRecover)GlobalData::getInstance().getBaseFunction("C_VerifyRecover");
	if(Base_C_VerifyRecover == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_VerifyRecover)(hSession, pSignature, ulSignatureLen, pData, pulDataLen);
}

PKCS_API CK_RV C_DigestEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen, CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DigestEncryptUpdate Base_C_DigestEncryptUpdate = (CK_C_DigestEncryptUpdate)GlobalData::getInstance().getBaseFunction("C_DigestEncryptUpdate");
	if(Base_C_DigestEncryptUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DigestEncryptUpdate)(hSession, pPart, ulPartLen, pEncryptedPart, pulEncryptedPartLen);
}

PKCS_API CK_RV C_DecryptDigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen, CK_BYTE_PTR pDecryptedPart, CK_ULONG_PTR pulDecryptedPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DecryptDigestUpdate Base_C_DecryptDigestUpdate = (CK_C_DecryptDigestUpdate)GlobalData::getInstance().getBaseFunction("C_DecryptDigestUpdate");
	if(Base_C_DecryptDigestUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DecryptDigestUpdate)(hSession, pPart, ulPartLen, pDecryptedPart, pulDecryptedPartLen);
}

PKCS_API CK_RV C_SignEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen, CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_SignEncryptUpdate Base_C_SignEncryptUpdate = (CK_C_SignEncryptUpdate)GlobalData::getInstance().getBaseFunction("C_SignEncryptUpdate");
	if(Base_C_SignEncryptUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_SignEncryptUpdate)(hSession, pPart, ulPartLen, pEncryptedPart, pulEncryptedPartLen);
}

PKCS_API CK_RV C_DecryptVerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart, CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DecryptVerifyUpdate Base_C_DecryptVerifyUpdate = (CK_C_DecryptVerifyUpdate)GlobalData::getInstance().getBaseFunction("C_DecryptVerifyUpdate");
	if(Base_C_DecryptVerifyUpdate == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DecryptVerifyUpdate)(hSession, pEncryptedPart, ulEncryptedPartLen, pPart, pulPartLen);
}

PKCS_API CK_RV C_GenerateKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GenerateKey Base_C_GenerateKey = (CK_C_GenerateKey)GlobalData::getInstance().getBaseFunction("C_GenerateKey");
	if(Base_C_GenerateKey == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GenerateKey)(hSession, pMechanism, pTemplate, ulCount, phKey);
}

PKCS_API CK_RV C_GenerateKeyPair(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_ATTRIBUTE_PTR pPublicKeyTemplate, CK_ULONG ulPublicKeyAttributeCount, CK_ATTRIBUTE_PTR pPrivateKeyTemplate, CK_ULONG ulPrivateKeyAttributeCount, CK_OBJECT_HANDLE_PTR phPublicKey, CK_OBJECT_HANDLE_PTR phPrivateKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GenerateKeyPair Base_C_GenerateKeyPair = (CK_C_GenerateKeyPair)GlobalData::getInstance().getBaseFunction("C_GenerateKeyPair");
	if(Base_C_GenerateKeyPair == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GenerateKeyPair)(hSession, pMechanism, pPublicKeyTemplate, ulPublicKeyAttributeCount, pPrivateKeyTemplate, ulPrivateKeyAttributeCount, phPublicKey, phPrivateKey);
}

PKCS_API CK_RV C_WrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hWrappingKey, CK_OBJECT_HANDLE hKey, CK_BYTE_PTR pWrappedKey, CK_ULONG_PTR pulWrappedKeyLen)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_WrapKey Base_C_WrapKey = (CK_C_WrapKey)GlobalData::getInstance().getBaseFunction("C_WrapKey");
	if(Base_C_WrapKey == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_WrapKey)(hSession, pMechanism, hWrappingKey, hKey, pWrappedKey, pulWrappedKeyLen);
}

PKCS_API CK_RV C_UnwrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hUnwrappingKey, CK_BYTE_PTR pWrappedKey, CK_ULONG ulWrappedKeyLen, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_UnwrapKey Base_C_UnwrapKey = (CK_C_UnwrapKey)GlobalData::getInstance().getBaseFunction("C_UnwrapKey");
	if(Base_C_UnwrapKey == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_UnwrapKey)(hSession, pMechanism, hUnwrappingKey, pWrappedKey, ulWrappedKeyLen, pTemplate, ulCount, phKey);
}

PKCS_API CK_RV C_DeriveKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hBaseKey, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phKey)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_DeriveKey Base_C_DeriveKey = (CK_C_DeriveKey)GlobalData::getInstance().getBaseFunction("C_DeriveKey");
	if(Base_C_DeriveKey == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_DeriveKey)(hSession, pMechanism, hBaseKey, pTemplate, ulCount, phKey);
}

PKCS_API CK_RV C_GetFunctionStatus(CK_SESSION_HANDLE hSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_GetFunctionStatus Base_C_GetFunctionStatus = (CK_C_GetFunctionStatus)GlobalData::getInstance().getBaseFunction("C_GetFunctionStatus");
	if(Base_C_GetFunctionStatus == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_GetFunctionStatus)(hSession);
}

PKCS_API CK_RV C_CancelFunction(CK_SESSION_HANDLE hSession)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_CancelFunction Base_C_CancelFunction = (CK_C_CancelFunction)GlobalData::getInstance().getBaseFunction("C_CancelFunction");
	if(Base_C_CancelFunction == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_CancelFunction)(hSession);
}

PKCS_API CK_RV C_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR pSlot, CK_VOID_PTR pReserved)
{
	if(!GlobalData::getInstance().isCryptokiInitialized()) return CKR_CRYPTOKI_NOT_INITIALIZED;
	
	CK_C_WaitForSlotEvent Base_C_WaitForSlotEvent = (CK_C_WaitForSlotEvent)GlobalData::getInstance().getBaseFunction("C_WaitForSlotEvent");
	if(Base_C_WaitForSlotEvent == NULL) return CKR_GENERAL_ERROR;
	
	return (*Base_C_WaitForSlotEvent)(flags, pSlot, pReserved);
}

