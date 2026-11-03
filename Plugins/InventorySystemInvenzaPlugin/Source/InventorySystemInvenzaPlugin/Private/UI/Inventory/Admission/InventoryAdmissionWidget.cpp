// Nublin Studio 2026 All Rights Reserved.

#include "UI/Inventory/Admission/InventoryAdmissionWidget.h"
#include "UI/Inventory/Admission/InventoryCategoryTreeEntry.h"
#include "ActorComponents/UIInventoryManager.h"
#include "Components/EditableTextBox.h"
#include "Components/TreeView.h"
#include "Data/Inventory/InventoryBase.h"
#include "Data/ItemData.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagsManager.h"
#include "Settings/InvenzaInventoryDeveloperSettings.h"
#include "Subsystems/ModalWindowManager.h"
#include "Subsystems/UIInputModeSubsystem.h"
#include "UI/Core/Buttons/UIButton.h"
#include "UI/Core/LabelBaseText.h"

static FText GetAdmissionText(FName Key)
{
	return GetDefault<UInvenzaInventoryDeveloperSettings>()->GetText(Key);
}

void UInventoryAdmissionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CategoryTree->SetOnGetItemChildren(this, &ThisClass::GetChildren);
	SearchInput->OnTextChanged.AddUniqueDynamic(this, &ThisClass::SearchChanged);
	ApplyButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::ApplyClicked);
	CloseButton->OnButtonClicked.AddUniqueDynamic(this, &ThisClass::CloseClicked);
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->RequestUIInput(this);
		}
	}
}

void UInventoryAdmissionWidget::ReleaseInput()
{
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Input = Local->GetSubsystem<UUIInputModeSubsystem>())
		{
			Input->ReleaseUIInput(this);
		}
	}
}

void UInventoryAdmissionWidget::NativeDestruct()
{
	ReleaseInput();
	if (InventoryRef)
	{
		InventoryRef->OnAdmissionRulesChanged.RemoveDynamic(this, &ThisClass::RulesChanged);
	}
	Super::NativeDestruct();
}

UIInventoryManager* UInventoryAdmissionWidget::GetManager() const
{
	const auto* Pawn = GetOwningPlayerPawn();
	return Pawn ? Pawn->FindComponentByClass<UIInventoryManager>() : nullptr;
}

bool UInventoryAdmissionWidget::CanEdit() const
{
	const auto* Manager = GetManager();
	return Manager && !PendingRequest.IsValid() && Manager->CanEditAdmissionRules(InventoryRef);
}

void UInventoryAdmissionWidget::SetInventory(UInventoryBase* Inventory)
{
	if (InventoryRef)
	{
		InventoryRef->OnAdmissionRulesChanged.RemoveDynamic(this, &ThisClass::RulesChanged);
	}
	InventoryRef = Inventory;
	PendingRequest.Invalidate();
	Search.Empty();
	SearchInput->SetText(FText::GetEmpty());
	if (InventoryRef)
	{
		InventoryRef->OnAdmissionRulesChanged.AddUniqueDynamic(this, &ThisClass::RulesChanged);
		DraftRules = InventoryRef->GetAdmissionRules();
		TitleLabel->UpdateText(FText::Format(GetAdmissionText(TEXT("Invenza.Admission.Title")),
			InventoryRef->GetInventorySettings().ContainerTitle));
	}
	bDirty = false;
	BuildTree();
	UpdateStatus();
}

void UInventoryAdmissionWidget::BuildTree()
{
	CategoryTree->ClearListItems();
	Nodes.Reset();
	const auto* Settings = GetDefault<UInvenzaInventoryDeveloperSettings>();
	const FGameplayTag Root = Settings->AdmissionCategoryRoot;
	if (!Root.IsValid())
	{
		return;
	}
	FGameplayTagContainer Tags = UGameplayTagsManager::Get().RequestGameplayTagChildren(Root);
	Tags.AddTag(Root);
	TArray<FGameplayTag> Sorted = Tags.GetGameplayTagArray();
	Sorted.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.ToString() < B.ToString();
	});
	TMap<FGameplayTag, UInventoryCategoryTreeEntry*> ByTag;
	for (FGameplayTag Tag : Sorted)
	{
		auto* Node = NewObject<UInventoryCategoryTreeEntry>(this);
		Node->Category = Tag;
		Node->Window = this;
		const FText* Label = Settings->AdmissionCategoryLabels.Find(Tag);
		Node->Label = Label ? *Label : (Tag == Root ? GetAdmissionText(TEXT("Invenza.Admission.AllItems")) :
			FText::FromName(Tag.GetTagLeafName()));
		Nodes.Add(Node);
		ByTag.Add(Tag, Node);
		if (auto* Parent = ByTag.FindRef(Tag.RequestDirectParent()))
		{
			Node->Depth = Parent->Depth + 1;
			Parent->Children.Add(Node);
		}
	}
	for (const auto& TableRef : Settings->AdmissionItemTables)
	{
		UDataTable* Table = TableRef.LoadSynchronous();
		if (!Table || Table->GetRowStruct() != FItemData::StaticStruct())
		{
			continue;
		}
		TArray<FName> Names = Table->GetRowNames();
		Names.Sort(FNameLexicalLess());
		for (FName Name : Names)
		{
			const FItemData* Data = Table->FindRow<FItemData>(Name, TEXT("Admission catalog"));
			auto* Parent = Data ? ByTag.FindRef(Data->ItemMetaData.ItemCategory) : nullptr;
			if (!Parent)
			{
				continue;
			}
			auto* Node = NewObject<UInventoryCategoryTreeEntry>(this);
			Node->Window = this;
			Node->Category = Data->ItemMetaData.ItemCategory;
			Node->Label = Data->ItemMetaData.ItemTextData.DisplayName;
			Node->ItemDefinition.DataTable = Table;
			Node->ItemDefinition.RowName = Name;
			Node->ItemIcon = Data->ItemMetaData.ItemAssetData.Icon;
			Node->Depth = Parent->Depth + 1;
			Parent->Children.Add(Node);
			Nodes.Add(Node);
		}
	}
	SyncSelectionFromRules();
	RefreshTree();
}

