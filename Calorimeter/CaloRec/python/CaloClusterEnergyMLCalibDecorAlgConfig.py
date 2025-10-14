# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/CaloClusterEnergyMLCalibDecorAlgConfig.py
# Purpose: Configure CaloClusterEnergyMLCalibDecorAlg.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from CaloClusterCorrection.CaloClusterMLCalibToolLiteCfg import CaloClusterMLCalibToolLiteCfg


def CaloClusterEnergyMLCalibDecorAlgCfg(flags, name="CaloClusterEnergyMLCalibDecorAlg"):
    ca = ComponentAccumulator()
    alg = CompFactory.CaloClusterEnergyMLCalibDecorAlg(name)
    alg.CaloClusterMLCalibToolLite = ca.popToolsAndMerge(CaloClusterMLCalibToolLiteCfg(flags))
    ca.addEventAlgo(alg)
    return ca
