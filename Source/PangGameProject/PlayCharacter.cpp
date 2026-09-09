// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayCharacter.h"
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

void APlayCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		if(PlayerInputConfig->MoveAction) EIC->BindAction(PlayerInputConfig->MoveAction, ETriggerEvent::Triggered, this, &APlayCharacter::InputActionMove);
		if(PlayerInputConfig->JumpAction) EIC->BindAction(PlayerInputConfig->JumpAction, ETriggerEvent::Started, this, &APlayCharacter::JumpActionMove);
		if(PlayerInputConfig->LookAction) EIC->BindAction(PlayerInputConfig->LookAction, ETriggerEvent::Triggered, this, &APlayCharacter::LookActionMove);
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

