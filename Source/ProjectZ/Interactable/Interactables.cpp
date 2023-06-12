// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables.h"
#include "Components/BoxComponent.h"


// Sets default values
AInteractables::AInteractables()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->CastShadow = false;
	Mesh->bCastDynamicShadow = false;
	RootComponent=Mesh;

	Box=CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetCollisionObjectType(ECC_GameTraceChannel1);
	Box->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AInteractables::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AInteractables::InteractAction(AFPS_Character* PlayerRef)
{
	return true;
}

// Called every frame
void AInteractables::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}