// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	WaveInProgress,
	// No longer spawning bots, waiting on player to kill bots.
	WaitingToComplete,
	GameOver,
	WaveComplete
};


/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
protected:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState)
	EWaveState WaveState;

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent)
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

public:
	UFUNCTION()
	void SetWaveState(EWaveState NewState);
};
