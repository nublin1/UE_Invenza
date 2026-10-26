//  Nublin Studio 2026 All Rights Reserved.

#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Internationalization/StringTable.h"

FName UInvenzaInventoryDeveloperSettings::GetCategoryName() const
{
	return NAME_Game;
}

FText UInvenzaInventoryDeveloperSettings::GetText(FName Key) const
{
	if (const UStringTable* Table = RuntimeTextTable.LoadSynchronous())
	{
		return FText::FromStringTable(Table->GetStringTableId(), Key.ToString());
	}
	return FText::GetEmpty();
}
