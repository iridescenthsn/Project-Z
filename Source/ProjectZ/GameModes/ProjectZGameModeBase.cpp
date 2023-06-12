// Copyright Epic Games, Inc. All Rights Reserved.


#include "ProjectZGameModeBase.h"

#include "../AI/Solider_AI_Controller.h"
#include "../AI/TurretAIController.h"
#include "../AI/ZombieSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"

void AProjectZGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),CharacterAIControllerClass,CharacterAIControllers);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),TurretAiControllerClass,TurretAIControllers);
}


void AProjectZGameModeBase::UpdateAiPerception()
{
	for (const auto AiController : CharacterAIControllers)
	{
		const auto controller = Cast<ASolider_AI_Controller>(AiController);
		if (controller)
		{
			controller->AIPerception->ForgetAll();
		}
	}
	
	for (const auto TurretController : TurretAIControllers)
	{
		const auto controller= Cast<ATurretAIController>(TurretController);
		if (controller)
		{
			controller->AIPerception->ForgetAll();
		}
	}
}

void AProjectZGameModeBase::EnemyDied()
{
	++DeadEnemies;

	CheckGameFinished();
}

void AProjectZGameModeBase::CheckGameFinished()
{
	if (DeadEnemies >= CharacterAIControllers.Num())
	{
		GameFinished.Broadcast();
	}
}
