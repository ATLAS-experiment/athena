#include "../CalibHitToCaloCell.h"
#include "../CalibHitIDCheck.h"
#include "../CaloCalibClusterMomentsMaker2.h"
#include "../CaloCalibClusterTruthAttributerTool.h"
#include "../CaloCalibClusterTruthMapMakerAlgorithm.h"
#include "../CaloCalibClusterDecoratorAlgorithm.h"

#include "../CalibHitToCaloCellTool.h"
#include "CaloCalibHitRec/CaloCalibClusterTruthMapMakerTool.h"
#include "CaloCalibHitRec/CaloCalibClusterDecoratorTool.h"
#include "CaloCalibHitRec/CaloCalibClusterDecoratorToolOOC.h"
#include "CaloCalibHitRec/CaloCalibClusterDecoratorToolDM.h"

DECLARE_COMPONENT( CalibHitToCaloCell )
DECLARE_COMPONENT( CalibHitIDCheck )

DECLARE_COMPONENT( CaloCalibClusterMomentsMaker2 )
DECLARE_COMPONENT( CaloCalibClusterTruthAttributerTool )
DECLARE_COMPONENT( CaloCalibClusterTruthMapMakerAlgorithm )
DECLARE_COMPONENT( CaloCalibClusterDecoratorAlgorithm )
DECLARE_COMPONENT( CalibHitToCaloCellTool )
DECLARE_COMPONENT( CaloCalibClusterTruthMapMakerTool )
DECLARE_COMPONENT( CaloCalibClusterDecoratorTool )
DECLARE_COMPONENT( CaloCalibClusterTruthAttributerTool )
DECLARE_COMPONENT( CaloCalibClusterDecoratorToolOOC )
DECLARE_COMPONENT( CaloCalibClusterDecoratorToolDM )