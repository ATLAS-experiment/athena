#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from glob import glob

def GetCustomAthArgs():
    from argparse import ArgumentParser
    IDPVMparser = ArgumentParser(description='Parser for IDPVM configuration')
    IDPVMparser.add_argument("--filesInput", required=True)
    IDPVMparser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    IDPVMparser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    IDPVMparser.add_argument("--doLargeD0Tracks", help='also run LRT plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doLowPtRoITracks", help='also run low pt tracks', action='store_true', default=False)
    IDPVMparser.add_argument("--doMergedLargeD0Tracks", help='also run merged STD+LRT plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doRecoOnly", help='skip truth-specific processing', action='store_true', default=False)
    IDPVMparser.add_argument("--doLoose", help='also run loose plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doTightPrimary", help='also run tight-primary plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doTracksInJets", help='also run tracks in jets', action='store_true', default=False)
    IDPVMparser.add_argument("--doTracksInBJets", help='also run tracks in jets', action='store_true', default=False)
    IDPVMparser.add_argument("--doTruthOrigin", help='make plots by track origin', action='store_true', default=False)
    IDPVMparser.add_argument("--doHitLevelPlots", help='make hit residual / eff plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doPerAuthor", help='make plots by track author', action='store_true', default=False)
    IDPVMparser.add_argument("--doExpertPlots", help='run additional expert-level plots', action='store_true', default=False)
    IDPVMparser.add_argument("--doMuonMatchedTracks", help='run plots for tracks matched to true muons', action='store_true', default=False)
    IDPVMparser.add_argument("--doElectronMatchedTracks", help='run plots for tracks matched to true electrons', action='store_true', default=False)
    IDPVMparser.add_argument("--doTruthToRecoNtuple", help='output track-to-truth ntuple', action='store_true', default=False)
    IDPVMparser.add_argument("--doActs", help='run plots for acts collections', action='store_true', default=False)
    IDPVMparser.add_argument("--disableDecoration", help='disable extra track and truth decoration if possible', action='store_true', default=False)
    IDPVMparser.add_argument("--hardScatterStrategy", help='Strategy to select the hard scatter. 0 = SumPt² 1 = SumPt , 2 = Sumptw, 3 = H->yy', choices=["0","1","2","3"], default="0")
    IDPVMparser.add_argument("--truthMinPt", help='minimum truth particle pT', type=float, default=None)
    IDPVMparser.add_argument("--outputFile", help='Name of output file',default="M_output.root")
    IDPVMparser.add_argument("--HSFlag", help='Hard-scatter flag - decides what is used for truth matching', choices=['HardScatter', 'All', 'PileUp'],default="HardScatter")
    IDPVMparser.add_argument("--jetsNameForHardScatter", help='Name of jet collection',default="AntiKt4EMTopoJets")
    IDPVMparser.add_argument("--ancestorIDList", help='List of ancestor truth IDs to match.', default = [], nargs='+', type=int)
    IDPVMparser.add_argument("--requiredSiHits", help='Number of truth silicon hits', type=int, default=0)
    IDPVMparser.add_argument("--selectedCharge", help='Charge of selected truth particles (0=inclusive)', type=int, default=0)
    IDPVMparser.add_argument("--maxProdVertRadius", help='Maximum production radius for truth particles', type=float, default=300)
    IDPVMparser.add_argument("--GRL", help='Which GRL(s) to use, if any, when running on data', choices=['2015', '2016', '2017', '2018', '2022'], nargs='+', default=[])
    IDPVMparser.add_argument("--validateExtraTrackCollections", help='List of extra track collection names to be validated in addition to Tracks.', nargs='+', default=[])
    IDPVMparser.add_argument("--doIDTIDE", help='run the output from IDTIDE derivation', action='store_true', default=False)
    IDPVMparser.add_argument("--doTechnicalEfficiency", help='fill the technical efficiency plot (requires additional si hit information in the input file)', action='store_true', default=False)
    return IDPVMparser.parse_args()

# Parse the arguments
MyArgs = GetCustomAthArgs()

from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

flags.Input.Files = []
for path in MyArgs.filesInput.split(','):
    flags.Input.Files += glob(path)
flags.PhysVal.OutputFileName = MyArgs.outputFile

# Set default truthMinPt depending on Run config
if MyArgs.truthMinPt is None:
    MyArgs.truthMinPt = 1000 if flags.GeoModel.Run >= LHCPeriod.Run4 \
                        else 500

flags.PhysVal.IDPVM.setTruthStrategy = MyArgs.HSFlag
flags.PhysVal.IDPVM.doExpertOutput   = MyArgs.doExpertPlots
flags.PhysVal.IDPVM.doPhysValOutput  = not MyArgs.doExpertPlots
flags.PhysVal.IDPVM.doValidateTruthToRecoNtuple = MyArgs.doTruthToRecoNtuple
flags.PhysVal.IDPVM.doValidateTracksInBJets = MyArgs.doTracksInBJets
flags.PhysVal.IDPVM.doValidateTracksInJets = MyArgs.doTracksInJets
flags.PhysVal.IDPVM.doIDTIDE= MyArgs.doIDTIDE
flags.PhysVal.IDPVM.doValidateLooseTracks = MyArgs.doLoose
flags.PhysVal.IDPVM.doValidateTightPrimaryTracks = MyArgs.doTightPrimary
flags.PhysVal.IDPVM.doTruthOriginPlots = MyArgs.doTruthOrigin
flags.PhysVal.IDPVM.doValidateMuonMatchedTracks = MyArgs.doMuonMatchedTracks
flags.PhysVal.IDPVM.doValidateElectronMatchedTracks = MyArgs.doElectronMatchedTracks
flags.PhysVal.IDPVM.doValidateLargeD0Tracks = MyArgs.doLargeD0Tracks
flags.PhysVal.IDPVM.doValidateMergedLargeD0Tracks = MyArgs.doMergedLargeD0Tracks
flags.PhysVal.IDPVM.doValidateLowPtRoITracks = MyArgs.doLowPtRoITracks
flags.PhysVal.IDPVM.doRecoOnly = MyArgs.doRecoOnly
flags.PhysVal.IDPVM.doPerAuthorPlots = MyArgs.doPerAuthor
flags.PhysVal.IDPVM.doHitLevelPlots = MyArgs.doHitLevelPlots
flags.PhysVal.IDPVM.runDecoration = not MyArgs.disableDecoration
flags.PhysVal.IDPVM.requiredSiHits = MyArgs.requiredSiHits
flags.PhysVal.IDPVM.selectedCharge = MyArgs.selectedCharge
flags.PhysVal.IDPVM.maxProdVertRadius = MyArgs.maxProdVertRadius
flags.PhysVal.IDPVM.ancestorIDs = MyArgs.ancestorIDList
flags.PhysVal.IDPVM.hardScatterStrategy = int(MyArgs.hardScatterStrategy)
flags.PhysVal.IDPVM.jetsNameForHardScatter = MyArgs.jetsNameForHardScatter
flags.PhysVal.IDPVM.truthMinPt = MyArgs.truthMinPt
flags.PhysVal.IDPVM.GRL = MyArgs.GRL
flags.PhysVal.IDPVM.validateExtraTrackCollections = MyArgs.validateExtraTrackCollections
flags.PhysVal.IDPVM.doTechnicalEfficiency = MyArgs.doTechnicalEfficiency
flags.PhysVal.doActs = MyArgs.doActs

flags.Exec.SkipEvents = MyArgs.skipEvents
flags.Exec.MaxEvents = MyArgs.maxEvents

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))

from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetPhysValMonitoringCfg
acc.merge(InDetPhysValMonitoringCfg(flags))

acc.printConfig(withDetails=True)

# Execute and finish
sc = acc.run()

# Success should be 0
import sys
sys.exit(not sc.isSuccess())
