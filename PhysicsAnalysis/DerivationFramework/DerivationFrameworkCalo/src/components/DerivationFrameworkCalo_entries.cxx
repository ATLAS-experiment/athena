#include "DerivationFrameworkCalo/CaloClusterThinning.h"
#include "DerivationFrameworkCalo/JetCaloClusterThinning.h"
#include "DerivationFrameworkCalo/CellsInConeThinning.h"
#include "DerivationFrameworkCalo/GainDecorator.h"
#include "DerivationFrameworkCalo/MaxCellDecorator.h"
#include "DerivationFrameworkCalo/ClusterEnergyPerLayerDecorator.h"
#include "DerivationFrameworkCalo/EGammaClusterCoreCellRecovery.h"
#include "DerivationFrameworkCalo/CaloCellDecorator.h"

using namespace DerivationFramework;
 
DECLARE_COMPONENT( CaloClusterThinning )
DECLARE_COMPONENT( JetCaloClusterThinning )
DECLARE_COMPONENT( CellsInConeThinning )
DECLARE_COMPONENT( GainDecorator )
DECLARE_COMPONENT( MaxCellDecorator )
DECLARE_COMPONENT( ClusterEnergyPerLayerDecorator )
DECLARE_COMPONENT( EGammaClusterCoreCellRecovery )
DECLARE_COMPONENT( CaloCellDecorator )


