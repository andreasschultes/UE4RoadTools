// Copyright (c) 2015 Andreas Schultes
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "RoadToolsPrivatePCH.h"
#include "RoadSplineComponent.h"

URoadSplineComponent::URoadSplineComponent(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer)
{
	RoadSplineSegmentInfo.Reset(10);
	RoadSplineSegmentInfo.Emplace();
    
}
#if WITH_EDITOR
void URoadSplineComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		static const FName ReparamStepsPerSegmentName = GET_MEMBER_NAME_CHECKED(USplineComponent, ReparamStepsPerSegment);
		static const FName StationaryEndpointsName = GET_MEMBER_NAME_CHECKED(USplineComponent, bStationaryEndpoints);
		static const FName DefaultUpVectorName = GET_MEMBER_NAME_CHECKED(USplineComponent, DefaultUpVector);
		

		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		if (PropertyName == ReparamStepsPerSegmentName ||
			PropertyName == StationaryEndpointsName ||
			PropertyName == DefaultUpVectorName )
		{
			if(AActor* Owner=GetOwner())
				Owner->RerunConstructionScripts();
		}
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif