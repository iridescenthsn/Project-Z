// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuyableObject.h"
#include "AmmoBuy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AAmmoBuy : public ABuyableObject
{
	GENERATED_BODY()

	virtual bool InteractAction(AFPS_Character* PlayerRef) override;

	virtual void BuyAction(AFPS_Character* PlayerRef) override;
};
