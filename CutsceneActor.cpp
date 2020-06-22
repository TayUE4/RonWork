// Copyright Void Interactive, 2020


#include "ASequenceInteraction.h"
#include "LevelSequenceActor.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "Characters/RoNPlayerController.h"

// Sets default values
AASequenceInteraction::AASequenceInteraction(const FObjectInitializer& Init)
	: Super(Init)
{
	PrimaryActorTick.bCanEverTick = true;
   RadiusComp = CreateDefaultSubobject<UBoxComponent>(TEXT("RadiusBox"));
   RadiusComp->SetupAttachment(RootComponent);
   RadiusComp->SetCollisionResponseToAllChannels(ECR_Ignore);
   RadiusComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AASequenceInteraction::BeginPlay()
{
	Super::BeginPlay();
		
	RadiusComp->OnComponentBeginOverlap.RemoveDynamic(this, &AASequenceInteraction::OnBoxOverlap);
	RadiusComp->OnComponentBeginOverlap.AddDynamic(this, &AASequenceInteraction::OnBoxOverlap);
	RadiusComp->OnComponentEndOverlap.RemoveDynamic(this, &AASequenceInteraction::OnBoxEndOverlap);
	RadiusComp->OnComponentEndOverlap.AddDynamic(this, &AASequenceInteraction::OnBoxEndOverlap);
}

void AASequenceInteraction::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AASequenceInteraction::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAutoActivateInRange)
		return;

	APlayerCharacter* pc = Cast<APlayerCharacter>(OtherActor);
	if (!pc)
		return;

	if (!pc->IsPlayerControlled())
		return;

	PlaySequence(pc);
}

void AASequenceInteraction::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AASequenceInteraction::PlaySequence(APlayerCharacter* OverlappedCharacter)
{
	if (!OverlappedCharacter)
		return;

	OverlappedCharacter->DisableInput(Cast<APlayerController>(OverlappedCharacter->GetController()));

	FMovieSceneSequencePlaybackSettings MovieSceneSequencePlaybackSettings;
	MovieSceneSequencePlaybackSettings.bAutoPlay = true;
	MovieSceneSequencePlaybackSettings.bHideHud = false;
	MovieSceneSequencePlaybackSettings.bPauseAtEnd = false;
	UDefaultLevelSequenceInstanceData* LevelSequenceData = (UDefaultLevelSequenceInstanceData*)DefaultInstanceData;
	bOverrideInstanceData = true;
	if (LevelSequenceData)
	{
		FTransform Transform;
		if (OverlappedCharacter->GetFirstPersonCameraComponent())
		{
			FVector ViewLocation;
			ViewLocation = OverlappedCharacter->GetFirstPersonCameraComponent()->GetComponentLocation() + OverlappedCharacter->GetFirstPersonCameraComponent()->GetForwardVector() * 50.0f;
			if (ReferencedCharacterViewTarget)
			{
				// Turn Character To Face Player
				FRotator ViewRotationFacePlayer = UKismetMathLibrary::FindLookAtRotation(ReferencedCharacterViewTarget->GetActorLocation(), ViewLocation);
				ViewRotationFacePlayer.Pitch = 0.0f;
				ViewRotationFacePlayer.Roll = 0.0f;
				ReferencedCharacterViewTarget->SetActorRotation(ViewRotationFacePlayer);
				
				// Turn Sequence to Face AI
				FRotator ViewRotation;
				ViewRotation = UKismetMathLibrary::FindLookAtRotation(ViewLocation, ReferencedCharacterViewTarget->GetActorLocation());
				ViewRotation.Pitch = 0.0f;
				Transform.SetRotation(ViewRotation.Quaternion());
			}
			Transform.SetLocation(ViewLocation);
		}
		LevelSequenceData->TransformOrigin = Transform;
	}
	SequencePlayer->Play();
}
