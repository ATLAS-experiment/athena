#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestRead3.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Sep 2016
# Purpose: Test reading xAOD objects data with renaming on input.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestReadFilterCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest
    acc.addEventAlgo (DMTest.xAODTestFilterCVec ('xAODTestFilterCVec'))
    acc.addEventAlgo (DMTest.xAODTestWriteCInfo ('xAODTestWriteCInfo',
                                                 Offset = 111))
    acc.addEventAlgo (DMTest.xAODTestDecor ('xAODTestDecor',
                                            DoCVec = False,
                                            DoCTrig = False))

    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge (InputRenameCfg ('DMTest::CVec', 'cvec', 'cvec_renamed'))
    acc.merge (InputRenameCfg ('DMTest::CAuxContainer',
                               'cvecAux.', 'cvec_renamedAux.'))

    itemList = [ 'DMTest::CVec#cvec',
                 'xAOD::AuxContainerBase!#cvecAux.',
                 'DMTest::C#cinfo',
                 'DMTest::CInfoAuxContainer#cinfoAux.' ]
    typeNames = [ 'DataVector<DMTest::C_v1>',
                  'DMTest::CAuxContainer_v1',
                  'DMTest::C_v1' ]
    acc.merge (TestOutputCfg (flags, 'Stream1', itemList, typeNames ))

    return acc


flags = DataModelTestFlags (infile = 'xaoddata.root',
                            Stream1 = 'xaoddata_filt.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestReadFilter', loadReadDicts = True)
cfg.merge (xAODTestReadFilterCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

