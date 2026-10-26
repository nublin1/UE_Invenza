//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "Styling/SlateBrush.h"
#include "InvenzaWidgetEditingLibrary.generated.h"

class UWidgetBlueprint;
class UWidget;
class UUserWidget;
class UStringTable;
class UButton;

/** Editor-only access to UMG template operations that are not exposed to Python. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaWidgetEditingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Functions
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
