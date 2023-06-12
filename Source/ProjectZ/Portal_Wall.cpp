// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal_Wall.h"

// Sets default values
APortal_Wall::APortal_Wall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	RootComponent=SceneRoot;
	
	WallMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall mesh"));
	WallMesh->SetupAttachment(RootComponent);
	WallMesh->CastShadow=false;
}

// Called when the game starts or when spawned
void APortal_Wall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortal_Wall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

