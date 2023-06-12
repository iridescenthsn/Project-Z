// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Interfaces/TakeDamage.h"
#include "../Effects/ImpactEffect.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectZ/CollisionChannels.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetSphereRadius(8.0f);
	Sphere->bReturnMaterialOnMove = true;
	Sphere->SetHiddenInGame(true);
	RootComponent=Sphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Sphere);
	Mesh->SetHiddenInGame(true);
	Mesh->CastShadow = false;
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 5000.0f;
	ProjectileMovement->MaxSpeed = 5000.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bShouldBounce = true;

	Sphere->OnComponentHit.AddUniqueDynamic(this, &AProjectile::OnHit);

	this->InitialLifeSpan = 5.0f;

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this,&AProjectile::OnSphereBeginOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this,&AProjectile::OnSphereEndOverlap);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor!=this)
	{
		AddDamage(Hit);
		SpawnImpactEffect(Hit);
		Destroy();
	}
}

void AProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor!=this)
	{
		if (OtherComp->GetCollisionObjectType()==TraceChannel_Portal)
			return;
		
		//When overlapping with a penetrable wall spawn an impact effect and save the impact location
		if (OtherComp->GetCollisionObjectType()==TraceChannel_Penetrable)
		{
			++WallsPassed;
			HitLocation=SweepResult.Location;
			SpawnImpactEffect(SweepResult);
		}
	}
}

void AProjectile::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor!=this)
	{
		if (OtherComp->GetCollisionObjectType()==TraceChannel_Portal)
			return;
		
		if (OtherComp->GetCollisionObjectType()==TraceChannel_Penetrable)
		{
			FCollisionObjectQueryParams ObjectsToTrace;
			ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_Penetrable);

			//Add player as ignored actor for line trace
			FCollisionQueryParams ActorToIgnore;
			ActorToIgnore.bReturnPhysicalMaterial = true;
			
			FHitResult HitResult;

			const FVector Startloc=GetActorLocation()+ GetActorForwardVector()*20;
			const FVector Endloc=GetActorLocation()+ GetActorForwardVector()*(-200);

			//Draw a line trace to find the location of the bullet coming out of the wall
			GetWorld()->LineTraceSingleByObjectType(HitResult,Startloc,Endloc,ObjectsToTrace,ActorToIgnore);

			/*DrawDebugLine(GetWorld(), Startloc, Endloc, FColor::Green, false, 4);
			DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 4);
			*/

			//find the thickness of the actor. if too thick then destroy otherwise go through
			const float ActorThickness = (HitLocation-HitResult.Location).Size();
			
			if (ActorThickness<=MaxPenetrate)
			{
				SpawnImpactEffect(HitResult);
			}
			else
			{
				Destroy();
			}
		}
	}
}

void AProjectile::AddDamage(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (HitActor)
	{
		ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(HitActor);
		if (TakeDamageInterface)
		{
			FAmmoData LocAmmoData{AmmoData};
			LocAmmoData.Damage=AmmoData.Damage * FMath::Pow(PenetrationDamageMultiplier,WallsPassed);
			TakeDamageInterface->TakeDamage(LocAmmoData, CriticalHitModifier, Hit);
		}
	}
}

void AProjectile::SpawnImpactEffect(const FHitResult& HitResult)
{
	FTransform SpawnTransForm(FRotator(0, 0, 0), HitResult.ImpactPoint);
	AImpactEffect* ImpactEffect = Cast<AImpactEffect>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ImpactEffectBP, SpawnTransForm));

	if (ImpactEffect != nullptr)
	{
		ImpactEffect->initialize(HitResult, HitResult.bBlockingHit);
		UGameplayStatics::FinishSpawningActor(ImpactEffect, SpawnTransForm);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

