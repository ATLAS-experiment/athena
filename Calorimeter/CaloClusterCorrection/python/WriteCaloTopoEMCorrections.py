#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloClusterCorrection/python/WriteCaloTopoEMCorrections.py
# Author: scott snyder <snyder@bnl.gov>
# Date: May 2024, from old config version
# Purpose: Write cluster correction constants to pool+cool.
#


CALOCORR_POOLFILE = 'CaloTopoEMCorrections.pool.root'
CALOCORR_COOLFILE = 'topoemcool.db'
CaloTopoEMCorrKeys = ['ele633', 'ele420', 'gam633']


from AthenaConfiguration.MainServicesConfig import \
        MainServicesCfg
from CaloClusterCorrection.WriteCorrectionsConfig import \
    WriteCorrectionsFlags, WriteCorrectionsCfg

flags = WriteCorrectionsFlags (CALOCORR_COOLFILE)
cfg = MainServicesCfg (flags)


from CaloClusterCorrection.CaloTopoEMCorrections import CaloTopoEMCorrections
from CaloClusterCorrection.constants import CALOCORR_EMTOPO
(corr_output_list, tag_list, ca) =\
                   CaloTopoEMCorrections.config_for_pool (flags,
                                                          CaloTopoEMCorrKeys,
                                                          CALOCORR_EMTOPO)
cfg.merge (ca)

cfg.merge (WriteCorrectionsCfg (flags, CALOCORR_POOLFILE,
                                corr_output_list, tag_list))


sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
