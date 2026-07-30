// Nublin Studio 2026 All Rights Reserved.


#include "Subsystems/ModalWindowManager.h"

#include "Components/NamedSlot.h"
#include "Data/Settings/InvenzaInventorySettingsAsset.h"
#include "HUD/GameHUD_Inz.h"
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

	PrimaryLayoutRef = Cast<AGameHUD_Inz>(PC->GetHUD())->GetPrimaryLayout();
	if (!PrimaryLayoutRef)
	{
		return;
	}
	
	ModalLayoutRef = Cast<UModalLayout>(PrimaryLayoutRef->ModalLayer);
}

void UModalWindowManager::OpenModalFlow(EModalHeaderType HeaderType, EModalFooterType FooterType,
                                        const TArray<EObjectInteractionType>& Actions, FModalResultDelegate OnResult)
{
	FinalDelegate = OnResult;
	PendingOriginalResult = FModalResult();

	ShowModalStep(HeaderType, FooterType, Actions);
}

void UModalWindowManager::ShowModalStep(EModalHeaderType HeaderType, EModalFooterType FooterType,
	const TArray<EObjectInteractionType>& Actions)
{
	UWorld* World = GetWorld();
	if (!World || !InvenzaInventorySettingsAsset) return;
	
	if (!ModalLayoutRef)
	{
		return;
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

	TArray<FModalAction> AvActions;
	for (auto Action : Actions)
	{
		AvActions.Add(InvenzaInventorySettingsAsset->ModalActions.FindRef(Action));
	}
	ModalDialogCasted->Configure(Actions, AvActions);
	
	
	//ModalDialog->AddToViewport(); // в исходнике этого шага не было — без него окно просто не покажется
}

void UModalWindowManager::HandleModalResponse(FModalResult Result)
{
	const EObjectInteractionType Interaction = Result.ResultInteractionType;

	if (Interaction == EObjectInteractionType::None || Interaction == EObjectInteractionType::Cancel)
	{
		PendingOriginalResult = FModalResult();
		if (FinalDelegate.IsBound())
		{
			FModalResult CancelResult;
			CancelResult.ResultInteractionType = EObjectInteractionType::None;
			FinalDelegate.Execute(CancelResult);
		}
		return;
	}
	
	if (PendingOriginalResult.StepRequirement != EModalStepRequirement::None)
	{
		FModalResult OriginalAction = PendingOriginalResult;
		PendingOriginalResult = FModalResult();

		if (OriginalAction.StepRequirement == EModalStepRequirement::RequiresConfirm)
		{
			if (Interaction == EObjectInteractionType::Yes)
			{
				OriginalAction.StepRequirement = EModalStepRequirement::None;
				FinalDelegate.Execute(OriginalAction);
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

	// Обычный первый шаг
	switch (Result.StepRequirement)
	{
	case EModalStepRequirement::RequiresConfirm:
		{
			PendingOriginalResult = Result;

			TArray<EObjectInteractionType> DummyActions;
			DummyActions.Add(EObjectInteractionType::Yes);
			DummyActions.Add(EObjectInteractionType::No);

			ShowModalStep(EModalHeaderType::SimpleText, EModalFooterType::Binary, DummyActions);
			return;
		}

	case EModalStepRequirement::RequiresAmount:
		{
			PendingOriginalResult = Result;
			// TODO: открыть AmountInput footer, когда он появится
			return;
		}

	case EModalStepRequirement::None:
	default:
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