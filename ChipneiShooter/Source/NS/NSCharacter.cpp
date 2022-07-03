// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "NSPlayerState.h"
#include "NSGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ANSCharacter

ANSCharacter::ANSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetupAttachment(FirstPersonCameraComponent);
	FP_Mesh->bCastDynamicShadow = false;
	FP_Mesh->CastShadow = false;
	FP_Mesh->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	FP_Mesh->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));
	TP_Gun->SetOwnerNoSee(true);
	TP_Gun->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	GetMesh()->SetOwnerNoSee(true);
}

void ANSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(FP_Mesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	GetMesh()->SetOwnerNoSee(true);
	TP_Gun->SetOwnerNoSee(true);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANSCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ANSCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ANSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANSCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANSCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANSCharacter::LookUpAtRate);
}

/*Fire Zone*/

void ANSCharacter::OnFire()
{
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FP_FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = FP_Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FP_FireAnimation, 1.f);
		}
	}

	//Check the position and dir of the cross and convert into WorldSpace Position & Dir
	FVector vCrossPos;
	FVector vCrossDir;

	APlayerController* pController = Cast<APlayerController>(GetController());
	FVector2D vScreenSize = GEngine->GameViewport->Viewport->GetSizeXY();
	pController->DeprojectScreenPositionToWorld(vScreenSize.X * 0.5f, vScreenSize.Y * 0.5f, vCrossPos, vCrossDir);

	vCrossDir *= 1000.f;
	//Fire(vCrossPos, vCrossDir);
	ServerFire(vCrossPos, vCrossDir);
}

bool ANSCharacter::ServerFire_Validate(const FVector& _vPos, const FVector& _vSize) //this function is used to validate the server call
{
	FVector vDist = GetActorLocation() - _vPos;
	return vDist.SizeSquared() < 200.f * 200.f;
}
void ANSCharacter::ServerFire_Implementation(const FVector& _vPos, const FVector& _vSize)
{
	Fire(_vPos, _vSize);
	MultiCastShootEffects();
}

void ANSCharacter::Fire(const FVector& _vPos, const FVector& _vSize)
{
	DrawDebugLine(GetWorld(), _vPos, _vPos + _vSize, FColor::Red, true, 10.f, 0, 5.f);


	FHitResult HitResult;
	FCollisionObjectQueryParams ObjQuery;
	FCollisionQueryParams ColQuery;
	ColQuery.AddIgnoredActor(this);
	ObjQuery.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	GetWorld()->LineTraceSingleByObjectType(HitResult, _vPos, _vPos + _vSize, ObjQuery, ColQuery);
	if (HitResult.bBlockingHit)
	{
		ANSCharacter* pOther = Cast<ANSCharacter>(HitResult.GetActor());

		if (pOther)
		{
			FDamageEvent oEvent(UDamageType::StaticClass());
			pOther->TakeDamage(10.f, oEvent, GetController(), this);
		}
	}
}

float ANSCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	ANSPlayerState* pPlayerState = Cast<ANSPlayerState>(GetPlayerState());

	if (GetLocalRole() == ROLE_Authority) //Checkear que solo el Server puede hacer esto
	{
		if (pPlayerState && pPlayerState->m_fHealth > 0.f)
		{
			pPlayerState->m_fHealth -= Damage;

			if (pPlayerState->m_fHealth <= 0.f)
			{
				pPlayerState->m_uiDeaths++;

				ANSCharacter* pOtherChar = Cast<ANSCharacter>(DamageCauser);
				ANSPlayerState* pOtherPlayerState = pOtherChar ? Cast<ANSPlayerState>(pOtherChar->GetPlayerState()) : nullptr;
				if (pOtherPlayerState)
				{
					pOtherPlayerState->Score += 1.f;
				}
				
				MultiCastRagdoll();
				//in 3 seconds	
				FTimerHandle oTimer;
				GetWorldTimerManager().SetTimer<ANSCharacter>(oTimer, this, &ANSCharacter::Respawn, 3.f, false);
			}
		}
	}
	return Damage;
}

void ANSCharacter::Respawn()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ANSPlayerState* pPlayerState = Cast<ANSPlayerState>(GetPlayerState());
		if (pPlayerState)
		{
			pPlayerState->m_fHealth = 100.f;
			
			Cast<ANSGameMode>(GetWorld()->GetAuthGameMode())->Respawn(this);
			Destroy(true, true);
		}
	}

}

void ANSCharacter::MultiCastShootEffects_Implementation()
{
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (TP_FireAnimation != NULL)
	{
		// Get the animation object for the mesh
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(TP_FireAnimation, 1.f);
		}
	}
}

void ANSCharacter::MultiCastRagdoll_Implementation()
{
	GetMesh()->SetPhysicsBlendWeight(1.f);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName("Ragdoll");
}


/*Fire Zone End*/



void ANSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ANSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ANSCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANSCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
