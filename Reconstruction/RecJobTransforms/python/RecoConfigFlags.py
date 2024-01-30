# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode
_all_domains = [
    "Trigger",
    "BeamSpotDecoration",
    "Calo", "Tracking",
    "HGTDExtension",
    "Muon", "CombinedMuon",
    "Egamma",
    "Isolation",
    "CaloExtension",
    "TrackCellAssociation",
    "PFlow", "Jet", "BTagging",
    "Tau",
    "GlobalFELinking",
    "Met",
    "CaloRinger",
    "AFP",
    "HI",
    "PostProcessing",
    "TrackOverlay",
]


def createRecoConfigFlags():
    """Return an AthConfigFlags object with required flags"""
    flags = AthConfigFlags()

    # The various reconstruction domains/steps.
    # Note that Calo and Muon System Reconstruction
    # depend just on the Detector.EnableCalo and
    # Detector.EnableMuon flags.

    # Enable Tracking Reconstruction
    flags.addFlag("Reco.EnableTracking",
                  lambda prevFlags: prevFlags.Detector.EnableID or
                  prevFlags.Detector.EnableITk)
    # Enable HGTD Reconstruction
    flags.addFlag("Reco.EnableHGTDExtension",
                  lambda prevFlags: prevFlags.Reco.EnableTracking and
                  prevFlags.Detector.EnableHGTD)
    # Enable Electron/Photon (EGamma) Reconstruction
    flags.addFlag("Reco.EnableEgamma",
                  lambda prevFlags: prevFlags.Detector.EnableCalo)
    # Enable Caching of InDet TrackParticles extension to
    # Calorimeter. Used by (Muon,PFlow,Tau)
    flags.addFlag("Reco.EnableCaloExtension", lambda prevFlags: (
        (
            prevFlags.Reco.EnablePFlow
            or prevFlags.Reco.EnableTau
            or prevFlags.Reco.EnableCombinedMuon
        )
        and prevFlags.Detector.EnableCalo
        and prevFlags.Reco.EnableTracking))
    # Enable Combined (InDet+MS) Muon Reconstruction
    flags.addFlag("Reco.EnableCombinedMuon",
                  lambda prevFlags: prevFlags.Detector.EnableMuon and
                  prevFlags.Reco.EnableTracking)
    # Enable TrackOverlay Reconstruction
    flags.addFlag("Reco.EnableTrackOverlay", lambda prevFlags: False)
    # Enable PFlow Reconstruction
    flags.addFlag("Reco.EnablePFlow", lambda prevFlags: (
        prevFlags.Reco.EnableTracking
        and prevFlags.Detector.EnableCalo
        and prevFlags.Tracking.doVertexFinding))
    # Enable Isolation Reconstruction
    flags.addFlag("Reco.EnableIsolation", lambda prevFlags: (
        prevFlags.Tracking.doVertexFinding
        and (prevFlags.Reco.EnableCombinedMuon
             or prevFlags.Reco.EnableEgamma)))
    # Enable Jet Reconstruction
    flags.addFlag("Reco.EnableJet", lambda prevFlags: (
        prevFlags.Detector.EnableCalo
        and prevFlags.Reco.EnableTracking
        and prevFlags.Reco.EnableEgamma
        and prevFlags.Reco.EnableCombinedMuon
        and prevFlags.Reco.EnablePFlow
        and prevFlags.Reco.HIMode is not HIMode.HI))

    # Enable Tau Reconstruction
    flags.addFlag("Reco.EnableTau", lambda prevFlags: prevFlags.Reco.EnableJet)
    # Enable BTagging Reconstruction
    flags.addFlag("Reco.EnableBTagging",
                  lambda prevFlags: prevFlags.Reco.EnableJet or 
                   (prevFlags.Reco.EnableHI and prevFlags.HeavyIon.doJet)) 
    # Enable MET Reconstruction
    flags.addFlag("Reco.EnableMet", lambda prevFlags: (
        prevFlags.Reco.EnableJet
        and prevFlags.Reco.EnableTau))
    # Enable the building of links between the newly
    # created jet constituents (GlobalFE)
    # and electrons,photons,muons and taus
    flags.addFlag("Reco.EnableGlobalFELinking",
                  lambda prevFlags: prevFlags.Reco.EnableJet and
                  prevFlags.Reco.EnableTau and prevFlags.Reco.EnablePFlow and
                  prevFlags.Reco.EnableEgamma and
                  prevFlags.Reco.EnableCombinedMuon)
    # Enable association of calorimeter cells to Tracks
    flags.addFlag("Reco.EnableTrackCellAssociation",
                  lambda prevFlags: prevFlags.Detector.EnableCalo and
                  prevFlags.Reco.EnableTracking)
    # Enable creation of "Rings" of calorimeter cells
    flags.addFlag("Reco.EnableCaloRinger",
                  lambda prevFlags: prevFlags.Reco.EnableEgamma and 
                  not prevFlags.Reco.EnableHI)

    # This flags enables trigger data decoding (not trigger simulation)
    # EDMVersion > 0 check prevents this flag being true in jobs before
    # the trigger has executed, or where it was not executed.
    flags.addFlag("Reco.EnableTrigger",
                  lambda prevFlags: prevFlags.Trigger.EDMVersion > 0)

    # enable automatically for HI data
    flags.addFlag("Reco.EnableHI",
                  lambda prevFlags: prevFlags.Reco.HIMode is not HIMode.pp)

    flags.addFlag("Reco.HIMode", _hiModeChoice, type=HIMode)

    # Enable alg for decorating EventInfo with BeamSpot info
    # (maybe not always available for calibration runs, etc)
    flags.addFlag("Reco.EnableBeamSpotDecoration",
                  lambda prevFlags: not prevFlags.Common.isOnline)

    # Enable ZDC reconstruction if ZDC data is in Bytestream or simulated
    flags.addFlag("Reco.EnableZDC",_recoZDC)
                  
    # Enable common thinning and other post-processing
    flags.addFlag("Reco.EnablePostProcessing", True)
    flags.addFlag("Reco.PostProcessing.ThinNegativeClusters",
                  lambda prevFlags: prevFlags.Reco.EnablePostProcessing and
                  prevFlags.Detector.EnableCalo and
                  prevFlags.Output.doWriteAOD and
                  prevFlags.Calo.Thin.NegativeEnergyCaloClusters and
                  not prevFlags.Reco.EnableHI)

    flags.addFlag("Reco.PostProcessing.TRTAloneThinning",
                  lambda prevFlags: prevFlags.Reco.EnablePostProcessing and
                  prevFlags.Reco.EnableTracking and
                  prevFlags.Output.doWriteAOD)
    flags.addFlag("Reco.PostProcessing.GeantTruthThinning",
                  lambda prevFlags: prevFlags.Reco.EnablePostProcessing and
                  prevFlags.Input.isMC and
                  prevFlags.Output.doWriteAOD)
    flags.addFlag("Reco.PostProcessing.InDetForwardTrackParticleThinning",
                  lambda prevFlags: prevFlags.Reco.EnablePostProcessing and
                  prevFlags.Reco.EnableTracking and
                  prevFlags.Reco.EnableCombinedMuon and
                  prevFlags.Output.doWriteAOD)
    return flags


