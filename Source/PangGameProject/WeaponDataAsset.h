// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	NormalGun UMETA(DisplayName = "Normal"),
	Shotgun UMETA(DisplayName = "Shot"),
	MachineGun UMETA(DisplayName = "Machine"),
	Boom UMETA(DisplayName = "Boom")
};

UCLASS()
class PANGGAMEPROJECT_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::NormalGun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMesh* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 Damage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FTransform AttachTransform = FTransform::Identity;
};
