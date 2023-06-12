// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuyableObject.h"
#include "AutomaticDoors.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AAutomaticDoors : public ABuyableObject
{
	GENERATED_BODY()
public:

	AAutomaticDoors();

private:

	virtual void BuyAction(AFPS_Character* PlayerRef) override;

	float AllAddedOffset=0.0f;
	
	FTimerHandle Handle;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Spawner")
	class AZombieSpawner* TheSpawner;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LeftDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	class UNavModifierComponent* NavModifier;
	
};
