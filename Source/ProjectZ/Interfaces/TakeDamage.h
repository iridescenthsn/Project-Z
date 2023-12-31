// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Weapons/WeaponBase.h"	
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TakeDamage.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI,Blueprintable)
class UTakeDamage : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTZ_API ITakeDamage
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void TakeDamage(const FAmmoData& AmmoData, float CriticalHitModifier,const FHitResult& HitResult);

	virtual void TakeDamage(float Damage);

	virtual void TakeRadialDamage(const FAmmoData& AmmoData, float CriticalHitModifier,const FHitResult& HitResult,const FVector& ExplosiveLocation=FVector::ZeroVector);

	virtual void TakeMeleeDamage(float Damage, const FHitResult& HitResult);
};
