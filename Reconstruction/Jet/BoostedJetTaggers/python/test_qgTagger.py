# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetToolHelpers.HelperConfig import HistoInputCfg

def qgAlgCfg(configFlags, **kwargs):

    acc = ComponentAccumulator()

    # input WP file
    inputfile = '/eos/atlas/atlascerngroupdisk/perf-jets/LocalStorage/TAGGING/PreliminaryConfigs/qgTagger/QGTagger_WPs.root'

    # configure the histogram reader
    tool_args = {}

    histo2D = HistoInputCfg(flags, "histo2D",
                            inputFile = inputfile,
                            histName = "QGTransformer_ConstScore__50WP",
                            varX = "pt", varY = "eta",
                            InterpType="None")

    tool_args.setdefault("HistoReader2D", histo2D)

    # pick the BJT tool
    tool = acc.popToolsAndMerge(BJTToolCfg(flags, **tool_args))

    # configure the bjt algo
    algo_args = {}
    algo_args.setdefault("tagger", tool)
    algo_args.setdefault("jets", "AntiKt4EMPFlowJets")
    acc.addEventAlgo(CompFactory.BJT.BoostedJetTaggerAlg("qgAlg", **algo_args))

    return acc

def BJTToolCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("CalibArea", "Local")
    kwargs.setdefault("ConfigFile", "/eos/atlas/atlascerngroupdisk/perf-jets/LocalStorage/TAGGING/PreliminaryConfigs/qgTagger/QGTagger_AntiKt04PFlow_Transformer.dat")
    kwargs.setdefault("ContainerName", "AntiKt4EMPFlowJets")
    kwargs.setdefault("IsMC", 1)
    kwargs.setdefault("OutputLevel", DEBUG)

    acc.setPrivateTools(CompFactory.BJT.qgTagger(**kwargs))

    return acc

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO, DEBUG 
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    fileName = "/eos/atlas/atlascerngroupdisk/perf-jets/TreeStorage/TAGGING/test-files/DAOD/mc20_13TeV.701125.Sh_2214_WlvWqq.deriv.DAOD_PHYS.e8547_s3797_r13145_p7018/DAOD_PHYS.46768158._000231.pool.root.1"

    flags = initConfigFlags()
    flags.Input.Files = [fileName]

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    testacc = qgAlgCfg(flags)
    cfg.merge(testacc)
    cfg.run(15)
