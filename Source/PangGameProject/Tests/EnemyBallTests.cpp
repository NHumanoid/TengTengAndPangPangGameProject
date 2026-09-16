#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "../EnemyBall.h"
#include "../BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyBallSplitTest,
    "PangGame.Balls.SplitAndBounce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemyBallSplitTest::RunTest(const FString& Parameters)
{
    UClass* BallClass = LoadClass<AEnemyBall>(nullptr,
        TEXT("/Game/Blueprint/EnemyBall/BP_EnemyBall.BP_EnemyBall_C"));
    if (!TestNotNull(TEXT("Existing BP_EnemyBall loads"), BallClass))
    {
        return false;
    }

    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    World->SetBegunPlay(true);

    auto GetBalls = [World]()
    {
        TArray<AEnemyBall*> Balls;
        for (TActorIterator<AEnemyBall> It(World); It; ++It)
        {
            if (IsValid(*It) && !It->IsActorBeingDestroyed())
            {
                Balls.Add(*It);
            }
        }
        return Balls;
    };

    AEnemyBall* Parent = World->SpawnActor<AEnemyBall>(BallClass,
        FVector(0.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
    if (TestNotNull(TEXT("Parent spawned"), Parent))
    {
        Parent->SetActorScale3D(FVector(1.7f));
        const float ParentRadius = Parent->FindComponentByClass<USphereComponent>()->GetScaledSphereRadius();
        const float ParentHeight = Parent->GetStageBounceHeight();

        ABulletProjectile* Bullet = World->SpawnActor<ABulletProjectile>(
            FVector(10000.0f, 0.0f, 500.0f), FRotator::ZeroRotator);
        if (TestNotNull(TEXT("Projectile spawned"), Bullet))
        {
            // Exercise the actual swept collision event, not only the split function.
            FHitResult Hit;
            Bullet->FindComponentByClass<USphereComponent>()->MoveComponent(
                Parent->GetActorLocation() - Bullet->GetActorLocation(),
                FQuat::Identity, true, &Hit);
            TestTrue(TEXT("Projectile physically hits BP_EnemyBall"), Hit.bBlockingHit);
            TestTrue(TEXT("Collision consumes the projectile"), Bullet->IsActorBeingDestroyed());
            Bullet->ProcessImpact(Parent);
        }
        TArray<AEnemyBall*> MediumBalls = GetBalls();
        TestEqual(TEXT("One hit creates exactly four balls, even with duplicate impact"), MediumBalls.Num(), 4);
        const float MediumHeight = MediumBalls.IsEmpty() ? ParentHeight : MediumBalls[0]->GetStageBounceHeight();
        const float MediumRadius = MediumBalls.IsEmpty() ? ParentRadius :
            MediumBalls[0]->FindComponentByClass<USphereComponent>()->GetScaledSphereRadius();
        for (AEnemyBall* Ball : MediumBalls)
        {
            TestEqual(TEXT("Child preserves BP class"), Ball->GetClass(), BallClass);
            TestTrue(TEXT("Child collision radius shrinks"),
                Ball->FindComponentByClass<USphereComponent>()->GetScaledSphereRadius() < ParentRadius);
            TestTrue(TEXT("Child bounce height is lower"), Ball->GetStageBounceHeight() < ParentHeight);
            TestTrue(TEXT("BP child is initialized"), Ball->HasActorBegunPlay());

            UProjectileMovementComponent* Movement = Ball->FindComponentByClass<UProjectileMovementComponent>();
            FHitResult FloorHit;
            FloorHit.Normal = FVector::UpVector;
            Movement->OnProjectileBounce.Broadcast(FloorHit, FVector(100.0f, 0.0f, -100.0f));
            const double FirstBounceSpeed = Movement->Velocity.Z;
            TestTrue(TEXT("Floor launches the ball upward"), FirstBounceSpeed > 0.0f);
            for (int32 Bounce = 0; Bounce < 20; ++Bounce)
            {
                Movement->OnProjectileBounce.Broadcast(FloorHit, FVector(100.0f, 0.0f, -100.0f));
                TestEqual(TEXT("Repeated floor bounces do not decay"), Movement->Velocity.Z, FirstBounceSpeed);
            }
            Ball->ReceiveProjectileHit();
        }

        TArray<AEnemyBall*> SmallBalls = GetBalls();
        TestEqual(TEXT("Four medium balls produce sixteen small balls"), SmallBalls.Num(), 16);
        for (AEnemyBall* Ball : SmallBalls)
        {
            TestTrue(TEXT("Second split reduces bounce height again"), Ball->GetStageBounceHeight() < MediumHeight);
            TestTrue(TEXT("Second split reduces collision radius again"),
                Ball->FindComponentByClass<USphereComponent>()->GetScaledSphereRadius() < MediumRadius);
            TestTrue(TEXT("Small ball keeps a nonzero bounce height"), Ball->GetStageBounceHeight() > 0.0f);
            Ball->ReceiveProjectileHit();
            Ball->ReceiveProjectileHit();
        }
        TestEqual(TEXT("Final generation disappears without further splitting"), GetBalls().Num(), 0);
    }

    World->EndPlay(EEndPlayReason::Quit);
    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}

#endif
