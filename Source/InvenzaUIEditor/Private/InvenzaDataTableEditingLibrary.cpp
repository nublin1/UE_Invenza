//  Nublin Studio 2026 All Rights Reserved.

#include "InvenzaDataTableEditingLibrary.h"
#include "Engine/DataTable.h"
#include "Misc/FeedbackContext.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "UObject/StructOnScope.h"

bool UInvenzaDataTableEditingLibrary::SetRowFromText(UDataTable* Table, FName RowName, const FString& RowText)
{
	if (!Table || !Table->GetRowStruct() || RowName.IsNone())
	{
		return false;
	}
	FStructOnScope Row(Table->GetRowStruct());
	if (!Table->GetRowStruct()->ImportText(*RowText, Row.GetStructMemory(), Table, PPF_None, GWarn,
										   TEXT("SetRowFromText")))
	{
		return false;
	}
	Table->Modify();
	Table->AddRow(RowName, *reinterpret_cast<const FTableRowBase*>(Row.GetStructMemory()));
	Table->MarkPackageDirty();
	return true;
}

int32 UInvenzaDataTableEditingLibrary::RemoveRows(UDataTable* Table, const TArray<FName>& RowNames)
{
	if (!IsValid(Table) || !Table->GetRowStruct() || RowNames.IsEmpty())
	{
		return 0;
	}
	const UScriptStruct* RowStruct = Table->GetRowStruct();
	TMap<FName, TUniquePtr<FStructOnScope>> RetainedRows;
	int32 RemovedCount = 0;
	for (const auto& Entry : Table->GetRowMap())
	{
		if (RowNames.Contains(Entry.Key))
		{
			++RemovedCount;
		}
		else
		{
			TUniquePtr<FStructOnScope> Copy = MakeUnique<FStructOnScope>(RowStruct);
			RowStruct->CopyScriptStruct(Copy->GetStructMemory(), Entry.Value);
			RetainedRows.Add(Entry.Key, MoveTemp(Copy));
		}
	}
	if (RemovedCount == 0)
	{
		return 0;
	}
	// Native copies preserve localized FText identities. UE 5.6's single-row removal
	// callback indexes the erased row, so rebuild only from the retained native rows.
	Table->Modify();
	Table->EmptyTable();
	for (const auto& Entry : RetainedRows)
	{
		Table->AddRow(Entry.Key, *reinterpret_cast<const FTableRowBase*>(Entry.Value->GetStructMemory()));
	}
	Table->MarkPackageDirty();
	return RemovedCount;
}

int64 UInvenzaDataTableEditingLibrary::ReplaceTableObjectReference(UDataTable* Table, UObject* PreviousObject,
																   UObject* ReplacementObject)
{
	if (!IsValid(Table) || !IsValid(PreviousObject) || !IsValid(ReplacementObject) ||
		!ReplacementObject->IsA(PreviousObject->GetClass()))
	{
		return 0;
	}

	Table->Modify();
	TMap<UObject*, UObject*> Replacements;
	Replacements.Add(PreviousObject, ReplacementObject);
	FArchiveReplaceObjectRef<UObject> Archive(Table, Replacements,
											  EArchiveReplaceObjectFlags::IgnoreOuterRef |
												  EArchiveReplaceObjectFlags::IgnoreArchetypeRef);
	if (Archive.GetCount() > 0)
	{
		Table->MarkPackageDirty();
	}
	return Archive.GetCount();
}
