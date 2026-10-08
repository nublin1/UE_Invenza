#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvenzaWidgetEditingLibrary.generated.h"

class UWidgetBlueprint;
class UWidget;
class UUserWidget;

/** Editor-only access to UMG template operations that are not exposed to Python. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaWidgetEditingLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
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
};
