//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "InvenzaInventorySettingsAsset.generated.h"


struct FBlockReasonData;
class UModalDialogBase;
class USimpleUserObjectListEntry;
class USimpleUserObjectList;
class UInvenzaBaseWidget;
class UDragContainerWidget;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaInventorySettingsAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	/* Filters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filters|Config", meta = (ToolTip = "Enable filter color override"))
	bool bUseFilterColor = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filters|Config", meta = (ToolTip = "Only for Grid inventory", EditCondition = "bUseFilterColor"))
	FLinearColor ItemFilterBorderColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filters|Config", meta=(ToolTip="Only for Grid inventory"))
	float FilterOpacity = 0.15f;
	
	// Inventory
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag MainInvTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag EquipmentInvTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<AActor> PickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FDataTableRowHandle CurrencyItemClass;
 
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag CurrencyGameplayTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag ConsumableGameplayTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag AnyCategoryGameplayTag;
	
	//Inv Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Widgets")
	TSubclassOf<UDragContainerWidget> DragContainerWidgetClass;
	
	// Craft System
	// Config
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Config")
	FGameplayTag InputInvTagByDefault;
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Config")
	FGameplayTag OutputInvTagByDefault;
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Config")
	FGameplayTag FuelInvTagByDefault;
	
	//
	UPROPERTY(EditDefaultsOnly, Category="Crafting")
	TArray<FBlockReasonData> AvailableBlockReasons;
	
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FGameplayTag Block_NoResources;
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FGameplayTag Block_NoOperator;
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FGameplayTag Block_NoFuel;
	
	const FBlockReasonData* FindBlockReason(const FGameplayTag& Tag) const;
	
	// Modal
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TMap<EObjectInteractionType, FModalActionConfig> InvItemsModalActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TMap<EObjectInteractionType, FModalActionConfig> InvContextModalActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TMap<EObjectInteractionType, FModalAction> ModalActions;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ResultModal_Yes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ResultModal_No;
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag DefaultModalCancelTag;
	
	// Modal Widgets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TSubclassOf<UModalDialogBase> DefaultModalDialogClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TSubclassOf<UInvenzaBaseWidget> DefaultModalBtnClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TMap<EModalHeaderType, TSubclassOf<UUserWidget>> ModalHeaderWidgets;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TMap<EModalFooterType, TSubclassOf<UUserWidget>> ModalFooterWidgets;
};
