#**************************************************************
#
# jopOptions file for TileCal commissioning analysis
#
#==============================================================

import sys
from os import system
from subprocess import check_output
from subprocess import CalledProcessError
import six

from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.AppMgr import theApp
svcMgr = theApp.serviceMgr()

from AthenaCommon.Logging import logging
log = logging.getLogger( 'jobOptions_TileCalibRec.py' )


#---  Output printout level -----------------------------------
#output threshold (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
if not 'OutputLevel' in dir():
    OutputLevel = 4
svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.MessageSvc.defaultLimit = 1000000
svcMgr.MessageSvc.Format = "% F%60W%S%7W%R%T %0W%M"
svcMgr.MessageSvc.useColors = False


if 'ReadRDO' in dir() and ReadRDO:
    doSim = True
    ReadPool = True
    ReadRch = True
if ('ReadESD' in dir() and ReadESD) or ('ReadAOD' in dir() and ReadAOD):
    doSim = True
    ReadPool = True
    ReadRch = False
    doCaloCell = False
if 'doRecoESD' in dir() and doRecoESD:
    doSim = False
    ReadESD = True
    ReadPool = True
    ReadDigits = True
    ReadRch = True
    if not 'useRODReco' in dir():
        useRODReco = True # enable DSP results in ntuple
    if not 'TileUseDCS' in dir():
        TileUseDCS = True # setup for real data
else:
    doRecoESD=False
if not 'doSim' in dir():
    doSim = False
if not 'ReadPool' in dir():
    ReadPool = doSim
if not 'ReadDigits' in dir():
    ReadDigits = (not ReadPool)
if not 'ReadRch' in dir():
    ReadRch = ReadPool

if not 'RunNumber' in dir():
    RunNumber = 0
Year = 0

if ReadPool:
    OldRun = False
    if not 'TilePhysRun' in dir():
        TilePhysRun = True
        TileRunType = 1

    if not 'FileFilter' in dir():
        if 'ReadESD' in dir() and ReadESD:
            FileFilter = "ESD"
        elif 'ReadAOD' in dir() and ReadAOD:
            FileFilter = "AOD"
        else:
            FileFilter = "pool"

    if not 'doTileMon' in dir():
        doTileMon = False
    if not 'useRODReco' in dir():
        useRODReco = False
    if not 'TileUseDCS' in dir():
        TileUseDCS = False

    if 'ReadAOD' in dir() and ReadAOD:
        if not 'doTileMuId' in dir():
            doTileMuId = False
        if not 'doTileRODMuId' in dir():
            doTileRODMuId = False
        if not 'doTileMuonFitter' in dir():
            doTileMuonFitter = False
else:
    #=============================================================
    #=== ByteStream Input
    #=============================================================
    include( "ByteStreamCnvSvc/BSEventStorageEventSelector_jobOptions.py" )
    theApp.ExtSvc += [ "ByteStreamCnvSvc" ]
    # frag to ROD mapping and additional options in external file
    svcMgr.ByteStreamCnvSvc.ROD2ROBmap = [ "-1" ]
    if 'TileMap' in dir():
        include( TileMap )

    # runs with 9 samples
    OldRun = (RunNumber > 10 and RunNumber < 40000)

    if not 'TilePhysRun' in dir():
        TilePhysRun = False

if not 'FileNameVec' in dir():
    if not 'FileName' in dir() or FileName == "":
        if not 'InputDirectory' in dir():
            InputDirectory = None
        if not 'RunStream' in dir():
            RunStream = None
        if not 'DataProject' in dir():
            DataProject = None
        if not 'DirectorySuffix' in dir():
            DirectorySuffix = None
        if not 'FileFilter' in dir():
            FileFilter = '.data'
        from TileRecEx import TileInputFiles
        FileNameVec = TileInputFiles.findFiles(RunNumber,InputDirectory,FileFilter,RunStream,DataProject,DirectorySuffix)
    else:
        FileNameVec = [ FileName ]

if 'InputDirectory' in dir():
    log.info("InputDirectory is " + str(InputDirectory))
log.info("RunNumber is " + str(RunNumber))
log.info("FullFileName is " + str(FileNameVec))

if len(FileNameVec) < 1:
    log.fatal("Input file not found")
    sys.exit(1)

if not 'Version' in dir():
    Version = "0"

if not 'OutputDirectory' in dir():
    OutputDirectory = "/tmp/Reco-" + str(RunNumber) + "-" + Version

system('mkdir -p %s' % (OutputDirectory))

if not 'EvtMin' in dir():
    EvtMin = 0
    # if (not 'doTileMon' in dir() or doTileMon):
    #    EvtMin = 1
    # else:
    #    EvtMin = 0
    EvtMinNotSet = True

if not 'EvtMax' in dir():
    EvtMax = 5000000

#-----------------------------------
# TileCal reconstruction for commissioning
#-----------------------------------

# if run type is set to non-zero value, it overrides event trig type
if not 'TileRunType' in dir():
    TileRunType = 0; # 1 - physics, 2 - laser, 4 - pedestal, 8 - CIS run

# if noise filter type is not set - disable it
if not 'TileNoiseFilter' in dir():
    TileNoiseFilter = 0; # 0 - no filter, 1 - Shimpei filter

# override run type with one of these flags
if not 'TileCisRun' in dir():
    TileCisRun = False
if not 'TileMonoRun' in dir():
    TileMonoRun = False
if not 'TilePedRun' in dir():
    TilePedRun = False
if not 'TileLasRun' in dir():
    TileLasRun = False
if not 'TilePhysRun' in dir():
    TilePhysRun = False
if not 'TileRampRun' in dir():
    TileRampRun = False
if not 'TileL1CaloRun' in dir():
    TileL1CaloRun = False

TileLasPulse = TileLasRun
TileCisPulse = (TileCisRun or TileMonoRun or TileRampRun or TileL1CaloRun)

if not (TilePhysRun or TileL1CaloRun) and 'EvtMinNotSet' in dir():
    EvtMin = 1

if not 'TilePhysTiming' in dir():
    TilePhysTiming = False

if TileCisRun:
    TileRunType = 8
    TileBiGainRun = True

    # do not convert ADC counts to pCb
    if not 'TileCalibrateEnergy' in dir():
        TileCalibrateEnergy = False

    # skip all events when just one channel is fired (4*48)
    if 'EvtMinNotSet' in dir():
        EvtMin = 192

elif TileMonoRun:
    TileRunType = 8
    TileBiGainRun = False

    # use pCb units for ntuple
    if not 'TileOfflineUnits' in dir():
        TileOfflineUnits = 1

elif TileRampRun:
    TileRunType = 8
    TileBiGainRun = False

    # use pCb units for ntuple
    if not 'TileOfflineUnits' in dir():
        TileOfflineUnits = 1

elif TileL1CaloRun:
    TileRunType = 8
    TileBiGainRun = False

    # use pCb units for ntuple
    if not 'TileOfflineUnits' in dir():
        TileOfflineUnits = 1

elif TilePedRun:
    TileRunType = 4
    TileBiGainRun = True

elif TileLasRun:
    TileRunType = 2
    TileBiGainRun = False

    # use pCb units for ntuple
    if not 'TileOfflineUnits' in dir():
        TileOfflineUnits = 1

else:
    if TilePhysRun:
        TileRunType = 1
    TileBiGainRun = False

    if not 'doLab' in dir():
        doLab = False

    if not 'doTileMuId' in dir():
        doTileMuId = doLab

    if not 'doTileRODMuId' in dir():
        doTileRODMuId = False

    if not 'doTileMuonFitter' in dir():
        doTileMuonFitter = doLab

    if not 'doCaloNtuple' in dir():
        doCaloNtuple = True

    if not 'doCaloTopoCluster' in dir():
        doCaloTopoCluster = False

# Compare mode to compare frag5 with frag0 and frag4
if not 'TileCompareMode' in dir():
    TileCompareMode = False

# emulate DSP reco offline and use those results in noise algorithm
if not 'TileEmulateDSP' in dir():
   TileEmulateDSP = False

# special options
# which algorithms to run
# and output from which algorithm to use as input for TileCellBuilder
# by default we use 3 methods - Fit, Opt2 and OptAtlas

if not 'doTileFlat' in dir():
    doTileFlat = False

if not 'doTileOpt2' in dir():
    doTileOpt2 = not TileCompareMode and ReadDigits

if not 'doTileOptATLAS' in dir():
    doTileOptATLAS = not TileCompareMode and ReadDigits

if not 'doTileMF' in dir():
    doTileMF = False

if not 'doTileOF1' in dir():
    doTileOF1 = False

if not 'doTileWiener' in dir():
    doTileWiener = False

if not 'doTileFit' in dir():
    doTileFit = not TileCompareMode and ReadDigits

if not 'doTileFitCool' in dir():
    doTileFitCool = False

if not 'TileOF1Ped' in dir():
    TileOF1Ped = -1

# for the moment enable TMDB only for data
if not 'useTMDB' in dir():
    useTMDB = not doSim

if TileBiGainRun:
    # only 7 samples are expected
    if not 'TileFrameLength' in dir():
        TileFrameLength = 7

    # do not put DSP resutls to ntuple
    if not 'useRODReco' in dir():
        useRODReco = False
