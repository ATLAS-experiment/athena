# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Utils.unixtools import find_datafile
from AthenaPython.PyAthenaComps import Alg, StatusCode
from TileConfiguration.TileConfigFlags import TileRunType

import os
import sys
import glob


def findAbsPath(path):
    refPaths = [os.environ.get('ATLAS_REFERENCE_DATA', None),
                '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art',
                '/afs/cern.ch/atlas/maxidisk/d33/referencefiles']
    for refPath in refPaths:
        if refPath:
            absPath = os.path.join(refPath, path)
            if os.path.exists(absPath):
                return absPath

    from AthenaCommon.Logging import logging
    msg = logging.getLogger('findAbsPath')
    msg.error(f'Cannot find abs path: {path}')

    return None


def getRefPath(directory):
    """ Find reference directory."""
    referenceTag = os.environ.get('ATLAS_REFERENCE_TAG', 'TileByteStream-02-00-00')
    directoryPath = find_datafile(os.path.join('TileByteStream', referenceTag))
    refDirectory = None
    if directoryPath:
        refDirectory = os.path.join(directoryPath, directory)
    if not refDirectory or not os.path.exists(refDirectory):
        refDirectory = findAbsPath(os.path.join('TileByteStream', referenceTag, directory))
    return refDirectory


def getRefAndDumpDirectories(flags, directory):
    refDirectory = getRefPath(f'{directory}')
    dumpDirectory = f'{directory}-{flags.Concurrency.NumThreads}'
    return refDirectory, dumpDirectory


def getInputFile(fileName):
    """ Find input file. """
    filePath = os.environ.get('ATLAS_REFERENCE_DATA',
                              '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests')
    inputFile = os.path.join(filePath, fileName)
    if not os.path.exists(inputFile):
        inputFile = getRefPath(fileName)
    return inputFile


class Finalizer(Alg):
    def __init__(self, name=None, **kw):
        self.refDirectory = ""
        self.dumpDirectory = ""
        super(Finalizer, self).__init__(name, **kw)

    def finalize(self):
        self.msg.info(f'Reference directory: {self.refDirectory}')
        self.msg.info(f'Dump directory: {self.dumpDirectory}')
        dumps = glob.glob(os.path.join(self.refDirectory, '*.dump'))
        for refDump in dumps:
            localDump = os.path.join(self.dumpDirectory, os.path.basename(refDump))
            os.system(f'diff -u {refDump} {localDump}')
        self.msg.info(f'Finalize: compared {len(dumps)} dumps')
        sys.stdout.flush()
        return StatusCode.Success


def TileDigitsReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile digits container from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=True, readRawChannel=False, readMuRcv=False, readMuRcvDigits=True)

    TileDigitsDumper = CompFactory.TileDigitsDumper
    cfg.addEventAlgo(TileDigitsDumper('TileDigitsCntDumper',
                                      TileDigitsContainer='TileDigitsCnt',
                                      Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(TileDigitsDumper('MuRcvDigitsCntDumper',
                                      TileDigitsContainer='MuRcvDigitsCnt',
                                      Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileDigitsCompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


def TileRawChannelReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile raw channel container from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=False, readRawChannel=True, readMuRcv=False, readMuRcvRawCh=True)

    TileRawChannelDumper = CompFactory.TileRawChannelDumper
    cfg.addEventAlgo(TileRawChannelDumper('TileRawChannelCntDumper',
                                          TileRawChannelContainer='TileRawChannelCnt',
                                          Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(TileRawChannelDumper('MuRcvRawChannelCntDumper',
                                          TileRawChannelContainer='MuRcvRawChCnt',
                                          Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileRawChannelCompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


def TileLaserObjectReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile laser object from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=False, readRawChannel=False, readMuRcv=False, readLaserObj=True)

    TileLaserObjectDumper = CompFactory.TileLaserObjectDumper
    cfg.addEventAlgo(TileLaserObjectDumper('TileLaserObjectDumper',
                                           TileLaserObject='TileLaserObj',
                                           Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileLaserObjCompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


def TileL2ReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile L2 container from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=False, readRawChannel=False, readMuRcv=False, readL2=True)

    TileL2Dumper = CompFactory.TileL2Dumper
    cfg.addEventAlgo(TileL2Dumper('TileL2CntDumper',
                                  TileL2Container='TileL2Cnt',
                                  Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileL2CompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


def TileBeamElemReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile beam elements container from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=False, readRawChannel=False, readMuRcv=False, readBeamElem=True)

    TileBeamElemDumper = CompFactory.TileBeamElemDumper
    cfg.addEventAlgo(TileBeamElemDumper('TileBeamElemCntDumper',
                                        TileBeamElemContainer='TileBeamElemCnt',
                                        Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileBeamElemCompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


def TileMuRcvReadTestCfg(flags, refDirectory, dumpDirectory, **kwargs):
    """ Configure test of reading the Tile muon receiver container from  BS """

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg = TileRawDataReadingCfg(flags, readDigits=False, readRawChannel=False, readMuRcv=True)

    TileMuonReceiverDumper = CompFactory.TileMuonReceiverDumper
    cfg.addEventAlgo(TileMuonReceiverDumper('TileMuonReceiverDumper',
                                            TileMuonReceiverContainer='TileMuRcvCnt',
                                            Prefix=f'{dumpDirectory}/'))

    cfg.addEventAlgo(Finalizer('TileMuRcvCompareAlg', refDirectory=refDirectory, dumpDirectory=dumpDirectory))
    return cfg


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO

    # Test setup
    log.setLevel(INFO)

    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    tests_group = parser.add_argument_group('Test of reading Tile raw data from BS')
    tests = tests_group.add_mutually_exclusive_group()
    tests.add_argument('--digits', action='store_true', help='Test of reading Tile digits container from BS')
    tests.add_argument('--murcv', action='store_true', help='Test of reading Tile muon receiver container from BS')
    tests.add_argument('--level2', action='store_true', help='Test of reading Tile L2 container from BS')
    tests.add_argument('--raw-channels', dest='rawChannels', action='store_true', help='Test of reading Tile raw channel container from BS')
    tests.add_argument('--laser-object', dest='laserObject', action='store_true', help='Test of reading Tile laser object from BS')
    tests.add_argument('--beam-elements', dest='beamElements', action='store_true', help='Test of reading Tile beam elements container from BS')
    args, _ = parser.parse_known_args()

    if not any([args.digits, args.rawChannels, args.murcv, args.laserObject, args.beamElements, args.level2]):
        log.info('No tests are requested. Exiting ...')
        sys.exit(0)

    if any([args.murcv, args.laserObject]):
        flags.Input.Files = [getInputFile('data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data')]
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    else:
        flags.Input.Files = [getInputFile('data12_8TeV.00204073.physics_JetTauEtmiss.merge.RAW._lb0144._SFO-5._0001.1')]
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2012

    flags.Exec.MaxEvents = 100
    flags.Tile.RunType = TileRunType.PHY
    flags.IOVDb.DatabaseInstance = 'CONDBR2'
    flags.fillFromArgs(parser=parser)
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    dumpDirectory = None
    if args.digits:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileDigitDumps')
        cfg.merge( TileDigitsReadTestCfg(flags, refDirectory, dumpDirectory) )
    elif args.rawChannels:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileRawChannelDumps')
        cfg.merge( TileRawChannelReadTestCfg(flags, refDirectory, dumpDirectory) )
    elif args.laserObject:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileLaserDumps')
        cfg.merge( TileLaserObjectReadTestCfg(flags, refDirectory, dumpDirectory) )
    elif args.level2:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileL2Dumps')
        cfg.merge( TileL2ReadTestCfg(flags, refDirectory, dumpDirectory) )
    elif args.beamElements:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileBeamElemDumps')
        cfg.merge( TileBeamElemReadTestCfg(flags, refDirectory, dumpDirectory) )
    elif args.murcv:
        refDirectory, dumpDirectory = getRefAndDumpDirectories(flags, 'TileMuRcvDumps')
        cfg.merge( TileMuRcvReadTestCfg(flags, refDirectory, dumpDirectory) )

    cfg.printConfig(withDetails=True)

    if dumpDirectory:
        os.system(f'rm -rf {dumpDirectory}')
        os.system(f'mkdir -p {dumpDirectory}')

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
