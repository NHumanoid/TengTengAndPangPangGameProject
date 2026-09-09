// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayCharacter.generated.h"

UCLASS()
class PANGGAMEPROJECT_API APlayCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayCharacter();

	virtual void BeginPlay();

	UPROPERTY(VisibleAnywhere, Category = "camera")
	//class USpringArmComponent* SpringArmComponent;
	TObjectPtr<class USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "camera")
	TObjectPtr<class UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UPlayerInputConfigDataAsset* PlayerInputConfig;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* EquippedWeaponMesh;

	/*UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookAction;*/

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void InputActionMove(const struct FInputActionValue& Value);
	void JumpActionMove(const struct FInputActionValue& Value);
	void LookActionMove(const struct FInputActionValue& Value);
};
