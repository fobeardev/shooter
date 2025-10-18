// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/ShooterGameMode.h"
#include "Characters/ShooterCharacter.h"
#include "Player/ShooterPlayerController.h"

AShooterGameMode::AShooterGameMode()
{
    DefaultPawnClass = AShooterCharacter::StaticClass();
    PlayerControllerClass = AShooterPlayerController::StaticClass();

    // If you later add a custom PlayerState, GameState, HUD, etc., set them here too.
    // PlayerStateClass = AShooterPlayerState::StaticClass();
    // GameStateClass   = AShooterGameState::StaticClass();
    // HUDClass         = AShooterHUD::StaticClass();
}

void AShooterGameMode::RequestRespawn(AController* Controller)
{
    if (!Controller) return;

    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &AShooterGameMode::RespawnPlayer, Controller);
    GetWorldTimerManager().SetTimer(RespawnTimer, RespawnDelegate, 2.0f, false);
}

void AShooterGameMode::RespawnPlayer(AController* Controller)
{
    if (!Controller) return;

    RestartPlayer(Controller);
}
