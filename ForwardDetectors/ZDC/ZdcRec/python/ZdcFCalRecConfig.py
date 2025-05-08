#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from ZdcNtuple.ZdcNtupleConfig import ZdcNtupleCfg
from ZdcRecConfig import ZdcGenericFlagSetting, ZdcStreamDependentFlagSetting

# -------------------------------- Configuration function for calorimeter & HIGlocal reconstructions -------------------------------- 
def ZdcFCalAdditionalFlagSetting(flags):
    """additional flag settings in possible scenarios we may need FCal info"""

    # turn necessary trigger inputs on if need ZDC triggers (ZdcCalib or UCC stream)
    if (flags.Input.TriggerStream == "calibration_ZDCCalib" or flags.Input.TriggerStream == "physics_UCC"):
        flags.Trigger.L1.doCaloInputs = True
        flags.Trigger.L1.doCTP = True

    # turn off Detector.EnableCalo for none-physics stream
    if not (flags.Input.TriggerStream == "physics_MinBias" or flags.Input.TriggerStream == "express_express" or flags.Input.TriggerStream == "physics_UCC"):
        flags.Detector.EnableCalo = False



def FCalRecCfg(flags):
    """Calorimeter and HIGlobal reconstruction config for ZDC-FCal correlations"""
    
    acc = ComponentAccumulator()

    from CaloRec.CaloRecoConfig import CaloRecoCfg
    acc.merge(CaloRecoCfg(flags))
    from HIGlobal.HIGlobalConfig import HIGlobalRecCfg
    acc.merge(HIGlobalRecCfg(flags))

    return acc

def ZdcNtupleWithCaloRun3Cfg(flags, outputlevel = 3, **kwargs):
    """ZDC Ntuple configuration for run3 data potential with Calorimeter-info retrieving & writing
    Depends on the flag Detector.EnableCalo to be correctly set
    Retrieves and writes calorimeter info in ZdcNTuple if and only if Detector.EnableCalo is true"""

    acc = ComponentAccumulator()
    ZdcNtupleAcc = ZdcNtupleCfg(flags,
                           useGRL = False,
                           zdcOnly = not flags.Detector.EnableCalo,
                           lhcf2022 = False,
                           lhcf2022zdc = False,
                           lhcf2022afp = False,
                           isMC = flags.Input.isMC,
                           enableTrigger = not flags.Input.isMC,
                           enableOutputSamples = True,
                           enableOutputTree = True,
                           writeOnlyTriggers = False,
                           enableRPD = flags.Detector.EnableZDC_RPD,
                           enableRPDAmp = flags.Detector.EnableZDC_RPD,
                           enableCentroid = flags.Detector.EnableZDC_RPD,
                           enableCalo = flags.Detector.EnableCalo,
                           reprocZdc = False,
                           **kwargs)
    ZdcNtupleAcc.getEventAlgo('ZdcNtuple').OutputLevel = outputlevel # turn on DEBUG messages
    acc.merge(ZdcNtupleAcc)

    acc.addService(CompFactory.THistSvc(Output = ["ANALYSIS DATAFILE='NTUP.root' OPT='RECREATE'"]))
    return acc


if __name__ == '__main__':

    """ This is selftest & ZDC calibration transform at the same time"""
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()

    ZdcGenericFlagSetting(flags) # set generic (stream-independent) ZDC flags
    
    ZdcFCalAdditionalFlagSetting(flags)

    flags.Output.AODFileName="AOD.pool.root"
    flags.Output.HISTFileName="HIST.root"
    flags.Output.doWriteAOD=True

    isLED, isInj, isCalib, pn = ZdcStreamDependentFlagSetting(flags) # set stream-dependent ZDC flags & get return values

    flags.lock()
    flags.dump(evaluate=True) # uncomment this line if needed for testing

    acc=MainServicesCfg(flags)

    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    if not flags.Input.isMC and pn != 'data_test': # trigger reco config not existing for MC or standalone
        from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
        acc.merge(TriggerRecoCfgData(flags))

    if isLED:
        #acc.merge(ZdcLEDTrigCfg(flags))
        from ZdcRecConfig import ZdcLEDRecCfg
        acc.merge(ZdcLEDRecCfg(flags))
    if isCalib: # should be able to run both if in standalone data
        from ZdcRecConfig import ZdcRecCfg
        acc.merge(ZdcRecCfg(flags))
    if isInj: # should be able to run both if in standalone data
        from ZdcRecConfig import ZdcRecCfg
        acc.merge(ZdcRecCfg(flags))

    if (flags.Input.TriggerStream == "physics_MinBias" or flags.Input.TriggerStream == "express_express" or flags.Input.TriggerStream == "physics_UCC"):
        acc.merge(FCalRecCfg(flags))

    if not flags.Input.isMC:
        if (isLED):
            from ZdcMonitoring.ZdcLEDMonitorAlgorithm import ZdcLEDMonitoringConfig
            acc.merge(ZdcLEDMonitoringConfig(flags,'ppPbPb2023'))
            from ZdcRecConfig import ZdcLEDNtupleCfg
            acc.merge(ZdcLEDNtupleCfg(flags))
            
        if (isCalib):
            from ZdcMonitoring.ZdcMonitorAlgorithm import ZdcMonitoringConfig
            zdcMonitorAcc = ZdcMonitoringConfig(flags)
            acc.merge(zdcMonitorAcc)
            acc.merge(ZdcNtupleWithCaloRun3Cfg(flags))

        if (isInj):
            from ZdcMonitoring.ZdcMonitorAlgorithm import ZdcMonitoringConfig            
            zdcMonitorAcc = ZdcMonitoringConfig(flags)
            acc.merge(zdcMonitorAcc)
            from ZdcRecConfig import ZdcInjNtupleCfg
            acc.merge(ZdcInjNtupleCfg(flags))            
    else:
        from ZdcRecConfig import ZdcRecCfg, ZdcNtupleLocalCfg
        acc.merge(ZdcRecCfg(flags))
        acc.merge(ZdcNtupleLocalCfg(flags))

    acc.printConfig(withDetails=True)

    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)

