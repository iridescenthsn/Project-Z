// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_AttackMelee.h"
#include "AIController.h"
#include "../AI/AI_Character.h"

EBTNodeResult::Type UAI_AttackMelee::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAI_Character* AiCharacter=Cast<AAI_Character>(OwnerComp.GetAIOwner()->GetPawn());
	if (AiCharacter)
	{
		AiCharacter->Melee();      
	}
	
	return EBTNodeResult::Succeeded; 
}
