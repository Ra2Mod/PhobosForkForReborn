#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EBX);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EDI);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

void TacticalClass_DrawPlacement_PlacementPreview()
{
	if(!Phobos::Config::EnableBuildingPlacementPreview)
		return;

	if (auto const pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding))
	{
		if (auto const pType = pBuilding->Type)
		{
			auto const pCell = MapClass::Instance->TryGetCellAt(Make_Global<CellStruct>(0x88095C) + Make_Global<CellStruct>(0x880960));
			if (!pCell)
				return;

			SHPStruct* pImage = nullptr;
			int nFrame = 0;

			if (pImage = pType->LoadBuildup())
			{
				nFrame = (pImage->Frames / 2) - 1;
			}
			else
			{
				pImage = pType->GetImage();
			}

			auto const nHeight = pCell->GetFloorHeight({ 0, 0 });
			CoordStruct const nOffset = { 0,-15,1 };
			Point2D nPoint { 0, 0 };
			TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, nHeight + nOffset.Z), &nPoint);
			nPoint.X += nOffset.X;
			nPoint.Y += nOffset.Y;
			auto nRect = DSurface::Temp()->GetRect();
			nRect.Height -= 32; // account for bottom bar

			auto const pPalette = pBuilding->GetDrawer();
			auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | BlitterFlags::TransLucent75;
			DSurface::Temp()->DrawSHP(pPalette, pImage, nFrame, &nPoint, &nRect, nFlag, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}
	}

	return;
}
static void __fastcall CellClass_Draw_It_Shape(Surface* Surface, ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
	const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags,
	int Remap,
	int ZAdjust,
	ZGradient ZGradientDescIndex,
	int Brightness,
	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	if(Phobos::Config::EnableBuildingPlacementPreview)
		Flags = Flags | BlitterFlags::TransLucent50;

	CC_Draw_Shape(Surface, Palette, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust,
		ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}


DEFINE_NAKED_LJMP(0x6D528A, _TacticalClass_DrawPlacement_PlacementPreview)
{
	_asm {call TacticalClass_DrawPlacement_PlacementPreview};
	_asm {mov ecx, 0x880990};
	_asm {mov ecx, [ecx]};
	_asm {mov edi, 0x6D5290};
	_asm {jmp edi};
}

DEFINE_POINTER_CALL(0x47EFB4, CellClass_Draw_It_Shape);
