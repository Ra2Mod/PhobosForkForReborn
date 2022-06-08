#include <Utilities/Macro.h>

#include "PhobosToolTip.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <HouseClass.h>

#include <GameOptionsClass.h>
#include <CCToolTip.h>
#include <BitFont.h>
#include <BitText.h>

#include <sstream>
#include <iomanip>

wchar_t* replace_wchar_t(wchar_t* str, wchar_t find, wchar_t replace){
    wchar_t *current_pos = wcschr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = wcschr(current_pos,find);
    }
    return str;
}

PhobosToolTip PhobosToolTip::Instance;

inline bool PhobosToolTip::IsEnabled() const
{
	return Phobos::UI::ExtendedToolTips;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(TechnoTypeExt::ExtData* pData) const
{
	return Phobos::Config::ExtendedToolTips && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

inline const wchar_t* PhobosToolTip::GetUIDescription(SWTypeExt::ExtData* pData) const
{
	return Phobos::Config::ExtendedToolTips && !pData->UIDescription.Get().empty()
		? pData->UIDescription.Get().Text
		: nullptr;
}

inline int PhobosToolTip::GetBuildTime(TechnoTypeClass* pType) const
{
	static char pTrick[0x6C8]; // Just big enough to hold all types
	switch (pType->WhatAmI())
	{
	case AbstractType::BuildingType:
		*reinterpret_cast<int*>(pTrick) = 0x7E3EBC; // BuildingClass::`vtable`
		reinterpret_cast<BuildingClass*>(pTrick)->Type = (BuildingTypeClass*)pType;
		break;
	case AbstractType::AircraftType:
		*reinterpret_cast<int*>(pTrick) = 0x7E22A4; // AircraftClass::`vtable`
		reinterpret_cast<AircraftClass*>(pTrick)->Type = (AircraftTypeClass*)pType;
		break;
	case AbstractType::InfantryType:
		*reinterpret_cast<int*>(pTrick) = 0x7EB058; // InfantryClass::`vtable`
		reinterpret_cast<InfantryClass*>(pTrick)->Type = (InfantryTypeClass*)pType;
		break;
	case AbstractType::UnitType:
		*reinterpret_cast<int*>(pTrick) = 0x7F5C70; // UnitClass::`vtable`
		reinterpret_cast<UnitClass*>(pTrick)->Type = (UnitTypeClass*)pType;
		break;
	}

	// TechnoTypeClass only has 4 final classes :
	// BuildingTypeClass, AircraftTypeClass, InfantryTypeClass and UnitTypeClass
	// It has to be these four classes, otherwise pType will just be nullptr
	reinterpret_cast<TechnoClass*>(pTrick)->Owner = HouseClass::Player;
	int nTimeToBuild = reinterpret_cast<TechnoClass*>(pTrick)->TimeToBuild();
	// 54 frames at least
	return nTimeToBuild < 54 ? 54 : nTimeToBuild;
}

inline int PhobosToolTip::GetPower(TechnoTypeClass* pType) const
{
	if (auto const pBldType = abstract_cast<BuildingTypeClass*>(pType))
		return pBldType->PowerBonus - pBldType->PowerDrain;

	return 0;
}

inline const wchar_t* PhobosToolTip::GetBuffer() const
{
	return this->TextBuffer.c_str();
}

void PhobosToolTip::HelpText(BuildType& cameo)
{
	if (cameo.ItemType == AbstractType::Special)
		this->HelpText(SuperWeaponTypeClass::Array->GetItem(cameo.ItemIndex));
	else
		this->HelpText(ObjectTypeClass::GetTechnoType(cameo.ItemType, cameo.ItemIndex));
}

void PhobosToolTip::HelpText(TechnoTypeClass* pType)
{
	if (!pType)
		return;

	// auto const pData = TechnoTypeExt::ExtMap.Find(pType);

	int nBuildTime = this->GetBuildTime(pType);
	int nSec = nBuildTime / 15 % 60;
	int nMin = nBuildTime / 15 / 60 /* % 60*/;
	// int nHour = pType->RechargeTime / 15 / 60 / 60;

	int cost = pType->GetActualCost(HouseClass::Player);

	std::wostringstream oss;
	wcscpy_s(Phobos::wideBuffer, pType->UIName);
	replace_wchar_t(Phobos::wideBuffer, 0x20, 0x0A);
	oss << Phobos::wideBuffer;

	oss << L"\n" << (cost < 0 ? L"+" : L"");
	oss << Phobos::UI::CostLabel << std::abs(cost);

	if (PhobosToolTip::IsEnabled())
	{
		oss << L" "
			<< Phobos::UI::TimeLabel
			// << std::setw(2) << std::setfill(L'0') << nHour << L":"
			<< std::setw(2) << std::setfill(L'0') << nMin << L":"
			<< std::setw(2) << std::setfill(L'0') << nSec;

		if (auto const nPower = this->GetPower(pType))
		{
			oss << L" " << Phobos::UI::PowerLabel;
			if (nPower > 0)
				oss << L"+";
			oss << std::setw(1) << nPower;
		}
	}

	// if (auto pDesc = this->GetUIDescription(pData))
	// 	oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
}

void PhobosToolTip::HelpText(SuperWeaponTypeClass* pType)
{
	// auto const pData = SWTypeExt::ExtMap.Find(pType);

	std::wostringstream oss;
	wcscpy_s(Phobos::wideBuffer, pType->UIName);
	replace_wchar_t(Phobos::wideBuffer, 0x20, 0x0A);
	oss << Phobos::wideBuffer;
	bool showCost = false;

	/*
		if (int nCost = std::abs(pData->Money_Amount))
		{
			oss << L"\n";

			if (pData->Money_Amount > 0)
				oss << '+';

			oss << Phobos::UI::CostLabel << nCost;
			showCost = true;
		}
	*/

	if (PhobosToolTip::IsEnabled() && pType->RechargeTime > 0)
	{
		if (!showCost)
			oss << L"\n";

		int nSec = pType->RechargeTime / 15 % 60;
		int nMin = pType->RechargeTime / 15 / 60 /* % 60*/;
		// int nHour = pType->RechargeTime / 15 / 60 / 60;

		oss << (showCost ? L" " : L"") << Phobos::UI::TimeLabel
			// << std::setw(2) << std::setfill(L'0') << nHour << L":"
			<< std::setw(2) << std::setfill(L'0') << nMin << L":"
			<< std::setw(2) << std::setfill(L'0') << nSec;
	}

	// if (auto pDesc = this->GetUIDescription(pData))
	// 	oss << L"\n" << pDesc;

	this->TextBuffer = oss.str();
}

// Hooks

const wchar_t* _stdcall SidebarClass_StripClass_HelpText(StripClass* pThis)
{
	PhobosToolTip::Instance.IsCameo = true;

	PhobosToolTip::Instance.HelpText(pThis->Cameos[0]); // pStrip->Cameos[nID] in fact
	return L"X";
}

DEFINE_NAKED_LJMP(0x6A9319, _SidebarClass_StripClass_HelpText)
{
	_asm {push eax};
	_asm {call SidebarClass_StripClass_HelpText};

	_asm {mov ecx, 0x6A93DE};
	_asm {jmp ecx};
}

const wchar_t* _stdcall CCToolTip_Draw2_SetBuffer(const wchar_t* def)
{
	if (PhobosToolTip::Instance.IsCameo)
		return PhobosToolTip::Instance.GetBuffer();

	return def;
}

DEFINE_NAKED_LJMP(0x478EE1, _CCToolTip_Draw2_SetBuffer)
{
	_asm {push eax};

	_asm {push edi};
	_asm {call CCToolTip_Draw2_SetBuffer};
	_asm {mov edi, eax};

	_asm {pop eax};

	_asm {mov ebp, 0x89C4D0};
	_asm {mov ebp, [ebp]};

	_asm {mov ecx, 0x478EE7};
	_asm {jmp ecx};
}

void _fastcall CCToolTip_Draw1(CCToolTip* pThis, void*, bool bFullRedraw)
{
	// !onSidebar or (onSidebar && ExtToolTip::IsCameo)
	if (!bFullRedraw || PhobosToolTip::Instance.IsCameo)
	{
		PhobosToolTip::Instance.IsCameo = false;
		PhobosToolTip::Instance.SlaveDraw = false;

		pThis->ToolTipManager::Process();	//this function re-create CCToolTip
	}

	if (pThis->CurrentToolTip)
	{
		if (!bFullRedraw)
			PhobosToolTip::Instance.SlaveDraw = PhobosToolTip::Instance.IsCameo;

		pThis->FullRedraw = bFullRedraw;
		pThis->DrawText(pThis->CurrentToolTipData);
	}
	return;
}
DEFINE_POINTER_LJMP(0x478E10, CCToolTip_Draw1)


// =====================
