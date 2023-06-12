// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponBase.h"
#include "../Effects/ImpactEffect.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Player/FPS_Character.h"
#include "../Effects/PistolAmmoShell.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Interfaces/TakeDamage.h"
#include "../Interactable/WeaponPickUp.h"
#include "Perception/AISense_Hearing.h"
#include "ProjectZ/CollisionChannels.h"
#include "ProjectZ/Portal.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create a scene component and set it as seen root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Create gun mesh and attach it to scene root
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetOnlyOwnerSee(false);
	GunMesh->CastShadow = false;
	GunMesh->bCastDynamicShadow = false;
	GunMesh->SetupAttachment(SceneRoot);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetSimulatePhysics(false);

	SuppressorMesh=CreateDefaultSubobject<UStaticMeshComponent>(FName("Attachment"));
	SuppressorMesh->CastShadow=false;
	SuppressorMesh->bCastDynamicShadow=false;
	SuppressorMesh->SetHiddenInGame(true);
	SuppressorMesh->SetupAttachment(GunMesh,FName("Attachment_Socket"));
}

void AWeaponBase::SpawnPickUp()
{
	const auto PickUp = GetWorld()->SpawnActor<AWeaponPickUp>(PickUpClass);
	PickUp->SetActorTransform(GetActorTransform());
		
	PickUp->CurrentAmmoInMag=CurrentAmmoInMag;
	PickUp->CurrentReservedAmmo=CurrentReservedAmmo;

	Destroy();
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Get camera and player ref
	Player = Cast<AFPS_Character>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	Camera = Player->FirstPersonCameraComponent;
	
	//Timeline float for recoil pitch binding
	if (RecoilPitchCurve)
	{
		FOnTimelineFloat RecoilPitchTMFloat;
		RecoilPitchTMFloat.BindUFunction(this, FName(TEXT("AddRecoilPitch")));
		RecoilTimeLine.AddInterpFloat(RecoilPitchCurve, RecoilPitchTMFloat);

		if (!bIsWeaponAuto)
		{
			FOnTimelineEvent RecoilTimeLineFinished;
			RecoilTimeLineFinished.BindUFunction(this, FName(TEXT("StopFire")));
			RecoilTimeLine.SetTimelineFinishedFunc(RecoilTimeLineFinished);
		}
	}
	
	//Timeline float for recoil Yaw binding
	if (RecoilYawCurve)
	{
		FOnTimelineFloat RecoilYawTMFloat;
		RecoilYawTMFloat.BindUFunction(this, FName(TEXT("AddRecoilYaw")));
		RecoilTimeLine.AddInterpFloat(RecoilYawCurve, RecoilYawTMFloat);
	}
	
	//timeline shouldn't loop
	RecoilTimeLine.SetLooping(false);

	//Set the current bullet spread to the hipfire spread
	BulletSpread=HipFireBulletSpread;

	PerBulletRecoilPitch.SetNum(MaxAmmoInMag);
	PerBulletRecoilYaw.SetNum(MaxAmmoInMag);

	CameraManager= UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);

	//Tick recoil timeline on a constant rate
	GetWorldTimerManager().SetTimer(RecoilTimelineTicker,[&]
	{
		RecoilTimeLine.TickTimeline(0.008f); 
	},0.008f,true);
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(RecoilTimelineTicker);
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AWeaponBase::CalculateShot(TArray<FHitResult>& InGoingShots, TArray<FHitResult>& OutGoingShots, FVector& TraceEnd) const
{
	//Randomize bullet rays
	FVector Startloc = Camera->GetComponentLocation();
	FVector Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(Camera->GetComponentRotation()) * LineTraceRange);
	TraceEnd=Endloc;
	
	Endloc.X = Endloc.X + FMath::RandRange(-BulletSpread, BulletSpread);
	Endloc.Y = Endloc.Y + FMath::RandRange(-BulletSpread, BulletSpread);
	Endloc.Z = Endloc.Z + FMath::RandRange(-BulletSpread, BulletSpread);

	
	//Add player as ignored actor for line trace
	FCollisionQueryParams ActorToIgnore;
	ActorToIgnore.AddIgnoredActor(Player);
	ActorToIgnore.bReturnPhysicalMaterial = true;

	//Add Objects to trace by (static and dynamic)
	FCollisionObjectQueryParams ObjectsToTrace;
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_CharacterBody); //Character mesh
	ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_Penetrable); //Penetrable
	ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_Hackable); //Hackable

	//do a line trace to find all in going shots
	bool AnyHit = GetWorld()->LineTraceMultiByObjectType(InGoingShots,Startloc,Endloc,ObjectsToTrace,ActorToIgnore);

	/*DrawDebugLine(GetWorld(), Startloc, Endloc, FColor::Green, false, 4);
	for (const auto& HitResult :InGoingShots)
	{
		DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);
	}*/

	
	if(AnyHit)
	{
		//If we hit a portal then shoot through the portal
		if (const APortal* Portal = Cast<APortal>(InGoingShots[0].GetActor()))
		{
			const FVector RelStartLoc = UKismetMathLibrary::InverseTransformLocation(Portal->RotatedTransform,InGoingShots[0].ImpactPoint);
			const FVector WorldStartLoc = UKismetMathLibrary::TransformLocation(Portal->LinkedPortal->GetTransform(),RelStartLoc);

			const FVector RelEndLoc= UKismetMathLibrary::InverseTransformLocation(Portal->RotatedTransform,Endloc);
			const FVector WorldEndLoc= UKismetMathLibrary::TransformLocation(Portal->LinkedPortal->GetTransform(),RelEndLoc);

			AnyHit = GetWorld()->LineTraceMultiByObjectType(InGoingShots,WorldStartLoc,WorldEndLoc,ObjectsToTrace,ActorToIgnore);

			/*DrawDebugLine(GetWorld(), WorldStartLoc, WorldEndLoc, FColor::Green, false, 4);
			for (const auto& HitResult :InGoingShots)
			{
				DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);
			}*/

			GetWorld()->LineTraceMultiByObjectType(OutGoingShots,WorldEndLoc,WorldStartLoc,ObjectsToTrace,ActorToIgnore);

			/*for (const auto& HitResult :OutGoingShots)
			{
				DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);
			}*/

			return AnyHit;
		}

		//if we hit something find all out going shots
		GetWorld()->LineTraceMultiByObjectType(OutGoingShots,Endloc,Startloc,ObjectsToTrace,ActorToIgnore);

		/*for (const auto& HitResult :OutGoingShots)
		{
			DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);
		}*/
	}
	
	return AnyHit;
}

