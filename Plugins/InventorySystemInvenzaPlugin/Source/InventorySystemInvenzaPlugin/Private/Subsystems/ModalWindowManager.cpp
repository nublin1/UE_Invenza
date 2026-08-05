// Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/ModalWindowManager.h"

#include "Components/NamedSlot.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "HUD/GameHUD_Inz.h"
#include "Interface/HUD/HUDProvider.h"
#include "Interface/Interaction/ObjectDataProvider.h"
#include "UI/Layout/PrimaryLayout.h"
#include "UI/Layout/UILayer.h"
#include "Utility/InvenzayUtility.h"

UModalWindowManager::UModalWindowManager()
{
}

void UModalWindowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	InvenzaInventorySettingsAsset = UInvenzayUtility::GetInvenzaGlobalSettings(World);
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

	ModalLayoutRef = PrimaryLayoutRef->ModalLayer;
	if (!ModalLayoutRef)
		return;
	
	ModalLayoutRef->OnBackgroundClicked.AddDynamic(this, &UModalWindowManager::ForceCancelModalFlow);
}

void UModalWindowManager::ForceCancelModalFlow()
{
	PendingOriginalResult = FModalResult();
	PendingOriginalAction = FModalActionConfig();
	CurrentStepActionsMap.Empty();
	CachedContextObject = nullptr;

	if (ModalLayoutRef)
	{
		ModalLayoutRef->ClearStack();
	}

	if (FinalDelegate.IsBound())
	{
		FModalResult CancelResult;
		CancelResult.ResultInteractionType = EObjectInteractionType::None;
		FinalDelegate.Execute(CancelResult);
	}
	
	FinalDelegate.Unbind();
}

void UModalWindowManager::OpenModalFlow(UObject* InObject, FModalHeaderData HeaderData, EModalFooterType FooterType,
                                        const TMap<EObjectInteractionType, FModalActionConfig>& Actions, FModalResultDelegate OnResult)
{
	CachedContextObject = InObject;
	
	FinalDelegate = OnResult;
	PendingOriginalResult = FModalResult();

	ShowModalStep(HeaderData, FooterType, Actions);
}

void UModalWindowManager::ShowModalStep(FModalHeaderData HeaderData, EModalFooterType FooterType,
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
	
	if (HeaderData.HeaderType != EModalHeaderType::None)
	{
		const TSubclassOf<UUserWidget>* HeaderClassPtr = InvenzaInventorySettingsAsset->ModalHeaderWidgets.Find(HeaderData.HeaderType);
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
	ModalDialogCasted->Configure(HeaderData, ActionKeys, AvActions);

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
			ModalLayoutRef->ClearStack();
			
			if (Interaction == EObjectInteractionType::Yes)
			{
				FModalResult FinalResult = OriginalResult; 
				FinalResult.HeaderResult = Result.HeaderResult;

				FinalDelegate.Execute(FinalResult);
			}
			else if (FinalDelegate.IsBound())
			{
				FModalResult CancelResult;
				CancelResult.ResultInteractionType = EObjectInteractionType::None;
				FinalDelegate.Execute(CancelResult);
			}
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
			
			FModalHeaderData HeaderData;
			HeaderData.HeaderType = EModalHeaderType::SimpleText;
			HeaderData.Title = ActionData->HeaderText;

			ShowModalStep(HeaderData, EModalFooterType::Binary, DummyActions);
			return;
		}

	case EModalStepRequirement::RequiresAmount:
		{
			if (!CachedContextObject)
				return;
			
			FModalHeaderData HeaderData;
			
			if (CachedContextObject->GetClass()->ImplementsInterface(UObjectDataProvider::StaticClass()))
			{
				FVector2D MinMax = IObjectDataProvider::Execute_GetMinMaxSplit(CachedContextObject);
				HeaderData.MinValue = MinMax[0];
				HeaderData.MaxValue = MinMax[1];
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[%hs] CachedContextObject '%s' does not implement IObjectDataProvider"),
					__FUNCTION__, *GetNameSafe(CachedContextObject));
				return;
			}
			
			PendingOriginalResult = Result;
			PendingOriginalAction = *ActionData;
			
			TMap<EObjectInteractionType, FModalActionConfig> DummyActions;
			DummyActions.Add(EObjectInteractionType::Yes, FModalActionConfig());
			DummyActions.Add(EObjectInteractionType::No, FModalActionConfig());
			
			
			HeaderData.HeaderType = EModalHeaderType::TextWithAmountSelection;
			HeaderData.Title = ActionData->HeaderText;
			
			ShowModalStep(HeaderData, EModalFooterType::Binary, DummyActions);
			
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