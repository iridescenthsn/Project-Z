// Fill out your copyright notice in the Description page of Project Settings.


#include "AutomaticDoors.h"

#include "NavModifierComponent.h"
#include "../AI/ZombieSpawner.h"

AAutomaticDoors::AAutomaticDoors()
{
	SceneRoot= CreateDefaultSubobject<USceneComponent>(FName("SceneRoot"));
	RootComponent=SceneRoot;
	SceneRoot->SetHiddenInGame(true);
	SceneRoot->SetCanEverAffectNavigation(false);

	LeftDoor= CreateDefaultSubobject<UStaticMeshComponent>(FName("LeftDoor"));
	LeftDoor->SetupAttachment(RootComponent);
	LeftDoor->Mobility=TEnumAsByte<EComponentMobility::Type>(EComponentMobility::Movable);
	LeftDoor->SetCanEverAffectNavigation(false);

	RightDoor= CreateDefaultSubobject<UStaticMeshComponent>(FName("RightDoor"));
	RightDoor->SetupAttachment(RootComponent);
	RightDoor->Mobility=TEnumAsByte<EComponentMobility::Type>(EComponentMobility::Movable);
	RightDoor->SetCanEverAffectNavigation(false);

	Mesh->SetupAttachment(RootComponent);
	
	NavModifier= CreateDefaultSubobject<UNavModifierComponent>(FName("Nav Modifier"));
}

void AAutomaticDoors::BuyAction(AFPS_Character* PlayerRef)
{
	GetWorldTimerManager().SetTimer(Handle,[&]
	{
		//Opens the door by moving
		LeftDoor->AddLocalOffset(FVector(1.0f,0.0f,0.0f));
		RightDoor->AddLocalOffset(FVector(1.0f,0.0f,0.0f));
		AllAddedOffset+=1.0f;
		if (AllAddedOffset>=200.0f)
		{
			if (TheSpawner)
			{
				TheSpawner->SetIsActive(true);
			}
			
			GetWorldTimerManager().ClearTimer(Handle);
			Destroy();
		}
	},0.01,true);
}
