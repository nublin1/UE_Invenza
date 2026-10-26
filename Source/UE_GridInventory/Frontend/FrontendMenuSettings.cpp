//  Nublin Studio 2026 All Rights Reserved.

#include "Frontend/FrontendMenuSettings.h"
#include "Internationalization/StringTable.h"

FText UFrontendMenuSettings::GetText(FName Key) const
{
	if (auto* Table = TextTable.LoadSynchronous())
	{
		return FText::FromStringTable(Table->GetStringTableId(), Key.ToString());
	}
	return FText::GetEmpty();
}