else:
    # select correct number of samples
    if not 'TileFrameLength' in dir():
        if OldRun: TileFrameLength = 9
        else:      TileFrameLength = 7

    # put DSP resutls to ntuple
    if not 'useRODReco' in dir():
        useRODReco = True

if not 'useDSPUnits' in dir():
    useDSPUnits = False
elif useDSPUnits:
    TileCalibrateEnergy = False

if not 'PhaseFromCOOL' in dir():
    PhaseFromCOOL = True

if not 'OfcFromCOOL' in dir():
    if TilePhysRun or TilePedRun:
        OfcFromCOOL = True
    else:
        OfcFromCOOL = False

if useRODReco or doTileOpt2 or doTileMF or doTileOF1 or doTileOptATLAS or doTileWiener or doTileFitCool or TileCompareMode or not 'TileUseCOOL' in dir():
    TileUseCOOL = True
    TileUseCOOLOFC = not ReadPool or OfcFromCOOL

if not 'TileUseDCS' in dir():
    TileUseDCS = True

# create monitoring histograms
if not 'doTileMon' in dir():
    doTileMon = False

# calculate calibration constants and store them in ROOT file
if not 'doTileCalib' in dir():
    doTileCalib = False

# all other parameters which can be set to True or False
# from the command line

# use PMT ordering in ntuple (convert channel to PMT number)
if not 'doTileCable' in dir():
    doTileCable = False

# convert ADC counts to MeV in output ntuple by default
if not 'TileCalibrateEnergy' in dir():
    TileCalibrateEnergy = True

# create TileRec/h2000 ntuple
if not 'doTileNtuple' in dir():
    doTileNtuple=True

# store results of reconstruction in POOL file
if not 'doCreatePool' in dir():
    doCreatePool=False

# run v-atlas event display
if not 'doEventDisplay' in dir():
    doEventDisplay=False

# prepare XML files for Atlantis
if not 'doAtlantis' in dir():
    doAtlantis=False

# create CALO D3PD with cell by cell info
if not 'doCaloNtuple' in dir():
    doCaloNtuple = False

# create TileTowers
if not 'doTileTower' in dir():
    doTileTower=False

# run TileMuId
if not 'doTileMuId' in dir():
    doTileMuId = False

# run TileRODMuId
if not 'doTileRODMuId' in dir():
    doTileRODMuId = False

# run TileMuonFitter
if not 'doTileMuonFitter' in dir():
    doTileMuonFitter = False

# run TopoClustering
if not 'doCaloTopoCluster' in dir():
    doCaloTopoCluster = False

# check if we want to create D3PD
if (doCaloNtuple or doTileMuId or doTileRODMuId or doTileMuonFitter or doCaloTopoCluster):
    doD3PD = True
else:
    doD3PD = False

# check if we want to create noise monitoring plots
if not 'doTileCellNoiseMon' in dir():
    doTileCellNoiseMon = False
if not 'doTileDigiNoiseMon' in dir():
    doTileDigiNoiseMon = doTileCellNoiseMon or TilePedRun
if not 'doTileRawChannelNoiseMon' in dir():
    doTileRawChannelNoiseMon = doTileCellNoiseMon or TilePedRun

# check if we want to create TMDB monitoring plots
if not 'doTileTMDBRawChannel' in dir():
    doTileTMDBRawChannel = useTMDB

if not 'doTileTMDBDigitsMon' in dir():
    doTileTMDBDigitsMon = useTMDB

if not 'doTileTMDBRawChannelMon' in dir():
    doTileTMDBRawChannelMon = useTMDB and doTileTMDBRawChannel

if not 'doTileTMDBMon' in dir():
    doTileTMDBMon = useTMDB

# check if we need to create TileCells
if not 'doCaloCell' in dir():
    if (doD3PD or doCaloNtuple or doCreatePool or doEventDisplay or doAtlantis or (doTileMon and (TilePhysRun or doTileCellNoiseMon))):
        doCaloCell = True
    else:
        doCaloCell = False

if not 'TileD3PDSavePosition' in dir():
    TileD3PDSavePosition = True

if not 'TileFragIDsToIgnoreDMUErrors' in dir():
    # List of Tile module frag IDs for which ignore DMU errors
    if RunNumber > 370000:
        TileFragIDsToIgnoreDMUErrors = [0x10D] # Tile Demonstrator
    else:
        TileFragIDsToIgnoreDMUErrors = []

if not 'doTilePedDiffMon' in dir():
    doTilePedDiffMon = False  # Needed during maintanance campaign

#---------------
# end of options
#---------------

#=============================================================
#=== init Det Descr
#=============================================================

from AthenaCommon.GlobalFlags import globalflags
#globalflags.DetGeo.set_Value_and_Lock('commis')
globalflags.DetGeo.set_Value_and_Lock('atlas')
globalflags.Luminosity.set_Value_and_Lock('zero')
if ReadPool:
    if TileUseDCS:
        globalflags.DataSource.set_Value_and_Lock('data')
    else:
        globalflags.DataSource.set_Value_and_Lock('geant4')
    globalflags.InputFormat.set_Value_and_Lock('pool')
else:
    globalflags.DataSource.set_Value_and_Lock('data')
    globalflags.InputFormat.set_Value_and_Lock('bytestream')

from AthenaCommon.BeamFlags import jobproperties
#jobproperties.Beam.beamType.set_Value_and_Lock('cosmics')
jobproperties.Beam.beamType.set_Value_and_Lock('collisions')

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput.set_Value_and_Lock(FileNameVec)

from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOff()  #Switched off to avoid geometry
DetFlags.ID_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

DetFlags.detdescr.ID_setOff()
DetFlags.detdescr.Muon_setOff()
DetFlags.detdescr.LAr_setOn()
DetFlags.detdescr.Tile_setOn()
if TileL1CaloRun:
    DetFlags.detdescr.LVL1_setOn()
if ReadPool:
    DetFlags.readRDOPool.Tile_setOn()
    if TileL1CaloRun:
        DetFlags.readRDOPool.LVL1_setOn()
else:
    DetFlags.readRDOBS.Tile_setOn()
    if TileL1CaloRun:
        DetFlags.readRDOBS.LVL1_setOn()
DetFlags.Print()

from RecExConfig.RecFlags import rec
rec.doLArg = False

# Get project name from file name and use it in RecFlags
# in order to set up right database instance  in condb
projectName = FileNameVec[0].split('/').pop().split('.')[0]
rec.projectName = projectName
rec.RunNumber = int(RunNumber)

if globalflags.DataSource() == 'data':
    if not 'RUN3' in dir():
        RUN3 = (RunNumber >= 411938) or (RunNumber<=0) or (RunNumber==3)
    if not 'RUN2' in dir():
        RUN2 = not RUN3 and ((RunNumber > 232000) or (RunNumber==2))
    if not (RUN2 or RUN3):
        # use RUN1 DB for runs taken before Jul-2014
        if projectName.startswith("data14_"): rec.projectName = "data13_tilecomm"
        globalflags.DatabaseInstance = "COMP200"
else:
    if not ('RUN3' in dir() or 'RUN2' in dir() or 'RUN1' in dir()):
        RUN3=True
        RUN2=False
    else:
        if not 'RUN3' in dir():
            RUN3 = False
        if not 'RUN2' in dir():
            RUN2 = False


if not 'doTileRawChannelTimeMonTool' in dir():
    doTileRawChannelTimeMonTool = (TileRunType == 2) and TilePhysTiming and (RUN2 or RUN3) and doTileFit


from IOVDbSvc.CondDB import conddb
from AthenaCommon.GlobalFlags import jobproperties
if ReadPool:
    #---  Load PartProp into the Detector store ---------------
    if not hasattr(svcMgr, 'PartPropSvc'):
        from PartPropSvc.PartPropSvcConf import PartPropSvc
        svcMgr += PartPropSvc()
    #--- Pool specific --------------------------------------------
    # - General Pool converters
    include( "AthenaPoolCnvSvc/ReadAthenaPool_jobOptions.py" )
    # - Pool input
    svcMgr.EventSelector.InputCollections = FileNameVec
else:
    # - ByteStream input
    svcMgr.EventSelector.Input = FileNameVec
    # Set Global tag for IOVDbSvc
    if not 'CondDbTag' in dir():
        if RUN3:
            if 'UPD4' in dir() and UPD4: CondDbTag = 'CONDBR2-BLKPA-2023-01'
            else:                        CondDbTag = 'CONDBR2-ES1PA-2023-01'
        elif RUN2:
            if 'UPD4' in dir() and UPD4: CondDbTag = 'CONDBR2-BLKPA-2018-16'
            else:                        CondDbTag = 'CONDBR2-ES1PA-2018-05'
        else:
            if 'UPD4' in dir() and UPD4 and RunNumber > 141066: CondDbTag = 'COMCOND-BLKPA-RUN1-06'
            else:                                               CondDbTag = 'COMCOND-ES1PA-006-05'

    jobproperties.Global.ConditionsTag = CondDbTag
    conddb.setGlobalTag(CondDbTag)

# Set Geometry version
if not 'DetDescrVersion' in dir():
    if RUN3:
        DetDescrVersion = 'ATLAS-R3S-2021-03-01-00'
    elif RUN2:
        DetDescrVersion = 'ATLAS-R2-2016-01-00-01'
    else:
        DetDescrVersion = 'ATLAS-R1-2012-03-02-00'
