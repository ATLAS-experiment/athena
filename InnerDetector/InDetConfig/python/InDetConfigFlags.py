# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType
from Campaigns.Utils import Campaign
from TrkConfig.TrkConfigFlags import PrimaryPassConfig


def createInDetConfigFlags():
    icf = AthConfigFlags()

    # Detector flags
    # Turn running of the truth seeded pseudo tracking only for pileup on and off.
    # Only makes sense to run on RDO file where SplitDigi was used!
    icf.addFlag("InDet.doSplitReco", False)
    # Turn on running of PRD MultiTruthMaker
    icf.addFlag("InDet.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # defines if the X1X mode is used for the offline or not
    icf.addFlag("InDet.selectSCTIntimeHits", lambda prevFlags: (
        not(prevFlags.Beam.Type is BeamType.Cosmics or
            prevFlags.Tracking.PrimaryPassConfig is PrimaryPassConfig.VtxBeamSpot)))
    icf.addFlag("InDet.useDCS", True)
    icf.addFlag("InDet.usePixelDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnablePixel))
    icf.addFlag("InDet.useSctDCS", lambda prevFlags: (
        prevFlags.InDet.useDCS and prevFlags.Detector.EnableSCT))
    # Use old (non CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoraCool", False)
    # Use new (CoolVectorPayload) SCT Conditions
    icf.addFlag("InDet.ForceCoolVectorPayload", False)
    # Turn on SCT_ModuleVetoSvc, allowing it to be configured later
    icf.addFlag("InDet.doSCTModuleVeto", False)
    # Enable check for dead modules and FEs
    icf.addFlag("InDet.checkDeadElementsOnTrack", True)
    # Turn running of Event Info TRT Occupancy Filling Alg on and off (also whether it is used in TRT PID calculation)
    icf.addFlag("InDet.doTRTGlobalOccupancy", False)
    icf.addFlag("InDet.noTRTTiming", lambda prevFlags:
                prevFlags.Beam.Type is BeamType.SingleBeam and
                prevFlags.Detector.EnableTRT)
    icf.addFlag("InDet.doTRTPhase", lambda prevFlags:
                prevFlags.Beam.Type is BeamType.Cosmics and
                prevFlags.Detector.EnableTRT)
    # Disabled for data-taking up to 2024 included and MC campaigns up to MC23e included
    icf.addFlag("InDet.doTRTArToTCorr", lambda prevFlags: (
        (not prevFlags.Input.isMC and prevFlags.Input.DataYear >= 2025) or
        (prevFlags.Input.isMC and prevFlags.Input.MCCampaign >= Campaign.MC23g)
    ))

    # Save cluster information to Derivation
    icf.addFlag("InDet.DRAWZSelection", False)
    icf.addFlag("InDet.DAODStorePixel", lambda prevFlags:
                prevFlags.Detector.EnablePixel)
    icf.addFlag("InDet.DAODStoreSCT", lambda prevFlags:
                prevFlags.Detector.EnableSCT)
    icf.addFlag("InDet.DAODStoreTRT", lambda prevFlags:
                prevFlags.Detector.EnableTRT)
    icf.addFlag("InDet.DAODStoreExtra", True)

    # Specific flags for pixel study
    icf.addFlag("InDet.PixelDumpMode", 1)
    icf.addFlag("InDet.PixelConfig.version", 'PixelConditionsAlgorithms/v1/')
    icf.addFlag("InDet.PixelConfig.UserInputFileName", '')

    # Save SiHitCollections to RDO
    icf.addFlag("InDet.savePixelSiHits", lambda prevFlags:
                prevFlags.BTagging.Trackless or
                prevFlags.BTagging.savePixelHits)
    icf.addFlag("InDet.saveSCTSiHits", lambda prevFlags:
                prevFlags.BTagging.Trackless or
                prevFlags.BTagging.saveSCTHits)

    # SCT prescale flags
    icf.addFlag("InDet.SCTxAODPrescale", 
                lambda prevFlags: 50 if prevFlags.Input.TriggerStream == 'express' else (10 if prevFlags.Input.TriggerStream == 'IDprescaledL1' else 1))

    # SCT skimming flags
    icf.addFlag("InDet.SCTxAODZmumuSkimming", False)
    icf.addFlag("InDet.SCTxAODSaveOnlyAssociatedMSOS", False)

    return icf
