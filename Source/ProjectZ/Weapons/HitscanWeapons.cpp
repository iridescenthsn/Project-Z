// Fill out your copyright notice in the Description page of Project Settings.


#include "HitscanWeapons.h"
#include "../Effects/Tracer.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void AHitscanWeapons::Shoot()
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

void AHitscanWeapons::AiShoot()
{
	const FHitResult HitResult = AiCalculateShot();

	const FVector TracerTargetLoc= HitResult.bBlockingHit ? HitResult.Location : HitResult.TraceEnd;

	ATracer* Tracer = GetWorld()->SpawnActor<ATracer>(TracerClass,GunMesh->GetSocketLocation(FName("MuzzleFlash")),
		UKismetMathLibrary::FindLookAtRotation(GunMesh->GetSocketLocation(FName("MuzzleFlash")),TracerTargetLoc));
	
	Tracer->SetLifeSpan(HitResult.Distance/Tracer->GetMovementComponent()->GetMaxSpeed());

	if (HitResult.bBlockingHit)
	{
		AddDamage(HitResult,GetAttachParentActor()->Tags[0],AmmoData.Damage);
	}

	SpawnImpactEffect(HitResult);
}
