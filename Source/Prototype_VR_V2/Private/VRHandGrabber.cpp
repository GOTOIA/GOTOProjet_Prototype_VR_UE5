// Fill out your copyright notice in the Description page of Project Settings.


#include "VRHandGrabber.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GameFramework/Actor.h" //pour GetOwner()



// Sets default values for this component's properties
UVRHandGrabber::UVRHandGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



// Called when the game starts
void UVRHandGrabber::BeginPlay()
{
	Super::BeginPlay();    
    
	
}

bool UVRHandGrabber::IsComponentAcceptable(const UPrimitiveComponent* C) const {


    if (!C || !C->IsSimulatingPhysics()) return false;

    if (AcceptChannel != ECC_OverlapAll_Deprecated &&
        C->GetCollisionObjectType() != AcceptChannel)
        return false;

    if (bFilterByTag)
    {
        bool HasTag = false;
        for (const FName T : AcceptTags)
        {
            if (C->ComponentHasTag(T)) { HasTag = true; break; }
        }
        if (!HasTag) return false;
    }
    return true;

}

//Improve performance GetBestCandidate
UPrimitiveComponent* UVRHandGrabber::GetBestCandidate()
{
	
    if (!GrabSphere) return nullptr;

    TArray<UPrimitiveComponent*> Overlaps;
    GrabSphere->GetOverlappingComponents(Overlaps);

    const AActor* SelfOwner = GetOwner();
    const FVector Pivot = GrabTarget ? GrabTarget->GetComponentLocation()
        : GrabSphere->GetComponentLocation();

    float BestD2 = TNumericLimits<float>::Max();
    UPrimitiveComponent* Best = nullptr;

    for (UPrimitiveComponent* C : Overlaps)
    {
        if (!C) continue;
        AActor* OtherOwner = C->GetOwner();
        if (!OtherOwner || OtherOwner == SelfOwner) continue; // ignore soi-m�me

		if (!IsComponentAcceptable(C)) continue;

        /*if (!C->IsSimulatingPhysics()) continue;

        if (bFilterByTag)
        {
            bool HasOne = false;
            for (const FName T : AcceptTags)
            {
                if (C->ComponentHasTag(T)) {
					UE_LOG(LogTemp, Warning, TEXT("GetBestCandidate: Found tag %s on %s"), *T.ToString(), *C->GetName());
                    HasOne = true; 
					isAcceptable(true);
                    break;
                }
				isAcceptable(false);
            }

            if (!HasOne) continue;
        }
        if (AcceptChannel != ECC_OverlapAll_Deprecated &&
            C->GetCollisionObjectType() != AcceptChannel)
        {
            continue;
        }*/

        const float D2 = FVector::DistSquared(Pivot, C->GetComponentLocation());
        if (D2 < BestD2) { BestD2 = D2; Best = C; }
    }
    return Best;
}


// Called every frame
void UVRHandGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (PhysicsHandle && PhysicsHandle->GrabbedComponent && GrabTarget)
    {
            PhysicsHandle->SetTargetLocationAndRotation(
            GrabTarget->GetComponentLocation(),
            GrabTarget->GetComponentRotation());
    }
}




void UVRHandGrabber::GrabObject()
{
    if (!PhysicsHandle || PhysicsHandle->GrabbedComponent) return;

    UPrimitiveComponent* Candidate = GetBestCandidate();

    if (Candidate) {
        UE_LOG(LogTemp, Warning, TEXT("GrabObject: Candidate=%s"), *Candidate->GetName());
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("GrabObject: Candidate=nullptr"));
    }

    // Fallback: petit LineTrace vers l'avant du GrabTarget si rien en overlap
    if (!Candidate && GrabTarget && TraceDistance > 0.f)
    {
        FHitResult Hit;
        const FVector Start = GrabTarget->GetComponentLocation();
        const FVector End = Start + GrabTarget->GetForwardVector() * TraceDistance;

        FCollisionQueryParams P(SCENE_QUERY_STAT(VRGrabTrace), false, GetOwner());
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_PhysicsBody, P))
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();
            if (HitComp && IsComponentAcceptable(HitComp))
            {
                Candidate = HitComp;
                HeldComp = Candidate;
            }
        }
        // Debug facultatif :
        // DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);
    }

    if (!Candidate) return;

    const FTransform T = GrabTarget->GetComponentTransform();
    PhysicsHandle->GrabComponentAtLocationWithRotation(
        Candidate, NAME_None, T.GetLocation(), T.Rotator());
}

void UVRHandGrabber::ReleaseObject()
{
    if (!PhysicsHandle) return;
    if (PhysicsHandle->GrabbedComponent)
    {
        PhysicsHandle->ReleaseComponent();
		HeldComp = nullptr;
    }
}



