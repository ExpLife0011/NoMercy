#include "NoMercy.h"
#include "Utils.h"

#include <lazy_importer.hpp>
#include <xorstr.hpp>

typedef bool(__cdecl* TInitializeFunction)(const char* c_szLicenseCode);
typedef bool(__cdecl* TFinalizeFunction)();
typedef bool(__cdecl* TMessageHandlerFunction)(TNMCallback lpMessageHandler);
typedef bool(__cdecl* TMessageSenderFunction)(int Code, const void* lpMessage);
typedef void(__cdecl* TOnTick)();

static HMODULE		s_hModule			= nullptr;
static std::string	s_szModuleName		= xorstr("NoMercy.dll").crypt_get();
static std::string	s_szLogFile			= xorstr("NoMercy.log").crypt_get();

static TMessageSenderFunction	s_MessageSenderFunc	= nullptr;
static TOnTick					s_OnTickFunc		= nullptr;

// ------------------
bool CNM_SDK::SecureNoMercyMessage(const void* c_lpMessage, uint32_t ulSize, uint8_t pKey, bool bCrypt) // TODO
{
	return true;
}

bool CNM_SDK::GetChecksumNoMercyMessage(const void* c_lpMessage, uint32_t ulSize, uint32_t * pulChecksum) // TODO
{
	return true;
}

std::unique_ptr <SNMClientMsgBody> CNM_SDK::BuildMessage(const void* c_lpMessage, uint32_t ulSize, bool bCrypt, uint8_t pCryptKey, std::uint32_t * pulErrorStep) // TODO
{
	auto pBody = std::make_unique<SNMClientMsgBody>();

#if 0
	if (!pBody || !pBody.get())
	{
		if (pulErrorStep) *pulErrorStep = 1;
		return pBody;
	}

	auto ulPureChecksum = 0U;
	if (false == GetChecksumNoMercyMessage(c_lpMessage, ulSize, &ulPureChecksum))
	{
		if (pulErrorStep) *pulErrorStep = 2;
		return pBody;
	}

	auto pCryptedMsg = std::make_unique<uint8_t>(ulSize);
	if (!pCryptedMsg || !pCryptedMsg.get())
	{
		if (pulErrorStep) *pulErrorStep = 3;
		return pBody;
	}

	std::memcpy(pCryptedMsg.get(), c_lpMessage, ulSize);

	if (bCrypt && pCryptKey && false == SecureNoMercyMessage(pCryptedMsg.get(), ulSize, pCryptKey, true))
	{
		if (pulErrorStep) *pulErrorStep = 4;
		return pBody;
	}

	auto ulCurrChecksum = 0U;
	if (false == GetChecksumNoMercyMessage(pCryptedMsg.get(), ulSize, &ulCurrChecksum))
	{
		if (pulErrorStep) *pulErrorStep = 5;
		return pBody;
	}

	auto bCompleted = false;

	std::memcpy(pBody->pMagic, "NMCY", sizeof(pBody->pMagic));
	std::memcpy(pBody->pContext, pCryptedMsg.get(), ulSize);
	pBody->ulContextSize = ulSize;
	pBody->ulContextPureSum = ulPureChecksum;
	pBody->ulContextSum = ulCurrChecksum;
	pBody->pCryptKey = pCryptKey;
	pBody->pbCompleted = &bCompleted;
#endif

	return pBody;
}

bool CNM_SDK::SendTestMessage(bool bCrypted) // TODO
{
#if 0
	std::string szMessage = "Hello from: " + GetCurrentProcessId();

	auto pTestMsgCtx = std::make_unique<SNMTestMsgCtx>();
	strcpy(pTestMsgCtx->szMessage, szMessage.c_str());

	auto uiErrorStep = 0U;
	auto pMessageBody = std::unique_ptr<SNMClientMsgBody>();

	if (bCrypted)
		pMessageBody = BuildMessage(pTestMsgCtx.get(), sizeof(SNMTestMsgCtx), true, NOMERCY_MESSAGE_CRYPT_KEY, &uiErrorStep);
	else
		pMessageBody = BuildMessage(pTestMsgCtx.get(), sizeof(SNMTestMsgCtx), false, 0x0, &uiErrorStep);

	printf("Build message completed! Data: %p Error step: %u", pMessageBody.get(), uiErrorStep);

	return SendDataToNoMercy(NM_DATA_SEND_TEST_MESSAGE, pMessageBody.get());
#endif

	return true;
}

// -------------------

