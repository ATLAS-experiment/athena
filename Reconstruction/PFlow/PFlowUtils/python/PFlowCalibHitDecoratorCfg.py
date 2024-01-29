# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PFlowCalibHitDecoratorCfg(flags):
    result=ComponentAccumulator()

    from CaloCalibHitRec.CaloCalibHitDecoratorCfg import CaloCalibHitDecoratorCfg 
    result.merge(CaloCalibHitDecoratorCfg(flags))

    #We use the cell links from topoclusters, so we also need to specify that the algorithm depends on the calorimeter cell container
    PFlowCalibPFODecoratorAlgorithm = CompFactory.PFlowCalibPFODecoratorAlgorithm(ExtraInputs = {('CaloCellContainer','StoreGateSvc+AllCalo')})
    PFlowCalibPFODecoratorAlgorithm.TruthAttributerTool = CompFactory.CaloCalibClusterTruthAttributerTool("PFlowCalibPFOTruthAttributerTool")
    result.addEventAlgo(PFlowCalibPFODecoratorAlgorithm)

    return result
