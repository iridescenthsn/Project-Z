// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectZGameModeBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameFinished);

/**
 * 
 */
UCLASS()
class PROJECTZ_API AProjectZGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable)
	FOnGameFinished GameFinished;

private:
	
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TArray<AActor*> CharacterAIControllers;

	UPROPERTY()
	TArray<AActor*> TurretAIControllers;

	void CheckGameFinished();

protected:
	
	UPROPERTY(EditDefaultsOnly,Category="Ai class")
	TSubclassOf<AActor> CharacterAIControllerClass;

	UPROPERTY(EditDefaultsOnly,Category="Ai class")
	TSubclassOf<AActor> TurretAiControllerClass;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	int32 NumberOfEnemies=0;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	int32 DeadEnemies=0;

public:
	
	void UpdateAiPerception();
	
	UFUNCTION(BlueprintCallable)
	void EnemyDied();
	
};
