// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowingObjects.h"

#include "../Interfaces/TakeDamage.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

void AThrowingObjects::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
                             FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor!=this)
	{
		AddDamage(Hit);
	}
}

//add damage if its an enemy else break and make noise
void AThrowingObjects::AddDamage(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (HitActor)
	{
		ITakeDamage* TakeDamageInterface = Cast<ITakeDamage>(HitActor);
		if (TakeDamageInterface)
		{
			TakeDamageInterface->TakeDamage(AmmoData,0,Hit);
		}
		else
		{
			UAISense_Hearing::ReportNoiseEvent(GetWorld(),Hit.Location,1,
				UGameplayStatics::GetPlayerPawn(GetWorld(),0),0,FName("AI_Noise"));
		}
	}
}

AThrowingObjects::AThrowingObjects()
{
	InitialLifeSpan=0;
}
