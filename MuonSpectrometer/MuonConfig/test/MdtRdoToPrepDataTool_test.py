#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: MuonMDT_CnvTools/share/MdtRdoToPrepDataTool_test.py
# Author: scott snyder
# Date: Jul, 2019
# Brief: Test for MdtRdoToPrepDataTool.
#        For now, only tests the dead tube handling.
#


from __future__ import print_function

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython.PyAthenaComps import Alg, StatusCode
import ROOT


class TestAlg (Alg):
    def __init__ (self, name):
        Alg.__init__ (self, name)
        return

    def initialize (self):
        ROOT.Muon.IMuonRdoToPrepDataTool

        self.tool = ROOT.ToolHandle(ROOT.Muon.IMuonRdoToPrepDataTool)('Muon::MdtRdoToPrepDataToolMT/Muon__MdtRdoToPrepDataTool')
        if not self.tool.retrieve():
            assert 0
        return StatusCode.Success


    def execute (self):
        return StatusCode.Success
        

def testCfg (flags):
    result = ComponentAccumulator()

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge (MuonGeoModelCfg(flags))

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge (AtlasFieldCacheCondAlgCfg(flags, UseDCS = False))

    Muon__MdtRdoToPrepDataTool = CompFactory.Muon.MdtRdoToPrepDataToolMT
    result.addPublicTool (Muon__MdtRdoToPrepDataTool ('Muon__MdtRdoToPrepDataTool', OutputLevel = 1))
    
    result.addEventAlgo (TestAlg ('TestAlg'))
    return result


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RAW_RUN2
flags.lock()
from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
acc=MainServicesCfg(flags)

acc.merge (testCfg (flags))

#Want to see all verbose messages in this test
acc.getService("MessageSvc").enableSuppression = False

acc.run(1)
