// Nublin Studio 2026 All Rights Reserved.

#include "InvenzaUIThemeEditingLibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Serialization/JsonSerializer.h"
#include "UI/InvenzaBaseWidget.h"
#include "UI/Theme/InvenzaUITheme.h"
#include "UI/Theme/InvenzaUIThemeLibrary.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"

static FString ExportInvenzaThemeProperty(FProperty* Property, const void* Value)
{
	FString Text;
	Property->ExportTextItem_Direct(Text, Value, nullptr, nullptr, PPF_None);
	return Text;
}

static void CollectInvenzaThemeStyle(UStruct* Type, const void* Container, const FString& Prefix,
	TArray<TSharedPtr<FJsonValue>>& Output, int32 Depth = 0)
{
	if (Depth > 8)
	{
		return;
	}
	for (TFieldIterator<FProperty> It(Type); It; ++It)
	{
		auto* Property = CastField<FStructProperty>(*It);
		if (!Property || Property->HasAnyPropertyFlags(CPF_Transient) ||
			Property->GetName().Contains(TEXT("Theme")) ||
			Property->GetFName() == TEXT("DefaultButtonBackgroundColor"))
		{
			continue;
		}
		const FString Path = Prefix + Property->GetName();
		const void* Value = Property->ContainerPtrToValuePtr<void>(Container);
		FString Kind;
		if (Property->Struct == TBaseStructure<FLinearColor>::Get())
		{
			Kind = TEXT("color");
		}
		else if (Property->Struct == FSlateColor::StaticStruct())
		{
			if (!static_cast<const FSlateColor*>(Value)->IsColorSpecified())
			{
				continue;
			}
			Kind = TEXT("slate_color");
		}
		else if (Property->Struct == FSlateFontInfo::StaticStruct())
		{
			Kind = TEXT("font");
		}
		else if (Property->Struct == FSlateBrush::StaticStruct())
		{
			Kind = TEXT("brush");
		}
		if (Kind.IsEmpty())
		{
			CollectInvenzaThemeStyle(Property->Struct, Value, Path + TEXT("."), Output, Depth + 1);
			continue;
		}
		auto Record = MakeShared<FJsonObject>();
		Record->SetStringField(TEXT("path"), Path);
		Record->SetStringField(TEXT("type"), Kind);
		Record->SetStringField(TEXT("value"), ExportInvenzaThemeProperty(Property, Value));
		if (Kind == TEXT("color") || Kind == TEXT("slate_color") || Kind == TEXT("brush"))
		{
			FLinearColor Color;
			if (Kind == TEXT("color"))
			{
				Color = *static_cast<const FLinearColor*>(Value);
			}
			else if (Kind == TEXT("slate_color"))
			{
				Color = static_cast<const FSlateColor*>(Value)->GetSpecifiedColor();
			}
			else
			{
				const auto& Brush = *static_cast<const FSlateBrush*>(Value);
				Color = Brush.TintColor.GetSpecifiedColor();
				Record->SetStringField(TEXT("resource"), GetPathNameSafe(Brush.GetResourceObject()));
			}
			Record->SetArrayField(TEXT("rgba"), {MakeShared<FJsonValueNumber>(Color.R), MakeShared<FJsonValueNumber>(Color.G),
				MakeShared<FJsonValueNumber>(Color.B), MakeShared<FJsonValueNumber>(Color.A)});
		}
		if (Kind == TEXT("font"))
		{
			const auto& Font = *static_cast<const FSlateFontInfo*>(Value);
			Record->SetNumberField(TEXT("size"), Font.Size);
			Record->SetStringField(TEXT("font"), GetPathNameSafe(Font.FontObject));
			Record->SetStringField(TEXT("face"), Font.TypefaceFontName.ToString());
		}
		Output.Add(MakeShared<FJsonValueObject>(Record));
	}
}

