# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/CaloClusterMLCalibAlgLiteConfig.py
# Purpose: Configure CaloClusterMLCalibAlgLite.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from CaloClusterCorrection.CaloClusterMLCalibToolLiteCfg import CaloClusterMLCalibToolLiteCfg

def CaloClusterMLCalibAlgLiteCfg(flags, name="CaloClusterMLCalibAlgLite", **kwargs):
    print("CaloClusterMLCalibAlgLiteCfg: Configuring CaloClusterMLCalibAlgLite...")
    ca = ComponentAccumulator()

    clusterContainerName = "CaloCalTopoClusters"         # the name you want on disk at the end

    alg = CompFactory.CaloClusterMLCalibAlgLite(name, **kwargs)
    alg.CaloClusterMLCalibToolLite = ca.popToolsAndMerge(CaloClusterMLCalibToolLiteCfg(flags))

    # Read the temp key, write the legacy key
    alg.ClusterContainer = clusterContainerName

    # Decor goes on the OUTPUT (legacy) collection
    alg.ClusterMLCalibratedEnergyKeyName = f"{clusterContainerName}.clusterE_ML"
    alg.ClusterMLCalibratedEnergyUncKeyName = f"{clusterContainerName}.clusterE_ML_unc"

    ca.addEventAlgo(alg)
    return ca
