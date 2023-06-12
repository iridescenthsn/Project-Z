// Fill out your copyright notice in the Description page of Project Settings.


#include "Zombie_AI_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"


AZombie_AI_Controller::AZombie_AI_Controller()
{
	
}

void AZombie_AI_Controller::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(TheTree);
	const auto Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	if(Player)
	{
		GetBlackboardComponent()->SetValueAsObject(FName("Player"),Player);
	}
	
}

