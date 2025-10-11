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
