// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalComponent.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Player/FPS_Character.h"
#include "CollisionChannels.h"
#include "Portal.h"
#include "Portal_Wall.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


const float PortalHalfHeight=125.0f;
const float PortalHalfWidth=90.0f;

// Sets default values for this component's properties
UPortalComponent::UPortalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPortalComponent::BeginPlay()
{
	Super::BeginPlay();

	PawnRef=Cast<AFPS_Character>(GetOwner());
	// ...
	
}


// Called every frame
void UPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPortalComponent::TrySpawnPortal(bool bPortalA)
{
	FVector StartLoc= PawnRef->FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLoc = StartLoc + PawnRef->FirstPersonCameraComponent->GetForwardVector() * PortalSpawnRange;

	FHitResult Hit;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PawnRef);
	CollisionParams.AddIgnoredActor(PawnRef->CurrentWeapon);
	CollisionParams.AddIgnoredActor(PortalA);
	CollisionParams.AddIgnoredActor(PortalB);
	
	GetWorld()->LineTraceSingleByChannel(Hit,StartLoc,EndLoc,ECC_Visibility,CollisionParams);

	if (bDebugLines)
	{
		DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Green, false, 5);
		DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(1, 1, 1), FColor::Cyan, false, 5,0,2);
	}

	
	if (APortal_Wall* PortalWall= Cast<APortal_Wall>(Hit.GetActor()))
	{
		//Find the relative location of the old portal to the wall
		FVector NewPortalRelLoc = UKismetMathLibrary::InverseTransformLocation(PortalWall->GetTransform(), Hit.ImpactPoint);
		FVector NewPortalRelClampedLoc = ClampPortalToWall(PortalWall, NewPortalRelLoc);

		//Handle intersection between portal A and B
		if (bPortalA&&PortalB)
		{
			HandleIntersect(bPortalA, PortalB, NewPortalRelClampedLoc, PortalWall);
		}
		else if (!bPortalA&&PortalA)
		{
			HandleIntersect(bPortalA, PortalA, NewPortalRelClampedLoc, PortalWall);
		}
		
		//Transform clamped location from relative to world location
		FVector PortalClampedWorldLoc = UKismetMathLibrary::TransformLocation(PortalWall->GetTransform(), NewPortalRelClampedLoc);

		//Spawn a portal on the clamped location
		APortal* SpawnedPortal = GetWorld()->SpawnActor<APortal>(PortalClass,PortalClampedWorldLoc,PortalWall->GetActorRotation());
		UGameplayStatics::PlaySound2D(GetWorld(),SpawnSound);

		//Initialize Portal
		if (bPortalA)
		{
			if (PortalA)
				PortalA->Destroy();
			
			PortalA=SpawnedPortal;
			PortalA->InitializePortal(true, PortalB, PortalWall);
		}
		else
		{
			if (PortalB)
				PortalB->Destroy();
			
			PortalB= SpawnedPortal;
			PortalB->InitializePortal(false, PortalA, PortalWall);
		}

		if (IsValid(PortalA))
			PortalA->LinkPortals(PortalB);
	

		if (IsValid(PortalB))
			PortalB->LinkPortals(PortalA); 
	}
}

FVector UPortalComponent::ClampPortalToWall(const APortal_Wall* PortalWall, const FVector& NewPortalRelLoc)
{
	//Find the Max value for Y and Z in which the portal will be inside the wall
	const float MaxZ = PortalWall->WallHeight / 2 - PortalHalfHeight;
	const float MaxY = PortalWall->WallWidth / 2 - PortalHalfWidth;


	//Clamp the Y and Z value so the portal always fits inside the wall
	const float CorrectZ = FMath::Clamp(NewPortalRelLoc.Z, -MaxZ, MaxZ);
	const float CorrectY = FMath::Clamp(NewPortalRelLoc.Y, -MaxY, MaxY);


	//Return relative clamped portal location
	return FVector(0, CorrectY, CorrectZ);
}


void UPortalComponent::HandleIntersect(bool bPortalA,APortal* OldPortal , const FVector NewPortalRelLoc, const APortal_Wall* PortalWall)
{
	//If we are not on the same wall dont do anything
	if (!(PortalWall== OldPortal->CurrentWall))
	{
		return;
	}

	//Find the relative location of the old portal to the wall
	const FVector OldPortalRelLoc = UKismetMathLibrary::InverseTransformLocation(PortalWall->GetTransform(), OldPortal->GetActorLocation());

	//Calculate New portals Min and Max points
	const FVector2D NewPortalMin(NewPortalRelLoc.Z - PortalHalfHeight, NewPortalRelLoc.Y - PortalHalfWidth);
	const FVector2D NewPortalMax(NewPortalRelLoc.Z + PortalHalfHeight, NewPortalRelLoc.Y + PortalHalfWidth);

	//Calculate Old portals Min and Max point
	const FVector2D OldPortalMin(OldPortalRelLoc.Z - PortalHalfHeight, OldPortalRelLoc.Y - PortalHalfWidth);
	const FVector2D OldPortalMax(OldPortalRelLoc.Z + PortalHalfHeight, OldPortalRelLoc.Y + PortalHalfWidth);

	//Make Box object using the min and max points of the portals
	const FBox2D NewPortalBox(NewPortalMin, NewPortalMax);
	const FBox2D OldPortalBox(OldPortalMin, OldPortalMax);

	//Find intersection between 2 boxes
	const bool bIntersect = NewPortalBox.Intersect(OldPortalBox);

	//If there is an intersection destroy the old portal
	if (bIntersect)
	{
		OldPortal->Destroy();
	}
}