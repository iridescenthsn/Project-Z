// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieGameMode.h"
#include "../AI/TurretAIController.h"
#include "../AI/ZombieSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"

void AZombieGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(),SpawnerClass,Spawners);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),TurretAiControllerClass,TurretAIControllers);

	if (Spawners.Num()>0)
	{
		NextRound();
	}
}

void AZombieGameMode::SpawnZombies()
{
	if (NumberOfZombiesToSpawn>ZombiesSpawnedThisRound&& MaxAliveAtTime>=ZombiesAlive)
	{
		int32 RandomSpawnerIndex;
		while (true)
		{
			RandomSpawnerIndex = FMath::RandRange(0,Spawners.Num()-1);
			if (Cast<AZombieSpawner>(Spawners[RandomSpawnerIndex])->GetIsActive())
			{
				break;
			}
		}
		
		Cast<AZombieSpawner>(Spawners[RandomSpawnerIndex])->SpawnZombies();
		ZombiesSpawnedThisRound++;
		ZombiesAlive++;

		UpdateRoundUI.Broadcast(CurrentRound,ZombiesAlive);
		
		GetWorldTimerManager().SetTimer(SpawnerHandle,this,&AZombieGameMode::SpawnZombies,SpawningRate,true);
	}
}

void AZombieGameMode::NextRound()
{
	CurrentRound++;
	NumberOfZombiesToSpawn= CurrentRound*2+9;
	ZombiesSpawnedThisRound=0;
	UpdateRoundUI.Broadcast(CurrentRound,ZombiesAlive);

	FTimerHandle StartRoundHandle; 
	GetWorldTimerManager().SetTimer(StartRoundHandle,this,&AZombieGameMode::SpawnZombies,TimeBetweenRounds);
}

void AZombieGameMode::IsRoundOver()
{
	if (ZombiesSpawnedThisRound>=NumberOfZombiesToSpawn&&ZombiesAlive<=0)
	{
		GetWorldTimerManager().ClearTimer(SpawnerHandle);
		if (Spawners.Num()>0)
		{
			NextRound();
		}
	}
}

void AZombieGameMode::UpdateAiPerception()
{
	for (const auto TurretController : TurretAIControllers)
	{
		const auto controller= Cast<ATurretAIController>(TurretController);
		if (controller)
		{
			controller->AIPerception->ForgetAll();
		}
	}
}