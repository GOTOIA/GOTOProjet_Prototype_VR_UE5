// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"





// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);

	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);

	}


}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp,Warning, TEXT("Relative Location %s"), *GetActorLocation().ToString());

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateDestinationMarker();
	UpdateBlinkers();


}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MovementAxisRight_Y"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MovementAxisRight_X"), this, &AVRCharacter::MoveRight);

	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Pressed, this, &AVRCharacter::BeginTeleport);



}


void AVRCharacter::MoveForward(float throttle)
{

	//UE_LOG(LogTemp, Warning, TEXT("En avant ou en arriere %f"), throttle);
	AddMovementInput(throttle * Camera->GetForwardVector());

}

void AVRCharacter::MoveRight(float throttle)
{
	//UE_LOG(LogTemp, Warning, TEXT("A droite ou a gauche %f"), throttle);
	AddMovementInput(throttle * Camera->GetRightVector());

}

void AVRCharacter::UpdateDestinationMarker()
{

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * MaxTeleportDistance;

	FHitResult HitResult;
	bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

	if (bHit)
	{

		DestinationMarker->SetVisibility(true);

		DestinationMarker->SetWorldLocation(HitResult.Location);


	}
	else {

		DestinationMarker->SetVisibility(false);
	}

}

void AVRCharacter::UpdateBlinkers()
{

	if (RadiusVsVelocity == nullptr) return;

	float Speed = GetVelocity().Size();
	float Radius = RadiusVsVelocity->GetFloatValue(Speed);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);

	FVector2D Center = GetBlinkerCenter();
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Center"), FLinearColor(Center.X, Center.Y, 0));
}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	FVector MoveDirection = GetVelocity().GetSafeNormal();
	if (MoveDirection.IsNearlyZero())
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;
	if (FVector::DotProduct(Camera->GetForwardVector(), MoveDirection) > 0)
	{
		WorldStationaryLocation = Camera->GetComponentLocation() + MoveDirection * 1000;
	}
	else
	{
		WorldStationaryLocation = Camera->GetComponentLocation() - MoveDirection * 1000;
	}

	

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC == NULL) {
		return FVector2D(0.5, 0.5);
	}
	FVector2D ScreenStationaryLocation;
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;


	return ScreenStationaryLocation;
}



void AVRCharacter::BeginTeleport() {

	UE_LOG(LogTemp, Warning, TEXT("Teleport"));


	OnTeleportFinished.BindUObject(this, &AVRCharacter::FinishTeleport);

	if (bHit) {
		
		StartFade(0, 1);	

		//wait for the fade to finish
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AVRCharacter::ExecuteFinishTeleport, TeleportFadeTime);


	}


}

void AVRCharacter::ExecuteFinishTeleport() {
	// Appeler FinishTeleport apr�s la fin du fondu de la cam�ra
	OnTeleportFinished.ExecuteIfBound();
}



void AVRCharacter::FinishTeleport() {

	UE_LOG(LogTemp, Warning, TEXT("Finish Teleport"));
	SetActorLocation(DestinationMarker->GetComponentLocation());

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC != NULL) {
		PC->PlayerCameraManager->StartCameraFade(1, 0, TeleportFadeTime, FLinearColor::Black);

	}
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC != NULL) {
		PC->PlayerCameraManager->StartCameraFade(FromAlpha,ToAlpha, TeleportFadeTime, FLinearColor::Black);
		UE_LOG(LogTemp, Warning, TEXT("Fade camera"));
	}
}

