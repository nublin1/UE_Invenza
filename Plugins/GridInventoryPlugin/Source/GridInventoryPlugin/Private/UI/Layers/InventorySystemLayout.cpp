// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Layers/InventorySystemLayout.h"

#include "Components/CanvasPanel.h"
#include "UI/Inventory/BaseInventoryWidget.h"

UInventorySystemLayout::UInventorySystemLayout()
{
}

void UInventorySystemLayout::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UInventorySystemLayout::Init);

}

void UInventorySystemLayout::Init()
{
	if (!ContentPanel)
		return;

	if (ContentPanel->GetChildrenCount() == 0)
		return;

	TArray<TObjectPtr<UBaseInventoryWidget>> InventoryWidgets_temp;

	for (int i = 0; i < ContentPanel->GetChildrenCount(); i++)
	{
		if (UWidget* ChildWidget = ContentPanel->GetChildAt(i))
		{
			auto WClass = ChildWidget->GetClass();
			if (WClass->IsChildOf(UBaseInventoryWidget::StaticClass()))
			{
				auto InventoryWidget = Cast<UBaseInventoryWidget>(ChildWidget);
				InventoryWidgets_temp.Add(InventoryWidget);
			}
		}
	}

	InventoryWidgets = InventoryWidgets_temp;
	if (!InventoryWidgets_temp.IsEmpty())
	{
		MainInventory = InventoryWidgets[0];
		MainInventory->SetUISettings(UISettings);
	}
}
