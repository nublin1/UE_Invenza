//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "UILayer.generated.h"

class UBorder;
class UPrimaryLayout;
/**
 *
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UUILayer : public UInvenzaBaseWidget
{
	GENERATED_BODY()

public:
	UUILayer();

protected:
	// Unreal overrides
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackgroundClicked);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLayerStackChanged, UUILayer*, Layer);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLayerContentRemoved, UInvenzaBaseWidget*, Widget);
#pragma endregion Delegates

	// Properties and variables
	UPROPERTY(BlueprintAssignable, Category = "ModalLayout")
	FOnBackgroundClicked OnBackgroundClicked;

	UPROPERTY(BlueprintAssignable, Category = "Invenza|UI|Layout")
	FOnLayerStackChanged OnStackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Invenza|UI|Layout")
	FOnLayerContentRemoved OnContentRemoved;

	// Functions
	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	void ShowTop();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	void CollapseTop();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintPure, BlueprintNativeEvent)
	UUserWidget* Peek();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	UInvenzaBaseWidget* PushContent(const TSoftClassPtr<UUserWidget>& WidgetClass);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	TArray<UInvenzaBaseWidget*> GetStack() const;

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	void ClearStack();

	UFUNCTION(Category = "Invenza|UI|Layout", BlueprintCallable, BlueprintNativeEvent)
	void PopContent();

	/** Reusing an instance raises its existing entry; different instances may share a class. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	bool PushWidget(UInvenzaBaseWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Invenza|UI|Layout")
	bool RemoveContent(UInvenzaBaseWidget* Widget);

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	bool ContainsContent(const UInvenzaBaseWidget* Widget) const;

	UFUNCTION(BlueprintPure, Category = "Invenza|UI|Layout")
	bool IsEmpty() const { return Stack.IsEmpty(); }

	void SetPrimaryLayout(UPrimaryLayout* Layout);
	void NotifyDiagnosticsChanged() const;

protected:
	// Runtime
	// Properties and variables
	//
	UPROPERTY(Category = "Invenza|UI|Layout", BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> MainBorder;

	//
	UPROPERTY(Category = "Invenza|UI|Layout", VisibleInstanceOnly, BlueprintReadWrite)
	TObjectPtr<UInvenzaBaseWidget> PushedWidget;

	UPROPERTY(Category = "Invenza|UI|Layout", VisibleInstanceOnly, BlueprintReadWrite)
	TArray<TObjectPtr<UInvenzaBaseWidget>> Stack;

	UPROPERTY(Transient)
	TWeakObjectPtr<UPrimaryLayout> PrimaryLayout;

	bool bUpdatingStack = false;

	TMap<TWeakObjectPtr<UInvenzaBaseWidget>, TArray<TWeakObjectPtr<UObject>>> SuspendedInputRequests;

	UPROPERTY(Category = "Invenza|UI|Layout", EditAnywhere, BlueprintReadWrite)
	bool bCollapseWhenStackIsEmpty = true;

	UPROPERTY(Category = "Invenza|UI|Layout", EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EHorizontalAlignment> DefaultHorizontalAlignment = HAlign_Center;

	UPROPERTY(Category = "Invenza|UI|Layout", EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EVerticalAlignment> DefaultVerticalAlignment = VAlign_Center;

	// Functions
	virtual void ShowTop_Implementation();
	virtual void CollapseTop_Implementation();
	virtual UUserWidget* Peek_Implementation();
	virtual UInvenzaBaseWidget* PushContent_Implementation(const TSoftClassPtr<UUserWidget>& WidgetClass);
	virtual void ClearStack_Implementation();
	virtual void PopContent_Implementation();

	void RemoveContentInternal(int32 Index);
	void RefreshStackPresentation();
	void ReleaseScreenInput(UInvenzaBaseWidget* Widget);
	void SuspendScreenInput(UInvenzaBaseWidget* Widget);
	void RestoreScreenInput(UInvenzaBaseWidget* Widget);
};
