//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Data/Interactable/InteractableData.h"
#include "Templates/SubclassOf.h"
#include "Styling/SlateBrush.h"
#include "InvenzaWidgetEditingLibrary.generated.h"

class UWidgetBlueprint;
class UBlueprint;
class UActorComponent;
class APawn;
class UPickupComponent;
class UIInventoryManager;
class UInventoryBase;
class UWidget;
class UUserWidget;
class UStringTable;
class UButton;
class UPanelWidget;
class UPanelSlot;
class UInvenzaSaveSubsystem;
class UEquipmentComponent;

/** Editor-only access to UMG template operations that are not exposed to Python. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaWidgetEditingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static AActor* SpawnPreviewActor(UWorld* World, TSubclassOf<AActor> ActorClass, const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Blueprint Editing")
	static bool CompileActorBlueprint(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool PreviewPickupInput(APawn* Pawn, UPickupComponent* Pickup, bool bPressed,
		EInteractionType Type = EInteractionType::Primary);

	/** Exercise the equipped bag's normal drop request, including client RPC callspace. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool DropPreviewEquippedBackpack(APawn* Pawn);

	/** Invoke the handler bound to the player's inventory input. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool TogglePreviewInventory(APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Blueprint Editing")
	static UActorComponent* AddBlueprintComponent(UBlueprint* Blueprint, TSubclassOf<UActorComponent> ComponentClass, FName Name);

	UFUNCTION(BlueprintPure, Category = "Invenza|Validation")
	static UUserWidget* GetPreviewInventoryUI(UIInventoryManager* Manager);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool SetEquipmentDefinitionRows(UEquipmentComponent* Equipment, const TArray<FDataTableRowHandle>& Rows);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static UPanelSlot* InsertTemplateChild(UPanelWidget* Panel, int32 Index, UWidget* Child);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static bool StartStandalonePreview();

	UFUNCTION(BlueprintPure, Category = "Invenza|Validation")
	static UInvenzaSaveSubsystem* GetPreviewSaveSubsystem(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Validation")
	static UInventoryBase* AddPreviewPlayerStorage(UIInventoryManager* Manager);
	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static UUserWidget* CreatePreview(UWorld* World, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static void ReleasePreview();

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static UWidget* AddTemplate(UWidgetBlueprint* Blueprint, TSubclassOf<UWidget> WidgetClass, FName Name);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static void SetRoot(UWidgetBlueprint* Blueprint, UWidget* Root);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static void SetSlotContent(UUserWidget* Widget, FName SlotName, UWidget* Content);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool CompileWidget(UWidgetBlueprint* Blueprint);

	/** Remove only explicitly selected legacy graphs before adopting native layout behavior. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool PrepareNativeLayoutBlueprint(UBlueprint* Blueprint, const TArray<FName>& FunctionNames,
		const TArray<FName>& VariableNames, bool bPruneEventGraph, const TArray<FName>& EventNodesToKeep);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool RenderWidget(UUserWidget* Widget, FVector2D Size, const FString& Filename);

	/** Read arranged bounds in native code, including geometry fields not reflected to Python. */
	UFUNCTION(BlueprintPure, Category = "Invenza|Widget Editing")
	static FVector4 GetPreviewWidgetRect(UWidget* Widget, UWidget* RelativeTo);

	/** Dispatch a transient drag-over to an arranged PIE widget without moving any item. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool PreviewDragOver(UUserWidget* SourceWidget, UWidget* SourceCell, UUserWidget* TargetWidget,
								UWidget* HoverTarget, FVector2D NormalizedPosition);

	/** Complete the retained drag in PIE through the target's existing drop handler. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool CompletePreviewDrop(UUserWidget* TargetWidget, UWidget* TargetCell);

	/** Dispatch a preview press in PIE without Python's editor-only, local-RPC callspace. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Widget Editing")
	static bool PressPreviewButton(UButton* Button);

	UFUNCTION(BlueprintCallable, Category = "Invenza|Localization Editing")
	static void SetStringTableEntry(UStringTable* Table, const FString& Key, const FString& SourceText);

	UFUNCTION(BlueprintPure, Category = "Invenza|Localization Editing")
	static FText StringTableText(UStringTable* Table, const FString& Key);

	UFUNCTION(BlueprintPure, Category = "Invenza|Widget Editing")
	static FSlateBrush GetCoreStyleBrush(FName BrushName);
};
