// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPS_Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectZ/Portal/PortalComponent.h"
#include "../Portal/Portal.h"
#include "ProjectZ/CollisionChannels.h"


// Sets default values for this component's properties
UClimbingComponent::UClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

	PawnRef=Cast<AFPS_Character>(GetOwner());

	//setup the climbing timeline
	if (ClimbCurve)
	{
		FOnTimelineFloat ClimbTimelineFloat;
		ClimbTimelineFloat.BindUFunction(this,FName("SetClimbAlpha"));
		ClimbTimeline.AddInterpFloat(ClimbCurve,ClimbTimelineFloat);

		FOnTimelineEvent ClimbFinishedFunction;
		ClimbFinishedFunction.BindUFunction(this,FName("ClimbFinished"));
		ClimbTimeline.SetTimelineFinishedFunc(ClimbFinishedFunction);
	}
	
}


// Called every frame
void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ClimbTimeline.TickTimeline(DeltaTime);
}

bool UClimbingComponent::CanClimb()
{
	if (bIsClimbing)
	{
		return false;
	}

	auto CapsuleRef= PawnRef->GetCapsuleComponent();

	FVector Startloc;
	FVector Endloc;

	Startloc = PawnRef->GetActorLocation() + (PawnRef->GetActorForwardVector()*ClimbingRange) +
		 FVector(0,0,ClimbCheckHeight);
	
	Endloc = PawnRef->GetActorLocation() + (PawnRef->GetActorForwardVector()*ClimbingRange) +
	 FVector(0,0,-CapsuleRef->GetScaledCapsuleHalfHeight());

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PawnRef);
	CollisionParams.AddIgnoredActor(PawnRef->CurrentWeapon);
	CollisionParams.AddIgnoredActor(PawnRef->PortalComponent->PortalA);
	CollisionParams.AddIgnoredActor(PawnRef->PortalComponent->PortalB);

	FCollisionObjectQueryParams ObjectsToTrace;
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectsToTrace.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectsToTrace.AddObjectTypesToQuery(TraceChannel_Penetrable);

	FHitResult Hit;

	//Do a line trace in front of character to check for surfaces existence
	bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit,Startloc,Endloc,ObjectsToTrace,CollisionParams);

	/*DrawDebugLine(GetWorld(), Startloc, Endloc, FColor::Green, false, 0);
	DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), FColor::Cyan, false, 0);*/
	
	if (!bHit)
	{
		return false;
	}
	
	return CanClimbHit(Hit,CapsuleRef);
	
}


bool UClimbingComponent::CanClimbHit(const FHitResult& Hit, UCapsuleComponent* CapsuleRef)
{
	bool CanClimb=UKismetMathLibrary::InRange_FloatFloat(Hit.Location.Z-Hit.TraceEnd.Z,MinClimbingHeight,MaxClimbingHeight);

	//too high or too low return false
	if (!CanClimb)
	{
		return false;
	}

	CanClimb= Hit.Normal.Z>PawnRef->GetCharacterMovement()->GetWalkableFloorZ();
	
	//too steep return false
	if(!CanClimb)
	{
		return false;
	}
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Vehicle));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Destructible));
	
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(PawnRef, 1);
 
	TArray<AActor*> OutActors;

	FVector LocEndingLocation = Hit.Location+ FVector(0,0,CapsuleRef->GetScaledCapsuleHalfHeight());


	//Check for collisions to see if the character can fit if we climb (standing up)
	CanClimb=!UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(),LocEndingLocation+FVector(0,0,20.0f),
		CapsuleRef->GetScaledCapsuleRadius(),CapsuleRef->GetScaledCapsuleHalfHeight(),ObjectTypes,nullptr,
		IgnoreActors,OutActors);

	//if there is a collision if character stands up in that surface check if there is a collision if we crouch
	//if there is no collision while crouching then crouch the character
	if (!CanClimb)
	{
		LocEndingLocation = Hit.Location+ FVector(0,0,PawnRef->GetCrouchHalfHeight());
		
		CanClimb=!UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(),LocEndingLocation+FVector(0,0,10),
		CapsuleRef->GetScaledCapsuleRadius(),PawnRef->GetCrouchHalfHeight(),ObjectTypes,nullptr,
		IgnoreActors,OutActors);
		
		if (CanClimb)
		{
			PawnRef->crouch();
		}

		/*DrawDebugCapsule(GetWorld(),LocEndingLocation+FVector(0,0,10),PawnRef->GetCrouchHalfHeight(),
		CapsuleRef->GetScaledCapsuleRadius(),FQuat::Identity,FColor::Green,false,5);*/
	}

	/*DrawDebugCapsule(GetWorld(),LocEndingLocation+FVector(0,0,20.0f),CapsuleRef->GetScaledCapsuleHalfHeight(),
		CapsuleRef->GetScaledCapsuleRadius(),FQuat::Identity,FColor::Green,false,5);*/

	if (CanClimb)
	{
		EndingLocation=LocEndingLocation;
		StartingLocation=PawnRef->GetActorLocation();
	}
	
	return CanClimb;
}


void UClimbingComponent::Climb()
{
	bIsClimbing=true;
	PawnRef->GetMesh1P()->SetHiddenInGame(true);
	
	if (PawnRef->CurrentWeapon)
	{
		PawnRef->CurrentWeapon->SetActorHiddenInGame(true);
	}
	
	ClimbTimeline.PlayFromStart();
}

void UClimbingComponent::SetClimbAlpha(float value)
{
	const FVector Location = FMath::Lerp(StartingLocation,EndingLocation,value);
	PawnRef->SetActorLocation(Location);
}

void UClimbingComponent::ClimbFinished()
{
	bIsClimbing=false;
	PawnRef->GetMesh1P()->SetHiddenInGame(false);
	if (PawnRef->CurrentWeapon)
	{
		PawnRef->CurrentWeapon->SetActorHiddenInGame(false);
	}
}



