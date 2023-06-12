// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoBuy.h"
#include "../Player/FPS_Character.h"

bool AAmmoBuy::InteractAction(AFPS_Character* PlayerRef)
{
	const auto CurrentWeapon= PlayerRef->GetCurrentWeapon();
	if (!CurrentWeapon)
	{
		return false;
	}
	
	if (PlayerRef->GetPoints()>=Cost&&(CurrentWeapon->GetCurrentReservedAmmo()<CurrentWeapon->GetMaxReservedAmmo()))
	{
		PlayerRef->SetPoints(PlayerRef->GetPoints()-Cost);
		PlayerRef->UpdateMainHud.Broadcast();
		BuyAction(PlayerRef);
		return true;
	}
	return false;
}

void AAmmoBuy::BuyAction(AFPS_Character* PlayerRef)
{
	const auto CurrentWeapon= PlayerRef->GetCurrentWeapon();
	CurrentWeapon->SetCurrentReservedAmmo(CurrentWeapon->GetMaxReservedAmmo());
	CurrentWeapon->SetCurrentAmmoInMag(CurrentWeapon->GetMaxAmmoInMag());
}
