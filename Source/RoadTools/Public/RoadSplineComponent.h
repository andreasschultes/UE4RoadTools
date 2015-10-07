// Copyright (c) 2015 Andreas Schultes
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once


#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "RoadSplineComponent.generated.h"


USTRUCT()
struct FRoadSplineSegmentInfo
{
    GENERATED_USTRUCT_BODY()
    
    /** Mesh for this Segment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Segment)
    UStaticMesh* Mesh;
    /** Number of Meshes for this segment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Segment)
    int32 NumberMeshPerSegment;
    
    FRoadSplineSegmentInfo():
    Mesh(nullptr),
    NumberMeshPerSegment(4)
    {};
};
    


UCLASS()
class URoadSplineComponent: public USplineComponent
{
    GENERATED_BODY()
public:
    URoadSplineComponent(const FObjectInitializer& ObjectInitializer= FObjectInitializer::Get());

    /** Road segment info array */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Segment)
    TArray<FRoadSplineSegmentInfo> RoadSplineSegmentInfo;
};