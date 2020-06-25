// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameState.h"
#include "SGameMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASGameMode();

protected:

	// How many bots do we wanna spawn in a wave?
	UPROPERTY()
	int32 NumBotsToSpawn;

	UPROPERTY()
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;
	
	// Hook for BP to spawn a single bot.
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	UFUNCTION()
	// Start spawning some bots.
	void StartWave();

	UFUNCTION()
	// Don't spawn bots.
	void EndWave();

	UFUNCTION()
	// Set timer for next wave.
	void PrepareForNextWave();

	UFUNCTION()
	void CheckWaveState();

	UFUNCTION()
	void CheckAnyPlayerAlive();

	UFUNCTION()
	void GameOver();

	UFUNCTION()
	void SetWaveState(EWaveState NewState);

	UPROPERTY()
	FTimerHandle TimerHandle_BotSpawner;

	UPROPERTY()
	FTimerHandle TimerHandle_NextWaveStart;

	UFUNCTION()
		void RestartDeadPlayers();

public:
	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "Game Mode")
	FOnActorKilled OnActorKilled;
};