FHitResult AWeaponBase::AiCalculateShot() const
{
	//Randomize bullet rays
	FVector Startloc = GunMesh->GetSocketLocation(FName(TEXT("MuzzleFlash")));
	FVector Endloc = Startloc + (UKismetMathLibrary::GetForwardVector(GunMesh->GetSocketRotation(FName("MuzzleFlash"))) * LineTraceRange);
	
	Endloc.X = Endloc.X + FMath::RandRange(-BulletSpread, BulletSpread);
	Endloc.Y = Endloc.Y + FMath::RandRange(-BulletSpread, BulletSpread);
	Endloc.Z = Endloc.Z + FMath::RandRange(-BulletSpread, BulletSpread);

	FHitResult HitResult;

	//Add player as ignored actor for line trace
	FCollisionQueryParams ActorToIgnore;
	ActorToIgnore.AddIgnoredActor(this); 
	ActorToIgnore.AddIgnoredActor(GetAttachParentActor());
	ActorToIgnore.bReturnPhysicalMaterial = true;

	//Add Objects to trace by (static and dynamic)
	FCollisionObjectQueryParams ObjectsToTrace;
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectsToTrace.AddObjectTypesToQuery(ECC_GameTraceChannel7); // Character mesh

	GetWorld()->LineTraceSingleByObjectType(HitResult,Startloc,Endloc,ObjectsToTrace,ActorToIgnore);

	/*DrawDebugLine(GetWorld(), Startloc, Endloc, FColor::Green, false, 4);
	DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);*/

	return HitResult;
}


//Add damage to the hit actor if it has TakeDamage interface
void AWeaponBase::AddDamage(const FHitResult &Hit, const FName& ShooterTag, float Damage) const
{
	AActor* HitActor = Hit.GetActor();

	//if shooter has the same tag as the hit actor dont apply any damage
	if (HitActor&&!HitActor->ActorHasTag(ShooterTag))
	{
		ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(HitActor);
		if (TakeDamageInterface)
		{
			FAmmoData LocAmmoData{AmmoData};
			LocAmmoData.Damage=Damage;
			TakeDamageInterface->TakeDamage(LocAmmoData, CriticalHitModifier, Hit);
		}
	}
}

