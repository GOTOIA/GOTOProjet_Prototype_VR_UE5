// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "VRCharacter.generated.h"

//#include "MotionControllerComponent.generated.h"

DECLARE_DELEGATE(FOnCameraFadeComplete);

UCLASS()
class PROTOTYPE_VR_V2_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()



public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FOnCameraFadeComplete OnTeleportFinished;


private:

	// Supposons que vous avez une variable membre pour stocker le composant de cam�ra
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY(VisibleAnywhere)
	class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* RadiusVsVelocity;	


	UPROPERTY(EditAnywhere)
	float MaxTeleportDistance = 1000;

	bool bHit;

	UPROPERTY(EditAnywhere)
	float TeleportFadeTime = 2.f;


	void MoveForward(float throttle);
	void MoveRight(float throttle);

	void BeginTeleport();
	void FinishTeleport();

	void StartFade(float FromAlpha, float ToAlpha);

	void ExecuteFinishTeleport();


	void UpdateDestinationMarker();

	void UpdateBlinkers();

	FVector2D GetBlinkerCenter();


	void DrawTeleportCurve(FHitResult HitResult);


};
