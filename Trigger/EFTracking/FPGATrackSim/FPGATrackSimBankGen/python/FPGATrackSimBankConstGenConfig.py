# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
'''
@file FPGATrackSimBankConstGenConfig.py
@author Riley Xu - rixu@cern.ch
@date Sept 22, 2020
@brief This file declares functions to configure components in FPGATrackSimBankMerge
'''

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimEventSelectionSvcCfg

def FPGATrackSimConstsGenCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("merged_file_path", flags.Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx)
    kwargs.setdefault("region", flags.Trigger.FPGATrackSim.region)
    kwargs.setdefault("CheckGood2ndStage",flags.Trigger.FPGATrackSim.CheckGood2ndStage)
    kwargs.setdefault("UseHitScaleFactor",flags.Trigger.FPGATrackSim.UseHitScaleFactor)
    kwargs.setdefault("IsSecondStage",flags.Trigger.FPGATrackSim.Is2ndStage)
    kwargs.setdefault("missHitsConsts",flags.Trigger.FPGATrackSim.missHitsConsts)

    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimMappingCfg
    FPGATrackSimMapping = acc.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    theFPGATrackSimConstGenAlg = CompFactory.FPGATrackSimConstGenAlgo(**kwargs)
    theFPGATrackSimConstGenAlg.FPGATrackSimMappingSvc = FPGATrackSimMapping 

    # We need the "full" event selection service to be set up.
    theFPGATrackSimConstGenAlg.FPGATrackSimEventSelectionSvc = acc.getPrimaryAndMerge(FPGATrackSimEventSelectionSvcCfg(flags))

    acc.addEventAlgo(theFPGATrackSimConstGenAlg)
    return acc

def prepareFlagsForFPGATrackSimBankGen(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags = initConfigFlags()
    flags.fillFromArgs()
    flags = prepareFlagsForFPGATrackSimBankGen(flags)
    flags.lock()

    acc=MainServicesCfg(flags)

    acc.merge(FPGATrackSimConstsGenCfg(flags))
    acc.store(open('FPGATrackSimConstsGenConfig.pkl','wb'))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    MatrixFileName=flags.Trigger.FPGATrackSim.outputMergedFPGATrackSimMatrixFile
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimTREEGOODOUT DATAFILE='"+MatrixFileName+"', OPT='RECREATE'"]))
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimCTREEOUT DATAFILE='const.root', OPT='RECREATE'"]))

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

