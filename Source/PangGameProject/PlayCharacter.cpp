// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayCharacter.h"
#include "BulletProjectile.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "PlayerInputConfigDataAsset.h"

// Sets default values
APlayCharacter::APlayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	BulletClass = ABulletProjectile::StaticClass();

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 40.0f, 70.0f));
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	// ¹«±â
	EquippedWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquippedWeaponMesh"));
	EquippedWeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
	EquippedWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedWeaponMesh->SetVisibility(false);

}

void APlayCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (PlayerInputConfig && PlayerInputConfig->InputMappingContext)
			{
				Subsystem->AddMappingContext(PlayerInputConfig->InputMappingContext, 0);
			}
		}
	}
}

void APlayCharacter::TakeNormalGun()
{

}

void APlayCharacter::TakeShotGun()
{

}

void APlayCharacter::TakeMachineGun()
{

}

void APlayCharacter::TakeBoom()
{

}

void APlayCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerInputConfig is not assigned on %s"), *GetName());
		return;
	}
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if(PlayerInputConfig->MoveAction) EIC->BindAction(PlayerInputConfig->MoveAction, ETriggerEvent::Triggered, this, &APlayCharacter::InputActionMove);
		if(PlayerInputConfig->JumpAction) EIC->BindAction(PlayerInputConfig->JumpAction, ETriggerEvent::Started, this, &APlayCharacter::JumpActionMove);
		if(PlayerInputConfig->LookAction) EIC->BindAction(PlayerInputConfig->LookAction, ETriggerEvent::Triggered, this, &APlayCharacter::LookActionMove);
		if (PlayerInputConfig->FireAction) EIC->BindAction(PlayerInputConfig->FireAction, ETriggerEvent::Started, this, &APlayCharacter::InputActionFire);
	}
}

void APlayCharacter::InputActionMove(const FInputActionValue& Value)
{
	const FVector2D MoveVec = Value.Get<FVector2D>();
	if (Controller == nullptr)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveVec.Y);
	AddMovementInput(RightDirection, MoveVec.X);
}

void APlayCharacter::JumpActionMove(const struct FInputActionValue& Value)
{
	Jump();
}

void APlayCharacter::LookActionMove(const struct FInputActionValue& Value)
{
	const FVector2D LookVec = Value.Get<FVector2D>();
	AddControllerYawInput(LookVec.X);
	AddControllerPitchInput(LookVec.Y);

}

void APlayCharacter::InputActionSniper(const FInputActionValue& Value)
{
}

void APlayCharacter::InputActionGrenade(const FInputActionValue& Value)
{
}

void APlayCharacter::InputActionMachine(const FInputActionValue& Value)
{
}

void APlayCharacter::InputActionBoom(const FInputActionValue& Value)
{
}

void APlayCharacter::InputActionFire(const FInputActionValue& Value)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    UWorld* World = GetWorld();
    if (!PC || !World || !BulletClass)
    {
        return;
    }

    int32 Width = 0;
    int32 Height = 0;
    PC->GetViewportSize(Width, Height);
    FVector ViewLocation;
    FVector ViewDirection;
    if (Width <= 0 || Height <= 0 ||
        !PC->DeprojectScreenPositionToWorld(Width * 0.5f, Height * 0.5f, ViewLocation, ViewDirection))
    {
        return;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    const FVector TraceEnd = ViewLocation + ViewDirection * AimDistance;
    FHitResult AimHit;
    const bool bAimHit = World->LineTraceSingleByChannel(
        AimHit, ViewLocation, TraceEnd, ECC_Visibility, QueryParams);
    const FVector TargetLocation = bAimHit ? AimHit.ImpactPoint : TraceEnd;

    const FVector FireOrigin = GetPawnViewLocation();
    const FVector FallbackDirection = GetActorForwardVector();
    FVector MuzzleLocation = FireOrigin + FallbackDirection *
        (GetCapsuleComponent()->GetScaledCapsuleRadius() + 15.0f);
    if (EquippedWeaponMesh && EquippedWeaponMesh->DoesSocketExist(MuzzleSocketName))
    {
        MuzzleLocation = EquippedWeaponMesh->GetSocketLocation(MuzzleSocketName);
    }

    // Do not spawn beyond a wall between the character and the muzzle.
    FHitResult ObstructionHit;
    if (World->SweepSingleByChannel(ObstructionHit, FireOrigin, MuzzleLocation,
        FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5.0f), QueryParams))
    {
        return;
    }

    const FVector FireDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();
    if (FireDirection.IsNearlyZero() || FVector::DotProduct(FireDirection, ViewDirection) <= 0.0f)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
    if (World->SpawnActor<ABulletProjectile>(BulletClass, MuzzleLocation, FireDirection.Rotation(), Params))
    {
        UE_LOG(LogTemp, Verbose, TEXT("Fired projectile toward screen center"));
    }
}

