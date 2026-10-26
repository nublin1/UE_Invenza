//  Nublin Studio 2026 All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InvenzaDataTableEditingLibrary.generated.h"

class UDataTable;

/** Editor bridge for adding a single native-struct row without reimporting unrelated rows. */
UCLASS()
class INVENZAUIEDITOR_API UInvenzaDataTableEditingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Invenza|Data Editing")
	static bool SetRowFromText(UDataTable* Table, FName RowName, const FString& RowText);

	/** Batches removals so UE 5.6 does not run a single-row callback for a deleted row. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Data Editing")
	static int32 RemoveRows(UDataTable* Table, const TArray<FName>& RowNames);

	/** Replaces references within this table without exporting or rebuilding its localized rows. */
	UFUNCTION(BlueprintCallable, Category = "Invenza|Data Editing")
	static int64 ReplaceTableObjectReference(UDataTable* Table, UObject* PreviousObject, UObject* ReplacementObject);
};
