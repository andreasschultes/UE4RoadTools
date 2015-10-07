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
