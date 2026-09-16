
#pragma once

#include "WeaponDataAsset.h"
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

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class ABulletProjectile> BulletClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName MuzzleSocketName = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "1.0", Units = "cm"))
    float AimDistance = 10000.0f;

	void TakeNormalGun();
	void TakeShotGun();
	void TakeMachineGun();
	void TakeBoom();

	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void InputActionMove(const struct FInputActionValue& Value);
	void JumpActionMove(const struct FInputActionValue& Value);
	void LookActionMove(const struct FInputActionValue& Value);

	void InputActionSniper(const struct FInputActionValue& Value);
	void InputActionGrenade(const struct FInputActionValue& Value);
	void InputActionMachine(const struct FInputActionValue& Value);
	void InputActionBoom(const struct FInputActionValue& Value);
	void InputActionFire(const struct FInputActionValue& Value);
};
