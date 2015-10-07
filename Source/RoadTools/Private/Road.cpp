// Copyright (c) 2014 Judd Cohen
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "RoadToolsPrivatePCH.h"
#include "Road.h"


ARoad::ARoad(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
/*	Root = ObjectInitializer.CreateAbstractDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;*/

	Spline = ObjectInitializer.CreateDefaultSubobject<URoadSplineComponent>(this, TEXT("Spline"));
	Spline->SetMobility(EComponentMobility::Static);
	Spline->AttachTo(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder
		(TEXT("StaticMesh'/RoadTools/Meshes/Road_Plain.Road_Plain'"));
	DefaultMesh = MeshFinder.Object;

}


#if WITH_EDITOR
void ARoad::PostEditMove(bool bFinished)
{
	// this event is the only indication the spline component gives us that anything has changed,
	// so force a complete update here so that we can see mesh changes as the user edits the spline
	if (bFinished)
	{
        if(Spline->GetNumSplinePoints()>1)
        {
            Spline->RoadSplineSegmentInfo.SetNum(Spline->GetNumSplinePoints()-1);
            //RerunConstructionScripts();
        }
		RerunConstructionScripts();
	}
	Super::PostEditMove(bFinished);
}

void ARoad::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    if (PropertyChangedEvent.Property != nullptr)
    {
        static const FName SplineName=GET_MEMBER_NAME_CHECKED(ARoad,Spline);
        const FName PropertyName(PropertyChangedEvent.Property->GetFName());
        if(PropertyName==SplineName)
        {

        }
    }
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif


void ARoad::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// start at 1 so this runs once per segment instead of once per point
	for (int32 i = 1; i < Spline->GetNumSplinePoints(); i++)
	{
		// make some variables for readability
		int32 SegmentIndex = i - 1;
		int32 SplineStartIndex = i - 1;
		int32 SplineEndIndex = i;

		// bail early if there aren't enough segments created to cover the whole spline
		if (SplineStartIndex > Spline->RoadSplineSegmentInfo.Num() - 1)
		{
			break;
		}

		// clear out the old segments and force GC to prevent memory leaks
		GetWorld()->ForceGarbageCollection(true);

		// regenerate this segment
		UpdateSplineSegment(SegmentIndex, SplineStartIndex, SplineEndIndex);
	}
}


FVector ARoad::GetLocalTangentAtDistanceAlongSpline(float Distance)
{
	const float Param = Spline->SplineReparamTable.Eval(Distance, 0.f);
	const FVector Tangent = Spline->SplineInfo.EvalDerivative(Param, FVector::ZeroVector);
	return Tangent;
}


void ARoad::UpdateSplineSegment(int32 SegmentIndex, int32 SplineStartIndex, int32 SplineEndIndex)
{
	FRoadSplineSegmentInfo& Segment = Spline->RoadSplineSegmentInfo[SegmentIndex];

	float distToStart = Spline->GetDistanceAlongSplineAtSplinePoint(SplineStartIndex);
	float distToEnd = Spline->GetDistanceAlongSplineAtSplinePoint(SplineEndIndex);

	float segmentDist = distToEnd - distToStart;
	float subdivDist = segmentDist / Segment.NumberMeshPerSegment;

	FVector segmentStartTangent = GetLocalTangentAtDistanceAlongSpline(distToStart);
	FVector segmentEndTangent = GetLocalTangentAtDistanceAlongSpline(distToEnd);

	for (int32 i = 0; i < Segment.NumberMeshPerSegment; i++)
	{
		float subdivStartDist = distToStart + (i * subdivDist);
		float subdivEndDist = distToStart + ((i + 1) * subdivDist);

		FVector startLoc = Spline->GetWorldLocationAtDistanceAlongSpline(subdivStartDist);
		startLoc = GetActorLocation() - Spline->ComponentToWorld.InverseTransformPosition(startLoc);

		FVector endLoc = Spline->GetWorldLocationAtDistanceAlongSpline(subdivEndDist);
		endLoc = GetActorLocation() - Spline->ComponentToWorld.InverseTransformPosition(endLoc);

		// make a new spline mesh
		USplineMeshComponent* comp = NewObject<USplineMeshComponent>(this);

		if (Segment.Mesh)
		{
			comp->SetStaticMesh(Segment.Mesh);
		}
		else if (DefaultMesh)
		{
			comp->SetStaticMesh(DefaultMesh);
		}

		comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		comp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

		// update start and end positions
		comp->SplineParams.StartPos = GetActorLocation() - startLoc;
		comp->SplineParams.EndPos = GetActorLocation() - endLoc;

		FVector startTangent = Spline->GetWorldDirectionAtDistanceAlongSpline(subdivStartDist);
		startTangent = Spline->ComponentToWorld.InverseTransformVector(startTangent);
		// the tangent is currently normalized (because we used GetWorldDirection), so scale it up
		startTangent *= segmentStartTangent.Size() / Segment.NumberMeshPerSegment;
		comp->SplineParams.StartTangent = startTangent;

		FVector endTangent = Spline->GetWorldDirectionAtDistanceAlongSpline(subdivEndDist);
		endTangent = Spline->ComponentToWorld.InverseTransformVector(endTangent);
		// the tangent is currently normalized (because we used GetWorldDirection), so scale it up
		endTangent *= segmentEndTangent.Size() / Segment.NumberMeshPerSegment;
		comp->SplineParams.EndTangent = endTangent;


		float startRoll=FMath::DegreesToRadians(Spline->GetRollAtDistanceAlongSpline(subdivStartDist,ESplineCoordinateSpace::Local));
		float endRoll=FMath::DegreesToRadians(Spline->GetRollAtDistanceAlongSpline(subdivEndDist,ESplineCoordinateSpace::Local));

		FVector startSplineScale=Spline->GetScaleAtDistanceAlongSpline(subdivStartDist);
		FVector2D startScale(startSplineScale.Y,startSplineScale.Z);

		FVector endSplineScale=Spline->GetScaleAtDistanceAlongSpline(subdivEndDist);
		FVector2D endScale(endSplineScale.Y,endSplineScale.Z);
 
		comp->SplineParams.StartRoll = startRoll;


		comp->SplineParams.EndRoll = endRoll;

		comp->SplineParams.StartScale = startScale;
		comp->SplineParams.EndScale = endScale;

		// make sure the spline data updates correctly
		comp->MarkSplineParamsDirty();

		// finish creating and registering the component
		comp->AttachTo(RootComponent);
		comp->AttachParent = RootComponent;
		comp->SetMobility(EComponentMobility::Static);
		comp->RegisterComponent();
        

        
		comp->CreationMethod = EComponentCreationMethod::SimpleConstructionScript;
	}
}

