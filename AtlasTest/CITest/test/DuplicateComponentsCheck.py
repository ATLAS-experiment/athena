#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
from AthenaCommon.ConfigurableDb import cfgDb, getConfigurable

getConfigurable('xAODMaker::EventInfoCnvAlg')

if cfgDb.duplicates():
    sys.exit(1)
