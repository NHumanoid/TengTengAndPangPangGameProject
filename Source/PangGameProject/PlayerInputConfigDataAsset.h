// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerInputConfigDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PANGGAMEPROJECT_API UPlayerInputConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "Input|Context")
    class UInputMappingContext* InputMappingContext;

    // Input Actions
    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* NormalGunAction;

    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* ShotGunAction;
	
    UPROPERTY(EditAnywhere, Category = "Input|Actions")
    class UInputAction* MachineGunAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	class UInputAction* BoomAction;

	UPROPERTY(EditAnywhere, Category = "Input|Actions")
	class UInputAction* FireAction;
};
