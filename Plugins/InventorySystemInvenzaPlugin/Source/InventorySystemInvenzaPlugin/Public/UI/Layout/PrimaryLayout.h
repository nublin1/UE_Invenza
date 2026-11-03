//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Layout/UILayoutTypes.h"
#include "PrimaryLayout.generated.h"

class UUILayer;
class ULayersDebuggerWidget;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UPrimaryLayout : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UPrimaryLayout();

protected:
	// Unreal overrides
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLayoutChanged);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> GameLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> GameMenuLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> MenuLayer;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUILayer> ModalLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invenza|UI|Layout", meta = (ExposeOnSpawn = "true"))
	TMap<FGameplayTag, TSoftClassPtr<UUserWidget>> InitialScreens;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Invenza|UI|Layout")
	TArray<FUILayerRegistration> LayerDefinitions;

	UPROPERTY(BlueprintAssignable, Category = "Invenza|UI|Layout")
	FOnLayoutChanged OnLayoutChanged;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	UUserWidget* PushContentToLayer(FGameplayTag LayerName, const TSoftClassPtr<UUserWidget>& WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	bool RegisterLayer(FGameplayTag LayerName, UUILayer* LayerWidget);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	bool UnRegisterLayer(FGameplayTag LayerName);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	UUILayer* FindLayer(FGameplayTag LayerName) const;

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	TArray<FGameplayTag> GetRegisteredLayerTags() const;

	FGameplayTag GetLayerTag(const UUILayer* Layer) const;

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void ClearLayer(FGameplayTag LayerName);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void ClearAllLayers();

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void ClearAllLayersExcept(FGameplayTag ExceptionLayerName);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void PopContentFromLayer(FGameplayTag LayerName);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void ChangeLayerVisbility(FGameplayTag LayerTag, ESlateVisibility State);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	void PushInitialScreens();

	void NotifyLayoutChanged();

protected:
	// Runtime
	// Properties and variables
	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	TArray<FGameplayTag> InitialCollapsedScreens;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> LayersDebugger;

	UPROPERTY(Category = "Invenza|UI|Layout", VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> GameMenuLayer_Obs;

	UPROPERTY(BlueprintReadOnly, Category = "Invenza|UI|Layout", meta = (BindWidgetOptional))
	TObjectPtr<ULayersDebuggerWidget> WBP_LayersDebugger;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UUILayer>> RegisteredLayers;

	UPROPERTY(Transient)
	TArray<FGameplayTag> RegisteredLayerOrder;

	bool bPushedInitialScreens = false;
	bool bDestroyingLayout = false;

	// Functions
	virtual UUserWidget* PushContentToLayer_Implementation(FGameplayTag LayerName, const TSoftClassPtr<UUserWidget>& WidgetClass);

	UFUNCTION()
	void LayerStackChanged(UUILayer* Layer);
};
