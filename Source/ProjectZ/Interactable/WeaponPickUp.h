// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables.h"
#include "WeaponPickUp.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AWeaponPickUp : public AInteractables
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	TSubclassOf<class AWeaponBase> WeaponClass;

public:

	virtual void BeginPlay() override;
	
	virtual bool InteractAction(AFPS_Character* PlayerRef) override;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Ammo")
	int32 CurrentAmmoInMag;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Ammo")
	int32 CurrentReservedAmmo;
	
};
