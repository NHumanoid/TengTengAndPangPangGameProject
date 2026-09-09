// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AEnemyBall::AEnemyBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    // 1. 충돌체 생성 (루트)
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(32.0f);
    CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    RootComponent = CollisionComp;

    // 2. 비주얼 메시 (외형은 블루프린트에서 세팅)
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 루트와 이중 충돌 방지

    // 3. 이동 컴포넌트 기본 세팅
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bShouldBounce = true;
    ProjectileMovement->Bounciness = 1.0f;
    ProjectileMovement->Friction = 0.0f;
    ProjectileMovement->BounceVelocityStopSimulatingThreshold = 0.0f;
}

// Called when the game starts or when spawned
void AEnemyBall::BeginPlay()
{
	Super::BeginPlay();

    ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AEnemyBall::OnBounce);

    // 액터가 바라보는 정면 방향으로 시작
    FVector ForwardDir = GetActorForwardVector();
    ForwardDir.Z = 0.0f;
    ForwardDir.Normalize();

    // 공이 바라보는 전방 방향(Forward) 속도 + 위쪽(Z) 속도 일괄 적용
    ProjectileMovement->Velocity = (ForwardDir * MoveSpeed) + FVector(0.0f, 0.0f, BounceSpeed);
}

void AEnemyBall::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    // 바닥에 닿았을 때만 속도를 다시 고정값으로 리셋
    FVector NewVelocity = ProjectileMovement->Velocity;

    // ----------------------------------------------------
    // [1] 바닥에 충돌한 경우 (Normal.Z가 위를 향함)
    // ----------------------------------------------------
    if (ImpactResult.Normal.Z > 0.5f)
    {
        // 점프 높이 유지
        NewVelocity.Z = BounceSpeed;

        // 수평 속도가 마찰로 줄지 않도록 MoveSpeed로 고정 유지
        FVector HorizDir = FVector(NewVelocity.X, NewVelocity.Y, 0.0f).GetSafeNormal();
        if (HorizDir.IsNearlyZero())
        {
            HorizDir = GetActorForwardVector();
        }

        NewVelocity.X = HorizDir.X * MoveSpeed;
        NewVelocity.Y = HorizDir.Y * MoveSpeed;
    }
    // ----------------------------------------------------
    // [2] 벽이나 장애물(옆면)에 충돌한 경우
    // ----------------------------------------------------
    else
    {
        FVector NextDirection = FVector::ZeroVector;

        if (bStrictReverseOnWall)
        {
            // [모드 A] 충돌 직전 이동 방향의 정확한 180도 반대 방향
            FVector PrevDir = FVector(ImpactVelocity.X, ImpactVelocity.Y, 0.0f).GetSafeNormal();
            NextDirection = -PrevDir;
        }
        else
        {
            // [모드 B] 벽면 Normal을 이용한 입사각/반사각 계산 (당구공 방식)
            FVector IncomingDir = FVector(ImpactVelocity.X, ImpactVelocity.Y, 0.0f).GetSafeNormal();
            FVector WallNormal = FVector(ImpactResult.Normal.X, ImpactResult.Normal.Y, 0.0f).GetSafeNormal();
            NextDirection = FMath::GetReflectionVector(IncomingDir, WallNormal).GetSafeNormal();
        }

        // 반사된 수평 방향에 MoveSpeed 적용
        NewVelocity.X = NextDirection.X * MoveSpeed;
        NewVelocity.Y = NextDirection.Y * MoveSpeed;

        // 공 액터의 회전도 새로 나아가는 방향을 바라보도록 갱신
        if (!NextDirection.IsNearlyZero())
        {
            SetActorRotation(NextDirection.Rotation());
        }
    }

    // 최종 속도 적용
    ProjectileMovement->Velocity = NewVelocity;
}