jobproperties.Global.DetDescrVersion = DetDescrVersion
log.info( "DetDescrVersion = %s" % (jobproperties.Global.DetDescrVersion()) )

from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from GeoModelSvc.GeoModelSvcConf import GeoModelSvc
GeoModelSvc = GeoModelSvc()
GeoModelSvc.IgnoreTagDifference = True
log.info( "GeoModelSvc.AtlasVersion = %s" % (GeoModelSvc.AtlasVersion) )
#GeoModelSvc.TileVersionOverride = "TileCal-GEO-13"
#log.info( "GeoModelSvc.TileVersionOverride = %s" % (GeoModelSvc.TileVersionOverride) )


#=============================================================
#=== setup all options for optimal filter
#=============================================================

if not 'TileCorrectAmplitude' in dir():
    TileCorrectAmplitude = False;  # don't do parabolic correction in OptATLAS

if not 'TileCorrectTime' in dir():
    if TilePhysRun or TilePhysTiming:
        TileCorrectTime = True;  # APPLY time correction in physics runs
    else:
        TileCorrectTime = False;  # do not apply time correction - to be compatible with DSP reco

if not 'doTileOverflowFit' in dir():
    doTileOverflowFit = False

include( "TileRec/TileDefaults_jobOptions.py" )

from TileRecUtils.TileRecFlags import jobproperties

jobproperties.TileRecFlags.readDigits = ReadDigits
jobproperties.TileRecFlags.noiseFilter = TileNoiseFilter
jobproperties.TileRecFlags.TileRunType = TileRunType
jobproperties.TileRecFlags.calibrateEnergy = False;  # don't need pC in raw channels, keep ADC counts
if 'BunchSpacing' in dir():
    halfBS = BunchSpacing/2.
    if halfBS > 25.1 or halfBS < 0.1:
        log.info("Bad bunch spacing %s, keeping default limits for parabolic correction" % BunchSpacing)
    else:
        jobproperties.TileRecFlags.TimeMinForAmpCorrection = -halfBS
        jobproperties.TileRecFlags.TimeMaxForAmpCorrection = halfBS

jobproperties.TileRecFlags.doTileOverflowFit = doTileOverflowFit

# if those two parameters are set to true, everything is loaded correctly,
# but only for physics runs, i.e. it will be wrong for cis or laser runs.
# so, we set them to false here, but then load proper OFC and phases
# for every type of run
jobproperties.TileRecFlags.OfcFromCOOL = OfcFromCOOL
jobproperties.TileRecFlags.BestPhaseFromCOOL = PhaseFromCOOL

jobproperties.print_JobProperties('tree&value')

#=============================================================
#=== setup TileConditions
#=============================================================
include( "TileConditions/TileConditions_jobOptions.py" )
tileInfoConfigurator.OutputLevel = OutputLevel

if RUN3 and 'SpecialDemoShape' in dir():
    if TileRunType == 2:
        # disable special treatment for demo in laser runs for the moment
        SpecialDemoShape = -1
    elif TileRunType == 8:
        # put CIS pulse shape for Demo in laser and physics structures
        SpecialDemoShape = 3
        tileInfoConfigurator.filename_lo_las = "pulselo_cis_demo_100.dat"
        tileInfoConfigurator.filename_hi_las = "pulsehi_cis_demo_100.dat"
        tileInfoConfigurator.filename_lo_las_der = "dpulselo_cis_demo_100.dat"
        tileInfoConfigurator.filename_hi_las_der = "dpulsehi_cis_demo_100.dat"
        tileInfoConfigurator.filename_lo_phys = "pulselo_cis_demo_5p2.dat"
        tileInfoConfigurator.filename_hi_phys = "pulsehi_cis_demo_5p2.dat"
        tileInfoConfigurator.filename_lo_phys_der = "dpulselo_cis_demo_5p2.dat"
        tileInfoConfigurator.filename_hi_phys_der = "dpulsehi_cis_demo_5p2.dat"
    else:
        # put physics pulse shape for Demo in laser structures
        SpecialDemoShape = 2
        tileInfoConfigurator.filename_lo_las = "pulselo_phys_demo.dat"
        tileInfoConfigurator.filename_hi_las = "pulsehi_phys_demo.dat"
        tileInfoConfigurator.filename_lo_las_der = "dpulselo_phys_demo.dat"
        tileInfoConfigurator.filename_hi_las_der = "dpulsehi_phys_demo.dat"

printfunc (tileInfoConfigurator)

#============================================================
#=== configure TileCondToolOfcCool
#============================================================
OfcFromCoolOF1 = doTileOF1 and OfcFromCOOL and (conddb.GetInstance() == 'CONDBR2') # there are OFCs for OF1 only in CONDBR2

#============================================================
#=== configure TileCondToolOfc
#============================================================
tileCondToolOfc = None
if not OfcFromCOOL and (doTileOpt2 or doTileOptATLAS or doTileOF1):
    from TileConditions.TileConditionsConf import TileCondToolOfc
    tileCondToolOfc = TileCondToolOfc()
    tileCondToolOfc.nSamples = TileFrameLength # default = 7
    tileCondToolOfc.OptFilterDeltaCorrelation = False # False - use matrix from DB
    tileCondToolOfc.OutputLevel = OutputLevel

    printfunc (tileCondToolOfc)

#============================================================
#=== adding Event Info
#============================================================
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

if not 'newRDO' in dir() or newRDO is None:
    if 'ReadRDO' in dir() and ReadRDO:
        from PyUtils.MetaReaderPeeker import convert_itemList
        from RecExConfig.ObjKeyStore import objKeyStore
        objKeyStore.addManyTypesInputFile(convert_itemList(layout = '#join'))

        newRDO = objKeyStore.isInInput( "xAOD::EventInfo" )
    else:
        newRDO = True

#=============================================================
#=== read ByteStream and reconstruct data
#=============================================================
tileRawChannelBuilderFitFilter = None
tileRawChannelBuilderFitFilterCool = None
tileRawChannelBuilderMF = None
tileRawChannelBuilderOF1 = None
tileRawChannelBuilderOpt2Filter = None
tileRawChannelBuilderOptATLAS = None
tileRawChannelBuilderWienerFilter = None
tileDigitsContainer = ''
if not ReadPool:
    include( "ByteStreamCnvSvcBase/BSAddProvSvc_RDO_jobOptions.py" )
    include( "TileRec/TileRec_jobOptions.py" )

    tileRawChannelBuilderFitFilter = theTileRawChannelGetter.TileRawChannelBuilderFitFilter()
    tileRawChannelBuilderFitFilterCool = theTileRawChannelGetter.TileRawChannelBuilderFitFilterCool()
    tileRawChannelBuilderMF = theTileRawChannelGetter.TileRawChannelBuilderMF()
    tileRawChannelBuilderOF1 = theTileRawChannelGetter.TileRawChannelBuilderOF1()
    tileRawChannelBuilderOpt2Filter = theTileRawChannelGetter.TileRawChannelBuilderOpt2Filter()
    tileRawChannelBuilderOptATLAS = theTileRawChannelGetter.TileRawChannelBuilderOptATLAS()
    tileRawChannelBuilderWienerFilter = theTileRawChannelGetter.TileRawChannelBuilderWienerFilter()

    from TileByteStream.TileByteStreamConf import TileROD_Decoder
    ToolSvc += TileROD_Decoder()
    if TileCompareMode:
        ToolSvc.TileROD_Decoder.useFrag5Raw  = True
        ToolSvc.TileROD_Decoder.useFrag5Reco = True

        TilePulseTypes = {0 : 'PHY', 1 : 'PHY', 2 : 'LAS', 4 : 'PHY', 8 : 'CIS'}
        TilePulse = TilePulseTypes[jobproperties.TileRecFlags.TileRunType()]

        tileInfoConfigurator.setupCOOLOFC(type = TilePulse)
        ToolSvc.TileROD_Decoder.TileCondToolOfcCool = ToolSvc.TileCondToolOfcCool
else:
    if ReadDigits:
        from TileRecUtils.TileRawChannelGetter import *
        theTileRawChannelGetter=TileRawChannelGetter()
        tileRawChannelBuilderFitFilter = theTileRawChannelGetter.TileRawChannelBuilderFitFilter()
        tileRawChannelBuilderFitFilterCool = theTileRawChannelGetter.TileRawChannelBuilderFitFilterCool()
        tileRawChannelBuilderMF = theTileRawChannelGetter.TileRawChannelBuilderMF()
        tileRawChannelBuilderOF1 = theTileRawChannelGetter.TileRawChannelBuilderOF1()
        tileRawChannelBuilderOpt2Filter = theTileRawChannelGetter.TileRawChannelBuilderOpt2Filter()
        tileRawChannelBuilderOptATLAS = theTileRawChannelGetter.TileRawChannelBuilderOptATLAS()
        tileRawChannelBuilderWienerFilter = theTileRawChannelGetter.TileRawChannelBuilderWienerFilter()
        if doRecoESD:
            topSequence.TileRChMaker.TileDigitsContainer="TileDigitsFlt"
            tileDigitsContainer = 'TileDigitsFlt'

from TileRecUtils.TileDQstatusAlgDefault import TileDQstatusAlgDefault
dqStatus = TileDQstatusAlgDefault (TileDigitsContainer = tileDigitsContainer,
                                   TileRawChannelContainer = '')

