// Fill out your copyright notice in the Description page of Project Settings.

#include "AutomaticWeapons.h"
#include "../Player/FPS_Character.h"
#include "../Effects/PistolAmmoShell.h"	


AAutomaticWeapons::AAutomaticWeapons()
{
	bIsWeaponAuto = true;
}

void AAutomaticWeapons::BeginPlay()
{
	Super::BeginPlay();
	
	PerBulletRecoilPitch.SetNum(MaxAmmoInMag);
	PerBulletRecoilYaw.SetNum(MaxAmmoInMag);

	PerBulletRecoilPitch[0]= 0.2f;
	PerBulletRecoilPitch[1]= 0.16f;
	PerBulletRecoilPitch[2]= 0.12f;
	PerBulletRecoilPitch[3]= 0.08f;
	PerBulletRecoilPitch[4]= 0.08f;
	PerBulletRecoilPitch[5]= 0.08f;
	PerBulletRecoilPitch[6]= 0.06f;
	PerBulletRecoilPitch[7]= 0.04f;
	PerBulletRecoilPitch[8]= 0.02f;
	PerBulletRecoilPitch[9]= 0.02f;
	PerBulletRecoilPitch[10]= 0.06f;
	PerBulletRecoilPitch[11]= 0.06f;
	PerBulletRecoilPitch[12]= 0.02f;
	PerBulletRecoilPitch[13]= 0.02f;
	PerBulletRecoilPitch[14]= 0.02f;
	PerBulletRecoilPitch[15]= 0.02f;
	PerBulletRecoilPitch[16]= 0.02f;
	PerBulletRecoilPitch[17]= 0.02f;
	PerBulletRecoilPitch[18]= 0.02f;
	PerBulletRecoilPitch[19]= 0.02f;
	PerBulletRecoilPitch[20]= 0.02f;
	PerBulletRecoilPitch[21]= 0.04f;
	PerBulletRecoilPitch[22]= -0.06f;
	PerBulletRecoilPitch[23]= 0.02f;

	PerBulletRecoilYaw[0]= 0.04f;
	PerBulletRecoilYaw[1]= 0.04f;
	PerBulletRecoilYaw[2]= 0.04f;
	PerBulletRecoilYaw[3]= 0.04f;
	PerBulletRecoilYaw[4]= 0.04f;
	PerBulletRecoilYaw[5]= 0.04f;
	PerBulletRecoilYaw[6]= 0.02f;
	PerBulletRecoilYaw[7]= -0.02f;
	PerBulletRecoilYaw[8]= -0.02f;
	PerBulletRecoilYaw[9]= -0.04f;
	PerBulletRecoilYaw[10]= -0.06f;
	PerBulletRecoilYaw[11]= -0.06f;
	PerBulletRecoilYaw[12]= -0.06f;
	PerBulletRecoilYaw[13]= -0.06f;
	PerBulletRecoilYaw[14]= -0.06f;
	PerBulletRecoilYaw[15]= -0.02f;
	PerBulletRecoilYaw[16]= -0.04f;
	PerBulletRecoilYaw[17]= -0.02f;
	PerBulletRecoilYaw[18]= 0.02f;
	PerBulletRecoilYaw[19]= 0.04f;
	PerBulletRecoilYaw[20]= 0.06f;
	PerBulletRecoilYaw[21]= 0.06f;
	PerBulletRecoilYaw[22]= 0.08f;
	PerBulletRecoilYaw[23]= 0.1f;

}

void AAutomaticWeapons::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//Call auto fire frequently based on fire rate for as long as holding the button down(sets timer)
void AAutomaticWeapons::WeaponFire()
{
	if (Player->bIsNearWall)
	{
		StopFire();
		return;
	}

	Super::WeaponFire();

	if (Player->bCanFire&&bIsReadyToFire)
	{
		bIsWeaponFiring = true;

		//dont pull weapon up(animation) if adsing
		if (!Player->bIsADSing)
		{
			Player->CharacterFireWeapon.Broadcast(WeaponType);
		}
		else
		{
			Player->CharacterStopFireWeapon.Broadcast();
		}

		bIsReadyToFire=false;
		GetWorldTimerManager().SetTimer(AutoFireHandle, this, &AAutomaticWeapons::AutoFire, FireRate, false);
	}
}

void AAutomaticWeapons::AutoFire()
{
	bIsReadyToFire=true;
	if (bIsWeaponFiring)
	{
		WeaponFire();
	}
}

//Stops calling the auto fire function when releasing the firing button
void AAutomaticWeapons::StopFire()
{
	if (bIsWeaponFiring)
	{
		Player->CharacterStopFireWeapon.Broadcast();

		CalculateReverseRecoil();

		GetWorldTimerManager().SetTimer(RecoilReverseHandle, this, &AAutomaticWeapons::RevertRecoil, RecoilReverseSpeed, true);
		
		GetWorldTimerManager().SetTimer(RecoilResetHandle, this, &AAutomaticWeapons::ResetRecoil, ResetRecoilRate, true);

		bIsWeaponFiring = false;
		Player->bCanFire=true;
	}
}






