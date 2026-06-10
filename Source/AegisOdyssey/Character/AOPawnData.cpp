// Fill out your copyright notice in the Description page of Project Settings.

#include "AOPawnData.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPawnData)

int32 UAOPawnData::GetXPRequiredForLevel(int32 Level) const
{
	if (!LevelUpXPTable)
	{
		return 0;
	}

	FName LevelName = *FString::FromInt(Level);

	for (const auto& CurveRow : LevelUpXPTable->GetCurves())
	{
		if (CurveRow.CurveName == LevelName)
		{
			const FRealCurve* Curve = CurveRow.CurveToEdit;
			if (Curve)
			{
				float XP = Curve->Eval(Level, 0.0f);
				return FMath::RoundToInt(XP);
			}
		}
	}

	return 0;
}

int32 UAOPawnData::GetAttributePointsForLevel(int32 Level) const
{
	if (!LevelUpAttributePointsTable)
	{
		return 0;
	}

	FName LevelName = *FString::FromInt(Level);

	for (const auto& CurveRow : LevelUpAttributePointsTable->GetCurves())
	{
		if (CurveRow.CurveName == LevelName)
		{
			const FRealCurve* Curve = CurveRow.CurveToEdit;
			if (Curve)
			{
				float Points = Curve->Eval(Level, 0.0f);
				return FMath::RoundToInt(Points);
			}
		}
	}

	return 0;
}

TArray<int32> UAOPawnData::GetAllAttributeValueFromLevel(int32 Level) const
{
	TArray<int32> Result;

	if (!AttributeDataTable)
	{
		return Result;
	}

	FName LevelName = *FString::FromInt(Level);

	for (const auto& CurveRow : AttributeDataTable->GetCurves())
	{
		if (CurveRow.CurveName == LevelName)
		{
			const FRealCurve* Curve = CurveRow.CurveToEdit;
			if (Curve)
			{
				float Points = Curve->Eval(Level, 0.0f);
				Result.Add(Points);
			}
		}
	}

	return Result;
}

int32 UAOPawnData::GetAttributeValueFromNameAndLevel(const FName AttributeName, const int32 Level) const
{
	if (!AttributeDataTable)
	{
		return 0;
	}

	for (const auto& CurveRow : AttributeDataTable->GetCurves())
	{
		if (CurveRow.CurveName == AttributeName)
		{
			const FRealCurve* Curve = CurveRow.CurveToEdit;
			if (Curve)
			{
				float Points = Curve->Eval(Level, 0.0f);
				return FMath::RoundToInt(Points);
			}
		}
	}

	return 0;
}

const UDataTable* UAOPawnData::GetCraftingRecipeDataTable() const
{
	return CraftingRecipeDataTable.IsNull() ? nullptr : CraftingRecipeDataTable.LoadSynchronous();
}
