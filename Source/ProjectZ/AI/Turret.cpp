// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"

#include "../Effects/ImpactEffect.h"
#include "../Effects/PistolAmmoShell.h"
#include "../Interfaces/TakeDamage.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Effects/Tracer.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "TurretAIController.h"
#include "Perception/AIPerceptionComponent.h"


// Sets default values
ATurret::ATurret()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Arrow=CreateDefaultSubobject<UArrowComponent>(FName("Arrow"));
	RootComponent=Arrow;
	
	TurretBody=CreateDefaultSubobject<UStaticMeshComponent>(FName("TurretBodyMesh"));
	TurretBody->SetupAttachment(Arrow);
}

void ATurret::FriendlyMode()
{
	StopFocusTarget();
	bIsFriendly=true;
	Tags[0]= FName("Player");
	Cast<ATurretAIController>(GetController())->AIPerception->ForgetAll();
}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();

	OriginalRotation=GetActorRotation();

	if (WeaponClass)
	{
		SpawnWeapon();
	}
}

// Called every frame
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATurret::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AiWeaponFire();
	}
}

void ATurret::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AiStopFire();
	}
}

void ATurret::StartFocusTarget(AActor* Target)
{
	FocusTarget=Target;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle,[&]
	{
		if (FocusTarget)
		{
			StartFire();
		}
	},0.3,false);
	
	GetWorldTimerManager().SetTimer(FocusHandle, this, &ATurret::Turn, 0.01, true);
}

void ATurret::StopFocusTarget()
{
	GetWorldTimerManager().ClearTimer(FocusHandle);
	StopFire();
	FocusTarget=nullptr;
	GetWorldTimerManager().SetTimer(FocusHandle, this, &ATurret::TurnToOriginalRot, 0.01, true);
}

void ATurret::Turn()
{
	if (!FocusTarget)
	{
		return;
	}

	const auto Rotation =UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),FocusTarget->GetActorLocation());
	const auto DeltaRot=Rotation-GetActorRotation();
	const float YawToTurn=FMath::Clamp(DeltaRot.Yaw,-1.0f,1.0f);
	const float PitchToTurn=FMath::Clamp(DeltaRot.Pitch,-1.0f,1.0f);
	const float RollToTurn=FMath::Clamp(DeltaRot.Roll,-1.0f,1.0f);

	
	if (FMath::Abs(DeltaRot.Yaw)<180)
	{
		SetActorRotation(GetActorRotation()+FRotator(PitchToTurn,YawToTurn,RollToTurn));
	}
	else
	{
		SetActorRotation(GetActorRotation()+FRotator(PitchToTurn,-YawToTurn,RollToTurn));
	}
}

void ATurret::TurnToOriginalRot()
{
	const auto DeltaRot=OriginalRotation-GetActorRotation();
	const float YawToTurn=FMath::Clamp(DeltaRot.Yaw,-1.0f,1.0f);
	const float PitchToTurn=FMath::Clamp(DeltaRot.Pitch,-1.0f,1.0f);
	const float RollToTurn=FMath::Clamp(DeltaRot.Roll,-1.0f,1.0f);

	
	if (FMath::Abs(DeltaRot.Yaw)<180)
	{
		SetActorRotation(GetActorRotation()+FRotator(PitchToTurn,YawToTurn,RollToTurn));
	}
	else
	{
		SetActorRotation(GetActorRotation()+FRotator(PitchToTurn,-YawToTurn,RollToTurn));
	}
	
	if (FMath::Abs(DeltaRot.Yaw)<=1.0f&&FMath::Abs(DeltaRot.Roll)<=1.0f&&FMath::Abs(DeltaRot.Pitch)<=1.0f)
	{
		GetWorldTimerManager().ClearTimer(FocusHandle);
	}
}

void ATurret::SpawnWeapon()
{
	CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
	CurrentWeapon->AttachToComponent(TurretBody, FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
		true), FName("GripPoint"));
}