if doTileFit and tileRawChannelBuilderFitFilter:
    tileRawChannelBuilderFitFilter.MaxTimeFromPeak = 250.0; # recover behaviour of rel 13.0.30
    tileRawChannelBuilderFitFilter.RMSChannelNoise = 3;
    tileRawChannelBuilderFitFilter.UseDSPCorrection = not TileBiGainRun
    if 'SpecialDemoShape' in dir() and SpecialDemoShape is not None:
        tileRawChannelBuilderFitFilter.SpecialDemoShape = SpecialDemoShape

    printfunc (tileRawChannelBuilderFitFilter)

if doTileFitCool and tileRawChannelBuilderFitFilterCool:
    tileRawChannelBuilderFitFilterCool.MaxTimeFromPeak = 250.0; # recover behaviour of rel 13.0.30
    tileRawChannelBuilderFitFilterCool.UseDSPCorrection = not TileBiGainRun

    printfunc (tileRawChannelBuilderFitFilterCool)

if doTileOpt2:

    if tileRawChannelBuilderOpt2Filter:
        if TileMonoRun or TileRampRun:
            tileRawChannelBuilderOpt2Filter.MaxIterations = 3 # 3 iterations to match DSP reco
            if TileCompareMode or TileEmulateDSP:
                tileRawChannelBuilderOpt2Filter.EmulateDSP = True # use dsp emulation
        tileRawChannelBuilderOpt2Filter.UseDSPCorrection = not TileBiGainRun
        if tileCondToolOfc:
            tileRawChannelBuilderOpt2Filter.TileCondToolOfc = tileCondToolOfc

        printfunc (tileRawChannelBuilderOpt2Filter)

if doTileOptATLAS and tileRawChannelBuilderOptATLAS:
    if ReadPool:
        tileRawChannelBuilderOptATLAS.TileRawChannelContainer = "TileRawChannelFixed"

    if PhaseFromCOOL:
        tileRawChannelBuilderOptATLAS.correctTime = False; # do not need to correct time with best phase

    tileRawChannelBuilderOptATLAS.BestPhase   = PhaseFromCOOL; # Phase from COOL or assume phase=0
    if TileCompareMode or TileEmulateDSP:
        tileRawChannelBuilderOptATLAS.EmulateDSP = True # use dsp emulation
    tileRawChannelBuilderOptATLAS.UseDSPCorrection = not TileBiGainRun
    if tileCondToolOfc:
        tileRawChannelBuilderOptATLAS.TileCondToolOfc = tileCondToolOfc

    printfunc (tileRawChannelBuilderOptATLAS)

if doTileMF and tileRawChannelBuilderMF:

    if PhaseFromCOOL:
        tileRawChannelBuilderMF.correctTime = False; # do not need to correct time with best phase

    tileRawChannelBuilderMF.BestPhase   = PhaseFromCOOL; # Phase from COOL or assume phase=0
    tileRawChannelBuilderMF.UseDSPCorrection = not TileBiGainRun
    if tileCondToolOfc:
        tileRawChannelBuilderMF.TileCondToolOfc = tileCondToolOfc

    printfunc (tileRawChannelBuilderMF )

if doTileOF1 and tileRawChannelBuilderOF1:
    tileRawChannelBuilderOF1.PedestalMode = TileOF1Ped

    if PhaseFromCOOL:
        tileRawChannelBuilderOF1.correctTime = False # do not need to correct time with best phase

    tileRawChannelBuilderOF1.BestPhase   = PhaseFromCOOL # Phase from COOL or assume phase=0
    if TileCompareMode or TileEmulateDSP:
        tileRawChannelBuilderOF1.EmulateDSP = True # use dsp emulation
    tileRawChannelBuilderOF1.UseDSPCorrection = not TileBiGainRun
    if tileCondToolOfc:
        tileRawChannelBuilderOF1.TileCondToolOfc = tileCondToolOfc

    printfunc (tileRawChannelBuilderOF1)

if doTileWiener and tileRawChannelBuilderWienerFilter:
    if PhaseFromCOOL:
        tileRawChannelBuilderWienerFilter.correctTime = False # do not need to correct time with best phase

    tileRawChannelBuilderWienerFilter.BestPhase = PhaseFromCOOL # Phase from COOL or assume phase=0

    if TileMonoRun or TileRampRun:
        if TileCompareMode or TileEmulateDSP:
            tileRawChannelBuilderWienerFilter.EmulateDSP = True # use dsp emulation
    tileRawChannelBuilderWienerFilter.UseDSPCorrection = not TileBiGainRun

    printfunc (tileRawChannelBuilderWienerFilter)

if (doEventDisplay or doCreatePool):
    # create TileHit from TileRawChannel and store it in TileHitVec
    from TileRecAlgs.TileHitFromRawChGetter import *
    theTileHitFromRawChGetter = TileHitFromRawChGetter()
    theTileHitFromRawChGetter.TileRawChannelToHit().TileRawChannelContainer = "TileRawChannelOpt2"
    theTileHitFromRawChGetter.TileRawChannelToHit().UseSamplFract = False

    printfunc (theTileHitFromRawChGetter.TileRawChannelToHit())

if doCaloCell:
   # create TileCell from TileRawChannel and store it in CaloCellContainer
   if TileBiGainRun:
       include( "TileRec/TileCellMaker_jobOptions_doublegain.py" )
       if OldRun: # disable masking on the fly
           topSequence.CaloCellMakerLG.CaloCellMakerToolNames["TileCellBuilderLG"].TileDSPRawChannelContainer=""
           topSequence.CaloCellMakerHG.CaloCellMakerToolNames["TileCellBuilderHG"].TileDSPRawChannelContainer=""
   else:
       include( "TileRec/TileCellMaker_jobOptions.py" )
       if OldRun: # disable masking on the fly
           topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].TileDSPRawChannelContainer=""
       if doRecoESD:
           topSequence.CaloCellMaker.CaloCellsOutputName = "AllCaloNewReco"
           topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].MBTSContainer = "MBTSContainerNewReco"
           topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].E4prContainer = "E4prContainerNewReco"
           topSequence.CaloCellMaker.CaloCellMakerToolNames["TileCellBuilder"].TileDSPRawChannelContainer=""

if doTileTower:
    include( "CaloRec/CaloCombinedTower_jobOptions.py" )
    #NB: ONLY Tile Towers
    CmbTowerBldr.TowerBuilderTools=[ "TileTowerBuilderTool/TileCmbTwrBldr" ]
    if doRecoESD:
        TileCmbTwrBldr.CellContainerName = "AllCaloNewReco"
        CmbTowerBldr.TowerContainerName="CombinedTowerNewReco"

if doCaloTopoCluster :
    if not 'includeLAr' in dir():
        includeLAr=False
    include ("TileMonitoring/TileMonTopoCluster_jobOptions.py")
    if doRecoESD:
        TileTopoMaker.CellsName = "AllCaloNewReco"

if doTileMuId:
    include ("TileMuId/TileMuId_cosmics_jobOptions.py")
    if doRecoESD:
        theTileLookForMuAlg.CellsNames = "AllCaloNewReco"
        theTileLookForMuAlg.TileMuTagsOutputName = "MuObjNewReco"

if doTileMuonFitter:
    include( "TileCosmicAlgs/TileMuonFitter_jobOptions.py")
    if doLab:
        theTileCosmicMuonGetter.TileMuonFitter().BeamType   = 'collisions'
        doTileMon=True
    if doRecoESD:
        theTileCosmicMuonGetter.TileMuonFitter().CaloCellContainer = "AllCaloNewReco"


if doTileTMDBRawChannel:
    # Set up TileCondToolPulseShape to be used in
    # TileCondToolOfc
    from TileConditions.TileCondToolConf import getTileCondToolMuRcvPulseShape
    muRcvPulseShape = getTileCondToolMuRcvPulseShape('FILE', 'TileCondToolMuRcvPulseShape')

    # Set up TileCondToolOfc to be used in TileRawChannelBuilderMF
    muRcvOfc = CfgMgr.TileCondToolOfc(name = 'TileCondToolMuRcvOfc'
                                      , OptFilterDeltaCorrelation = True
                                      , TileCondToolPulseShape = muRcvPulseShape)

    # Set up TileRawChannelBuilderOpt2 to be used
    muRcvRawChannelBuilder = CfgMgr.TileRawChannelBuilderOpt2Filter(name = 'TileMuRcvRawChannelBuilderOpt2'
                                                                    , TileRawChannelContainer = 'TileMuRcvRawChannelOpt2'
                                                                    , PedestalMode = 1
                                                                    , Minus1Iteration = TRUE
                                                                    , calibrateEnergy = False
                                                                    , correctTime = False
                                                                    , TileCondToolOfc = muRcvOfc)

    topSequence += CfgMgr.TileRawChannelMaker(name = 'TileMuRcvRChMaker'
                                              , TileDigitsContainer = 'MuRcvDigitsCnt'
                                              , TileRawChannelBuilder = [ muRcvRawChannelBuilder ])

