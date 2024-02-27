#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

import os, sys


def setFlagsFromRunArgs(flags, runArgs):
    #--- Reading RunNumber from FileName
    if hasattr( runArgs, 'RunNumber' ):
        flags.SCTCalib.RunNumber = runArgs.RunNumber

    #--- Reading EventNumber from FileName
    if hasattr( runArgs, 'EventNumber' ):
        flags.SCTCalib.EventNumber = runArgs.EventNumber

    #--- Setting MaxEvents from runArguments
    if hasattr( runArgs, 'maxEvents' ):
        flags.SCTCalib.EvtMax = runArgs.maxEvents

    #--- Setting ReadBS from runArguments
    if hasattr( runArgs, 'InputType' ) and runArgs.InputType == 'RAW':
        flags.SCTCalib.ReadBS = True
    else:
        flags.SCTCalib.ReadBS = False

    #--- Setting InputType from runArguments
    if hasattr( runArgs, 'InputType' ):
        flags.SCTCalib.InputType = runArgs.InputType

    #--- Setting which algorithms to be run
    if hasattr( runArgs, 'part' ):
        if 'doNoisyStrip' in runArgs.part:
            flags.SCTCalib.DoNoisyStrip = True
        else:
            flags.SCTCalib.DoNoisyStrip = False
        if 'doHV' in runArgs.part:
            flags.SCTCalib.DoHV = True
        else:
            flags.SCTCalib.DoHV = False
        if 'doDeadStrip' in runArgs.part:
            flags.SCTCalib.DoDeadStrip = True
        else:
            flags.SCTCalib.DoDeadStrip = False
        if 'doDeadChip' in runArgs.part:
            flags.SCTCalib.DoDeadChip = True
        else:
            flags.SCTCalib.DoDeadChip = False
        if 'doQuietStrip' in runArgs.part:
            flags.SCTCalib.DoQuietStrip = True
        else:
            flags.SCTCalib.DoQuietStrip = False
        if 'doQuietChip' in runArgs.part:
            flags.SCTCalib.DoQuietChip = True
        else:
            flags.SCTCalib.DoQuietChip = False
        if 'doNoiseOccupancy' in runArgs.part:
            flags.SCTCalib.DoNoiseOccupancy = True
        else:
            flags.SCTCalib.DoNoiseOccupancy = False
        if 'doRawOccupancy' in runArgs.part:
            flags.SCTCalib.DoRawOccupancy = True
        else:
            flags.SCTCalib.DoRawOccupancy = False
        if 'doEfficiency' in runArgs.part:
            flags.SCTCalib.DoEfficiency = True
        else:
            flags.SCTCalib.DoEfficiency = False
        if 'doBSErrorDB' in runArgs.part:
            flags.SCTCalib.DoBSErrorDB = True
        else:
            flags.SCTCalib.DoBSErrorDB = False
        if 'doLorentzAngle' in runArgs.part:
            flags.SCTCalib.DoLorentzAngle = True
        else:
            flags.SCTCalib.DoLorentzAngle = False


    if hasattr( runArgs, 'splitHitMap' ):
        if runArgs.splitHitMap == 0:
            if flags.SCTCalib.DoNoisyStrip:
                flags.SCTCalib.DoHitMapsLB = True
            else:
                flags.SCTCalib.DoHitMapsLB = False
            flags.SCTCalib.DoHitMaps   = True
            flags.SCTCalib.ReadHitMaps = False
        if runArgs.splitHitMap == 1:
            if flags.SCTCalib.DoNoisyStrip:
                flags.SCTCalib.DoHitMapsLB = True
            else:
                flags.SCTCalib.DoHitMapsLB = False
            flags.SCTCalib.DoHitMaps   = True
            flags.SCTCalib.ReadHitMaps = True
        elif runArgs.splitHitMap == 2:
            flags.SCTCalib.DoHitMapsLB = False
            flags.SCTCalib.DoHitMaps   = False
            flags.SCTCalib.ReadHitMaps = True
            if flags.SCTCalib.EvtMax != 1:
                print('WARNING! EvtMax is not 1, although HitMap analysis is run!')
    else:
        flags.SCTCalib.DoHitMapsLB = False
        flags.SCTCalib.DoHitMaps   = False
        flags.SCTCalib.ReadHitMaps = False


