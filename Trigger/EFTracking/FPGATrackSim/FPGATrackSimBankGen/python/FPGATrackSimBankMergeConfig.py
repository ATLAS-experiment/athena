# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
'''
@file FPGATrackSimBankMergeConfig.py
@author Riley Xu - rixu@cern.ch
@date Sept 22, 2020
@brief This file declares functions to configure components in FPGATrackSimBankMerge
'''

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import glob

def getListOfFiles(regex):
  path = []
  for item in regex.split(','):
    if ('matrix' in item and 'root' in item):
      path += glob.glob(item)
  return path

def FPGATrackSimMatrixMergeCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("nbank",flags.Trigger.FPGATrackSim.FPGATrackSimNBanks)
    kwargs.setdefault("allregion",flags.Trigger.FPGATrackSim.FPGATrackSimallBanks)
    kwargs.setdefault("region",flags.Trigger.FPGATrackSim.FPGATrackSimBankRegion)

    theFPGATrackSimMatrixMergeAlg = CompFactory.FPGATrackSimMatrixMergeAlgo()
    file_path = getListOfFiles(flags.Trigger.FPGATrackSim.FPGATrackSimMatrixFileRegEx)

    if flags.Trigger.FPGATrackSim.FPGATrackSimMaxnMatrixInputFiles > 0:
        theFPGATrackSimMatrixMergeAlg.file_path = file_path[0:flags.Trigger.FPGATrackSim.MaxInputFiles]
    else:
        theFPGATrackSimMatrixMergeAlg.file_path = file_path

    acc.addEventAlgo(theFPGATrackSimMatrixMergeAlg)
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()

    acc=MainServicesCfg(flags)

    acc.merge(FPGATrackSimMatrixMergeCfg(flags))
    acc.store(open('FPGATrackSimMatrixMergeConfig.pkl','wb'))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    MatrixFileName=flags.Trigger.FPGATrackSim.outputMergedFPGATrackSimMatrixFile
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimMATRIXOUT DATAFILE='"+MatrixFileName+"', OPT='RECREATE'"]))

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

