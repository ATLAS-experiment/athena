#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


def defaultSCTCalibFlags(flags, **kwargs):
    flags.addFlag('SCTCalib.EventNumber', 999999)           # default; set in the job json file
    flags.addFlag('SCTCalib.RunNumber', 999999)             # default; set in the job json file
    flags.addFlag('SCTCalib.ForceRefRunNumber', False)      # force reference run to current run number when checking information uploaded to COOL for previous runs
    flags.addFlag('SCTCalib.RunStartTime', 999999)          # default; set in the job json file
    flags.addFlag('SCTCalib.RunEndTime', 999999)            # default; set in the job json file
    flags.addFlag('SCTCalib.EvtMax', -1)                    # default; set in the job json file
    flags.addFlag('SCTCalib.LBMax', -1)                     # default; set in the job json file
    flags.addFlag('SCTCalib.GeometryTag', '')               # empty in default
    flags.addFlag('SCTCalib.ConditionsTag', '')             # empty in default
    flags.addFlag('SCTCalib.DataSource', 'data')            # 'data' or 'geant4', although geant has not been used since >10 years
    flags.addFlag('SCTCalib.beamType', 'cosmics')           # 'cosmics', 'singlebeam' or 'collisions'

    #------------------------------------------------------------
    # Flags for input
    # - BS   : NoisyStrips, DeadChip, DeadStrip, QuietChip, QuietStrip (, HV)
    # - HIST : NoiseOccupancy, RawOccupancy, Efficiency, BSErrorDB, LorentzAngle
    #------------------------------------------------------------
    flags.addFlag('SCTCalib.ReadBS', True)
    flags.addFlag('SCTCalib.InputType', '')
    flags.addFlag('SCTCalib.InputHist', [''])

    #------------------------------------------------------------
    # Algorithm turned on
    # - Set only one flag to True and others to False
    #------------------------------------------------------------
    #--- Algs using TrkVal or BS
    flags.addFlag('SCTCalib.DoHitMaps', False)
    flags.addFlag('SCTCalib.DoHitMapsLB', False)
    flags.addFlag('SCTCalib.DoHV', False)
    flags.addFlag('SCTCalib.DoNoisyStrip', True)
    flags.addFlag('SCTCalib.DoDeadStrip', False)
    flags.addFlag('SCTCalib.DoDeadChip', False)
    flags.addFlag('SCTCalib.DoQuietStrip', False)
    flags.addFlag('SCTCalib.DoQuietChip', False)
    #--- Algs using HIST
    flags.addFlag('SCTCalib.DoNoiseOccupancy', False)
    flags.addFlag('SCTCalib.DoRawOccupancy', False)
    flags.addFlag('SCTCalib.DoEfficiency', False)
    flags.addFlag('SCTCalib.DoBSErrorDB', False)
    flags.addFlag('SCTCalib.DoLorentzAngle', False)

    #------------------------------------------------------------
    # Default setup for specific flags in SCTCalib
    # - This default is for NoisyStrips
    #------------------------------------------------------------
    flags.addFlag('SCTCalib.UseDCS', False)
    flags.addFlag('SCTCalib.UseConfiguration', True)
    flags.addFlag('SCTCalib.UseCalibration', True)
    flags.addFlag('SCTCalib.UseMajority', True)
    flags.addFlag('SCTCalib.UseBSError', False)
    flags.addFlag('SCTCalib.ReadHitMaps', True)            # True = two steps of HitMap production and analysis, False = simultaneous production and analysis
    flags.addFlag('SCTCalib.DoBSErrors', False)

    #------------------------------------------------------------
    # Properties in SCTCalib
    #------------------------------------------------------------
    #--- local DB
    flags.addFlag('SCTCalib.WriteToCool', True)            # To create the db output

    # hitmaps
    flags.addFlag('SCTCalib.LbsPerWindow', 30)             # number of lumi blocks when time dependent histograms are created

    #--- Noisy strips
    flags.addFlag('SCTCalib.NoisyUpdate', False)            # True = Write out difference from the existing data, False = Write out all (default since 2017)
    flags.addFlag('SCTCalib.NoisyWriteAllModules', True)
    flags.addFlag('SCTCalib.NoisyMinStat', 10000)           # 50000 (enough), 10000 (still possible), 5000 (for commissioning)
    flags.addFlag('SCTCalib.NoisyStripAll', True)           # True = All noisy strips into DB, False = Only newly found strips
    flags.addFlag('SCTCalib.NoisyStripThrDef', True)        # True = Offline, False = Calib
    flags.addFlag('SCTCalib.NoisyStripThrOffline', 1.5E-2)  # Threshold in Offline
    flags.addFlag('SCTCalib.NoisyStripThrOnline', 1.5E-3)   # Threshold in Online/Calibration (TBD)
    flags.addFlag('SCTCalib.NoisyWaferFinder', True)        # True = turn on noisy wafer criteria, False = turn off
    flags.addFlag('SCTCalib.NoisyWaferWrite', True)         # True = include noisy wafer, False = do not include
    flags.addFlag('SCTCalib.NoisyWaferAllStrips', False)    # True = write out all strips, False = only noisy strips
    flags.addFlag('SCTCalib.NoisyWaferThrBarrel', 1.0E-3)
    flags.addFlag('SCTCalib.NoisyWaferThrECA', 1.0E-3)
    flags.addFlag('SCTCalib.NoisyWaferThrECC', 1.0E-3)
    flags.addFlag('SCTCalib.NoisyWaferFraction', 0.5)
    flags.addFlag('SCTCalib.NoisyChipFraction', 0.5)
    flags.addFlag('SCTCalib.NoisyUploadTest', True)         # only false during the first few runs
    flags.addFlag('SCTCalib.NoisyModuleAverageInDB', -1.)
    flags.addFlag('SCTCalib.NoisyStripLastRunInDB', -999.)
    flags.addFlag('SCTCalib.NoisyStripAverageInDB', -999.)
    flags.addFlag('SCTCalib.NoisyModuleList', 5000)
    flags.addFlag('SCTCalib.NoisyModuleDiff', 0.3)
    flags.addFlag('SCTCalib.NoisyStripDiff', 0.3)
    flags.addFlag('SCTCalib.NoisyReadNumRuns', 3)           # Number of runs to calculate the recent average

    #--- Dead strips/chips
    flags.addFlag('SCTCalib.DeadStripMinStat', 5000)
    flags.addFlag('SCTCalib.DeadStripMinStatBusy', 5000)
    flags.addFlag('SCTCalib.DeadChipMinStat', 5000)
    flags.addFlag('SCTCalib.DeadStripSignificance', 6)
    flags.addFlag('SCTCalib.DeadChipSignificance', 6)
    flags.addFlag('SCTCalib.BusyThr4DeadFinding', 1.0E-4)
    flags.addFlag('SCTCalib.NoisyThr4DeadFinding', 5.0E-2)
    flags.addFlag('SCTCalib.DeadChipUploadTest', True)
    flags.addFlag('SCTCalib.DeadStripUploadTest', True)
    flags.addFlag('SCTCalib.QuietThresholdStrip', 0.5)
    flags.addFlag('SCTCalib.QuietThresholdChip', 0.5)

    #--- Efficiency
    flags.addFlag('SCTCalib.EfficiencyDoChips', False)

    #--- HIST
    flags.addFlag('SCTCalib.NoiseOccupancyTriggerAware', False)
    flags.addFlag('SCTCalib.NoiseOccupancyMinStat', 5000)
    flags.addFlag('SCTCalib.RawOccupancyMinStat', 5000)
    flags.addFlag('SCTCalib.EfficiencyMinStat', 5000)
    flags.addFlag('SCTCalib.BSErrorDBMinStat', 5000)
    flags.addFlag('SCTCalib.LorentzAngleMinStat', 5000)

    #--- Lorentz Angle Debug Mode
    flags.addFlag('SCTCalib.LorentzAngleDebugMode', True)

    #--------------------------------------------------------------
    # Tags for local DB and XML files in SCTCalib/SCTCalibWriteSvc
    #--------------------------------------------------------------
    flags.addFlag('SCTCalib.TagID4NoisyStrips', 'SctDerivedMonitoring-RUN2-UPD4-005')  # UPD4, open IOV for BLK
    flags.addFlag('SCTCalib.TagID4DeadStrips', 'SctDerivedDeadStrips-003-00')
    flags.addFlag('SCTCalib.TagID4DeadChips', 'SctDerivedDeadChips-003-00')
    flags.addFlag('SCTCalib.TagID4NoiseOccupancy', 'SctDerivedNoiseOccupancy-003-01')
    flags.addFlag('SCTCalib.TagID4RawOccupancy', 'SctDerivedRawOccupancy-003-00')
    flags.addFlag('SCTCalib.TagID4Efficiency', 'SctDerivedEfficiency-003-00')
    flags.addFlag('SCTCalib.TagID4BSErrors', 'SctDerivedBSErrorsRun2-001-00')
    flags.addFlag('SCTCalib.TagID4LorentzAngle', 'SctDerivedLorentzAngleRun2_v2-001-00')

    #--------------------------------------------------------------
    # A list of GeometryTags
    # - One tag is chosen in topOptions from magnet current info
    #--------------------------------------------------------------
    flags.addFlag('SCTCalib.GeometryTagSTF', 'ATLAS-R2-2015-03-00-00')     # Solenoid=ON,  BarrelToroid=ON
    flags.addFlag('SCTCalib.GeometryTagNTF', 'ATLAS-R2-2015-03-00-00')     # Solenoid=ON,  BarrelToroid=OFF
    flags.addFlag('SCTCalib.GeometryTagNSF', 'ATLAS-R2-2015-03-00-00')     # Solenoid=OFF,  BarrelToroid=ON
    flags.addFlag('SCTCalib.GeometryTagNF', 'ATLAS-R2-2015-03-00-00')      # Solenoid=OFF,  BarrelToroid=OFF
    flags.addFlag('SCTCalib.GeometryTagMC', 'ATLAS-R2-2015-03-00-00')      # MC

    #--------------------------------------------------------------
    # A list of ConditionsTags
    # - One tag is chosen in topOptions from magnet current info
    #--------------------------------------------------------------
    #--- MC
    flags.addFlag('SCTCalib.ConditionsTagMC', 'OFLCOND-DR-BS7T-ANom-00')   # digi+rec for MC09 - Nominal
    #--- BLK : bulk production
    flags.addFlag('SCTCalib.ConditionsTagSTF', 'CONDBR2-ES1PA-2016-03')    # Solenoid=ON,  BarrelToroid=ON
    flags.addFlag('SCTCalib.ConditionsTagNTF', 'CONDBR2-ES1PA-2016-03')    # Solenoid=ON,  BarrelToroid=OFF
    flags.addFlag('SCTCalib.ConditionsTagNSF', 'CONDBR2-ES1PA-2016-03')    # Solenoid=OFF,  BarrelToroid=ON
    flags.addFlag('SCTCalib.ConditionsTagNF', 'CONDBR2-ES1PA-2016-03')     # Solenoid=OFF,  BarrelToroid=OFF

    #--------------------------------------------------------------
    # Read /SCT/Derived/Monotoring in COOL
    flags.addFlag('SCTCalib.TagID4NoisyUploadTest', 'SctDerivedMonitoring-RUN2-UPD4-005')
    #--------------------------------------------------------------
