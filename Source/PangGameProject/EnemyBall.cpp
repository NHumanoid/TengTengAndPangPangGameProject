#include "EnemyBall.h"
#include "BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"

AEnemyBall::AEnemyBall()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(32.0f);
    CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bShouldBounce = true;
    ProjectileMovement->Bounciness = 1.0f;
    ProjectileMovement->Friction = 0.0f;
    ProjectileMovement->BounceVelocityStopSimulatingThreshold = 0.0f;
}

float AEnemyBall::GetStageBounceHeight() const
{
    const float InitialHeight = FMath::Max(1.0f, InitialBounceHeight);
    const float MinHeight = FMath::Clamp(MinimumBounceHeight, 1.0f, InitialHeight);
    return FMath::Max(MinHeight, InitialHeight * FMath::Pow(
        FMath::Clamp(BounceHeightMultiplier, 0.1f, 1.0f), FMath::Max(0, SplitGeneration)));
}

float AEnemyBall::CalculateBounceSpeed() const
{
    // v = sqrt(2gh). Recalculate from the stage, never from the last impact speed.
    return FMath::Sqrt(2.0f * FMath::Abs(ProjectileMovement->GetGravityZ()) * GetStageBounceHeight());
}

void AEnemyBall::IgnoreOtherBalls()
{
    for (TActorIterator<AEnemyBall> It(GetWorld()); It; ++It)
    {
        AEnemyBall* OtherBall = *It;
        if (OtherBall != this && IsValid(OtherBall))
        {
            CollisionComp->IgnoreActorWhenMoving(OtherBall, true);
            OtherBall->CollisionComp->IgnoreActorWhenMoving(this, true);
        }
    }
}

void AEnemyBall::BeginPlay()
{
    Super::BeginPlay();

    CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AEnemyBall::OnBounce);
    ProjectileMovement->bForceSubStepping = true;
    IgnoreOtherBalls();

    BounceSpeed = CalculateBounceSpeed();
    FVector ForwardDir = GetActorForwardVector().GetSafeNormal2D();
    if (ForwardDir.IsNearlyZero())
    {
        ForwardDir = FVector::ForwardVector;
    }
    ProjectileMovement->Velocity = ForwardDir * MoveSpeed + FVector::UpVector * BounceSpeed;
}

void AEnemyBall::ReceiveProjectileHit()
{
    if (bProcessingHit || IsActorBeingDestroyed())
    {
        return;
    }
    bProcessingHit = true;

    if (SplitGeneration >= FMath::Clamp(MaxSplitGeneration, 0, 4))
    {
        SetActorEnableCollision(false);
        Destroy();
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        bProcessingHit = false;
        return;
    }

    const float ScaleFactor = FMath::Clamp(SplitScaleMultiplier, 0.1f, 0.9f);
    const FVector ChildScale = GetActorScale3D() * ScaleFactor;
    const float ChildRadius = CollisionComp->GetScaledSphereRadius() * ScaleFactor;
    const FVector SplitLocation = GetActorLocation();
    FVector BaseDirection = ProjectileMovement->Velocity.GetSafeNormal2D();
    if (BaseDirection.IsNearlyZero())
    {
        BaseDirection = FVector::ForwardVector;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    for (TActorIterator<AEnemyBall> It(World); It; ++It)
    {
        QueryParams.AddIgnoredActor(*It);
    }

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

    SetActorEnableCollision(false);
    TArray<AEnemyBall*> SpawnedBalls;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FVector Direction = BaseDirection.RotateAngleAxis(45.0f + Index * 90.0f, FVector::UpVector);
        FVector ChildLocation = SplitLocation + Direction * (ChildRadius * 1.5f + 2.0f);
        FHitResult PlacementHit;
        if (World->SweepSingleByObjectType(PlacementHit, SplitLocation, ChildLocation,
            FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(ChildRadius), QueryParams))
        {
            // The parent's center already has room for a smaller sphere.
            // Start there near walls instead of spawning beyond the wall.
            ChildLocation = SplitLocation;
        }

        const FTransform SpawnTransform(Direction.Rotation(), ChildLocation, ChildScale);
        AEnemyBall* Child = World->SpawnActorDeferred<AEnemyBall>(GetClass(), SpawnTransform,
            GetOwner(), GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn,
            ESpawnActorScaleMethod::OverrideRootScale);
        if (!Child)
        {
            break;
        }

        // Set these before BeginPlay initializes movement. Keep placed-instance tuning.
        Child->SplitGeneration = SplitGeneration + 1;
        Child->MaxSplitGeneration = MaxSplitGeneration;
        Child->SplitScaleMultiplier = SplitScaleMultiplier;
        Child->InitialBounceHeight = InitialBounceHeight;
        Child->MinimumBounceHeight = MinimumBounceHeight;
        Child->BounceHeightMultiplier = BounceHeightMultiplier;
        Child->MoveSpeed = MoveSpeed;
        Child->bStrictReverseOnWall = bStrictReverseOnWall;
        Child->ProjectileMovement->ProjectileGravityScale = ProjectileMovement->ProjectileGravityScale;
        Child->CollisionComp->SetSphereRadius(CollisionComp->GetUnscaledSphereRadius());
        Child->FinishSpawning(SpawnTransform);
        SpawnedBalls.Add(Child);
        QueryParams.AddIgnoredActor(Child);
    }

    if (SpawnedBalls.Num() != 4)
    {
        // Keep the original ball if spawning the full group fails.
        for (AEnemyBall* Child : SpawnedBalls)
        {
            Child->Destroy();
        }
        SetActorEnableCollision(true);
        bProcessingHit = false;
        UE_LOG(LogTemp, Warning, TEXT("Could not spawn all four child balls for %s"), *GetName());
        return;
    }

    Destroy();
}

void AEnemyBall::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    if (ABulletProjectile* Bullet = Cast<ABulletProjectile>(ImpactResult.GetActor()))
    {
        Bullet->ProcessImpact(this);
        return;
    }
    if (bProcessingHit)
    {
        return;
    }

    FVector NewVelocity = ProjectileMovement->Velocity;
    if (ImpactResult.Normal.Z > 0.5f)
    {
        BounceSpeed = CalculateBounceSpeed();
        NewVelocity.Z = BounceSpeed;
        FVector HorizDir = NewVelocity.GetSafeNormal2D();
        if (HorizDir.IsNearlyZero())
        {
            HorizDir = GetActorForwardVector().GetSafeNormal2D();
        }
        NewVelocity.X = HorizDir.X * MoveSpeed;
        NewVelocity.Y = HorizDir.Y * MoveSpeed;
    }
    else if (ImpactResult.Normal.Z > -0.5f)
    {
        const FVector IncomingDir = ImpactVelocity.GetSafeNormal2D();
        const FVector WallNormal = ImpactResult.Normal.GetSafeNormal2D();
        const FVector NextDirection = bStrictReverseOnWall ? -IncomingDir :
            FMath::GetReflectionVector(IncomingDir, WallNormal).GetSafeNormal2D();
        NewVelocity.X = NextDirection.X * MoveSpeed;
        NewVelocity.Y = NextDirection.Y * MoveSpeed;
        if (!NextDirection.IsNearlyZero())
        {
            SetActorRotation(NextDirection.Rotation());
        }
    }
    // Ceiling impacts keep the movement component's downward reflection.
    ProjectileMovement->Velocity = NewVelocity;
}
