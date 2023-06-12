// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosiveProjectile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Interfaces/TakeDamage.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "ProjectZ/CollisionChannels.h"


AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionForce = CreateDefaultSubobject<URadialForceComponent>(FName("ExplsionForce"));
	ExplosionForce->SetupAttachment(Sphere);
}

void AExplosiveProjectile::AddDamage(const FHitResult& Hit)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(this, 1);

	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Hit.ImpactPoint, AmmoData.DamageRadius, ObjectTypes, nullptr, IgnoreActors, OutActors);

	for (size_t i = 0; i < OutActors.Num(); i++)
	{
		AActor* OverLapActor = OutActors[i];
		if (OverLapActor)
		{
			//if there is a hit check if its a damageable actor,
			//if so do a line trace from explosion location to actor to see if there is anything between them
			ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(OverLapActor); 
			if (TakeDamageInterface)
			{
				FCollisionObjectQueryParams Params;
				Params.AddObjectTypesToQuery(ECC_WorldDynamic);
				Params.AddObjectTypesToQuery(ECC_WorldStatic);
				Params.AddObjectTypesToQuery(ECC_GameTraceChannel1);
				Params.AddObjectTypesToQuery(ECC_GameTraceChannel5);
				
				FHitResult LineTraceHit;

				const bool bHit = GetWorld()->LineTraceSingleByObjectType(LineTraceHit,Hit.ImpactPoint,OverLapActor->GetActorLocation(),Params);

				if (!bHit)
				{
					TakeDamageInterface->TakeRadialDamage(AmmoData, CriticalHitModifier, Hit,Hit.ImpactPoint);
				}
				
			}
		}
	}
	
	ExplosionForce->FireImpulse();
}

void AExplosiveProjectile::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AExplosiveProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->GetCollisionObjectType()==TraceChannel_Portal)
		return;
	//On overlap with penetrable wall treat it as a Hit
	OnHit(OverlappedComp,OtherActor,OtherComp,FVector(0,0,0),SweepResult);
}
