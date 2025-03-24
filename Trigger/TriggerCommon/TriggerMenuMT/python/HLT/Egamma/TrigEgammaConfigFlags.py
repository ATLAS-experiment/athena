# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTrigEgammaConfigFlags():
    flags = AthConfigFlags()

    # Particle ID tune
    flags.addFlag('Trigger.egamma.electronPidVersion'          , 'ElectronPhotonSelectorTools/trigger/rel21_20232105/')
    flags.addFlag('Trigger.egamma.electronNoPixPidVersion'     , 'ElectronPhotonSelectorTools/trigger/rel21_20232105/')
    flags.addFlag('Trigger.egamma.electronNoGSFPidVersion'     , 'ElectronPhotonSelectorTools/trigger/rel21_20232105/')
    flags.addFlag('Trigger.egamma.electronNoGSFNoPixPidVersion', 'ElectronPhotonSelectorTools/trigger/rel21_20232105/')
    flags.addFlag('Trigger.egamma.electronHIPidVersion'        , 'ElectronPhotonSelectorTools/trigger/rel22_20210611/')
    flags.addFlag('Trigger.egamma.photonPidVersion'            , 'ElectronPhotonSelectorTools/trigger/rel22_20210611/')
    flags.addFlag('Trigger.egamma.dnnVersion'                  , 'ElectronPhotonSelectorTools/trigger/R22_20241216_OfflineTargets/')
    flags.addFlag('Trigger.egamma.ringerVersion'               , 'RingerSelectorTools/trigger/Run3_20230316_v1')
    flags.addFlag('Trigger.egamma.photonRingerVersion'         , 'ElectronPhotonSelectorTools/trigger/rel23_20250321')
    flags.addFlag('Trigger.egamma.electronRingerFastElectronVersion'         , 'ElectronPhotonSelectorTools/trigger/rel24_20250205')


    # cluster correction version, allowed value is: None or v12phiflip_noecorrnogap
    flags.addFlag('Trigger.egamma.clusterCorrectionVersion', 'v12phiflip_noecorrnogap')

    # tune of MVA
    flags.addFlag('Trigger.egamma.Calib.precCaloMVAVersion'  , 'egammaMVACalib/online/v7')
    flags.addFlag('Trigger.egamma.Calib.precEgammaMVAVersion', 'egammaMVACalib/online/v7')

    # Dump rings cells information at FastCalo
    flags.addFlag('Trigger.egamma.dumpCellsFastCaloInfo', False)

    # Do QuarterRings feature extraction at Fastcalo
    flags.addFlag('Trigger.egamma.doQuarterRings', False)

    # Build rings with a sigmanoise factor constraint / phase2 studies
    flags.addFlag('Trigger.egamma.doNoiseThrRings', False)
    flags.addFlag('Trigger.egamma.sigmaNoiseFactor',2.0)

    # Fastcalo bdt calibration 
    flags.addFlag('Trigger.egamma.fastCaloETCalibration',False)
    flags.addFlag('Trigger.egamma.fastCaloETCalibrationVersion','egammaFastCaloCalib/online/v0')
    flags.addFlag('Trigger.egamma.CalibrationETThreshold', 3.)

    # Precision Electron Isolation Validation 
    flags.addFlag('Trigger.egamma.isoValidation', False)
    
    # Fast Electron Ringer Validation
    flags.addFlag('Trigger.egamma.enableFastElectronRinger', False)

    return flags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()
    flags.dump("Egamma|Trigger")
