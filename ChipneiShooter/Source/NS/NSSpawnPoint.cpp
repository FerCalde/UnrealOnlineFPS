// Fill out your copyright notice in the Description page of Project Settings.


#include "NSSpawnPoint.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ANSSpawnPoint::ANSSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	m_pSpawnCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	m_pSpawnCapsule->SetCollisionProfileName("OverlapAllDynamic");
	m_pSpawnCapsule->SetGenerateOverlapEvents(true);
	m_pSpawnCapsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	OnActorBeginOverlap.AddDynamic(this, &ANSSpawnPoint::ActorBeginOverlaps);
	OnActorEndOverlap.AddDynamic(this, &ANSSpawnPoint::ActorEndOverlaps);
}

// Called when the game starts or when spawned
void ANSSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
}


void ANSSpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (m_eTeam == ETeam::Team_BLUE)
	{
		m_pSpawnCapsule->ShapeColor = FColor::Blue;
	}
	else 
	{
		m_pSpawnCapsule->ShapeColor = FColor::Red;
	}
}

// Called every frame
void ANSSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	m_pSpawnCapsule->UpdateOverlaps();

}

void ANSSpawnPoint::ActorBeginOverlaps(AActor* MyOverlappedActor, AActor* OtherActor)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (!m_tOverlappingActors.Contains(OtherActor))
		{
			m_tOverlappingActors.Add(OtherActor);
		}
	}
}

void ANSSpawnPoint::ActorEndOverlaps(AActor* MyOverlappedActor, AActor* OtherActor)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (m_tOverlappingActors.Contains(OtherActor))
		{
			m_tOverlappingActors.Remove(OtherActor);
		}
	}
}

