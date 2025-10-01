#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def L0MuonMDTSimCfg(flags, name = "L0MuonMDTSim", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.MDTSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = 'L0MuonMDTSim'
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)

    alg.MonTool = monTool
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile = f"{name}.root", outStream="EXPERT"))
    result.addEventAlgo(alg)
    return result


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()
    parser.set_defaults(inputFile= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/myRDO.R3.pool.root"])
    parser.set_defaults(nEvents = 20)
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    from MuonConfig.MuonByteStreamCnvTestConfig import MdtRdoToMdtDigitCfg
    acc.merge(MdtRdoToMdtDigitCfg(flags))
    # example simulation alg
    acc.merge(L0MuonMDTSimCfg(flags,
                             name = "L0MuonMDTSim",
                             OutputLevel = DEBUG))

    executeTest(acc)

