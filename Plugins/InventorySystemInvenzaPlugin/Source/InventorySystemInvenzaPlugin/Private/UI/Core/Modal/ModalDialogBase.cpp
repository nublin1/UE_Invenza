// Nublin Studio 2026 All Rights Reserved.


#include "UI/Core/Modal/ModalDialogBase.h"

#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Interface/UIInterface.h"
#include "Interface/UI/ModalInterface.h"
#include "UI/Core/Buttons/UIButton.h"
#include "Utility/InterfaceUtils.h"

void UModalDialogBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UModalDialogBase::ForceClose(FModalResult Result)
{
	DynamicResultDelegate.Execute(Result);
}

void UModalDialogBase::Configure(FModalHeaderData HeaderData, const TArray<EObjectInteractionType>& Actions, const TArray<FModalAction>& Display)
{
	ConfigureHeader(HeaderData);
	ConfigureFooter(Actions, Display);
}

void UModalDialogBase::ConfigureHeader(FModalHeaderData HeaderData)
{
	UWidget* Content = Upper_Slot->GetContent();
	if (!Content)
	{
		UE_LOG(LogTemp, Warning, TEXT("UModalDialogBase::Configure — Upper_Slot has no content."));
		Upper_Slot->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	if (Content->GetClass()->ImplementsInterface(UModalInterface::StaticClass()))
	{
		IModalInterface::Execute_ConfigureHeader(Content,HeaderData);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UModalDialogBase::Configure — Widget '%s' does NOT implement UModalButtonsPanelInterface."),
			*Content->GetClass()->GetName()
		);
		return;
	}
}

void UModalDialogBase::ConfigureFooter(const TArray<EObjectInteractionType>& Actions,
                                       const TArray<FModalAction>& Display)
{
	ButtonToResultMap.Empty();

	UWidget* Content = Down_Slot->GetContent();
	if (!Content)
	{
		UE_LOG(LogTemp, Warning, TEXT("UModalDialogBase::Configure — Down_Slot has no content."));
		return;
	}

	TArray<UUIButton*> Buttons;
	if (Content->GetClass()->ImplementsInterface(UModalInterface::StaticClass()))
	{
		IModalInterface::Execute_ConfigureButtons(Content, Actions, Display);
		Buttons = IUIInterface::Execute_GetButtons(Content);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UModalDialogBase::Configure — Widget '%s' does NOT implement UModalButtonsPanelInterface."),
			*Content->GetClass()->GetName()
		);
		return;
	}
	if (Buttons.IsEmpty()) return;

	ConfigureButtons(Buttons, Actions, Display);
}

void UModalDialogBase::ConfigureButtons(TArray<UUIButton*> InBtns, const TArray<EObjectInteractionType>& Actions,
                                        const TArray<FModalAction>& Display)
{
	for (auto Btn : InBtns)
	{
		Btn->OnButtonClicked.AddDynamic(this, &UModalDialogBase::OnButtonClicked);
	}

	if (Actions.Num() > InBtns.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Configure: Not enough buttons. Actions: %d, Buttons: %d"), Actions.Num(), InBtns.Num());
	}

	const int32 Count = FMath::Min(InBtns.Num(), Actions.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		FModalResult Result;
		Result.ResultInteractionType = Actions[i];
		ButtonToResultMap.Add(InBtns[i], Result);
	}
}

void UModalDialogBase::OnButtonClicked(UUIButton* UIButton)
{
	const FModalResult* Found = ButtonToResultMap.Find(UIButton);
	if (!Found) return;

	FModalResult Result = *Found;

	UWidget* HeaderContent = Upper_Slot ? Upper_Slot->GetContent() : nullptr;
	if (HeaderContent &&
		HeaderContent->GetClass()->ImplementsInterface(UModalInterface::StaticClass()) &&
		UInterfaceUtils::IsFunctionOverridden(
			HeaderContent,
			TEXT("GetHeaderResult"),
			UModalInterface::StaticClass()))
	{
		Result.HeaderResult = IModalInterface::Execute_GetHeaderResult(HeaderContent);
	}

	DynamicResultDelegate.Execute(Result);
}
