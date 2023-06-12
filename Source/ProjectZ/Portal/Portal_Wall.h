// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal_Wall.generated.h"

UCLASS()
class PROJECTZ_API APortal_Wall : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WallMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

public:	
	// Sets default values for this actor's properties
	APortal_Wall();

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Size")
	float WallHeight=100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Size")
	float WallWidth=100.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Ignore")
	TArray<AActor*> IgnoredWalls;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
