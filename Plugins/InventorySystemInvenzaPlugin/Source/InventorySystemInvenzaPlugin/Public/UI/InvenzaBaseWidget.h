//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Theme/InvenzaUIThemeTypes.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"
#include "InvenzaBaseWidget.generated.h"

class UUILayer;

/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UInvenzaBaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Unreal overrides
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

public:
	// Properties and variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Theme")
	EInvenzaUIThemeSource ThemeSource = EInvenzaUIThemeSource::Local;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Theme")
	TArray<FInvenzaUIThemeBinding> ThemeBindings;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "UI|Theme")
	void ApplyUITheme();

	// Computed values never replace the editable authoring properties on this UserWidget.
	FInvenzaUIThemeValue ResolveThemeValue(const FString& Path, FInvenzaUIThemeValue Local, FName WidgetName = NAME_None) const;
	void CaptureLocalThemeValues();
#if WITH_EDITOR
	void RecordDesignerStyleEdit(UObject* Changed, const FPropertyChangedEvent& Event);
#endif
	template<typename T>
	T ResolveThemeStyle(const FString& Prefix, const T& Local) const
	{
		T Result = Local;
		UInvenzaUIThemeLibrary::ResolveStructStyle(this, Prefix, T::StaticStruct(), &Result);
		return Result;
	}
	FSlateFontInfo ResolveThemeFont(const FString& Path, const FSlateFontInfo& Local) const;
	FSlateColor ResolveThemeSlateColor(const FString& Path, const FSlateColor& Local) const;
	FSlateBrush ResolveThemeBrush(const FString& Path, const FSlateBrush& Local) const;
	FLinearColor ResolveThemeWidgetColor(FName WidgetName, const FString& Path, FLinearColor Local) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Theme")
	void OnUIThemeApplied();

	bool UsesUITheme() const { return ThemeSource != EInvenzaUIThemeSource::Local; }

	/** Resolve an explicit self binding when runtime data supplies a color fallback. */
	UFUNCTION(BlueprintPure, Category = "UI|Theme")
	FLinearColor ResolveThemeColorForProperty(const FString& PropertyPath, FLinearColor Fallback) const;

	/** Explicit appearance setters retain their values through subsequent construction/synchronization. */
	UFUNCTION(BlueprintCallable, Category = "UI|Theme")
	void SetThemePropertyOverride(const FString& PropertyPath, bool bOverride);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	bool IsLayerScreenActive() const { return bLayerScreenActive; }

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	UUILayer* GetOwningLayer() const;

	void SetLayerScreenState(UUILayer* Layer, bool bActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "Invenza|UI|Layout")
	void OnLayerScreenActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Invenza|UI|Layout")
	void OnLayerScreenDeactivated();

	UFUNCTION(BlueprintNativeEvent, Category = "Invenza|UI|Layout")
	void OnRemovedFromLayer();

	UFUNCTION()
	virtual void CalculateParentWidget();

	UFUNCTION()
	virtual void SetParentWidget(UInvenzaBaseWidget* Parent) { ParentWidget = Parent; }

protected:
	// Runtime
	// Properties and variables
	UPROPERTY()
	TObjectPtr<UInvenzaBaseWidget> ParentWidget;

	UPROPERTY(Transient)
	TWeakObjectPtr<UUILayer> OwningLayer;

	bool bLayerScreenActive = false;
	bool bApplyingUITheme = false;

	UPROPERTY(Transient)
	TArray<FInvenzaUIThemeLocalValue> LocalThemeValues;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "UI|Theme")
	TArray<FInvenzaUIThemeBindingResult> ThemeBindingResults;


	// Functions
	virtual void RefreshThemeAppearance();
	virtual void OnRemovedFromLayer_Implementation();
};
