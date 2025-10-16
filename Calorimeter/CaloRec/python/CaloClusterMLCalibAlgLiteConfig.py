# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/CaloClusterMLCalibAlgLiteConfig.py
# Purpose: Configure CaloClusterMLCalibAlgLite.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SGComps.AddressRemappingConfig import InputRenameCfg
from CaloClusterCorrection.CaloClusterMLCalibToolLiteCfg import CaloClusterMLCalibToolLiteCfg


def CaloClusterMLCalibAlgLiteCfg(flags, name="CaloClusterMLCalibAlgLite", **kwargs):
    ca = ComponentAccumulator()

    legacy = "CaloCalTopoClusters"         # the name you want on disk at the end
    tempIn = "CaloCalTopoClusters_in"      # a private, in-memory name for reading

    # --- Read old key under a temp name
    ca.merge(InputRenameCfg("xAOD::CaloClusterContainer",    legacy,        tempIn))
    ca.merge(InputRenameCfg("xAOD::CaloClusterAuxContainer", legacy+"Aux.", tempIn+"Aux."))

    alg = CompFactory.CaloClusterMLCalibAlgLite(name, **kwargs)
    alg.CaloClusterMLCalibToolLite = ca.popToolsAndMerge(CaloClusterMLCalibToolLiteCfg(flags))

    # Read the temp key, write the legacy key
    alg.InputClusterContainer  = tempIn
    alg.OutputClusterContainer = legacy

    # Decor goes on the OUTPUT (legacy) collection
    alg.ClusterMLCalibratedEnergyUncKeyName = f"{legacy}.clusterE_ML_unc"

    ca.addEventAlgo(alg)
    return ca
