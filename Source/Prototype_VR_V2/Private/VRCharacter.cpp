// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavMesh/RecastNavMesh.h"
#include "Engine/World.h"




// Sets default values
AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	// Création des Motion Controllers
	//Left Hand------------------------------------------
	LeftHandMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHandMC->SetupAttachment(VRRoot); // Attache à la racine du VRRoot
	LeftHandMC->SetTrackingSource(EControllerHand::Left); // Définit la main gauche

	HandOffsetLeft = CreateDefaultSubobject<USceneComponent>(TEXT("HandOffsetLeft"));
	HandOffsetLeft->SetupAttachment(LeftHandMC);

	LeftHandMeshMC= CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandMesh"));
	LeftHandMeshMC->SetupAttachment(HandOffsetLeft);

	PointerOrigin_Left = CreateDefaultSubobject<USceneComponent>(TEXT("PointerOrigin_Left"));
	PointerOrigin_Left->SetupAttachment(HandOffsetLeft);

	L_GrabTarget = CreateDefaultSubobject<USceneComponent>(TEXT("L_GrabTarget"));
	L_GrabTarget->SetupAttachment(LeftHandMC);

	L_GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("L_GrabSphere"));
	L_GrabSphere->SetupAttachment(L_GrabTarget);

	//------------------------------

	//Right Hand------------------------------------------
	RightHandMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHandMC->SetupAttachment(VRRoot); // Attache à la racine du VRRoot
	RightHandMC->SetTrackingSource(EControllerHand::Right); // Définit la main droite

	HandOffsetRight = CreateDefaultSubobject<USceneComponent>(TEXT("HandOffsetRight"));
	HandOffsetRight->SetupAttachment(RightHandMC);

	RightHandMeshMC = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandMesh"));
	RightHandMeshMC->SetupAttachment(HandOffsetRight);

	PointerOrigin_Right = CreateDefaultSubobject<USceneComponent>(TEXT("PointerOrigin_Right"));
	PointerOrigin_Right->SetupAttachment(HandOffsetRight);

	R_GrabTarget = CreateDefaultSubobject<USceneComponent>(TEXT("R_GrabTarget"));
	R_GrabTarget->SetupAttachment(RightHandMC);

	R_GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("R_GrabSphere"));
	R_GrabSphere->SetupAttachment(R_GrabTarget);

	//------------------------------

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

	//Physics Handle
	PhysicsHandle_L = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle_L"));
	PhysicsHandle_R= CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle_R"));



	//Grabber
	// Création des composants (tu peux aussi les ajouter depuis le Blueprint parent)
	LeftGrabber = CreateDefaultSubobject<UVRHandGrabber>(TEXT("LeftGrabber"));
	RightGrabber = CreateDefaultSubobject<UVRHandGrabber>(TEXT("RightGrabber"));


	

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	////Left Get grabber set propriétés
	if (LeftGrabber)
	{
		UE_LOG(LogTemp, Warning, TEXT("LeftGrabber a une reference"));
		LeftGrabber->GrabTarget = L_GrabTarget;
		LeftGrabber->GrabSphere = L_GrabSphere;
		LeftGrabber->PhysicsHandle = PhysicsHandle_L;
	}


	////Right Get grabber set propriétés
	if (LeftGrabber)
	{
		UE_LOG(LogTemp, Warning, TEXT("RightGrabber a une reference"));
		RightGrabber->GrabTarget = R_GrabTarget;
		RightGrabber->GrabSphere = R_GrabSphere;
		RightGrabber->PhysicsHandle = PhysicsHandle_R;
	}


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
	PlayerInputComponent->BindAxis(TEXT("MovementAxisLeft_X"), this, &AVRCharacter::TurnAtRate);

	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Pressed, this, &AVRCharacter::BeginTeleport);

	// Mappe ces actions dans Project Settings > Input
	PlayerInputComponent->BindAction("GrabLeft", IE_Pressed, this, &AVRCharacter::OnGrabLeftPressed);
	PlayerInputComponent->BindAction("GrabLeft", IE_Released, this, &AVRCharacter::OnGrabLeftReleased);
	
	PlayerInputComponent->BindAction("GrabRight", IE_Pressed, this, &AVRCharacter::OnGrabRightPressed);
	PlayerInputComponent->BindAction("GrabRight", IE_Released, this, &AVRCharacter::OnGrabRightReleased);


}


void AVRCharacter::MoveForward(float throttle)
{

	AddMovementInput(throttle * Camera->GetForwardVector());

}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(throttle * Camera->GetRightVector());

}

void AVRCharacter::TurnAtRate(float Rate)
{
	float YawDelta = Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
	AddControllerYawInput(YawDelta);

	// Stocker le taux de rotation pour UpdateBlinkers
	CurrentRotationRate = FMath::Abs(Rate);
}

void AVRCharacter::UpdateDestinationMarker()
{
	bool onNavMesh = false;

	FVector Start = PointerOrigin_Right->GetComponentLocation();
	FVector End = Start + PointerOrigin_Right->GetForwardVector() * MaxTeleportDistance;

	FHitResult HitResult;
	bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

	FVector projectedLocation = FVector::ZeroVector;
	onNavMesh = IsOnNavMesh(GetWorld(), End, projectedLocation, 50.f, nullptr);

	if (bHit && onNavMesh)
	{

		DestinationMarker->SetVisibility(true);
		DrawTeleportCurve(HitResult);
		DestinationMarker->SetWorldLocation(HitResult.Location);


	}
	else {

		DestinationMarker->SetVisibility(false);
	}

}