bool CNM_SDK::InitNoMercy(const char* c_szLicenseCode)
{
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy initilization started!").crypt_get());

	LI_FIND(CreateDirectoryA)(xorstr("NoMercy").crypt_get(), NULL);
	LI_FIND(DeleteFileA)(s_szLogFile.c_str());

	if (NNoMercyUtils::IsFileExist(s_szModuleName.c_str()) == false)
	{
		NNoMercyUtils::FileLog(s_szLogFile, xorstr("Error! NoMercy DLL file not found!").crypt_get());
		return false;
	}
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy DLL module succesfully found!").crypt_get());

	auto s_hModule = LI_FIND(LoadLibraryA)(s_szModuleName.c_str());
	if (!s_hModule)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy DLL file can not load! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLogf(s_szLogFile, xorstr("NoMercy DLL file succesfully loaded!").crypt_get());

	auto NoMercyInitializeFunction = reinterpret_cast<TInitializeFunction>(LI_FIND(GetProcAddress)(s_hModule, xorstr("Initialize").crypt_get()));
	if (!NoMercyInitializeFunction)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy Initialize function not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy Initialize function found!").crypt_get());

	if (!NoMercyInitializeFunction(c_szLicenseCode)) 
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy Initilization call fail! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy Initializion completed!").crypt_get());
	return true;
}

bool CNM_SDK::ReleaseNoMercy()
{
	if (!s_hModule)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy DLL file not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy DLL file found!").crypt_get());

	auto NoMercyFinalizeFunction = reinterpret_cast<TFinalizeFunction>(LI_FIND(GetProcAddress)(s_hModule, xorstr("Finalize").crypt_get()));
	if (!NoMercyFinalizeFunction)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy Finalize function not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLogf(s_szLogFile, xorstr("NoMercy Finalize function found!").crypt_get());

	if (!NoMercyFinalizeFunction())
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy Finalize call fail! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
	NNoMercyUtils::FileLog(s_szLogFile, xorstr("NoMercy Finalization completed!").crypt_get());
	return true;
}


bool CNM_SDK::OnGameTick()
{
	if (!s_OnTickFunc)
	{
		if (!s_hModule)
		{
			NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy DLL file not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
			return false;
		}
//		NNoMercyUtils::FileLog(XOR("NoMercy DLL file found!"));

		s_OnTickFunc = reinterpret_cast<TOnTick>(LI_FIND(GetProcAddress)(s_hModule, xorstr("OnTick").crypt_get()));
		if (!s_OnTickFunc)
		{
			NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy OnTick function not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
			return false;
		}
//		NNoMercyUtils::FileLog(XOR("NoMercy OnTick function found!"));
	}

	s_OnTickFunc();
//	NNoMercyUtils::FileLog(XOR("NoMercy OnTick call completed!"));
	return true;
}


bool CNM_SDK::RegisterNoMercyMessageHandler(TNMCallback lpMessageHandler)
{
	if (!s_hModule)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy DLL file not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
//	NNoMercyUtils::FileLog(XOR("NoMercy DLL file found!"));

	auto NoMercyMsgHandlerFunction = reinterpret_cast<TMessageHandlerFunction>(LI_FIND(GetProcAddress)(s_hModule, xorstr("SetupMsgHandler").crypt_get()));
	if (!NoMercyMsgHandlerFunction)
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy MsgHandler function not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
//	NNoMercyUtils::FileLog(XOR("NoMercy MsgHandler function found!"));

	if (!NoMercyMsgHandlerFunction(lpMessageHandler))
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy MsgHandler call fail! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
//	NNoMercyUtils::FileLog(XOR("NoMercy MsgHandler initilization completed!"));
	return true;
}

bool CNM_SDK::SendDataToNoMercy(NM_DATA_CODES Code, const void* lpMessage)
{
	if (!s_MessageSenderFunc)
	{
		if (!s_hModule)
		{
			NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy DLL file not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
			return false;
		}
//		NNoMercyUtils::FileLog(XOR("NoMercy DLL file found!"));

		s_MessageSenderFunc = reinterpret_cast<TMessageSenderFunction>(LI_FIND(GetProcAddress)(s_hModule, xorstr("MsgHelper").crypt_get()));
		if (!s_MessageSenderFunc)
		{
			NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy MsgSender function not found! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
			return false;
		}
//		NNoMercyUtils::FileLog(XOR("NoMercy MsgSender function found!"));
	}

	if (!s_MessageSenderFunc(static_cast<int>(Code), lpMessage))
	{
		NNoMercyUtils::FileLogf(s_szLogFile, xorstr("Error! NoMercy MsgSender call fail! Error code: %u").crypt_get(), LI_FIND(GetLastError)());
		return false;
	}
//	NNoMercyUtils::FileLog(XOR("NoMercy MsgSender initilization completed!"));
	return true;
}

