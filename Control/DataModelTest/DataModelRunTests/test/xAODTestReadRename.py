#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestReadRename.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Aug 2016
# Purpose: Test reading xAOD objects data with renaming on input.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg, rnt


def xAODTestReadRenameCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    is_rntuple, rntkey = rnt (flags)

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec',
                                               CVecKey = 'cvec_renamed',
                                               TestDecorSE = not is_rntuple))
    acc.addEventAlgo (DMTest.xAODTestReadDecor ('xAODTestReadDecor',
                                                CVecName = 'cvec_renamed',
                                                DecorName = 'dInt1_renamed'))

    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge (InputRenameCfg ('DMTest::CVec', 'cvec', 'cvec_renamed'))
    acc.merge (InputRenameCfg ('DMTest::CAuxContainer',
                               'cvecAux.', 'cvec_renamedAux.'))
    acc.merge (InputRenameCfg ('DMTest::CVec', 'cvec.dInt1',
                               'cvec_renamed.dInt1_renamed'))
    acc.merge (InputRenameCfg ('DMTest::C', 'cinfo.dInt1',
                               'cinfo.dInt1_renamed'))
    acc.merge (InputRenameCfg ('DMTest::C', 'cinfo.dInt1Base',
                               'cinfo.dInt1_renamedBase'))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestReadRename', loadReadDicts = True)
cfg.merge (xAODTestReadRenameCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