if (doTileNtuple or doD3PD):

    theApp.HistogramPersistency = "ROOT"

    if not hasattr(svcMgr,"THistSvc"):
        from GaudiSvc.GaudiSvcConf import THistSvc
        svcMgr+=THistSvc()
    datafile = '%(dir)s/tile_%(RunNum).f_%(Version)s.aan.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    svcMgr.THistSvc.Output += [ "AANT DATAFILE='" + datafile + "' OPT='RECREATE' " ]
    svcMgr.THistSvc.MaxFileSize = 32768

    from AnalysisTools.AthAnalysisToolsConf import AANTupleStream
    topSequence += AANTupleStream( "AANTupleStream1" )
    AANTupleStream1 = topSequence.AANTupleStream1
    AANTupleStream1.ExtraRefNames = [ "StreamESD","StreamRDO" ]
    datafile = '%(dir)s/tile_%(RunNum).f_%(Version)s.aan.root' % {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    AANTupleStream1.OutputName = datafile
    AANTupleStream1.ExistDataHeader = False


if doD3PD:
    try:
        import CaloSysD3PDMaker
    except ImportError:
        doD3PD = False
        log.warning ('CaloSysD3PDMaker not available; not making D3PD.')

if doD3PD:

    def _args (level, name, kwin, **kw):
        kw = kw.copy()
        kw['level'] = level
        for (k, v) in kwin.items():
            if k.startswith (name + '_'):
                kw[k[len(name)+1:]] = v
        return kw

    def TileD3PD (file,
                 level = 4,
                 tuplename = 'caloD3PD',
                 allCells = True,
                 seq = topSequence,
                 D3PDSvc = 'D3PD::RootD3PDSvc',
                 streamNameRoot = None,
                 **kw):
        from D3PDMakerCoreComps.MakerAlg import MakerAlg

        alg = MakerAlg(tuplename, seq, file = file ,D3PDSvc =D3PDSvc,streamNameRoot =streamNameRoot)

        if doCaloNtuple:
            from CaloSysD3PDMaker.TileDetailsD3PDObject import TileDetailsD3PDObject
            from CaloSysD3PDMaker.CaloInfoD3PDObject import CaloInfoD3PDObject
            from CaloD3PDMaker.MBTSD3PDObject import MBTSD3PDObject

            if doRecoESD and doCaloCell:
                alg += TileDetailsD3PDObject (**_args(1, 'TileDetails',kw, sgkey='AllCaloNewReco', prefix='tile_', \
                                                              Kinematics_WriteEtaPhi = True, TileDetails_SavePositionInfo = TileD3PDSavePosition))

                alg += CaloInfoD3PDObject (**_args(0, 'CaloInfo',kw, sgkey='AllCaloNewReco', prefix='calo_'))
                alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix='mbts_', sgkey='MBTSContainerNewReco'))

