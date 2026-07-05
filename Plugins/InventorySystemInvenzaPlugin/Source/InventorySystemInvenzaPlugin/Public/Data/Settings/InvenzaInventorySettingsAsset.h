//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActorComponents/Crafting/CraftingStructs.h"
#include "UI/Core/Modal/ModalTypes.h"
#include "InvenzaInventorySettingsAsset.generated.h"


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
	TSubclassOf<AActor> PickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FDataTableRowHandle CurrencyItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	FGameplayTag CurrencyGameplayTag;

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
	
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modal|Config")
	TArray<FObjectModalActionRule> InvItemsModalActionRules;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	TMap<FGameplayTag, TObjectPtr<UModalAction>> AvailableObjectActions;

	UFUNCTION(BlueprintCallable)
	UModalAction* GetObjectAction(FGameplayTag FindTag) const;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ResultModal_Yes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ResultModal_No;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modal")
	FGameplayTag ResultModalCancel;*/
};
