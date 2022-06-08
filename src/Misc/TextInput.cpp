#include <Utilities\Macro.h>

// Allow message entry in Skirmish
// DEFINE_LJMP(0x55E484, 0x55E48D);

wchar_t* IMEBuffer = reinterpret_cast<wchar_t*>(0xB730EC);

inline
UINT GetCurentCodepage()
{
	char szLCData[6 + 1];
	WORD lang = LOWORD(GetKeyboardLayout(NULL));
	LCID locale = MAKELCID(lang, SORT_DEFAULT);
	GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, szLCData, _countof(szLCData));

	return atoi(szLCData);
}

DWORD _stdcall LocalizeCaracter(DWORD character)
{
	if (!IMEBuffer[0])
	{
		wchar_t result;
		UINT codepage = GetCurentCodepage();
		char c = (char)character;
		MultiByteToWideChar(codepage, MB_USEGLYPHCHARS, &c, 1, &result, 1);
		return result;
	}
	return character;
}

DEFINE_NAKED_LJMP(0x5D46B9, MessageListClass_Input)
{
	_asm {push ebx};
	_asm {call LocalizeCaracter};
	_asm {mov ebx, eax};

	_asm {mov eax, [esi+2B0h]};

	_asm {mov ecx, 0x5D46BF};
	_asm {jmp ecx};
}

DEFINE_NAKED_LJMP(0x61526C, WWUI_NewEditCtrl)
{
	_asm {push edi};
	_asm {call LocalizeCaracter};
	_asm {mov edi, eax};

	_asm {mov eax, [esi+40h]};
	_asm {push eax};
	_asm {push edi};

	_asm {mov ecx, 0x615271};
	_asm {jmp ecx};
}

//DEFINE_HOOK(0x5D46C7, MessageListClass_Input, 5)
//{
//	if (!IMEBuffer[0])
//		R->EBX<wchar_t>(LocalizeCaracter(R->EBX<char>()));
//
//	return 0;
//}
//
//DEFINE_HOOK(0x61526C, WWUI_NewEditCtrl, 5)
//{
//	if (!IMEBuffer[0])
//		R->EDI<wchar_t>(LocalizeCaracter(R->EDI<char>()));
//
//	return 0;
//}

// It is required to add Imm32.lib to AdditionalDependencies
/*
HIMC& IMEContext = *reinterpret_cast<HIMC*>(0xB7355C);
wchar_t* IMECompositionString = reinterpret_cast<wchar_t*>(0xB73318);

DEFINE_HOOK(0x777F15, IMEUpdateCompositionString, 7)
{
	IMECompositionString[0] = 0;
	ImmGetCompositionStringW(IMEContext, GCS_COMPSTR, IMECompositionString, 256);

	return 0;
}
*/
