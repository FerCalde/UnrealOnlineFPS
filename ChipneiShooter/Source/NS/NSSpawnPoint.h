#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NSGameMode.h"
#include "NSSpawnPoint.generated.h"


class UCapsuleComponent;

UCLASS()
class NS_API ANSSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANSSpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	bool GetBlocked() { return m_tOverlappingActors.Num() != 0; }

	UFUNCTION()
		void ActorBeginOverlaps(AActor* MyOverlappedActor, AActor* OtherActor);
	UFUNCTION()
		void ActorEndOverlaps(AActor* MyOverlappedActor, AActor* OtherActor);


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ETeam m_eTeam;


private:
	UCapsuleComponent* m_pSpawnCapsule;
	TArray<AActor*> m_tOverlappingActors;

};