void UInventoryAdmissionWidget::GetChildren(UObject* Item, TArray<UObject*>& Children)
{
	if (const auto* Node = Cast<UInventoryCategoryTreeEntry>(Item))
	{
		for (UInventoryCategoryTreeEntry* Child : Node->Children)
		{
			if (Child->bMatchesSearch)
			{
				Children.Add(Child);
			}
		}
	}
}

void UInventoryAdmissionWidget::RefreshTree()
{
	for (int32 Index = Nodes.Num() - 1; Index >= 0; --Index)
	{
		UInventoryCategoryTreeEntry* Node = Nodes[Index];
		Node->bMatchesSearch = Search.IsEmpty() || Node->Label.ToString().Contains(Search) ||
			Node->Category.ToString().Contains(Search);
		for (UInventoryCategoryTreeEntry* Child : Node->Children)
		{
			Node->bMatchesSearch |= Child->bMatchesSearch;
		}
		Node->OnChanged.Broadcast();
	}
	TArray<UObject*> Roots;
	if (!Nodes.IsEmpty() && Nodes[0]->bMatchesSearch)
	{
		Roots.Add(Nodes[0]);
	}
	// TreeView caches the expanded hierarchy separately from its root list.
	CategoryTree->CollapseAll();
	CategoryTree->SetListItems(Roots);
	for (UInventoryCategoryTreeEntry* Node : Nodes)
	{
		if (Node->bMatchesSearch)
		{
			CategoryTree->SetItemExpansion(Node, true);
		}
	}
}

EInventoryAdmissionSelection UInventoryAdmissionWidget::GetNodeSelection(const UInventoryCategoryTreeEntry* Node) const
{
	return Node ? Node->Selection : EInventoryAdmissionSelection::Blocked;
}

void UInventoryAdmissionWidget::SyncSelectionFromRules()
{
	for (UInventoryCategoryTreeEntry* Node : Nodes)
	{
		if (Node->Children.IsEmpty())
		{
			const bool bAllowed = Node->ItemDefinition.IsNull() ? DraftRules.Allows(Node->Category) :
				DraftRules.AllowsItem(Node->Category, Node->ItemDefinition);
			Node->Selection = bAllowed ? EInventoryAdmissionSelection::Allowed : EInventoryAdmissionSelection::Blocked;
		}
	}
	UpdateSelectionStates();
}

void UInventoryAdmissionWidget::UpdateSelectionStates()
{
	// Parents summarize their children, including individual item leaves.
	for (int32 Index = Nodes.Num() - 1; Index >= 0; --Index)
	{
		UInventoryCategoryTreeEntry* Node = Nodes[Index];
		if (Node->Children.IsEmpty())
		{
			continue;
		}
		bool bAllAllowed = true;
		bool bAllBlocked = true;
		for (const UInventoryCategoryTreeEntry* Child : Node->Children)
		{
			bAllAllowed &= Child->Selection == EInventoryAdmissionSelection::Allowed;
			bAllBlocked &= Child->Selection == EInventoryAdmissionSelection::Blocked;
		}
		Node->Selection = bAllAllowed ? EInventoryAdmissionSelection::Allowed :
			(bAllBlocked ? EInventoryAdmissionSelection::Blocked : EInventoryAdmissionSelection::Partial);
	}
}

