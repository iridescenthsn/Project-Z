// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeLauncherProjectile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Interfaces/TakeDamage.h"
#include "Kismet/GameplayStatics.h"
#include "../Effects/GrenadeImpactEffect.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "DrawDebugHelpers.h"



void AGrenadeLauncherProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != this)
	{
		/*
		* on hit find the angle between surface normal and projectile 
		* if its more than activation angle then explode
		* if its less
		* bounce and set timer on first bounce to explode
		*/
		const float VectorDotProduct = FVector::DotProduct(Hit.ImpactNormal, GetActorForwardVector());
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(VectorDotProduct));
		
		if (Angle >= ActivationAngle)
		{
			Explode(Hit);
		}
		else
		{
			//play bounce sound
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), BounceSound, Hit.ImpactPoint,BounceVolumeMultiplier);

			if (!GetWorldTimerManager().IsTimerActive(ExplsionHandle))
			{
				ExplosionDelegate.BindUFunction(this, FName(TEXT("Explode")), Hit);
				GetWorldTimerManager().SetTimer(ExplsionHandle, ExplosionDelegate, ExplosionTime, false);
			}
		}
	}
}


void AGrenadeLauncherProjectile::Explode(const FHitResult& Hit)
{
	AddDamageAtLocation(Hit,GetActorLocation());
	SpawnImpactEffectAtLocation(Hit, GetActorLocation()); 
	Destroy();
}

void AGrenadeLauncherProjectile::AddDamageAtLocation(const FHitResult& Hit, const FVector& ExplosiveLocation)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	TArray<AActor*> ignoreActors;
	ignoreActors.Init(this, 1);

	TArray<AActor*> OutActors;

	//make a sphere and find overlapping actors to apply damage to
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), ExplosiveLocation, AmmoData.DamageRadius, ObjectTypes, nullptr, ignoreActors, OutActors);

	//DrawDebugSphere(GetWorld(), GetActorLocation(), AmmoData.DamageRadius, 12, FColor::Red, true, 10.0f);
	
	ExplosionForce->FireImpulse();

	//apply damage to actors who have TakeDamage interface
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

				const bool bHit = GetWorld()->LineTraceSingleByObjectType(LineTraceHit,ExplosiveLocation,OverLapActor->GetActorLocation(),Params);

				if (!bHit)
				{
					TakeDamageInterface->TakeRadialDamage(AmmoData, CriticalHitModifier, Hit,ExplosiveLocation);
				}
			}
		}
	}
}

void AGrenadeLauncherProjectile::SpawnImpactEffectAtLocation(const FHitResult& HitResult, const FVector& ExplosiveLocation) const
{
	const FTransform SpawnTransForm(FRotator(0, 0, 0), ExplosiveLocation);
	AGrenadeImpactEffect* ImpactEffect = Cast<AGrenadeImpactEffect>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), GrenadeImpactEffectBP, SpawnTransForm));

	if (ImpactEffect != nullptr)
	{
		ImpactEffect->initializeParams(HitResult, HitResult.bBlockingHit, ExplosiveLocation);
		UGameplayStatics::FinishSpawningActor(ImpactEffect, SpawnTransForm);
	}
}

