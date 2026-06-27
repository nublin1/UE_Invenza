// Nublin Studio 2026 All Rights Reserved.


#include "UI/Craft/CraftControlPanel.h"

UCraftControlPanel::UCraftControlPanel()
{
}

void UCraftControlPanel::NativeConstruct()
{
	Super::NativeConstruct();
}

TArray<UUIButton*> UCraftControlPanel::GetButtons_Implementation() const
{
	TArray<UUIButton*> Result;

	if (Btn_AddTask)
		Result.Add(Btn_AddTask);

	if (Btn_Pause)
		Result.Add(Btn_Pause);

	//if (ClearQueueButton)
	//	Result.Add(ClearQueueButton);

	return Result;
}
