// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class GRIDINVENTORYPLUGIN_API UBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	virtual void SetParentWidget(UBaseUserWidget* Parent) {ParentWidget = Parent;}

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> ParentWidget;


	virtual void NativeConstruct() override;
	
};
