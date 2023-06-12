// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"

// Sets default values
APortalActor::APortalActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh=CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("StaticMesh")));
	RootComponent=StaticMesh;

	StaticMesh->SetCollisionProfileName(FName(TEXT("PortalableActor")));
	StaticMesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

