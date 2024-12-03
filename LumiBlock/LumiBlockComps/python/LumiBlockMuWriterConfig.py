# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# File: LumiBlockComps/python/LumiBlockMuWriterConfig.py
# Created: May 2020, sss
# Purpose: Configure LumiBlockMuWriter.
#


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType


def LumiBlockMuWriterCfg (flags, name = 'LumiBlockMuWriter', seqName="AthAlgSeq"):
    result = ComponentAccumulator(seqName)

    if flags.Beam.Type is BeamType.Cosmics or flags.Input.isMC:
        condkey = ''
    else:
        from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
        result.merge (LuminosityCondAlgCfg (flags))
        condkey = result.getCondAlgo ('LuminosityCondAlg').LuminosityOutputKey

    from AthenaConfiguration.Enums import ProductionStep
    kwargs={}
    if flags.Common.ProductionStep in [ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
        actualLumiKey=f"{flags.Overlay.BkgPrefix}EventInfo.actualInteractionsPerCrossing"
        averageLumiKey=f"{flags.Overlay.BkgPrefix}EventInfo.averageInteractionsPerCrossing"

        kwargs.update({"actualInteractionsPerCrossingKey" : actualLumiKey,
                       "averageInteractionsPerCrossingKey": averageLumiKey})

    LumiBlockMuWriter = CompFactory.LumiBlockMuWriter # LumiBlockComps
    alg = LumiBlockMuWriter (name,
                             LumiDataKey = condkey,
                             **kwargs)
    #In the HLT we want to run LumiBlockMuWriter as a normal EventAlgo, but in a pre-event sequence (HLTBeginSeq)
    if  flags.Trigger.doHLT:
         result.addEventAlgo(alg)
    #For offline and particularly serial athena, add LumiBlockMuWriter to AthCondSeq to ensure it runs first (ATR-24721)
    else:
         result.addCondAlgo(alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- collisions')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = LumiBlockMuWriterCfg (flags1)
    acc1.printCondAlgs (summariseProps=True)
    acc1.wasMerged()

    print ('--- cosmics')
    flags2 = initConfigFlags()
    flags2.Input.Files = []
    flags2.Beam.Type = BeamType.Cosmics
    flags2.lock()
    acc2 = LumiBlockMuWriterCfg (flags2)
    acc2.printCondAlgs (summariseProps=True)
    acc2.wasMerged()
