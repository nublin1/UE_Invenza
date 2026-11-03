// Nublin Studio 2026 All Rights Reserved.

#include "HUD/GameHUD_Inz.h"
#include "UI/Layout/PrimaryLayout.h"

AGameHUD_Inz::AGameHUD_Inz()
{
}

UPrimaryLayout* AGameHUD_Inz::GetPrimaryLayoutInstance() const
{
	return PrimaryLayout;
}