#                if RUN2:
#                    alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'e4pr_', sgkey = 'E4prContainerNewReco'))

            else:
                if TileBiGainRun:
                    if allCells:
                        alg += TileDetailsD3PDObject (**_args(1, 'TileDetails', kw, sgkey = 'AllCaloHG', prefix = 'tile_', \
                                                                  Kinematics_WriteEtaPhi = True, TileDetails_SavePositionInfo = TileD3PDSavePosition))
                        alg += CaloInfoD3PDObject (**_args(0, 'CaloInfo', kw, sgkey = 'AllCaloHG', prefix = 'calo_'))

                    alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'mbts_', sgkey = 'MBTSContainerHG'))
                    alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'mbtsLG_', sgkey = 'MBTSContainerLG'))

                    if RUN2:
                        alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'e4pr_', sgkey = 'E4prContainerHG', MBTS_SaveEtaPhiInfo = False))
                        alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'e4prLG_', sgkey = 'E4prContainerLG', MBTS_SaveEtaPhiInfo = False))
                else:
                    if allCells:
                        alg += TileDetailsD3PDObject (**_args(1, 'TileDetails', kw, sgkey = 'AllCalo', prefix = 'tile_', \
                                                              Kinematics_WriteEtaPhi = True, TileDetails_SavePositionInfo = TileD3PDSavePosition))
                        alg += CaloInfoD3PDObject (**_args(0, 'CaloInfo', kw, sgkey = 'AllCalo', prefix = 'calo_'))

                    alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'mbts_', sgkey = 'MBTSContainer'))

                    if RUN2:
                        alg += MBTSD3PDObject (**_args(1, 'MBTS', kw, prefix = 'e4pr_', sgkey = 'E4prContainer', MBTS_SaveEtaPhiInfo = False))

        if doCaloTopoCluster:
            from CaloD3PDMaker.xAODClusterD3PDObject import xAODClusterD3PDObject

            if not includeLAr:
                from CaloD3PDMaker import ClusterSamplingFillerTool
                ClusterSamplingFillerTool.Samplings=[12,13,14,15,16,17,18,19,20]

            alg+= xAODClusterD3PDObject(**_args(3, 'topo_cl',kw, sgkey='TileTopoCluster', prefix='topo_'))

        if doTileMuId:
            from CaloSysD3PDMaker.TileMuD3PDObject import TileMuD3PDObject
            alg += TileMuD3PDObject(**_args(0,'TileMus',kw,sgkey='TileMuObj',prefix='tilemuid_'))

        if doTileRODMuId:
            from CaloSysD3PDMaker.TileL2D3PDObject import TileL2D3PDObject
            alg += TileL2D3PDObject(**_args(2,'TileL2s',kw,exclude=['TileL2'],sgkey='TileL2Cnt',prefix='tilemurod_'))

        if doTileMuonFitter:
            from CaloSysD3PDMaker.TileCosmicMuonD3PDObject import TileCosmicMuonD3PDObject
            if not 'doTMFMethod' in dir():
                doTMFMethod = 'Hough'

            if(doTMFMethod == 'Hough' or doTMFMethod == 'Both'):
                alg += TileCosmicMuonD3PDObject(**_args(2,'TileCosMusHT',kw,sgkey='TileCosmicMuonHT',prefix='TileCosmicsHT_'))

            if(doTMFMethod == 'Minuit' or doTMFMethod == 'Both'):
                alg += TileCosmicMuonD3PDObject(**_args(2,'TileCosMusMF',kw,sgkey='TileCosmicMuonMF',prefix='TileCosmicsMF_'))

        return alg

    tupleName= '%(dir)s/tile_%(RunNum).f_%(Version)s.aan.root' %{'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    allC = not ('MBTSonly' in dir() and MBTSonly)
    TheAlg = TileD3PD(file = tupleName , seq = topSequence, allCells = allC)

if doTileNtuple:
    # create TileCal standalone ntuple
    include( "TileRec/TileNtuple_jobOptions.py" )

    # CompressionSettings: algorithm * 100 + level
    TileNtuple.CompressionSettings = 204

    if 'ReadAOD' in dir() and ReadAOD:
        TileNtuple.TileDigitsContainerFlt = ""

    if TileCompareMode:
        useRODReco = True
        useDSPUnits = True
        TileNtuple.CalibMode = True
        TileNtuple.CompareMode = True

    if useDSPUnits:
        TileNtuple.UseDspUnits = True
        TileNtuple.CalibrateEnergy = True
        TileNtuple.OfflineUnits = -1
    elif 'TileOfflineUnits' in dir():
        TileNtuple.UseDspUnits = False
        TileNtuple.CalibrateEnergy = True
        TileNtuple.OfflineUnits = TileOfflineUnits
    else:
        TileNtuple.CalibrateEnergy = TileCalibrateEnergy

    if not ReadRch and ReadPool and not doRecoESD:
        TileNtuple.TileRawChannelContainer = ""

    if useRODReco:
        if 'ReadAOD' in dir() and ReadAOD:
            TileNtuple.TileRawChannelContainerDsp = ""
            TileNtuple.TileDigitsContainer = ""
            TileNtuple.TileDigitsContainerFlt = ""
        elif ('ReadESD' in dir() and ReadESD):
            TileNtuple.TileRawChannelContainerDsp = "TileRawChannelFlt"
            TileNtuple.TileDigitsContainer = ""
            TileNtuple.TileDigitsContainerFlt = "TileDigitsFlt"
        else:
            TileNtuple.TileRawChannelContainerDsp = "TileRawChannelCnt"
    elif TilePhysTiming:
        TileNtuple.TileDigitsContainerFlt = "TileDigitsCnt"
        TileNtuple.TileDigitsContainer = "" # do not save various error bits

    if 'doTileNtupleReduced' in dir() and doTileNtupleReduced:
        if 'ReadAOD' in dir() and ReadAOD:
            TileNtuple.TileRawChannelContainerDsp = ""
        elif ('ReadESD' in dir() and ReadESD):
            TileNtuple.TileRawChannelContainerDsp = "TileRawChannelFlt"
        else:
            TileNtuple.TileRawChannelContainerDsp = "TileRawChannelCnt"
        TileNtuple.Reduced = True
        TileNtuple.TileRawChannelContainer = ""

    if ReadPool:
        TileNtuple.TileBeamElemContainer = ""
        TileNtuple.BSInput = False

    if TileLasRun:
        TileNtuple.SkipEvents = 4

    TileNtuple.PMTOrder = doTileCable

    TileNtuple.CheckDCS = TileUseDCS

    if TilePhysTiming or TilePhysRun:
        dqStatus.TileBeamElemContainer = ""
    else:
        beamElemContainer   = getattr (TileNtuple,
                                       'TileBeamElemContainer',
                                       TileNtuple.getDefaultProperty('TileBeamElemContainer'))
        if str(beamElemContainer):
            dqStatus.TileBeamElemContainer = beamElemContainer

    digitsContainer     = getattr (TileNtuple,
                                   'TileDigitsContainer',
                                    TileNtuple.getDefaultProperty('TileDigitsContainer'))
    if str(digitsContainer):
        dqStatus.TileDigitsContainer = digitsContainer

    rawChannelContainer = getattr (TileNtuple,
                                   'TileRawChannelContainerDsp',
                                   TileNtuple.getDefaultProperty('TileRawChannelContainerDsp'))
    if str(rawChannelContainer):
        dqStatus.TileRawChannelContainer = rawChannelContainer

if doTileMon:

    # Monitoring historgrams
    if not hasattr(svcMgr,"THistSvc"):
        from GaudiSvc.GaudiSvcConf import THistSvc
        svcMgr+=THistSvc()
    datafile = '%(dir)s/tilemon_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    svcMgr.THistSvc.Output += [ "Tile DATAFILE='" + datafile + "' OPT=\'RECREATE\' " ]

    if doTileTMDBMon:
        from AthenaCommon.Resilience import treatException
        try:
            from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
            from AthenaConfiguration.AllConfigFlags import ConfigFlags
            from TileConfiguration.TileConfigFlags import TileRunType as TileType

            runTypes = {0 : TileType.PHY, 1 : TileType.PHY, 2 : TileType.LAS, 4 : TileType.PED, 8 : TileType.CIS}
            runTypeName = runTypes[jobproperties.TileRecFlags.TileRunType()]

            ConfigFlags.Input.Files = FileNameVec
            ConfigFlags.GeoModel.AtlasVersion = DetDescrVersion
            ConfigFlags.IOVDb.GlobalTag = CondDbTag
            ConfigFlags.Output.HISTFileName = datafile
            ConfigFlags.DQ.useTrigger = False
            ConfigFlags.DQ.enableLumiAccess = False
            ConfigFlags.Tile.RunType = runTypeName
            ConfigFlags.lock()

            from TileMonitoring.TileTMDBMonitorAlgorithm import TileTMDBMonitoringConfig
            ca = CAtoGlobalWrapper(TileTMDBMonitoringConfig, ConfigFlags)

            alg = ca.getEventAlgo('TileTMDBMonAlg')
            for tool in alg.GMTools:
                tool.Histograms = [h.replace('OFFLINE','ONLINE') for h in tool.Histograms]

        except Exception:
            treatException("Could not translate TileTMDBMonitoringConfig to old cfg")

    if (TileMonoRun):
        runType = 9
    elif (TileL1CaloRun):
        runType = 9
    elif (TileRampRun):
        runType = 10
    else:
        runType = TileRunType

    from AthenaMonitoring.AthenaMonitoringConf import *
    TileMon = AthenaMonManager( "TileMon" )

    TileMon.ManualRunLBSetup    = True
    TileMon.ManualDataTypeSetup = True
    TileMon.Environment         = "user"
    TileMon.FileKey             = "Tile"
    TileMon.Run                 = RunNumber
    TileMon.LumiBlock           = 1

    #from AthenaCommon.AppMgr import ToolSvc
    from TileMonitoring.TileMonitoringConf import *

    doTileMonDigi = ReadDigits
    doTileMonRch = (ReadDigits or ReadRch)
    doTileMonDQ = (TilePhysRun and ReadDigits)

    if not 'doTileMonCell' in dir():
        doTileMonCell = TilePhysRun

    doTileDigiNoiseMon = doTileDigiNoiseMon and ReadDigits
    doTileCellNoiseMon = doTileCellNoiseMon and doCaloCell

    if doTileMonDigi:
        b2d = TilePedRun
        theTileDigitsMon = TileDigitsMonTool ( name            ="TileDigitsMon",
                                               histoPathBase   = "/Tile/Digits",
                                               bookAllDrawers  = True,
                                               book2D          = b2d,
                                               runType         = runType,
                                               FragIDsToIgnoreDMUErrors = TileFragIDsToIgnoreDMUErrors,
                                               FillPedestalDifference = doTilePedDiffMon)

        if not TileBiGainRun:
            theTileDigitsMon.ZeroLimitHG = 2
            theTileDigitsMon.SaturationLimitHG = 1022

        TileMon.AthenaMonTools += [ theTileDigitsMon ]
        printfunc (theTileDigitsMon)

    if doTileMonRch:
        b2d = TileCisRun or TileRampRun
        theTileRawChannelMon = TileRawChannelMonTool ( name            ="TileRawChannelMon",
                                                       histoPathBase   = "/Tile/RawChannel",
                                                       bookAllDrawers  = True,
                                                       book2D          = b2d,
                                                       PlotDSP         = useRODReco,
                                                       FragIDsToIgnoreDMUErrors = TileFragIDsToIgnoreDMUErrors,
                                                       runType         = runType )

        theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelCnt"; # default for simulation

        if doTileOF1:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelOF1"

        if doTileOptATLAS:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelFixed"

        if doTileFit:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelFit"

        if doTileFitCool:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelFitCool"

        if doTileOpt2:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelOpt2"

        if doTileMF:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelMF"

        if doTileWiener:
            theTileRawChannelMon.TileRawChannelContainer = "TileRawChannelWiener"

        if useRODReco:
            theTileRawChannelMon.TileRawChannelContainerDSP = "TileRawChannelCnt"

        if TileLasRun:
            theTileRawChannelMon.overlaphists = True

        theTileRawChannelMon.MinAmpForCorrectedTime = 0.1
        theTileRawChannelMon.CalibUnit = 1

        TileMon.AthenaMonTools += [ theTileRawChannelMon ]
        printfunc (theTileRawChannelMon)

    if doTileMonDQ:
        theTileDQFragMon = TileDQFragMonTool(name               = 'TileDQFragMon',
                                             OutputLevel        = 3,
                                             TileRawChannelContainerDSP    = "TileRawChannelCnt",
                                             TileRawChannelContainerOffl   = jobproperties.TileRecFlags.TileRawChannelContainer(),
                                             TileDigitsContainer           = "TileDigitsCnt",
                                             NegAmpHG           = -200.,
                                             NegAmpLG           = -15.,
                                             SkipMasked         = True,
                                             SkipGapCells       = True,
                                             doOnline           = False,
                                             doPlots            = False,
                                             CheckDCS           = TileUseDCS,
                                             histoPathBase      = "/Tile/DMUErrors");

        theTileDQFragMon.TileRawChannelContainerOffl = "TileRawChannelCnt"; # default for simulation

        if doTileOptATLAS:
            theTileDQFragMon.TileRawChannelContainerOffl = "TileRawChannelFixed"

        if doTileFit:
            theTileDQFragMon.TileRawChannelContainerOffl = "TileRawChannelFit"

        if doTileFitCool:
            theTileDQFragMon.TileRawChannelContainerOffl = "TileRawChannelFitCool"

        if doTileOpt2:
            theTileDQFragMon.TileRawChannelContainerOffl = "TileRawChannelOpt2"

        if useRODReco:
            theTileDQFragMon.TileRawChannelContainerDSP = "TileRawChannelCnt"

        TileMon.AthenaMonTools += [ theTileDQFragMon ];
        printfunc (theTileDQFragMon)

    if doTileMonCell:
        if TileBiGainRun:
            theTileCellMonHG = TileCellMonTool(name               = 'TileCellMonHG',
                                             OutputLevel        = 3,
                                             doOnline           = False,
                                             cellsContainerName = "AllCaloHG",
                                             negEnergyThreshold = -2000,
                                             energyThreshold    = 300,
                                             histoPathBase      = "/Tile/Cell");

            TileMon.AthenaMonTools += [ theTileCellMonHG ];
            printfunc (theTileCellMonHG)

        else:
            theTileCellMon = TileCellMonTool(name               = 'TileCellMon',
                                             OutputLevel        = 3,
                                             doOnline           = False,
                                             negEnergyThreshold = -2000,
                                             energyThreshold    = 300,
                                             cellsContainerName = "AllCalo",
                                             histoPathBase      = "/Tile/Cell");

            theTileCellMon.energyThreshold = 300.
            theTileCellMon.energyThresholdForTime = 150.
            theTileCellMon.FillTimeHistograms = True
            TileMon.AthenaMonTools += [ theTileCellMon ];
            printfunc (theTileCellMon)

    if doTileDigiNoiseMon:
        TileDigiNoiseMon = TileDigiNoiseMonTool(name               = 'TileDigiNoiseMon',
                                                OutputLevel        = OutputLevel,
                                                TileDigitsContainer = "TileDigitsCnt",
                                                CheckDCS           = TileUseDCS,
                                                FragIDsToIgnoreDMUErrors = TileFragIDsToIgnoreDMUErrors,
                                                histoPathBase = "/Tile/DigiNoise" );

        if not TileBiGainRun: TileDigiNoiseMon.TriggerTypes = [ 0x82 ]

        TileMon.AthenaMonTools += [ TileDigiNoiseMon ];
        printfunc (TileDigiNoiseMon)

    if TileBiGainRun and doTileCellNoiseMon:
        TileCellNoiseMonLG = TileCellNoiseMonTool(name               = 'TileCellNoiseMonLG',
                                                  OutputLevel        = OutputLevel,
                                                  doOnline           = athenaCommonFlags.isOnline(),
                                                  cellsContainerName = "AllCaloLG",
                                                  histoPathBase = "/Tile/CellNoise/LG");
        TileCellNoiseMonLG.Xmin          = -2000.;
        TileCellNoiseMonLG.Xmax          =  2000.;
        TileMon.AthenaMonTools += [ TileCellNoiseMonLG ];
        printfunc (TileCellNoiseMonLG)


        TileCellNoiseMonHG = TileCellNoiseMonTool(name               = 'TileCellNoiseMonHG',
                                                  OutputLevel        = OutputLevel,
                                                  doOnline           = athenaCommonFlags.isOnline(),
                                                  cellsContainerName = "AllCaloHG",
                                                  histoPathBase = "/Tile/CellNoise/HG");
        TileCellNoiseMonHG.Xmin          = -300.;
        TileCellNoiseMonHG.Xmax          =  300.;
        TileMon.AthenaMonTools += [ TileCellNoiseMonHG ];
        printfunc (TileCellNoiseMonHG)

    if (not TileBiGainRun) and doTileCellNoiseMon:
        TileCellNoiseMon = TileCellNoiseMonTool(name               = 'TileCellNoiseMon',
                                                OutputLevel        = OutputLevel,
                                                doOnline           = athenaCommonFlags.isOnline(),
                                                cellsContainerName = "AllCalo",
                                                histoPathBase = "/Tile/CellNoise");
        TileCellNoiseMon.Xmin          = -2000.;
        TileCellNoiseMon.Xmax          =  2000.;
        TileMon.AthenaMonTools += [ TileCellNoiseMon ];
        printfunc (TileCellNoiseMon)


    if doTileRawChannelTimeMonTool:
        TileRawChannelTimeMon = TileRawChannelTimeMonTool ( name            = "TileRawChannelTimeMon",
                                                           histoPathBase    = "/Tile/RawChannelTime",
                                                           runType          = TileRunType,
                                                           LowGainThreshold = 10.0,
                                                           HiGainThreshold  = 40.0,
                                                           doOnline         = athenaCommonFlags.isOnline(),
                                                           CheckDCS           = TileUseDCS,
                                                           TileRawChannelContainer = "TileRawChannelFit")

        TileMon.AthenaMonTools += [ TileRawChannelTimeMon ];
        printfunc (TileRawChannelTimeMon)




    ############ doTileRawChannelNoiseMonTool #########
    if TileBiGainRun and doTileRawChannelNoiseMon:
        TileRawChannelNoiseMonLG = TileRawChannelNoiseMonTool(name          = 'TileRawChannelNoiseMonLG',
                                                              OutputLevel   = OutputLevel,
                                                              doOnline      = athenaCommonFlags.isOnline(),
                                                              histoPathBase = "/Tile/RawChannelNoise/LG",
                                                              Xmin          = -10.,
                                                              Xmax          =  10.,
                                                              Gain          = "LG",
                                                              do2GFit       = True,
                                                              # doFit         = True,
                                                              CheckDCS           = TileUseDCS,
                                                              SummaryUpdateFrequency = 0 );

        TileMon.AthenaMonTools += [ TileRawChannelNoiseMonLG ];
        printfunc (TileRawChannelNoiseMonLG)


        TileRawChannelNoiseMonHG = TileRawChannelNoiseMonTool(name          = 'TileRawChannelNoiseMonHG',
                                                              OutputLevel   = OutputLevel,
                                                              doOnline      = athenaCommonFlags.isOnline(),
                                                              histoPathBase = "/Tile/RawChannelNoise/HG",
                                                              Xmin          = -10.,
                                                              Xmax          =  10.,
                                                              Gain          = "HG",
                                                              do2GFit       = True,
                                                              # doFit         = True,
                                                              SummaryUpdateFrequency = 0 );

        TileMon.AthenaMonTools += [ TileRawChannelNoiseMonHG ];
        printfunc (TileRawChannelNoiseMonHG)

    if (not TileBiGainRun) and doTileRawChannelNoiseMon:
        TileRawChannelNoiseMon = TileRawChannelNoiseMonTool(name               = 'TileRawChannelNoiseMon',
                                                            OutputLevel        = OutputLevel,
                                                            doOnline           = athenaCommonFlags.isOnline(),
                                                            histoPathBase = "/Tile/RawChannelNoise",
                                                            Xmin          = -10.0,
                                                            Xmax          =  10.0,
                                                            Gain          = "HG",
                                                            do2GFit       = True,
                                                            # doFit         = True,
                                                            SummaryUpdateFrequency = 0);

        # fill raw channel noise mon histograms only for certain trigger types
        # if not defined here, then by default all triggers will be considered
        TileRawChannelNoiseMon.TriggerTypes           = [ 0x82 ];

        TileMon.AthenaMonTools += [ TileRawChannelNoiseMon ];
        printfunc (TileRawChannelNoiseMon)


    ########### end doTileCellNoiseMon ##########

    if doTileTMDBDigitsMon:
        TileTMDBDigitsMon = CfgMgr.TileTMDBDigitsMonTool(name                  = 'TileTMDBDigitsMon'
                                                , OutputLevel         = INFO
                                                , TileDigitsContainer = "MuRcvDigitsCnt"
                                                , histoPathBase       = "/Tile/TMDBDigits")

        TileMon.AthenaMonTools += [ TileTMDBDigitsMon ]
        printfunc (TileTMDBDigitsMon)


    if doTileTMDBRawChannelMon:
        TileTMDBRawChannelDspMon = CfgMgr.TileTMDBRawChannelMonTool(name            = 'TileTMDBRawChannelDspMon'
                                                    , OutputLevel   = INFO
                                                    , NotDSP           = False
                                                    , TileRawChannelContainer = "MuRcvRawChCnt"
                                                    , histoPathBase = "/Tile/TMDBRawChannel/Dsp")


        TileMon.AthenaMonTools += [TileTMDBRawChannelDspMon ]
        printfunc (TileTMDBRawChannelDspMon)

        TileTMDBRawChannelMon = CfgMgr.TileTMDBRawChannelMonTool(name                      = 'TileTMDBRawChannelMon'
                                                    , OutputLevel             = INFO
                                                    , TileRawChannelContainer = "TileMuRcvRawChannelOpt2"
                                                    , NotDSP                   = True
                                                    , AmplitudeThresholdForTime = 10.0
                                                    , histoPathBase           = "/Tile/TMDBRawChannel")


        TileMon.AthenaMonTools += [TileTMDBRawChannelMon ]
        printfunc (TileTMDBRawChannelMon)


    if doTileMonDigi or doTileMonRch or doTileMonCell or doTileMonDQ                \
            or doTileDigiNoiseMon or doTileCellNoiseMon or doTileRawChannelNoiseMon \
            or doTileTMDBDigitsMon or doTileTMDBRawChannelMon:

        topSequence += TileMon;


if doTileCalib:
    # new options to be written
    if TilePhysRun or TilePedRun:
        #Add Noise Calib Tool
        from TileCalibAlgs.TileCalibAlgsConf import TileDigiNoiseCalibAlg
        from TileCalibAlgs.TileCalibAlgsConf import TileRawChNoiseCalibAlg

        if Version == "0" or Version == "Ped.0" or Version == "Ped" : # prudce digi noise ntuple only for default version
            theTileDigiNoiseCalibAlg = TileDigiNoiseCalibAlg( "theTileDigiNoiseCalibAlg" )
            theTileDigiNoiseCalibAlg.DoAvgCorr = False # False=> Full AutoCorr matrix calculation
            if TileNoiseFilter > 0:
                theTileDigiNoiseCalibAlg.FileNamePrefix = 'Digi_NoiseCalib_%(Version)s'  %  {'Version': TileNoiseFilter }
            if Version != "0" and Version != "Ped.0" and Version != "Ped" :
                VF = Version+"_tnf"+str(TileNoiseFilter)
                theTileDigiNoiseCalibAlg.FileNamePrefix = 'Digi_NoiseCalib_%(Version)s'  %  {'Version': VF }
            topSequence += theTileDigiNoiseCalibAlg
        theTileRawChNoiseCalibAlg = TileRawChNoiseCalibAlg("theTileRawChNoiseCalibAlg")
        theTileRawChNoiseCalibAlg.UseforCells = 1  # 1= Fixed , 2= Opt2
        if TileNoiseFilter > 0:
            theTileRawChNoiseCalibAlg.FileNamePrefix = 'RawCh_NoiseCalib_%(Version)s' % {'Version': TileNoiseFilter }
        if Version != "0" and Version != "Ped.0" and Version != "Ped" :
            VF = Version + "_tnf" + str(TileNoiseFilter)
            theTileRawChNoiseCalibAlg.FileNamePrefix = 'RawCh_NoiseCalib_%(Version)s'  %  {'Version': VF }
        topSequence += theTileRawChNoiseCalibAlg

        if TileEmulateDSP:
            theTileRawChNoiseCalibAlg.UseforCells=1 # i.e. from TileRawChannelFixed, which is like DSP results

        if doSim:
            theTileRawChNoiseCalibAlg.doFit   = False
            theTileRawChNoiseCalibAlg.doFixed = False
            theTileRawChNoiseCalibAlg.doOpt   = False
            theTileRawChNoiseCalibAlg.doDsp   = True
            theTileRawChNoiseCalibAlg.UseforCells=3 # i.e. from TileRawChannelCnt (like DSP)
        else:
            theTileRawChNoiseCalibAlg.doDsp   = False

    elif TileCisRun:
        # CIS calibration using top calib alg
        from TileCalibAlgs.TileCalibAlgsConf import TileTopCalibAlg
        from TileCalibAlgs.TileCalibAlgsConf import TileCisDefaultCalibTool

        TileCalibAlg = TileTopCalibAlg()
        TileCalibAlg.RunNumber        = RunNumber
        TileCalibAlg.RunType          = 8
        TileCalibAlg.FileName = '%(dir)s/tileCalibCIS_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }

        # declare CIS tool(s) and set jobOptions if necessary
        TileCisTool = TileCisDefaultCalibTool()
        dqStatus.TileRawChannelContainer = 'TileRawChannelCnt'

        if hasattr(ToolSvc, 'TileDigitsMon'):
            TileCisTool.StuckBitsProbsTool = ToolSvc.TileDigitsMon

        TileCisTool.removePed = True
        #from AthenaCommon.AppMgr import ToolSvc
        TileCalibAlg.TileCalibTools += [ TileCisTool ]

        topSequence += TileCalibAlg
    elif TileL1CaloRun:
        if RUN2 or RUN3: include("TrigT1CaloByteStream/ReadLVL1CaloBSRun2_jobOptions.py")
        else: include( "TrigT1CaloByteStream/ReadLVL1CaloBS_jobOptions.py" )

        # Trigger calibration using top calib alg
        from TileCalibAlgs.TileCalibAlgsConf import TileTopCalibAlg
        from TileCalibAlgs.TileCalibAlgsConf import TileTriggerDefaultCalibTool

        TileCalibAlg = TileTopCalibAlg()
        TileCalibAlg.RunNumber        = RunNumber
        TileCalibAlg.RunType          = 8
        TileCalibAlg.FileName = '%(dir)s/tileCalibL1Calo_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }

        # declare Trigger tool(s) and set jobOptions if necessary
        TileTriggerTool = TileTriggerDefaultCalibTool()
        dqStatus.TileRawChannelContainer = 'TileRawChannelCnt'

        #from AthenaCommon.AppMgr import ToolSvc
        TileCalibAlg.TileCalibTools += [ TileTriggerTool ]

        topSequence += TileCalibAlg
    elif TileLasRun:
        # Laser calibration
        from TileCalibAlgs.TileCalibAlgsConf import TileLaserCalibAlg
        from TileCalibAlgs.TileCalibAlgsConf import TileLaserDefaultCalibTool

        TileCalibAlg = TileLaserCalibAlg()
        TileCalibAlg.FileName = '%(dir)s/tileCalibLAS_%(RunNum).f_%(Version)s.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }

        # declare LASER tool(s) and set jobOptions if necessary
        TileLaserTool = TileLaserDefaultCalibTool()
        TileLaserTool.TileRawChannelContainer    = "TileRawChannelOpt2"
        TileLaserTool.TileLaserObject      = "TileLaserObj"

        if hasattr(ToolSvc, 'TileDigitsMon'):
            TileLaserTool.StuckBitsProbsTool = ToolSvc.TileDigitsMon

        #from AthenaCommon.AppMgr import ToolSvc
        TileCalibAlg.Tools = [ TileLaserTool ]

        topSequence += TileCalibAlg
    else:
        log.warning( "TileCalib options are not ready yet for this runtype" )

# Provides handle to give a time stamp for calib runs without time in the event header.
if 'ForceTimeStamp' in dir():
    svcMgr.IOVDbSvc.forceTimestamp = ForceTimeStamp


if doCreatePool:
    # Pool Output
    include( "AthenaPoolCnvSvc/WriteAthenaPool_jobOptions.py" )
    from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
    FileName = '%(dir)s/tile_%(RunNum).f_%(Version)s.pool.root' %  {'dir': OutputDirectory, 'RunNum': RunNumber, 'Version': Version }
    Stream1 = AthenaPoolOutputStream( "Stream1", FileName )

    # list of output objects
    Stream1.ItemList += [ "TileHitVector#*" ]
    Stream1.ItemList += [ "TileDigitsContainer#*" ]
    Stream1.ItemList += [ "TileBeamElemContainer#*" ]
    Stream1.ItemList += [ "TileRawChannelContainer#*" ]
    Stream1.ItemList += [ "TileMuContainer#*" ]
    Stream1.ItemList += [ "TileL2Container#*" ]
    Stream1.ItemList += [ "TileCosmicMuonContainer#*" ]
    Stream1.ItemList += [ "TileCellContainer#*" ]
    Stream1.ItemList += [ "CaloCellContainer#*" ]


if doEventDisplay:
    from VP1Algs.VP1AlgsConf import VP1Alg
    topSequence += VP1Alg()
    topSequence.TimeOut = 0


if doAtlantis:
    include("JiveXML/JiveXML_jobOptionBase.py")
    #from AthenaCommon.AppMgr import ToolSvc

    if 'doAtlantisStreamToServer' in dir() and doAtlantisStreamToServer:
        from JiveXML.JiveXMLConf import JiveXML__ONCRPCServerSvc
        svcMgr += JiveXML__ONCRPCServerSvc("ONCRPCServerSvc", OutputLevel = DEBUG)

        from JiveXML.JiveXMLConf import JiveXML__StreamToServerTool
        StreamToServerTool = JiveXML__StreamToServerTool(OutputLevel = DEBUG
                                                         , ServerService = svcMgr.ONCRPCServerSvc
                                                         , StreamName = "Test")

        ToolSvc += StreamToServerTool
        theEventData2XML.StreamTools += [ StreamToServerTool ]

    else:
        from JiveXML.JiveXMLConf import JiveXML__StreamToFileTool
        theStreamToFileTool=JiveXML__StreamToFileTool("theStreamToFileTool")
        ToolSvc += theStreamToFileTool
        theEventData2XML.StreamTools = [ theStreamToFileTool ]
        theEventData2XML.WriteToFile = False



    if doCaloCell or doCaloNtuple:
        from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloTileRetriever
        theCaloTileRetriever = JiveXML__CaloTileRetriever (name = "CaloTileRetriever")
        theCaloTileRetriever.DoTileCellDetails = True
        theCaloTileRetriever.DoTileDigit = ReadDigits or ('ReadESD' in dir() and ReadESD)
        theCaloTileRetriever.CellThreshold = 50.0
        ToolSvc += theCaloTileRetriever
        theEventData2XML.DataTypes += ["JiveXML::CaloTileRetriever/CaloTileRetriever"]

        from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloMBTSRetriever
        theCaloMBTSRetriever = JiveXML__CaloMBTSRetriever (name = "CaloMBTSRetriever")
        theCaloMBTSRetriever.DoMBTSCellDetails = True
        theCaloMBTSRetriever.DoMBTSDigits = ReadDigits or ('ReadESD' in dir() and ReadESD)
        theCaloMBTSRetriever.MBTSThreshold = 0.05
        ToolSvc += theCaloMBTSRetriever
        theEventData2XML.DataTypes += ["JiveXML::CaloMBTSRetriever/CaloMBTSRetriever"]

    if doCaloTopoCluster:
        from xAODJiveXML.xAODJiveXMLConf import JiveXML__xAODCaloClusterRetriever
        thexAODCaloClusterRetriever = JiveXML__xAODCaloClusterRetriever (name = "xAODCaloClusterRetriever")
        thexAODCaloClusterRetriever.FavouriteClusterCollection = "TileTopoCluster"
        thexAODCaloClusterRetriever.OtherClusterCollections = [ "" ]
        ToolSvc += thexAODCaloClusterRetriever
        theEventData2XML.DataTypes += ["JiveXML::xAODCaloClusterRetriever/xAODCaloClusterRetriever"]

    printfunc (theEventData2XML)

#-----------------------
# And some final options
#-----------------------

theAuditorSvc = svcMgr.AuditorSvc
theAuditorSvc.Auditors =  [ "ChronoAuditor" ]

if not ReadPool:
    svcMgr.EventSelector.MaxBadEvents = 10000
    svcMgr.EventSelector.ProcessBadEvent = True
    if OutputLevel < 2:
        #svcMgr.ByteStreamInputSvc.DumpFlag = True
        ToolSvc.TileROD_Decoder.VerboseOutput = True
    if OutputLevel > 3:
        ToolSvc.TileROD_Decoder.OutputLevel = 3

if OutputLevel < 2:
    printfunc (topSequence)

svcMgr.MessageSvc.OutputLevel = OutputLevel
svcMgr.EventSelector.SkipEvents = EvtMin
theApp.EvtMax=EvtMax

from AthenaServices.AthenaServicesConf import AthenaEventLoopMgr
svcMgr += AthenaEventLoopMgr()
svcMgr.AthenaEventLoopMgr.EventPrintoutInterval = 100

if not 'db' in dir():
    from DBReplicaSvc.DBReplicaSvcConf import DBReplicaSvc
    svcMgr += DBReplicaSvc(UseCOOLSQLite=False)

