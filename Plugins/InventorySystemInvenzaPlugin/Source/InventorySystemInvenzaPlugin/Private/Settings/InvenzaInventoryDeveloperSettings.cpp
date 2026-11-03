//  Nublin Studio 2026 All Rights Reserved.

#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Internationalization/StringTable.h"
#include "Engine/Engine.h"
#include "Subsystems/InvenzaUIThemeSubsystem.h"

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

#if WITH_EDITOR
void UInvenzaInventoryDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GEngine && PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, UITheme))
	{
		GEngine->GetEngineSubsystem<UInvenzaUIThemeSubsystem>()->RefreshDesignerWidgets();
	}
}
#endif
