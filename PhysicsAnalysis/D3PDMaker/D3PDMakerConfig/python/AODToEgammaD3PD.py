#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
#
# File: D3PDMakerConfig/python/AODToEgammaD3PD.py
# Author: snyder@bnl.gov
# Date: Dec 2023, from old config
# Purpose: Make egamma D3PD.
#

from D3PDMakerConfig.D3PDMakerFlags import configFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

configFlags.Reco.EnableBTagging = False
configFlags.fillFromArgs()
configFlags.lock()

cfg = MainServicesCfg (configFlags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge (PoolReadCfg (configFlags))

# Remake jets.
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
trigger_lists_helper = TriggerListsHelper (configFlags)
from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
cfg.merge (PhysCommonAugmentationsCfg (configFlags, TriggerListsHelper = trigger_lists_helper))

from D3PDMakerConfig.egammaD3PDConfig import egammaD3PDCfg
cfg.merge (egammaD3PDCfg (configFlags))

sc = cfg.run (configFlags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

