// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuyableObject.h"
#include "ArmorBuy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AArmorBuy : public ABuyableObject
{
	GENERATED_BODY()

	virtual void BuyAction(AFPS_Character* PlayerRef) override;

	virtual bool InteractAction(AFPS_Character* PlayerRef) override;
};
