//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActorComponents/Crafting/CraftingStructs.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "InvenzaInventorySettingsAsset.generated.h"


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
	UPROPERTY(EditDefaultsOnly, Category="Crafting")
	TArray<FBlockReasonData> AvailableBlockReasons;
	
	UPROPERTY(EditDefaultsOnly, Category="Crafting|Blocks")
	FGameplayTag Block_NoResources;
	
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
