// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MotionControllerComponent.h"
#include "VRHandGrabber.h"
#include "VRCharacter.generated.h"



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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* LeftHandMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* LeftHandMeshMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* HandOffsetLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* L_GrabTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USphereComponent* L_GrabSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class UMotionControllerComponent* RightHandMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USkeletalMeshComponent* RightHandMeshMC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* HandOffsetRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* PointerOrigin_Left;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	class USceneComponent* PointerOrigin_Right;

	// Ajoute 2 composants "grabber", un pour chaque main
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Grab")
	UVRHandGrabber* LeftGrabber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR|Grab")
	UVRHandGrabber* RightGrabber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR", meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* PhysicsHandle_L;

	// Bindings d’inputs
    void OnGrabLeftPressed();
    void OnGrabLeftReleased();
    /*void OnGrabRightPressed();
    void OnGrabRightReleased();*/

	


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

	bool IsOnNavMesh(UWorld* World, const FVector& Location, FVector& OutProjected, float QueryRadius, TSubclassOf<UNavigationQueryFilter> FilterClass);

	void ExecuteFinishTeleport();


	void UpdateDestinationMarker();

	void UpdateBlinkers();

	FVector2D GetBlinkerCenter();


	void DrawTeleportCurve(FHitResult HitResult);


};