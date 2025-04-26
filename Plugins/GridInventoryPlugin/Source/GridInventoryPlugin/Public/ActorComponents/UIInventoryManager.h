//  Nublin Studio 2025 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Settings/Settings.h"
#include "UI/Container/InvBaseContainerWidget.h"
#include "UI/Core/CoreHUDWidget.h"
#include "UI/Inventory/InventoryTypes.h"
#include "UIInventoryManager.generated.h"

class FIteract;
enum class EInteractableType : uint8;
class UItemCollection;
struct FItemMoveData;
struct FInputActionInstance;
class UInputAction;
class UInteractableComponent;
class UCoreHUDWidget;

#pragma region Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOninitializationComplete, bool, Status);
#pragma endregion Delegates

UCLASS(ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYPLUGIN_API UIInventoryManager : public UActorComponent
{
	GENERATED_BODY()

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOninitializationComplete OninitializationCompleteDelegate;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UIInventoryManager();

	UFUNCTION(BlueprintCallable)
	void OnQuickTransferItem(FItemMoveData ItemMoveData);
	UFUNCTION(BlueprintCallable)
	void ItemTransferRequest(FItemMoveData ItemMoveData);

	UFUNCTION()
	UInvBaseContainerWidget* GetMainInventory() {return CoreHUDWidget->GetMainInvWidget();}

	FUISettings GetUISettings() {return UISettings;}
	UCoreHUDWidget* GetCoreHUDWidget() {return CoreHUDWidget;}
	FInventoryModifierState GetInventoryModifierStates() const {return InventoryModifierState;}

protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI)
	UCoreHUDWidget* CoreHUDWidget;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInvBaseContainerWidget> CurrentInteractInvWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FUISettings UISettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FInventoryModifierState InventoryModifierState;

	//
	UPROPERTY(EditAnywhere, Category = "Item Initialization")
	TObjectPtr<UDataTable>ItemDataTable;

	//
	UPROPERTY()
	bool bIsShowingInventoryMenu = false;
	
	//====================================================================
	// FUNCTIONS
	//====================================================================
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void VendorRequest(FItemMoveData ItemMoveData);

	UFUNCTION(BlueprintCallable)
	void SetInteractableType(UInteractableComponent* IteractData);
	UFUNCTION(BlueprintCallable)
	void ClearInteractableType(UInteractableComponent* IteractData = nullptr);

	UFUNCTION(BlueprintCallable)
	void BindEvents(AActor* TargetActor);
	UFUNCTION(BlueprintCallable)
	void UIIteract(UInteractableComponent* TargetInteractableComponent);
	UFUNCTION()
	void OnQuickGrabPressed(const FInputActionInstance& Instance);
	UFUNCTION()
	void OnQuickGrabReleased(const FInputActionInstance& Instance);
	UFUNCTION(BlueprintCallable)
	void InitializeBindings();
	UFUNCTION()
	void InitializeInvSlotsBindings();

	UFUNCTION(BlueprintCallable)
	void ToggleInventoryLayout();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction);
};
