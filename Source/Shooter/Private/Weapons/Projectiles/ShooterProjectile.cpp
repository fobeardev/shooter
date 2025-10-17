#include "Weapons/Projectiles/ShooterProjectile.h"

AShooterProjectile::AShooterProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    RootComponent = Mesh;
}

void AShooterProjectile::BeginPlay()
{
    Super::BeginPlay();
}
