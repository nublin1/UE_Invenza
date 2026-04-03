//  Nublin Studio 2025 All Rights Reserved.


#include "UI/InvenzaBaseWidget.h"

#include "Components/PanelWidget.h"

void UInvenzaBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CalculateParentWidget();
}

void UInvenzaBaseWidget::CalculateParentWidget()
{
	if ( this->GetParent())
	{
		if (auto PW = this->GetParent()->GetOuter()->GetOuter())
			ParentWidget = Cast<UInvenzaBaseWidget>(PW);
		
	}
}