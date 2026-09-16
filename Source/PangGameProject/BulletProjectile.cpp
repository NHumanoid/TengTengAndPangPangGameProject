#include "BulletProjectile.h"
#include "EnemyBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ABulletProjectile::ABulletProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    InitialLifeSpan = 5.0f;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->SetGenerateOverlapEvents(false);
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(CollisionComp);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetRelativeScale3D(FVector(0.1f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        MeshComp->SetStaticMesh(SphereMesh.Object);
    }

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->SetUpdatedComponent(CollisionComp);
    ProjectileMovement->InitialSpeed = 3000.0f;
    ProjectileMovement->MaxSpeed = 3000.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bInitialVelocityInLocalSpace = true;
    ProjectileMovement->Velocity = FVector::ForwardVector;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
}

void ABulletProjectile::BeginPlay()
{
    Super::BeginPlay();
    CollisionComp->IgnoreActorWhenMoving(GetOwner(), true);
    if (GetInstigator())
    {
        CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
    }
    CollisionComp->OnComponentHit.AddDynamic(this, &ABulletProjectile::OnHit);
}

void ABulletProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    ProcessImpact(OtherActor);
}

void ABulletProjectile::ProcessImpact(AActor* OtherActor)
{
    if (bImpactProcessed || (OtherActor && (OtherActor == GetOwner() || OtherActor == GetInstigator())))
    {
        return;
    }

    bImpactProcessed = true;
    SetActorEnableCollision(false);
    ProjectileMovement->StopMovementImmediately();

    if (AEnemyBall* Ball = Cast<AEnemyBall>(OtherActor))
    {
        Ball->ReceiveProjectileHit();
    }

    Destroy();
}
