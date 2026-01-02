// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRHandGrabber.generated.h"


class USphereComponent;
class UPhysicsHandleComponent;
class USceneComponent;

// Delegate Blueprint
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnGrabStateChanged,
	bool,
	bIsGrabbing
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROTOTYPE_VR_V2_API UVRHandGrabber : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRHandGrabber();

	UFUNCTION(BlueprintCallable, Category = "VR|Grab")
	void GrabObject();

	UFUNCTION(BlueprintCallable, Category = "VR|Grab")
	void ReleaseObject();

	UFUNCTION(BlueprintCallable, Category = "VR|Grab")
	bool IsHolding() const { return HeldComp != nullptr; }

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPrimitiveComponent* GetBestCandidate();


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// Assigne-les dans le Blueprint du Character (Left/Right)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Refs",
		meta = (UseComponentPicker = "true"))
	USceneComponent* GrabTarget = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Refs",
		meta = (UseComponentPicker = "true"))
	USphereComponent* GrabSphere = nullptr;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Refs",
		meta = (UseComponentPicker = "true"))
	UPhysicsHandleComponent* PhysicsHandle = nullptr;

	
	// Filtres
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Filter")
	bool bFilterByTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Filter", meta = (EditCondition = "bFilterByTag"))
	TArray<FName> AcceptTags{ FName("Grabbable") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Filter")
	TEnumAsByte<ECollisionChannel> AcceptChannel = ECC_PhysicsBody;

	// Fallback si pas d’overlap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR|Grab")
	float TraceDistance = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int32 DebugValue = 42;

	// grab state
	UPROPERTY(BlueprintReadOnly, Category = "VR|Grab")
	bool bIsGrabbing = false;

	////// Event Blueprint
	UPROPERTY(BlueprintAssignable, Category = "VR|Grab")
	FOnGrabStateChanged OnGrabStateChanged;

	
	

private:
	UPROPERTY()
	UPrimitiveComponent* HeldComp = nullptr;



	bool IsComponentAcceptable(const UPrimitiveComponent* C) const;
		
};