void AVRCharacter::UpdateBlinkers()
{


	if (BlinkerMaterialInstance == nullptr) return;

	// Calcul du radius basé sur la vélocité de déplacement
	float SpeedRadius = 1.0f; // Valeur par défaut (pas de blinker)
	if (RadiusVsVelocity != nullptr)
	{
		float Speed = GetVelocity().Size();
		SpeedRadius = RadiusVsVelocity->GetFloatValue(Speed);
	}

	// Calcul du radius basé sur la rotation
	float MinRadius = 0.3f;
	float MaxRadius = 1.0f;
	float RotationRadius = FMath::Lerp(MaxRadius, MinRadius, CurrentRotationRate);

	// Utiliser le radius le plus restrictif (le plus petit)
	float FinalRadius = FMath::Min(SpeedRadius, RotationRadius);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), FinalRadius);

	// Déterminer le centre
	FVector2D Center;
	if (CurrentRotationRate > 0.1f)
	{
		// Pendant la rotation, centrer sur l'écran
		Center = FVector2D(0.5f, 0.5f);
	}
	else
	{
		// Pendant le mouvement, utiliser la direction du mouvement
		Center = GetBlinkerCenter();
	}

	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Center"), FLinearColor(Center.X, Center.Y, 0.f));

	// Réinitialiser le taux de rotation (sera mis à jour par TurnAtRate si rotation active)
	CurrentRotationRate = FMath::FInterpTo(CurrentRotationRate, 0.f, GetWorld()->GetDeltaSeconds(), 5.f);

}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	FVector MoveDirection = GetVelocity().GetSafeNormal();
	if (MoveDirection.IsNearlyZero())
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;
	if (FVector::DotProduct(PointerOrigin_Right->GetForwardVector(), MoveDirection) > 0)
	{
		WorldStationaryLocation = PointerOrigin_Right->GetComponentLocation() + MoveDirection * 1000;
	}
	else
	{
		WorldStationaryLocation = PointerOrigin_Right->GetComponentLocation() - MoveDirection * 1000;
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

//TODO Add Spline + texture Component to draw the curve
void AVRCharacter::DrawTeleportCurve(FHitResult HitResult)
{

	FVector StartLocation = PointerOrigin_Right->GetComponentLocation();//Point de la main
	FVector TargetLocation = HitResult.Location; // Point d'impact (marker)
	

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f, 0, 2.0f);
	
	TArray<FVector> PathPoints;

	// Paramètres de la parabole
	float ArcHeight = 100.0f; // Hauteur de la courbe (à ajuster)
	int NumPoints = 30; // Nombre de segments de la courbe
	float TimeStep = 1.0f / NumPoints; // Petit incrément de temps

	for (int i = 0; i <= NumPoints; i++)
	{
		float Alpha = i * TimeStep; // Valeur entre 0 et 1
		FVector Point = FMath::Lerp(StartLocation, TargetLocation, Alpha); // Interpolation linéaire

		// Appliquer l'effet de la parabole (modifie la hauteur Y)
		Point.Z += ArcHeight * FMath::Sin(Alpha * PI); // Courbe sinus pour la hauteur

		PathPoints.Add(Point);
	}

	// Tracer la courbe avec des lignes vertes
	for (int i = 0; i < PathPoints.Num() - 1; i++)
	{
		DrawDebugLine(GetWorld(), PathPoints[i], PathPoints[i + 1], FColor::Green, false, 0.1f, 0, 2.0f);
	}
	


}



void AVRCharacter::BeginTeleport() {

	UE_LOG(LogTemp, Warning, TEXT("Teleport"));


	OnTeleportFinished.BindUObject(this, &AVRCharacter::FinishTeleport);

	FVector projectedLocation = FVector::ZeroVector;


	bool onNavMesh = IsOnNavMesh(GetWorld(), DestinationMarker->GetComponentLocation(), projectedLocation, 50.f,nullptr);


	if (bHit && onNavMesh) {
		
		UE_LOG(LogTemp, Warning, TEXT("Destination : %s "), *DestinationMarker->GetComponentLocation().ToString());
		
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
//TODO Test if on navmesh	
bool AVRCharacter:: IsOnNavMesh(UWorld* World, const FVector& Location,
	FVector& OutProjected, float QueryRadius = 50.f,
	TSubclassOf<UNavigationQueryFilter> FilterClass = nullptr)
{
	if (!World) return false;

	UNavigationSystemV1* NavSysteme = UNavigationSystemV1::GetCurrent(World);
	if (!NavSysteme) return false;

	ANavigationData* NavData = NavSysteme->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!NavData) return false;

	FNavLocation NavLoc;
	// Taille de la boîte de recherche autour du point
	const FVector QueryExtent(QueryRadius, QueryRadius, 200.f);

	//FSharedConstNavQueryFilter QueryFilter = NavSysteme->GetDefaultNavDataInstance()->GetQueryFilter(FilterClass);

	FSharedConstNavQueryFilter QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, FilterClass);
	
	const bool bOk = NavSysteme->ProjectPointToNavigation(
		Location,                 // point à tester
		NavLoc,                   // out
		QueryExtent,              // rayon/extent de recherche
		NavData,                  // nav data (null = défaut)
		QueryFilter                 // filtre de nav
	);

	if (bOk) OutProjected = NavLoc.Location;
	
	return bOk;
}


void AVRCharacter::OnGrabLeftPressed() { 
	
	if (LeftGrabber)  
		LeftGrabber->GrabObject(); 

}

void AVRCharacter::OnGrabLeftReleased() { 

	if (LeftGrabber)  
		LeftGrabber->ReleaseObject(); 

}



void AVRCharacter::OnGrabRightPressed() { if (RightGrabber) RightGrabber->GrabObject(); }
void AVRCharacter::OnGrabRightReleased() { if (RightGrabber) RightGrabber->ReleaseObject(); }