static void AppendAdmissionSelectedBranch(const UInventoryCategoryTreeEntry* Node, FInventoryAdmissionRules& Rules)
{
	if (Node->Selection == EInventoryAdmissionSelection::Allowed)
	{
		if (Node->ItemDefinition.IsNull())
		{
			Rules.AllowedCategories.AddTag(Node->Category);
		}
		else
		{
			Rules.AllowedItems.AddUnique(Node->ItemDefinition);
		}
	}
	else if (Node->Selection == EInventoryAdmissionSelection::Partial)
	{
		for (const UInventoryCategoryTreeEntry* Child : Node->Children)
		{
			AppendAdmissionSelectedBranch(Child, Rules);
		}
	}
}

void UInventoryAdmissionWidget::RebuildDraftRules()
{
	if (Nodes.IsEmpty())
	{
		return;
	}
	DraftRules = {};
	const UInventoryCategoryTreeEntry* Root = Nodes[0];
	switch (Root->Selection)
	{
	case EInventoryAdmissionSelection::Allowed:
		// Empty allowances retain the project's established "accept everything" behavior.
		break;
	case EInventoryAdmissionSelection::Blocked:
		// Keep an explicit reject-all rule; an empty allowance list alone means accept all.
		DraftRules.ExcludedCategories.AddTag(Root->Category);
		break;
	case EInventoryAdmissionSelection::Partial:
		AppendAdmissionSelectedBranch(Root, DraftRules);
		break;
	}
}

void UInventoryAdmissionWidget::SetNodeAllowed(UInventoryCategoryTreeEntry* Node, bool bAllowed)
{
	if (!CanEdit() || !IsValid(Node) || !Nodes.Contains(Node))
	{
		return;
	}
	for (UInventoryCategoryTreeEntry* Leaf : Nodes)
	{
		if (Leaf->Children.IsEmpty() && (Leaf == Node ||
			(Node->ItemDefinition.IsNull() && Leaf->Category.MatchesTag(Node->Category))))
		{
			Leaf->Selection = bAllowed ? EInventoryAdmissionSelection::Allowed : EInventoryAdmissionSelection::Blocked;
		}
	}
	UpdateSelectionStates();
	RebuildDraftRules();
	bDirty = !(DraftRules == InventoryRef->GetAdmissionRules());
	for (UInventoryCategoryTreeEntry* Entry : Nodes)
	{
		Entry->OnChanged.Broadcast();
	}
	UpdateStatus();
}

void UInventoryAdmissionWidget::ToggleNode(UInventoryCategoryTreeEntry* Node)
{
	SetNodeAllowed(Node, GetNodeSelection(Node) != EInventoryAdmissionSelection::Allowed);
}

void UInventoryAdmissionWidget::UpdateStatus(FName Message)
{
	ApplyButton->SetIsEnabled(CanEdit() && bDirty);
	const FName DefaultKey = PendingRequest.IsValid() ? TEXT("Invenza.Admission.Submitting") :
		(CanEdit() ? TEXT("Invenza.Admission.Help") : TEXT("Invenza.Admission.ReadOnly"));
	StatusLabel->UpdateText(GetAdmissionText(Message.IsNone() ? DefaultKey : Message));
}

void UInventoryAdmissionWidget::RulesChanged()
{
	if (!bDirty && InventoryRef)
	{
		DraftRules = InventoryRef->GetAdmissionRules();
		SyncSelectionFromRules();
	}
	for (UInventoryCategoryTreeEntry* Node : Nodes)
	{
		Node->OnChanged.Broadcast();
	}
	UpdateStatus();
}

void UInventoryAdmissionWidget::SearchChanged(const FText& Text)
{
	Search = Text.ToString().TrimStartAndEnd();
	RefreshTree();
}

void UInventoryAdmissionWidget::ApplyClicked(UUIButton* Button)
{
	if (!CanEdit() || !bDirty)
	{
		return;
	}
	PendingRequest = FGuid::NewGuid();
	for (UInventoryCategoryTreeEntry* Node : Nodes)
	{
		Node->OnChanged.Broadcast();
	}
	UpdateStatus();
	GetManager()->Server_SetAdmissionRules(InventoryRef, DraftRules, PendingRequest);
}

void UInventoryAdmissionWidget::CompleteSubmission(FGuid RequestID, bool bSuccess)
{
	if (RequestID != PendingRequest)
	{
		return;
	}
	PendingRequest.Invalidate();
	if (bSuccess)
	{
		CloseClicked(nullptr);
		return;
	}
	for (UInventoryCategoryTreeEntry* Node : Nodes)
	{
		Node->OnChanged.Broadcast();
	}
	UpdateStatus(TEXT("Invenza.Admission.Rejected"));
}

void UInventoryAdmissionWidget::CloseClicked(UUIButton* Button)
{
	if (auto* Local = GetOwningLocalPlayer())
	{
		if (auto* Modals = Local->GetSubsystem<UModalWindowManager>())
		{
			Modals->CloseAdmissionRules();
		}
	}
}

FReply UInventoryAdmissionWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape)
	{
		CloseClicked(nullptr);
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(Geometry, Event);
}

FReply UInventoryAdmissionWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event)
{
	return FReply::Handled();
}