def readNoisyModulesAndStripsFromDB(flags):
    #--------------------------------------------------------------
    # Read /SCT/Derived/Monotoring in COOL
    # - Used in a criteria for upload of NoisyStrips
    #--------------------------------------------------------------
    from SCT_CalibAlgs.ReadCoolUPD4 import GetRunList, GetNumNoisyMods, GetNumNoisyStrips

    tag      = flags.SCTCalib.TagID4NoisyUploadTest
    numRuns  = flags.SCTCalib.NoisyReadNumRuns
    print('RUNNUMBER')
    print(flags.SCTCalib.RunNumber)
    dbstring = 'ATLAS_COOLOFL_SCT/CONDBR2'
    folder   = '/SCT/Derived/Monitoring'
    print('Getting run list from ReadCoolUPD4.py %s '% dbstring)
    RunList  = GetRunList( dbstring, folder, tag, flags.SCTCalib.RunNumber, numRuns )
    if ( len(RunList) != 0 ) :
        #--- List of data and average num of modules w/ >= 1 noisy strip
        print('---------------> Noisy strips in COOL : last ', numRuns, ' runs <---------------')
        sumNoisyModulesInDB = 0
        sumNoisyStripsInDB = 0
        for i in range( len(RunList) ) :
            numNoisyModules = GetNumNoisyMods( dbstring, folder, tag, RunList[i] )
            numNoisyStrips  = GetNumNoisyStrips( dbstring, folder, tag, RunList[i] )
            print('[ run, modules, strips ] = [', RunList[i], ',', numNoisyModules, ',', numNoisyStrips, ']')
            sumNoisyModulesInDB = sumNoisyModulesInDB + numNoisyModules
            sumNoisyStripsInDB = sumNoisyStripsInDB + numNoisyStrips
        NoisyModuleAverageInDB = float(sumNoisyModulesInDB) / float(len(RunList))
        NoisyStripAverageInDB = float(sumNoisyStripsInDB) / float(len(RunList))

        #--- Num of noisy strips in the last run
        NoisyStripLastRunInDB = GetNumNoisyStrips( dbstring, folder, tag, RunList[0] )

        print('Average num of modules w/ >= 1 noisy strip  : ',         NoisyModuleAverageInDB)
        print('Num of noisy strips in the last run', RunList[0], ' : ', NoisyStripLastRunInDB)
        print('Average num of noisy strips in the last runs  : ',       NoisyStripAverageInDB)
        print('----------------------------------------------------------------------')

        flags.SCTCalib.NoisyModuleAverageInDB = NoisyModuleAverageInDB
        flags.SCTCalib.NoisyStripLastRunInDB  = NoisyStripLastRunInDB
        flags.SCTCalib.NoisyStripAverageInDB  = NoisyStripAverageInDB


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
    commonRunArgsToFlags(runArgs, flags)

    flags.Input.Files = runArgs.inputNames

    from SCT_CalibAlgs.SCTCalibFlags import defaultSCTCalibFlags
    defaultSCTCalibFlags(flags)

    setFlagsFromRunArgs(flags, runArgs)

    from AthenaConfiguration.Enums import Format
    flags.Input.Format = Format.BS if flags.SCTCalib.ReadBS else ''

    flags.IOVDb.GlobalTag = flags.SCTCalib.ConditionsTagSTF
    flags.GeoModel.AtlasVersion = flags.SCTCalib.GeometryTagSTF

    from AthenaConfiguration.DetectorConfigFlags import disableDetectors, allDetectors
    disableDetectors(flags, allDetectors)
    flags.Detector.EnableSCT = True

    flags.fillFromArgs()

    if flags.SCTCalib.DoNoisyStrip and flags.SCTCalib.NoisyUploadTest:
        readNoisyModulesAndStripsFromDB(flags)

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    if flags.SCTCalib.ReadBS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags))

        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import SCTRawDataProviderCfg
        cfg.merge(SCTRawDataProviderCfg(flags))


    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    cfg.merge(IOVDbSvcCfg(flags))

    #--- Setting output prefix
    if hasattr( runArgs, 'prefix' ) and runArgs.prefix != '':
        prefix = runArgs.prefix+'.'
    else:
        prefix = ''

    #--------------------------------------------------------------
    # Read start/end time stamp and LB for HIST
    #--------------------------------------------------------------
    SORTime   = ''
    EORTime   = ''
    nLB       = ''

    if os.path.exists('./runInfo.txt'):
        runInfo = open( './runInfo.txt', 'r' ).read()
        RIlist  = runInfo.split( ' ' ) # RIlist[1] -> ProjTag, RIlist[5] -> DAQConfig, both not used at the moment
        SORTime   = RIlist[2]
        EORTime   = RIlist[3]
        nLB       = RIlist[4]
    else:
        SORTime   = 'cannot retrieve SORTime'
        EORTime   = 'cannot retrieve EORTime'
        nLB       = 'cannot retrieve LB'

    #--------------------------------------------------------------
    # Saving ROOT histograms
    #--------------------------------------------------------------
    from AthenaConfiguration.ComponentFactory import CompFactory
    if flags.SCTCalib.DoHitMaps:
        cfg.addService(CompFactory.THistSvc(Output = [ "HitMaps  DATAFILE='"+prefix+"SCTHitMaps.root'  OPT='RECREATE'" ]))
    if flags.SCTCalib.DoHitMapsLB:
        cfg.addService(CompFactory.THistSvc(Output = [ "LB       DATAFILE='"+prefix+"SCTLB.root'       OPT='RECREATE'" ]))
    if flags.SCTCalib.DoBSErrors and flags.SCTCalib.DoHitMaps:
        cfg.addService(CompFactory.THistSvc(Output = [ "BSErrors DATAFILE='"+prefix+"SCTBSErrors.root' OPT='RECREATE'" ]))

    #--------------------------------------------------------------
    # Configuring SCTCalibAlg
    #--------------------------------------------------------------
    from SCT_CalibAlgs.SCTCalibAlgConfig import SCTCalibAlgCfg
    cfg.merge(SCTCalibAlgCfg(flags, SORTime = SORTime, EORTime = EORTime, nLB = nLB, prefix = prefix))


    with open('SCTCalibAlgCfg.pkl', 'wb') as f:
        cfg.store(f)

    sc = cfg.run()
    sys.exit(not sc.isSuccess())