//Ejects ammo shells when shooting from ammoeject socket on gunmesh
void AWeaponBase::AmmoShellEject() const
{
	if (AmmoShellClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FRotator SpawnRotation = GunMesh->GetSocketRotation(FName(TEXT("AmmoEject")));
			const FVector SpawnLocation = GunMesh->GetSocketLocation(FName(TEXT("AmmoEject")));

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			World->SpawnActor<APistolAmmoShell>(AmmoShellClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
}

//Plays the recoil timeline
void AWeaponBase::AddRecoil()
{
	if (RecoilYawCurve && RecoilPitchCurve)
	{
		RecoilTimeLine.PlayFromStart();
	}
	else if (RecoilPitchCurve && !bIsWeaponAuto)
	{
		RecoilTimeLine.PlayFromStart();
	}
}

//Adds recoil pitch
void AWeaponBase::AddRecoilPitch(float Value)
{
	const float PitchToAdd= FMath::Lerp(PerBulletRecoilPitch[NumberOfShot]*RecoilMultiplier,0.0f,Value);
	Player->AddControllerPitchInput(-PitchToAdd);
	
	RecoilAllAddedPitch-=PitchToAdd;
}


//Add yaw recoil
void AWeaponBase::AddRecoilYaw(float Value)
{
	const float YawToAdd=FMath::Lerp(PerBulletRecoilYaw[NumberOfShot]*RecoilMultiplier,0.0f,Value);
	Player->AddControllerYawInput(YawToAdd);
	
	RecoilAllAddedYaw+=YawToAdd;
}


//Calculate Revert recoil based on the amount of the recoil added and player pitch input
void AWeaponBase::RevertRecoil()
{
	Player->AddControllerPitchInput(-PitchPullDown);
	Player->AddControllerYawInput(-YawPullDown);

	static int32 TimesCalled=0;
	TimesCalled++;

	if (TimesCalled==100)
	{
		GetWorldTimerManager().ClearTimer(RecoilReverseHandle);
		TimesCalled=0;
	}
}


void AWeaponBase::ResetRecoil()
{
	if (!bIsWeaponFiring)
	{
		NumberOfShot--;
		NumberOfShot=FMath::Clamp(NumberOfShot,0,MaxAmmoInMag-1);
		if (NumberOfShot==0)
		{
			GetWorldTimerManager().ClearTimer(RecoilResetHandle);
		}
	}
}

void AWeaponBase::CalculateReverseRecoil()
{
	/*
	If same directions then ignore player input

	If opposite directions

		If player input abs is less than recoil abs then add them together

		If player input abs is more than or equal to recoil abs then dont pull 
	*/

	if (FMath::Sign(PlayerPitchInput)!=FMath::Sign(RecoilAllAddedPitch))
	{
		if (FMath::Abs(PlayerPitchInput)<FMath::Abs(RecoilAllAddedPitch))
		{
			RecoilAllAddedPitch += PlayerPitchInput;
		}
		else
		{
			RecoilAllAddedPitch = 0;
		}
	}
	PitchPullDown=RecoilAllAddedPitch/100;

	if (FMath::Sign(PlayerYawInput) != FMath::Sign(RecoilAllAddedYaw))
	{
		if (FMath::Abs(PlayerYawInput) < FMath::Abs(RecoilAllAddedYaw))
		{
			RecoilAllAddedYaw += PlayerYawInput;
		}
		else
		{
			RecoilAllAddedYaw = 0;
		}
	}
	YawPullDown=RecoilAllAddedYaw/100;
}

//Firing method on weapon reduces 1 ammo everytime its called
void AWeaponBase::WeaponFire()
{
	if (Player->bCanFire && bIsReadyToFire)
	{
		if (MagStatus().bHasAmmo)
		{
			bIsWeaponFiring = true;
			
			CurrentAmmoInMag--;

			Shoot();
			
			if (!bIsWeaponSuppressed)
			{
				UAISense_Hearing::ReportNoiseEvent(GetWorld(),GetActorLocation(),1,Player,0,FName("AI_Noise"));
				GunMesh->PlayAnimation(FireAnimation, false);
			}
			else
			{
				GunMesh->PlayAnimation(SuppressedFireAnimation,false);
				UAISense_Hearing::ReportNoiseEvent(GetWorld(),GetActorLocation(),1,Player,200,FName("AI_Noise"));
			}
			
			AddRecoil();
			NumberOfShot++;
			NumberOfShot=FMath::Clamp(NumberOfShot,0,MaxAmmoInMag-1);

			//Stops gun recoil animation and sets the weapon shooting readiness state
			if (!bIsWeaponAuto)
			{
				Player->CharacterFireWeapon.Broadcast(WeaponType);
				
				bIsReadyToFire = false;
				GetWorldTimerManager().SetTimer(ShootingDelayHandle, this, &AWeaponBase::SetWeaponState, FireRate, false);

				GetWorldTimerManager().SetTimer(StopFiringHandle, this, &AWeaponBase::CharacterStopFireWeapon, FireRate, false);
			}
			
			if (CameraShake)
			{
				CameraManager->StartCameraShake(CameraShake,3);
			}

			AmmoShellEject();
		}
		else
		{
			StopFire();
			Player->bCanFire = false;
			
			if (HasReservedAmmo())
			{
				Player->Reload();
			}
		}
	}
}

void AWeaponBase::AiWeaponFire()
{
	if (bIsReadyToFire)
	{
		bIsWeaponFiring = true;
		
		AiShoot();
		
		GunMesh->PlayAnimation(FireAnimation, false);
		
		bIsReadyToFire=false;
		GetWorldTimerManager().SetTimer(AutoFireHandle, this, &AWeaponBase::AiAutoFire, FireRate, false);
		
		AmmoShellEject();
	}
}

void AWeaponBase::Shoot()
{
}

void AWeaponBase::AiShoot()
{
}

void AWeaponBase::AiAutoFire()
{
	bIsReadyToFire=true;
	if (bIsWeaponFiring)
	{
		AiWeaponFire();
	}
}

void AWeaponBase::SetWeaponState()
{
	bIsReadyToFire=true;
	GetWorldTimerManager().ClearTimer(ShootingDelayHandle);
}

void AWeaponBase::CharacterStopFireWeapon()
{
	Player->CharacterStopFireWeapon.Broadcast();
	GetWorldTimerManager().ClearTimer(StopFiringHandle);
}

void AWeaponBase::SpawnImpactEffect(const FHitResult &HitResult) const
{
	const FTransform SpawnTransForm(FRotator(0, 0, 0), HitResult.ImpactPoint);
	AImpactEffect* ImpactEffect = Cast<AImpactEffect>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ImpactEffectBP, SpawnTransForm));

	if (ImpactEffect != nullptr)
	{
		ImpactEffect->initialize(HitResult, HitResult.bBlockingHit);
		UGameplayStatics::FinishSpawningActor(ImpactEffect, SpawnTransForm);
	}
}

