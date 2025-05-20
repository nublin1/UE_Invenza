//  Nublin Studio 2025 All Rights Reserved.


#include "UI/BaseUserWidget.h"

#include "Components/PanelWidget.h"

void UBaseUserWidget::CalculateParentWidget()
{
	if ( this->GetParent())
	{
		if (auto PW = this->GetParent()->GetOuter()->GetOuter())
			ParentWidget = Cast<UUserWidget>(PW);
		
	}
}

void UBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CalculateParentWidget();
}
