// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables.h"
#include "HideBodyBox.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZ_API AHideBodyBox : public AInteractables
{
	GENERATED_BODY()

public:
	virtual bool InteractAction(AFPS_Character* PlayerRef) override;
};
