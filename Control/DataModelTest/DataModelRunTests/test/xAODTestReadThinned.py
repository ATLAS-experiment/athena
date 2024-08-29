#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/xAODTestReadThinned.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Aug 2019
# Purpose: Test thinning xAOD objects.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def xAODTestReadThinnedCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec'))
    acc.addEventAlgo (DMTest.xAODTestReadCLinks ('xAODTestReadCLinks'))
    acc.addEventAlgo (DMTest.xAODTestReadCVec ('xAODTestReadCVec2',
                                               CVecKey = 'cvec2',
                                               Brief = True))
    acc.addEventAlgo (DMTest.xAODTestReadJVec ('xAODTestReadJVec',
                                               JVecInfoKey = ''))
    acc.addEventAlgo (DMTest.xAODTestReadPLinks ('xAODTestReadPLinks',
                                                 PLinksInfoKey = ''))

    return acc


flags = DataModelTestFlags (infile = 'xaodthinned1.root')
flags.fillFromArgs()
flags.lock()

cfg = DataModelTestCfg (flags, 'xAODTestReadThinned', loadWriteDicts = True)
cfg.merge (xAODTestReadThinnedCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