static TSharedPtr<FJsonObject> InspectInvenzaThemeObject(UObject* Object)
{
	auto Record = MakeShared<FJsonObject>();
	Record->SetStringField(TEXT("name"), Object->GetName());
	Record->SetStringField(TEXT("class"), Object->GetClass()->GetName());
	Record->SetBoolField(TEXT("themed"), Object->IsA<UInvenzaBaseWidget>());
	TArray<TSharedPtr<FJsonValue>> Styles;
	CollectInvenzaThemeStyle(Object->GetClass(), Object, TEXT(""), Styles);
	Record->SetArrayField(TEXT("styles"), Styles);
	auto Protected = MakeShared<FJsonObject>();
	for (const FName Name : {FName(TEXT("Visibility")), FName(TEXT("RenderOpacity")), FName(TEXT("Clipping")),
		FName(TEXT("bIsEnabled")), FName(TEXT("bIsFocusable")), FName(TEXT("RenderTransform")),
		FName(TEXT("RenderTransformPivot")), FName(TEXT("Text")), FName(TEXT("DefaultText")),
		FName(TEXT("HintText")), FName(TEXT("DefaultImage")), FName(TEXT("DefaultSize")), FName(TEXT("CurrentSlotSize"))})
	{
		if (auto* Property = FindFProperty<FProperty>(Object->GetClass(), Name))
		{
			Protected->SetStringField(Name.ToString(),
				ExportInvenzaThemeProperty(Property, Property->ContainerPtrToValuePtr<void>(Object)));
		}
	}
	if (const auto* Widget = Cast<UWidget>(Object))
	{
		Record->SetStringField(TEXT("parent"), GetNameSafe(Widget->GetParent()));
		if (Widget->Slot)
		{
			auto Slot = MakeShared<FJsonObject>();
			for (TFieldIterator<FProperty> It(Widget->Slot->GetClass()); It; ++It)
			{
				if (It->HasAnyPropertyFlags(CPF_Edit) && !CastField<FObjectPropertyBase>(*It))
				{
					Slot->SetStringField(It->GetName(), ExportInvenzaThemeProperty(*It, It->ContainerPtrToValuePtr<void>(Widget->Slot)));
				}
			}
			Record->SetObjectField(TEXT("slot"), Slot);
		}
	}
	Record->SetObjectField(TEXT("protected"), Protected);
	return Record;
}

FString UInvenzaUIThemeEditingLibrary::InspectBlueprint(UWidgetBlueprint* Blueprint)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return TEXT("{}");
	}
	auto Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("asset"), Blueprint->GetPathName());
	Root->SetStringField(TEXT("parent"), GetPathNameSafe(Blueprint->ParentClass));
	Root->SetNumberField(TEXT("animation_count"), Blueprint->Animations.Num());
	Root->SetNumberField(TEXT("delegate_binding_count"), Blueprint->Bindings.Num());
	Root->SetObjectField(TEXT("defaults"), InspectInvenzaThemeObject(Blueprint->GeneratedClass->GetDefaultObject()));
	TArray<TSharedPtr<FJsonValue>> Widgets;
	if (Blueprint->WidgetTree)
	{
		TArray<UWidget*> Templates;
		Blueprint->WidgetTree->GetAllWidgets(Templates);
		for (auto* Widget : Templates)
		{
			Widgets.Add(MakeShared<FJsonValueObject>(InspectInvenzaThemeObject(Widget)));
		}
	}
	Root->SetArrayField(TEXT("widgets"), Widgets);
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	TArray<TSharedPtr<FJsonValue>> Pins;
	TArray<TSharedPtr<FJsonValue>> Calls;
	for (auto* Graph : Graphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (auto* Call = Cast<UK2Node_CallFunction>(Node))
			{
				Calls.Add(MakeShared<FJsonValueString>(Call->FunctionReference.GetMemberName().ToString()));
			}
			for (auto* Pin : Node->Pins)
			{
				auto* Struct = Cast<UScriptStruct>(Pin->PinType.PinSubCategoryObject.Get());
				if (Pin->Direction != EGPD_Input || Pin->LinkedTo.Num() > 0 || Pin->DefaultValue.IsEmpty() ||
					!Struct || (Struct != TBaseStructure<FLinearColor>::Get() && Struct != FSlateColor::StaticStruct() &&
					Struct != FSlateFontInfo::StaticStruct() && Struct != FSlateBrush::StaticStruct()))
				{
					continue;
				}
				auto Entry = MakeShared<FJsonObject>();
				Entry->SetStringField(TEXT("graph"), Graph->GetName());
				Entry->SetStringField(TEXT("node"), Node->NodeGuid.ToString());
				Entry->SetStringField(TEXT("pin"), Pin->PinName.ToString());
				Entry->SetStringField(TEXT("value"), Pin->DefaultValue);
				Entry->SetStringField(TEXT("type"), Struct->GetName());
				Entry->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
				Pins.Add(MakeShared<FJsonValueObject>(Entry));
			}
		}
	}
	Root->SetArrayField(TEXT("pins"), Pins);
	Root->SetArrayField(TEXT("calls"), Calls);
	FString Result;
	FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Result));
	return Result;
}

