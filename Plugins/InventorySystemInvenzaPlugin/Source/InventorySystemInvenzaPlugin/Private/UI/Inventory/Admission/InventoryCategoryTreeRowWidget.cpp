// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/Admission/InventoryCategoryTreeRowWidget.h"
#include "UI/Inventory/Admission/InventoryCategoryTreeEntry.h"
#include "UI/Inventory/Admission/InventoryAdmissionWidget.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Engine/Texture2D.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/TreeView.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"

void UInventoryCategoryTreeRowWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StateButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::StateClicked);
	ExpandButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::ExpandClicked);
	RefreshRow();
}

void UInventoryCategoryTreeRowWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ReleaseEntry();
	Entry = Cast<UInventoryCategoryTreeEntry>(ListItemObject);
	if (Entry)
	{
		Entry->OnChanged.AddUObject(this, &ThisClass::RefreshRow);
	}
	RefreshRow();
}

void UInventoryCategoryTreeRowWidget::ReleaseEntry()
{
	if (Entry)
	{
		Entry->OnChanged.RemoveAll(this);
	}
	Entry = nullptr;
}

void UInventoryCategoryTreeRowWidget::NativeOnEntryReleased()
{
	ReleaseEntry();
	IUserObjectListEntry::NativeOnEntryReleased();
}

void UInventoryCategoryTreeRowWidget::NativeOnItemExpansionChanged(bool bIsExpanded)
{
	IUserObjectListEntry::NativeOnItemExpansionChanged(bIsExpanded);
	RefreshRow();
}

void UInventoryCategoryTreeRowWidget::NativeDestruct()
{
	ReleaseEntry();
	Super::NativeDestruct();
}

void UInventoryCategoryTreeRowWidget::RefreshRow()
{
	if (!Entry || !Entry->Window || !StateButton || !StateIcon || !CategoryLabel || !ExpandButton || !Indentation)
	{
		return;
	}
	Indentation->SetWidthOverride(Entry->Depth * IndentWidth);
	ExpandButton->SetIsEnabled(!Entry->Children.IsEmpty());
	if (ExpandButton->MainImage)
	{
		if (Entry->ItemIcon)
		{
			ExpandButton->MainImage->SetBrushFromTexture(Entry->ItemIcon);
			ExpandButton->MainImage->SetRenderTransformAngle(0.f);
		}
		else
		{
			ExpandButton->MainImage->SetBrush(ResolveThemeBrush(TEXT("CategoryExpansionBrush"), CategoryExpansionBrush));
			ExpandButton->MainImage->SetRenderTransformAngle(IsListItemExpanded() ? 0.f : -90.f);
		}
	}
	CategoryLabel->UpdateText(Entry->Label);
	CategoryLabel->SetToolTipText(FText::FromString(Entry->ItemDefinition.IsNull() ? Entry->Category.ToString() :
		Entry->ItemDefinition.DataTable->GetPathName() + TEXT(":") + Entry->ItemDefinition.RowName.ToString()));
	const bool bEditable = Entry->Window->CanEdit();
	FName TooltipKey;
	switch (Entry->Window->GetNodeSelection(Entry))
	{
	case EInventoryAdmissionSelection::Allowed:
		StateIcon->SetBrush(ResolveThemeBrush(TEXT("AllowedStateBrush"), AllowedStateBrush));
		TooltipKey = bEditable ? TEXT("Invenza.Admission.AllowedAction") : TEXT("Invenza.Admission.Allowed");
		break;
	case EInventoryAdmissionSelection::Blocked:
		StateIcon->SetBrush(ResolveThemeBrush(TEXT("BlockedStateBrush"), BlockedStateBrush));
		TooltipKey = bEditable ? TEXT("Invenza.Admission.BlockedAction") : TEXT("Invenza.Admission.Blocked");
		break;
	case EInventoryAdmissionSelection::Partial:
		StateIcon->SetBrush(ResolveThemeBrush(TEXT("PartialStateBrush"), PartialStateBrush));
		TooltipKey = bEditable ? TEXT("Invenza.Admission.PartialAction") : TEXT("Invenza.Admission.Mixed");
		break;
	}
	StateButton->SetToolTipText(GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(TooltipKey));
}

void UInventoryCategoryTreeRowWidget::ExpandClicked(UUIButton* Button)
{
	if (Entry && !Entry->Children.IsEmpty())
	{
		if (auto* Tree = Cast<UTreeView>(GetOwningListView()))
		{
			Tree->SetItemExpansion(Entry, !IsListItemExpanded());
		}
	}
}

void UInventoryCategoryTreeRowWidget::StateClicked(UUIButton* Button)
{
	if (Entry && Entry->Window)
	{
		Entry->Window->ToggleNode(Entry);
	}
}