void AWeaponBase::StopFire()
{
	if (bIsWeaponFiring)
	{
		Player->CharacterStopFireWeapon.Broadcast();

		CalculateReverseRecoil();

		GetWorldTimerManager().SetTimer(RecoilReverseHandle, this, &AWeaponBase::RevertRecoil, RecoilReverseSpeed, true);
		
		GetWorldTimerManager().SetTimer(RecoilResetHandle, this, &AWeaponBase::ResetRecoil, ResetRecoilRate, true);

		bIsWeaponFiring = false;
	}
}

void AWeaponBase::AiStopFire()
{
	if (bIsWeaponFiring)
	{
		bIsWeaponFiring = false;
	}
}

//Reload function that fills the mag
void AWeaponBase::Reload()
{
	//fill the mag full or fill with any ammo that's left
	const int32 ToFill= MaxAmmoInMag-CurrentAmmoInMag;
	
	if (CurrentReservedAmmo>=ToFill)
	{
		CurrentAmmoInMag=MaxAmmoInMag;
		CurrentReservedAmmo-=ToFill;
	}
	else
	{
		CurrentAmmoInMag+=CurrentReservedAmmo;
		CurrentReservedAmmo=0;
	}
}

bool AWeaponBase::HasReservedAmmo() const
{
	return CurrentReservedAmmo>0;
}

FMagStatus AWeaponBase::MagStatus() const
{
	FMagStatus Mag;
	CurrentAmmoInMag > 0 ? Mag.bHasAmmo = true : Mag.bHasAmmo = false;

	CurrentAmmoInMag == MaxAmmoInMag ? Mag.bMagFull = true : Mag.bMagFull = false;

	return Mag;
}



