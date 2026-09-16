#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBall.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PANGGAMEPROJECT_API AEnemyBall : public AActor
{
    GENERATED_BODY()

public:
    AEnemyBall();

    // A single hit splits this generation, or removes the final generation.
    UFUNCTION(BlueprintCallable, Category = "Ball")
    void ReceiveProjectileHit();

    UFUNCTION(BlueprintPure, Category = "Ball")
    float GetStageBounceHeight() const;

protected:
    virtual void BeginPlay() override;

    // 충돌 판정용 구체 (루트)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionComp;

    // 블루프린트에서 외형 메시를 넣을 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComp;

    // 이동 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    // 앞으로 나아가는 일정한 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeed = 400.0f;

    // 바닥에 닿았을 때 위로 튀어 오르는 일정한 속도
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
    float BounceSpeed = 600.0f;

    // Heights are measured from the launch position, in centimeters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Bounce", meta = (ClampMin = "1.0", Units = "cm"))
    float InitialBounceHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Bounce", meta = (ClampMin = "1.0", Units = "cm"))
    float MinimumBounceHeight = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Bounce", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float BounceHeightMultiplier = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Split", meta = (ClampMin = "0", ClampMax = "4"))
    int32 MaxSplitGeneration = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Split", meta = (ClampMin = "0.1", ClampMax = "0.9"))
    float SplitScaleMultiplier = 0.6f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Split")
    int32 SplitGeneration = 0;

    bool bProcessingHit = false;

    float CalculateBounceSpeed() const;
    void IgnoreOtherBalls();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bStrictReverseOnWall = false;

    // 바닥 충돌 시 속도 리셋 함수
    UFUNCTION()
    void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};