#include "TrkVertexWeightCalculators/SumPtVertexWeightCalculator.h"
#include "TrkVertexWeightCalculators/TrueVertexDistanceWeightCalculator.h"
#include "TrkVertexWeightCalculators/BDTVertexWeightCalculator.h"
#include "TrkVertexWeightCalculators/GNNVertexWeightCalculator.h"
#include "TrkVertexWeightCalculators/DecorateVertexScoreAlg.h"
#include "TrkVertexWeightCalculators/JetRestrictedSumPtVertexWeightCalculator.h"

using namespace Trk;

DECLARE_COMPONENT( SumPtVertexWeightCalculator )
DECLARE_COMPONENT( TrueVertexDistanceWeightCalculator )
DECLARE_COMPONENT( BDTVertexWeightCalculator )
DECLARE_COMPONENT( GNNVertexWeightCalculator )
DECLARE_COMPONENT( DecorateVertexScoreAlg )
DECLARE_COMPONENT( JetRestrictedSumPtVertexWeightCalculator )
