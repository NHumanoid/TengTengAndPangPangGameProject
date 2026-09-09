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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float BounceSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bStrictReverseOnWall = false;

    // 바닥 충돌 시 속도 리셋 함수
    UFUNCTION()
    void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};