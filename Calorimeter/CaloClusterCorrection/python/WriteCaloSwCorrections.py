#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloClusterCorrection/python/WriteCaloSwCorrections.py
# Author: scott snyder <snyder@bnl.gov>
# Date: May 2024, from old config version
# Purpose: Write cluster correction constants to pool+cool.
#


CALOCORR_POOLFILE = 'CaloSwCorrections.pool.root'
CALOCORR_COOLFILE = 'swcool.db'
CaloSwCorrKeys = ['ele55', 'ele35', 'ele37',
                  'gam55', 'gam35', 'gam37']


from AthenaConfiguration.MainServicesConfig import \
        MainServicesCfg
from CaloClusterCorrection.WriteCorrectionsConfig import \
    WriteCorrectionsFlags, WriteCorrectionsCfg

flags = WriteCorrectionsFlags (CALOCORR_COOLFILE)
cfg = MainServicesCfg (flags)


from CaloClusterCorrection.CaloSwCorrections import CaloSwCorrections
from CaloClusterCorrection.constants import CALOCORR_SW
(corr_output_list, tag_list, ca) =\
                   CaloSwCorrections.config_for_pool (flags,
                                                      CaloSwCorrKeys,
                                                      CALOCORR_SW)
cfg.merge (ca)

cfg.merge (WriteCorrectionsCfg (flags, CALOCORR_POOLFILE,
                                corr_output_list, tag_list))


sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())
