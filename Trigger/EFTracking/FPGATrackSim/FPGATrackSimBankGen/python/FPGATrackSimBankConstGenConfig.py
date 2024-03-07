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

def FPGATrackSimConstsGenCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("merged_file_path", flags.Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx)
    kwargs.setdefault("region", flags.Trigger.FPGATrackSim.FPGATrackSimBankRegion)
    kwargs.setdefault("CheckGood2ndStage",flags.Trigger.FPGATrackSim.CheckGood2ndStage)
    kwargs.setdefault("UseHitScaleFactor",flags.Trigger.FPGATrackSim.UseHitScaleFactor)
    kwargs.setdefault("IsSecondStage",flags.Trigger.FPGATrackSim.Is2ndStage)
    kwargs.setdefault("missHitsConsts",flags.Trigger.FPGATrackSim.missHitsConsts)

    FPGATrackSimMapping = acc.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))
    theFPGATrackSimConstGenAlg = CompFactory.FPGATrackSimConstGenAlgo(**kwargs)
    theFPGATrackSimConstGenAlg.FPGATrackSimMappingSvc = FPGATrackSimMapping 

    eventSelector = CompFactory.FPGATrackSimEventSelectionSvc()
    eventSelector.regions = "HTT/TrigHTTMaps/V1/map_file/slices_v01_Jan21.txt"
    eventSelector.regionID = 0
    eventSelector.sampleType = flags.Trigger.FPGATrackSim.sampleType
    eventSelector.withPU = False
    acc.addService(eventSelector, create=True, primary=True)

    acc.addEventAlgo(theFPGATrackSimConstGenAlg)
    return acc

def FPGATrackSimMappingCfg(flags):
    result=ComponentAccumulator()

    mappingSvc = CompFactory.FPGATrackSimMappingSvc()
    mappingSvc.mappingType = "FILE"
    mappingSvc.rmap = flags.Trigger.FPGATrackSim.mapsDir+"/eta0103phi0305.rmap" # we need more configurability here i.e. file choice should depend on some flag
    mappingSvc.subrmap =  flags.Trigger.FPGATrackSim.mapsDir+"/eta0103phi0305.subrmap" # presumably also here we want to be able to change the slices definition file
    mappingSvc.pmap = flags.Trigger.FPGATrackSim.mapsDir+"/pmap"
    mappingSvc.modulemap = flags.Trigger.FPGATrackSim.mapsDir+"/moduleidmap"
    mappingSvc.NNmap = ""
    mappingSvc.layerOverride = []
    result.addService(mappingSvc, create=True, primary=True)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags = initConfigFlags()
    flags.fillFromArgs()
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

