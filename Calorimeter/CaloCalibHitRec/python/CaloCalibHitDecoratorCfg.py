# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloCalibHitDecoratorCfg(flags, name = "CaloCalibClusterDecoratorAlgorithm", **kwargs):
    result=ComponentAccumulator()

    result.addEventAlgo(CompFactory.CaloCalibClusterTruthMapMakerAlgorithm())

    kwargs.setdefault("CaloClusterWriteDecorHandleKey_NLeadingTruthParticles", "CaloCalTopoClusters."+flags.Calo.TopoCluster.CalibrationHitDecorationName)

    #We use the cell links from topoclusters, so we also need to specify that the algorithm depends on the calorimeter cell container
    CaloCalibClusterDecoratorAlgorithm = CompFactory.CaloCalibClusterDecoratorAlgorithm(name,**kwargs,ExtraInputs =  {('CaloCellContainer','StoreGateSvc+AllCalo')})
    CaloCalibClusterDecoratorAlgorithm.TruthAttributerTool = CompFactory.CaloCalibClusterTruthAttributerTool()
    result.addEventAlgo(CaloCalibClusterDecoratorAlgorithm)

    return result

#This configures a version of the tool that stores the full, rather than only the visible, calibration hit truth energy in the cluster
def CaloCalibHitDecoratorFullEnergyCfg( flags, name = "CaloCalibCLusterDecoratorAlgorithm_Full", **kwargs):

    result = ComponentAccumulator()
    result.merge(CaloCalibHitDecoratorCfg(flags, name, **kwargs))
    result.getEventAlgo(name).TruthAttributerTool.storeFullTruthEnergy = True
    return result
