#include "../CaloTowerAlgorithm.h"
#include "../CaloTopoTowerAlgorithm.h"
#include "../CaloClusterMaker.h"
#include "../CaloTopoClusterMaker.h"
#include "../CaloTopoClusterSplitter.h"
#include "../CaloClusterCopier.h"
#include "../CaloClusterMomentsMaker.h"
#include "../CaloClusterMomentsMaker_DigiHSTruth.h"
#include "../CaloCell2ClusterMapper.h"
#include "../CaloCellContainerCorrectorTool.h"
#include "../CaloCellContainerCheckerTool.h"
#include "../CaloCellContainerFinalizerTool.h"
#include "../CaloCellMaker.h"
#include "../CaloConstCellMaker.h"
//#include "CaloRec/CaloClusterCellSlimmer.h" Really obsolete AFAIK (commissioning legacy)
#include "../CaloClusterCorrDBWriter.h"
#include "../CaloClusterCorrDumper.h"
#include "../CaloTopoTowerAlg.h"
#include "../Blob2ToolConstants.h"
#include "../CaloCellFastCopyTool.h"
#include "../CaloCellContCopyTool.h"
#include "../CaloTowerxAODFromCells.h"
#include "../CaloTowerxAODFromClusters.h"
#include "../CaloClusterSnapshot.h"
#include "../CaloBCIDAvgAlg.h"
#include "../CaloBCIDCoeffsCondAlg.h"
#include "../CaloBCIDLumiCondAlg.h"
#include "../CaloCellDumper.h"
#include "../CaloThinCellsByClusterAlg.h"
#include "../CaloThinCellsBySamplingAlg.h"
#include "../CaloCellContainerAliasAlg.h"
#include "../ToolConstantsCondAlg.h"
#include "../CaloNoiseSigmaDiffCondAlg.h"
#include "../CaloClusterMLCalibAlgLite.h"
#include "../CaloClusterEnergyMLCalibDecorAlg.h"

//Includes for CaloTopoTowers
#include "../CaloTopoClusterTowerMerger.h"
#include "../CaloTopoTowerFromClusterCalibrator.h"
#include "../CaloTopoTowerFromClusterMaker.h"
#include "../CaloTopoTowerMaker.h"
#include "../CaloTopoTowerBuilderTool.h"

#include "../CaloTowerStoreTestAlg.h"


DECLARE_COMPONENT( CaloTowerAlgorithm )
DECLARE_COMPONENT( CaloTopoTowerAlgorithm )
DECLARE_COMPONENT( CaloClusterMaker )
DECLARE_COMPONENT( CaloCell2ClusterMapper )
DECLARE_COMPONENT( CaloCellMaker )
DECLARE_COMPONENT( CaloConstCellMaker )

//DECLARE_COMPONENT( CaloClusterCellSlimmer )
DECLARE_COMPONENT( CaloClusterCorrDBWriter )
DECLARE_COMPONENT( CaloClusterCorrDumper )
DECLARE_COMPONENT( CaloTopoTowerAlg )

DECLARE_COMPONENT( CaloTowerxAODFromCells )
DECLARE_COMPONENT( CaloTowerxAODFromClusters )

DECLARE_COMPONENT( CaloTopoClusterMaker )
DECLARE_COMPONENT( CaloTopoClusterSplitter )
DECLARE_COMPONENT( CaloClusterCopier )
DECLARE_COMPONENT( CaloClusterMomentsMaker )
DECLARE_COMPONENT( CaloClusterMomentsMaker_DigiHSTruth )
DECLARE_COMPONENT( CaloCellContainerCorrectorTool )
DECLARE_COMPONENT( CaloCellContainerCheckerTool )
DECLARE_COMPONENT( CaloCellContainerFinalizerTool )

DECLARE_COMPONENT( Blob2ToolConstants )
DECLARE_COMPONENT( CaloCellFastCopyTool )
DECLARE_COMPONENT( CaloCellContCopyTool )
DECLARE_COMPONENT( CaloClusterSnapshot )

DECLARE_COMPONENT( CaloBCIDAvgAlg )
DECLARE_COMPONENT( CaloBCIDCoeffsCondAlg )
DECLARE_COMPONENT( CaloBCIDLumiCondAlg )

DECLARE_COMPONENT (CaloCellDumper)

DECLARE_COMPONENT (CaloThinCellsByClusterAlg)
DECLARE_COMPONENT (CaloThinCellsBySamplingAlg)
DECLARE_COMPONENT (CaloCellContainerAliasAlg)
DECLARE_COMPONENT (ToolConstantsCondAlg)
DECLARE_COMPONENT (CaloNoiseSigmaDiffCondAlg)

DECLARE_COMPONENT( CaloTopoClusterTowerMerger )
DECLARE_COMPONENT( CaloTopoTowerFromClusterMaker )
DECLARE_COMPONENT( CaloTopoTowerFromClusterCalibrator )
DECLARE_COMPONENT( CaloTopoTowerMaker )

DECLARE_COMPONENT ( CaloTopoTowerBuilderTool )

DECLARE_COMPONENT( CaloTowerStoreTestAlg )
DECLARE_COMPONENT( CaloClusterMLCalibAlgLite)
DECLARE_COMPONENT( CaloClusterEnergyMLCalibDecorAlg )
