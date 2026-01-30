# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Simple script to run a
# Tau job
#
# Usefull for quick testing using EMPFlowML jets for tau seeding
# run with
#
# athena --CA runTauOnly_EMPFlowML.py 
# or
# python runTauOnly_EMPFlowML.py

import sys

def tauSpecialContent(flags,cfg):
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName
    StreamAOD = cfg.getEventAlgo(outputStreamName("AOD"))
    newList = [x for x in StreamAOD.ItemList if "Tau" in x]
    StreamAOD.ItemList = newList

    StreamESD = cfg.getEventAlgo(outputStreamName("ESD"))
    newList = [x for x in StreamESD.ItemList if "Tau" in x]
    StreamESD.ItemList = newList

def _run():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # input
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    flags.Exec.MaxEvents = 20
    flags.Input.Files = defaultTestFiles.RDO_RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Reconstruction

    # output
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Output.AODFileName = "myAOD.pool.root"

    # uncomment given something like export ATHENA_CORE_NUMBER=2
    # flags.Concurrency.NumThreads = 2

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=True,
                       toggle_geometry=True, keep_beampipe=True)

    # Schedule Tau Reco
    from tauRec.ConfigurationHelpers import StandaloneTauRecoFlags
    StandaloneTauRecoFlags(flags)
    flags.Tau.TauRec.SeedJetCollection = "AntiKt4EMPFlowMLJets" 
    flags.Tau.doTauEleRMRec = False

    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    # keep only tau containers
    tauSpecialContent(flags,acc)

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg
    acc.merge(DigitizationMessageSvcCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags
    printRecoFlags(flags)

    # running
    statusCode = acc.run()

    return statusCode


if __name__ == "__main__":
    statusCode = None
    statusCode = _run()
    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())