def printRecoFlags(flags):
    # setup logging
    from AthenaCommon.Logging import logging
    log = logging.getLogger('RecoSteering')

    # load flags
    flags._loadDynaFlags('Detector')
    flags._loadDynaFlags('Reco')

    # generate common formatting string
    item_len = 7
    format_common = f'%-{item_len}s'

    domain_len = 7
    for d in _all_domains:
        domain_len = max(domain_len, len(d) + 2)

    enabled = []
    for d in _all_domains:
        if flags.hasFlag(f'Detector.Enable{d}'):
            name = f'Detector.Enable{d}'
        elif flags.hasFlag(f'Reco.Enable{d}'):
            name = f'Reco.Enable{d}'
        else:
            raise RuntimeError(f'Unknown reconstruction domain {d}')

        if flags(name) is not False:
            enabled.append('ON')
        else:
            enabled.append('--')

    format_header = f'%{domain_len}s   ' + format_common
    format = f'%{domain_len}s : ' + format_common
    data = [_all_domains, enabled]
    data = list(map(list, zip(*data)))

    # print header rows
    log.info(format_header, *(['', 'Enbl.']))
    # print data
    for row in data:
        log.info(format, *row)


def recoRunArgsToFlags(runArgs, flags):
    if hasattr(runArgs, "runNumber") and not flags.Input.isMC and runArgs.runNumber not in flags.Input.RunNumbers:
        flags.Input.RunNumbers = [runArgs.runNumber]
        flags.Input.OverrideRunNumber = True

    if hasattr(runArgs, "projectName"):
        flags.Input.projectName = runArgs.projectName

    # TODO: not handled yet
    # --autoConfiguration
    # --trigStream
    # --topOptions
    # --valid

    # --AMITag
    # --userExec
    # --triggerConfig
    # --trigFilterList


def _recoZDC(prevFlags):
    if prevFlags.Input.isMC: 
        return prevFlags.Detector.EnableZDC
    else:
        from AthenaConfiguration.AutoConfigFlags import GetFileMD
        from libpyeformat_helper import SubDetector, DetectorMask
        maskbits=GetFileMD(prevFlags.Input.Files).get("detectorMask",[0x0])
        maskbits=maskbits[0] #Check the first input file
        detMask=DetectorMask(maskbits & 0xFFFFFFFFFFFFFFFF, maskbits >> 64) #DetectorMask constructor swallows two 64bit ints
        return detMask.is_set(SubDetector.FORWARD_ZDC)



def _hiModeChoice(prevFlags):

    if ("_hip" in prevFlags.Input.ProjectName):
        return HIMode.HIP
    elif ("_hi" in prevFlags.Input.ProjectName):
        if (prevFlags.Input.TriggerStream == "physics_UPC"):
            return HIMode.UPC
        else:
            return HIMode.HI
    return HIMode.pp
