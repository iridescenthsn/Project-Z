// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickUp.h"
#include "../Player/FPS_Character.h"
#include "Components/BoxComponent.h"


void AWeaponPickUp::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmoInMag=WeaponClass.GetDefaultObject()->GetMaxAmmoInMag();
	CurrentReservedAmmo=WeaponClass.GetDefaultObject()->GetMaxReservedAmmo();

	Box->SetCanEverAffectNavigation(false);
	Mesh->SetCanEverAffectNavigation(false);
}


bool AWeaponPickUp::InteractAction(AFPS_Character* PlayerRef)
{
	PlayerRef->PickUpWeapon(WeaponClass,CurrentAmmoInMag,CurrentReservedAmmo);
	Destroy();
	return true;
}