bool UInvenzaUIThemeEditingLibrary::ConfigureWidget(UObject* Widget,
	const TArray<FInvenzaUIThemeBinding>& Bindings, EInvenzaUIThemeSource Source)
{
	auto* Themed = Cast<UInvenzaBaseWidget>(Widget);
	if (!Themed)
	{
		return false;
	}
	Themed->Modify();
	Themed->ThemeSource = Source;
	Themed->ThemeBindings = Bindings;
	return true;
}

bool UInvenzaUIThemeEditingLibrary::CompileThemeBlueprint(UWidgetBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

bool UInvenzaUIThemeEditingLibrary::SetThemeStyle(UInvenzaUITheme* Theme, FName Role,
	const FString& Type, const FString& Value, FName ColorRole, bool bUseDefaultFont)
{
	if (!Theme)
	{
		return false;
	}
	Theme->Modify();
	if (Type == TEXT("color") || Type == TEXT("slate_color"))
	{
		FLinearColor Color;
		if (Type == TEXT("color"))
		{
			if (!Color.InitFromString(Value))
			{
				return false;
			}
		}
		else
		{
			FSlateColor Slate;
			if (!FSlateColor::StaticStruct()->ImportText(*Value, &Slate, nullptr, PPF_None, nullptr, TEXT("Theme")))
			{
				return false;
			}
			Color = Slate.GetSpecifiedColor();
		}
		Theme->Palette.Add(Role, Color);
		return true;
	}
	if (Type == TEXT("font"))
	{
		FInvenzaUIThemeFont Style;
		if (!FSlateFontInfo::StaticStruct()->ImportText(*Value, &Style.Font, nullptr, PPF_None, nullptr, TEXT("Theme")))
		{
			return false;
		}
		Style.bUseDefaultFont = bUseDefaultFont;
		Theme->Typography.Add(Role, Style);
		return true;
	}
	if (Type == TEXT("brush"))
	{
		FInvenzaUIThemeBrush Style;
		if (!FSlateBrush::StaticStruct()->ImportText(*Value, &Style.Brush, nullptr, PPF_None, nullptr, TEXT("Theme")))
		{
			return false;
		}
		Style.ColorRole = ColorRole;
		Theme->Brushes.Add(Role, Style);
		return true;
	}
	return false;
}

bool UInvenzaUIThemeEditingLibrary::CopyThemeStyle(UInvenzaUITheme* Theme, FName Role,
	UObject* Source, const FString& PropertyPath, FName ColorRole, bool bUseDefaultFont)
{
	if (!Theme || !Source)
	{
		return false;
	}
	TArray<FString> Parts;
	PropertyPath.ParseIntoArray(Parts, TEXT("."));
	UStruct* Type = Source->GetClass();
	void* Container = Source;
	for (int32 Index = 0; Index < Parts.Num(); ++Index)
	{
		auto* Property = FindFProperty<FStructProperty>(Type, FName(Parts[Index]));
		if (!Property)
		{
			return false;
		}
		void* Value = Property->ContainerPtrToValuePtr<void>(Container);
		if (Index + 1 < Parts.Num())
		{
			Container = Value;
			Type = Property->Struct;
			continue;
		}
		Theme->Modify();
		if (Property->Struct == FSlateBrush::StaticStruct())
		{
			FInvenzaUIThemeBrush Style;
			Style.Brush = *static_cast<const FSlateBrush*>(Value);
			Style.ColorRole = ColorRole;
			Theme->Brushes.Add(Role, Style);
			return true;
		}
		if (Property->Struct == FSlateFontInfo::StaticStruct())
		{
			FInvenzaUIThemeFont Style;
			Style.Font = *static_cast<const FSlateFontInfo*>(Value);
			Style.bUseDefaultFont = bUseDefaultFont;
			Theme->Typography.Add(Role, Style);
			return true;
		}
		return false;
	}
	return false;
}

bool UInvenzaUIThemeEditingLibrary::AdoptBaseWidget(UWidgetBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return false;
	}
	if (Blueprint->ParentClass == UUserWidget::StaticClass())
	{
		Blueprint->Modify();
		Blueprint->ParentClass = UInvenzaBaseWidget::StaticClass();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
	return Blueprint->ParentClass->IsChildOf(UInvenzaBaseWidget::StaticClass());
}

bool UInvenzaUIThemeEditingLibrary::ConnectThemePin(UBlueprint* Blueprint, const FString& GraphName,
	const FString& NodeID, FName PinName, FName Role)
{
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (auto* Graph : Graphs)
	{
		if (Graph->GetName() != GraphName)
		{
			continue;
		}
		for (UEdGraphNode* Node : TArray<TObjectPtr<UEdGraphNode>>(Graph->Nodes))
		{
			if (Node->NodeGuid.ToString() != NodeID)
			{
				continue;
			}
			auto* Pin = Node->FindPin(PinName, EGPD_Input);
			auto* Struct = Pin ? Cast<UScriptStruct>(Pin->PinType.PinSubCategoryObject.Get()) : nullptr;
			if (!Pin || !Struct || !Pin->LinkedTo.IsEmpty())
			{
				return false;
			}
			FName FunctionName;
			if (Struct == TBaseStructure<FLinearColor>::Get())
			{
				FunctionName = GET_FUNCTION_NAME_CHECKED(UInvenzaUIThemeLibrary, ThemeColor);
			}
			else if (Struct == FSlateColor::StaticStruct())
			{
				FunctionName = GET_FUNCTION_NAME_CHECKED(UInvenzaUIThemeLibrary, ThemeSlateColor);
			}
			else if (Struct == FSlateFontInfo::StaticStruct())
			{
				FunctionName = GET_FUNCTION_NAME_CHECKED(UInvenzaUIThemeLibrary, ThemeFont);
			}
			else if (Struct == FSlateBrush::StaticStruct())
			{
				FunctionName = GET_FUNCTION_NAME_CHECKED(UInvenzaUIThemeLibrary, ThemeBrush);
			}
			else
			{
				return false;
			}
			Blueprint->Modify();
			Graph->Modify();
			auto* Getter = NewObject<UK2Node_CallFunction>(Graph);
			Getter->SetFromFunction(UInvenzaUIThemeLibrary::StaticClass()->FindFunctionByName(FunctionName));
			Getter->CreateNewGuid();
			Graph->AddNode(Getter, false, false);
			Getter->PostPlacedNewNode();
			Getter->AllocateDefaultPins();
			Getter->NodePosX = Node->NodePosX - 280;
			Getter->NodePosY = Node->NodePosY;
			Getter->FindPinChecked(TEXT("Role"))->DefaultValue = Role.ToString();
			Getter->FindPinChecked(TEXT("Fallback"))->DefaultValue = Pin->DefaultValue;
			if (!Graph->GetSchema()->TryCreateConnection(Getter->GetReturnValuePin(), Pin))
			{
				Getter->DestroyNode();
				return false;
			}
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			return true;
		}
	}
	return false;
}

TArray<FInvenzaUIThemeBindingResult> UInvenzaUIThemeEditingLibrary::ValidateThemeWidget(UInvenzaBaseWidget* Widget, UWidgetTree* Tree)
{
	return UInvenzaUIThemeLibrary::ValidateBindings(Widget, Tree);
}

void UInvenzaUIThemeEditingLibrary::MakeDesignerPreview(UInvenzaBaseWidget* Widget)
{
	if (Widget)
	{
		Widget->SetDesignerFlags(EWidgetDesignFlags::Designing);
	}
}

bool UInvenzaUIThemeEditingLibrary::EditPreviewStyle(UInvenzaBaseWidget* Owner, FName WidgetName,
	const FString& PropertyPath, const FInvenzaUIThemeValue& Value)
{
	FInvenzaUIThemeBinding Binding;
	Binding.WidgetName = WidgetName;
	Binding.PropertyPath = PropertyPath;
	auto* Target = UInvenzaUIThemeLibrary::FindBindingTarget(Owner, Binding);
	if (!UInvenzaUIThemeLibrary::WriteStyleValue(Target, PropertyPath, Value))
	{
		return false;
	}
	FString Top, Remainder;
	if (!PropertyPath.Split(TEXT("."), &Top, &Remainder))
	{
		Top = PropertyPath;
	}
	FPropertyChangedEvent Event(FindFProperty<FProperty>(Target->GetClass(), FName(Top)), EPropertyChangeType::ValueSet);
	Target->PostEditChangeProperty(Event);
	return true;
}
