// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "../Player/FPS_Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Effects/Tracer.h"
#include "GameFramework/ProjectileMovementComponent.h"

AShotGun::AShotGun()
{
	bIsWeaponAuto = false;
}

void AShotGun::BeginPlay()
{
	Super::BeginPlay();
}

void AShotGun::Shoot()
{
	for (size_t j = 0; j < NumberOfPallets; j++)
	{
		TArray<FHitResult> InGoingShots;
		TArray<FHitResult> OutGoingShots;
		FVector TraceEnd;
		CalculateShot(InGoingShots,OutGoingShots,TraceEnd);

		//Shoots a cosmetic bullet tracer out of the guns barrel
		const FVector TracerTargetLoc= InGoingShots.Num()>0 ? InGoingShots.Top().ImpactPoint : TraceEnd;
		ATracer* Tracer = GetWorld()->SpawnActor<ATracer>(TracerClass,GunMesh->GetSocketLocation(FName("MuzzleFlash")),
			UKismetMathLibrary::FindLookAtRotation(GunMesh->GetSocketLocation(FName("MuzzleFlash")),TracerTargetLoc));
		Tracer->SetLifeSpan(2);

		
		for (size_t i=0 ; i<InGoingShots.Num() ; ++i)
		{
			//if the actor is penetrable 
			if (InGoingShots[i].GetComponent()->GetCollisionObjectType()==ECC_GameTraceChannel5)
			{
				//if the the bullet can penetrate, spawn impact then continue, else stop checking hit points and spawn impact
				const float ActorThickness = (InGoingShots[i].ImpactPoint-OutGoingShots[OutGoingShots.Num()-(i+1)].ImpactPoint).Size();
				UE_LOG(LogTemp,Warning,TEXT("Thickness : %f"),ActorThickness)
				if (ActorThickness<=MaxPenetrate)
				{
					SpawnImpactEffect(InGoingShots[i]);
					SpawnImpactEffect(OutGoingShots[OutGoingShots.Num()-(i+1)]);
					continue;
				}
				else
				{
					SpawnImpactEffect(InGoingShots[i]);
					Tracer->SetLifeSpan(InGoingShots[i].Distance/Tracer->GetMovementComponent()->GetMaxSpeed());
					break;
				}
			}
			else
			{
				//if not penetrable then try add damage
				//for every wall we pass through multiply damage by given multiplier
				AddDamage(InGoingShots[i],FName("Player"),AmmoData.Damage * FMath::Pow(PenetrationDamageMultiplier,i));
				SpawnImpactEffect(InGoingShots[i]);
				Tracer->SetLifeSpan(InGoingShots[i].Distance/Tracer->GetMovementComponent()->GetMaxSpeed());
				break;
			}
		}
	}
}

void AShotGun::AiShoot()
{
	for (size_t i=0;i<NumberOfPallets;++i)
	{
		const FHitResult Hit = AiCalculateShot();

		//Shoots a cosmetic bullet tracer out of the guns barrel
		const FVector TracerTargetLoc= Hit.ImpactPoint;
		ATracer* Tracer = GetWorld()->SpawnActor<ATracer>(TracerClass,GunMesh->GetSocketLocation(FName("MuzzleFlash")),
			UKismetMathLibrary::FindLookAtRotation(GunMesh->GetSocketLocation(FName("MuzzleFlash")),TracerTargetLoc));

		Tracer->SetLifeSpan(Hit.Distance/Tracer->GetMovementComponent()->GetMaxSpeed());

		if (Hit.bBlockingHit)
		{
			AddDamage(Hit,FName("EnemyAI"),AmmoData.Damage);
		}

		SpawnImpactEffect(Hit);
	}
}
