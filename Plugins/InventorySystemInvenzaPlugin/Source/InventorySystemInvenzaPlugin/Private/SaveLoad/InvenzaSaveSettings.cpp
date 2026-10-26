//  Nublin Studio 2026 All Rights Reserved.

#include "SaveLoad/InvenzaSaveSettings.h"
#include "Internationalization/StringTable.h"

FName UInvenzaSaveSettings::GetCategoryName() const
{
	return NAME_Game;
}

FText UInvenzaSaveSettings::GetSaveText(FName Key) const
{
	if (auto* Table = TextTable.LoadSynchronous())
	{
		return FText::FromStringTable(Table->GetStringTableId(), Key.ToString());
	}
	UE_LOG(LogTemp, Warning, TEXT("Save text table is not configured; requested key: %s"), *Key.ToString());
	return FText::GetEmpty();
}
