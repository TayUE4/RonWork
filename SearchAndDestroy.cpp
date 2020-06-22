// Copyright Void Interactive, 2019

#include "SearchAndDestroyGM.h"
#include "ReadyOrNot.h"


void ASearchAndDestroyGM::BeginPlay()
{
	Super::BeginPlay();

}


void ASearchAndDestroyGM::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASearchAndDestroyGM::StartMatch()
{
	Super::StartMatch();

}

void ASearchAndDestroyGM::RoundEnd()
{
	Super::RoundEnd();

	bBombPlanted = false;
	bBombDefused = false;
	bBombDetonate = false;

}


void ASearchAndDestroyGM::MatchEnd()
{
	Super::MatchEnd();
}


void ASearchAndDestroyGM::TimeLimitVictoryConditions()
{
	Super::TimeLimitVictoryConditions();

	AReadyOrNotGameState* gs = GetGameState<AReadyOrNotGameState>();
	if (!gs)
	{
		return;
	}

	if (bBombPlanted && !bBombDefused)
	{
		// Bomb is counting down
	}
	else if (bBombDefused)
	{
		// bomb defused, blue team wins
		RoundWonTeam(ETeamType::TT_SERT_BLUE);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_BLUE);
	}
	else if (bBombDetonate)
	{
		// bomb detonated red team wins
		RoundWonTeam(ETeamType::TT_SERT_RED);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_RED);
	}
	else
	{
		RoundWonTeam(ETeamType::TT_SERT_BLUE);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_BLUE);
	}
}


AActor* ASearchAndDestroyGM::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	FName playerSpawnTag = "";
	ARoNPlayerState* ps = Cast<ARoNPlayerState>(Player->PlayerState);
	if (ps)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Finding Player start for " + ps->GetPlayerName());
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Player is on team: " + FString::FromInt((uint8)ps->GetTeam()));
		switch (ps->GetTeam())
		{
		case ETeamType::TT_SERT_BLUE:
			playerSpawnTag = SWATBlueStartTag;
			break;
		case ETeamType::TT_SERT_RED:
			playerSpawnTag = SWATRedStartTag;
			break;
		case ETeamType::TT_SUSPECT:
			playerSpawnTag = SuspectStartTag;
			break;
		}
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, playerSpawnTag.ToString());
	}

	TArray<AActor*> compatibleStarts;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* start = *It;
		if (start)
		{
			if (start->PlayerStartTag == playerSpawnTag)
			{
				compatibleStarts.Add(start);
			}
		}
	}

	if (compatibleStarts.Num() > 0)
	{
		return compatibleStarts[FMath::FRandRange(0, compatibleStarts.Num() - 1)];
	}
	else
	{
		return GetRandomSafeStart();
	}

	return nullptr;
}

// void ASearchAndDestroyGM::RespawnPlayer(APlayerController* Player, bool bForceSpectator)
// {
// 	APlayerController* pc = Cast<APlayerController>(Player);
// 	if (pc && (GetMatchState() == EMatchState::MS_Playing || GetMatchState() == EMatchState::MS_Warmup))
// 	{
// 
// 		ARoNPlayerState* ps = Cast<ARoNPlayerState>(pc->PlayerState);
// 		FTransform SpawnTransform;
// 
// 		if (bForceSpectator)
// 		{
// 			SpawnSpectator(pc, (TSubclassOf<ASpectatorPawn>)(SpectatorClass), SpawnTransform);
// 		}
// 	}
// 	else
// 	{
// 		Super::SpawnPlayer
// 	}
// 
// }

int32 ASearchAndDestroyGM::GetNumberOfActivePlayersOnTeam(ETeamType Team)
{
	int32 ActivePlayers = 0;

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), Actors);

	for (int32 i = 0; i < Actors.Num(); i++)
	{
		APlayerCharacter* pc = Cast<APlayerCharacter>(Actors[i]);
		if (pc->GetTeam() == Team)
		{
			if (!pc->IsDeadOrUnconscious() && !pc->bArrestComplete)
			{
				ActivePlayers++;
			}
		}
	}

	return ActivePlayers;
}



void ASearchAndDestroyGM::CheckVictoryConditions()
{
	AReadyOrNotGameState* gs = GetGameState<AReadyOrNotGameState>();
	if (!gs)
	{
		return;
	}

	if (bBombDetonate == true)
	{
		//Red Team Detonated Bomb
		RoundWonTeam(ETeamType::TT_SERT_RED);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_RED);
	}

	if (bBombDefused == true)
	{
		//Swat defused Bomb
		RoundWonTeam(ETeamType::TT_SERT_BLUE);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_BLUE);
	}

	if (GetNumberOfActivePlayersOnTeam(ETeamType::TT_SERT_RED) <= 0)
	{
		// no active players left on red = blue team wins
		RoundWonTeam(ETeamType::TT_SERT_BLUE);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_BLUE);
	}
	else if (GetNumberOfActivePlayersOnTeam(ETeamType::TT_SERT_BLUE) <= 0)
	{
		// no active players left on blue = red team wins
		RoundWonTeam(ETeamType::TT_SERT_RED);
		gs->Multicast_AnnounceTeamVictory(ETeamType::TT_SERT_RED);
	}
}

void ASearchAndDestroyGM::PlayerArrested(APlayerCharacter* ArrestedCharacter, APlayerCharacter* InstigatorCharacter)
{
	Super::PlayerArrested(ArrestedCharacter, InstigatorCharacter);
}

void ASearchAndDestroyGM::PlayerKilled(AActor* Causer, ACharacter* InstigatorCharacter, ACharacter* KilledCharacter, struct FDamageEvent const& DamageEvent, APlayerState* PlayerState)
{
	Super::PlayerKilled(Causer, InstigatorCharacter, KilledCharacter, DamageEvent, PlayerState);
}

