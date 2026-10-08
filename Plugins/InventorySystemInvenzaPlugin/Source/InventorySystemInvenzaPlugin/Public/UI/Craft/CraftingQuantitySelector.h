// Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/InvenzaBaseWidget.h"
#include "CraftingQuantitySelector.generated.h"

class UUIButton;
class UEditableLabelBaseText;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEMINVENZAPLUGIN_API UCraftingQuantitySelector : public UInvenzaBaseWidget
{
	GENERATED_BODY()

	
#pragma region Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuantityChanged, int32, NewQuantity);
#pragma endregion
	
public:
	UCraftingQuantitySelector();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_SetMin;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_Decrease;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UUIButton> Btn_Increase;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidgetOptional))
	TObjectPtr<UUIButton> Btn_SetMax;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Components", meta = (BindWidget))
	TObjectPtr<UEditableLabelBaseText> CurrentQuantityText;

	//=============================
	// EVENTS            
	// ============================
	UPROPERTY(BlueprintAssignable, Category="Quantity")
	FOnQuantityChanged OnQuantityChanged;

	//====================================================================
	// FUNCTIONS
	//====================================================================
	UFUNCTION(BlueprintCallable, Category="Quantity")
	void SetToMin(UUIButton* ButtonPressed);

	UFUNCTION(BlueprintCallable, Category="Quantity")
	void SetToMax(UUIButton* ButtonPressed);

	UFUNCTION(BlueprintCallable, Category="Quantity")
	void Increase(UUIButton* ButtonPressed);

	UFUNCTION(BlueprintCallable, Category="Quantity")
	void Decrease(UUIButton* ButtonPressed);

	UFUNCTION(BlueprintCallable, Category="Quantity")
	void SetQuantity(int32 NewValue);
	
	UFUNCTION(BlueprintCallable, Category="Quantity")
	int32 GetCurrentQuantity() const;

	// Resource maximum is a shortcut, not a limit on queued work.
	UFUNCTION(BlueprintCallable, Category="Quantity")
	void SetResourceMaximum(int32 NewMaximum);

	UFUNCTION(BlueprintCallable, Category="Quantity")
	void CommitPendingQuantity();
	
protected:
	//====================================================================
	// PROPERTIES AND VARIABLES
	//====================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quantity")
	int32 MinQuantity = 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quantity")
	int32 CurrentQuantity = 1;

	//====================================================================
	// FUNCTIONS
	//====================================================================

	UFUNCTION()
	void OnTextCommitted(const FText& NewText);

	UFUNCTION()
	void UpdateText();

	UFUNCTION()
	void HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod);

	int32 ResourceMaximum = 0;
	bool bUpdatingText = false;
	bool bHasPendingText = false;
	FText PendingText;
	void UpdateButtons();
};
