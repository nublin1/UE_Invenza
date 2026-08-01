// Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/ModalWindowManager.h"

#include "Components/NamedSlot.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "HUD/GameHUD_Inz.h"
#include "Interface/HUD/HUDProvider.h"
#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/UILayer.h"
#include "Utility/InventoryUtility.h"

UModalWindowManager::UModalWindowManager()
{
}

void UModalWindowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	InvenzaInventorySettingsAsset = UInventoryUtility::GetInvenzaGlobalSettings(World);
	if (!InvenzaInventorySettingsAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ModalWindowManager: Global Settings not found!"));
		return ;
	}
}

void UModalWindowManager::InitializeUI()
{
	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (!PC)
	{
		return;
	}

	AHUD* HUD = PC->GetHUD();
	if (!HUD || !HUD->Implements<UHUDProvider>())
	{
		return;
	}

	PrimaryLayoutRef = IHUDProvider::Execute_GetPrimaryLayout(HUD);
	if (!PrimaryLayoutRef)
	{
		return;
	}

	ModalLayoutRef = Cast<UModalLayout>(PrimaryLayoutRef->ModalLayer);
}

void UModalWindowManager::OpenModalFlow(EModalHeaderType HeaderType, const FText& HeaderText, EModalFooterType FooterType,
                                        const TMap<EObjectInteractionType, FModalActionConfig>& Actions, FModalResultDelegate OnResult)
{
	FinalDelegate = OnResult;
	PendingOriginalResult = FModalResult();

	ShowModalStep(HeaderType, HeaderText, FooterType, Actions);
}

void UModalWindowManager::ShowModalStep(EModalHeaderType HeaderType,const FText& HeaderText, EModalFooterType FooterType,
	 const TMap<EObjectInteractionType, FModalActionConfig>& Actions)
{
	UWorld* World = GetWorld();
	if (!World || !InvenzaInventorySettingsAsset) return;
	
	if (!ModalLayoutRef)
	{
		return;
	}
	
	// Если в стеке уже есть окно (это не первый шаг цепочки) — скрываем его, прежде чем показать следующее.
	if (ModalLayoutRef->GetStack().Num() > 0)
	{
		ModalLayoutRef->CollapseTop();
	}
	
	UInvenzaBaseWidget* ModalDialog = ModalLayoutRef->PushContent(InvenzaInventorySettingsAsset->DefaultModalDialogClass.Get());
	if (!ModalDialog)
	{
		return;
	}
	
	auto ModalDialogCasted = Cast<UModalDialogBase>(ModalDialog);
	if (!ModalDialogCasted)
		return;
	
	if (HeaderType != EModalHeaderType::None)
	{
		const TSubclassOf<UUserWidget>* HeaderClassPtr = InvenzaInventorySettingsAsset->ModalHeaderWidgets.Find(HeaderType);
		AttachChildWidget(World, ModalDialogCasted->Upper_Slot, HeaderClassPtr ? *HeaderClassPtr : nullptr);
	}

	const TSubclassOf<UUserWidget>* FooterClassPtr = InvenzaInventorySettingsAsset->ModalFooterWidgets.Find(FooterType);
	AttachChildWidget(World, ModalDialogCasted->Down_Slot, FooterClassPtr ? *FooterClassPtr : nullptr);

	ModalDialogCasted->DynamicResultDelegate.BindDynamic(this, &UModalWindowManager::HandleModalResponse);

	TArray<EObjectInteractionType> ActionKeys;
	Actions.GenerateKeyArray(ActionKeys);

	TArray<FModalAction> AvActions;
	AvActions.Reserve(ActionKeys.Num());
	for (const EObjectInteractionType& ActionKey : ActionKeys)
	{
		AvActions.Add(InvenzaInventorySettingsAsset->ModalActions.FindRef(ActionKey));
	}
	ModalDialogCasted->Configure(HeaderText, ActionKeys, AvActions);

	// Запоминаем полную карту текущего шага — понадобится в HandleModalResponse,
	// чтобы по нажатому Interaction найти соответствующий FObjectModalAction.
	CurrentStepActionsMap = Actions;

	ModalLayoutRef->ShowTop();
}

void UModalWindowManager::HandleModalResponse(FModalResult Result)
{
	const EObjectInteractionType Interaction = Result.ResultInteractionType;

	if (Interaction == EObjectInteractionType::None || Interaction == EObjectInteractionType::Cancel)
	{
		PendingOriginalResult = FModalResult();
		ModalLayoutRef->ClearStack();
		if (FinalDelegate.IsBound())
		{
			FModalResult CancelResult;
			CancelResult.ResultInteractionType = EObjectInteractionType::None;
			FinalDelegate.Execute(CancelResult);
		}
		return;
	}
	
	if (PendingOriginalAction.StepRequirement != EModalStepRequirement::None)
	{
		const FModalResult OriginalResult = PendingOriginalResult;
		const FModalActionConfig OriginalAction = PendingOriginalAction;
		PendingOriginalResult = FModalResult();
		PendingOriginalAction = FModalActionConfig();

		if (OriginalAction.StepRequirement == EModalStepRequirement::RequiresConfirm)
		{
			ModalLayoutRef->ClearStack();
			
			if (Interaction == EObjectInteractionType::Yes)
			{
				FinalDelegate.Execute(OriginalResult);
			}
			else if (FinalDelegate.IsBound())
			{
				FModalResult CancelResult;
				CancelResult.ResultInteractionType = EObjectInteractionType::None;
				FinalDelegate.Execute(CancelResult);
			}
			return;
		}

		if (OriginalAction.StepRequirement == EModalStepRequirement::RequiresAmount)
		{
			// TODO: склеить введённое количество с OriginalAction и разбродкастить
			return;
		}
	}
	
	const FModalActionConfig* ActionData = CurrentStepActionsMap.Find(Interaction);
	const EModalStepRequirement Requirement = ActionData ? ActionData->StepRequirement : EModalStepRequirement::None;

	// Обычный первый шаг
	switch (Requirement)
	{
	case EModalStepRequirement::RequiresConfirm:
		{
			PendingOriginalResult = Result;
			PendingOriginalAction = *ActionData; 

			TMap<EObjectInteractionType, FModalActionConfig> DummyActions;
			DummyActions.Add(EObjectInteractionType::Yes, FModalActionConfig());
			DummyActions.Add(EObjectInteractionType::No, FModalActionConfig());

			ShowModalStep(EModalHeaderType::SimpleText, ActionData->HeaderText, EModalFooterType::Binary, DummyActions);
			return;
		}

	case EModalStepRequirement::RequiresAmount:
		{
			PendingOriginalResult = Result;
			PendingOriginalAction = *ActionData;
			// TODO: открыть AmountInput footer, когда он появится
			return;
		}

	case EModalStepRequirement::None:
	default:
		ModalLayoutRef->ClearStack();
		FinalDelegate.Execute(Result);
		break;
	}
}

void UModalWindowManager::AttachChildWidget(UWorld* World, UPanelWidget* Slot, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!Slot || !WidgetClass) return;

	UUserWidget* ChildWidget = CreateWidget<UUserWidget>(World, WidgetClass);
	if (!ChildWidget) return;

	Slot->AddChild(ChildWidget);
}