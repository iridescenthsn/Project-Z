// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables.h"
#include "BuyableObject.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API ABuyableObject : public AInteractables
{
	GENERATED_BODY()


protected:
	
	virtual bool InteractAction(AFPS_Character* PlayerRef) override;
	
	virtual void BuyAction(AFPS_Character* PlayerRef);

public:
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Buy")
	int32 Cost;
};
