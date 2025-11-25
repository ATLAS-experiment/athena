#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from TrigEDMConfig.TriggerEDM import recordable
import AthenaCommon.SystemOfUnits as Units
from TrigMinBias.TrigMinBiasMonitoring import MbtsHypoToolMonitoring

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from ..Config.MenuComponents import InViewRecoCA, InEventRecoCA, SelectionCA, MenuSequence

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def SPCountHypoToolGen(flags, chainDict):
    hypo = CompFactory.SPCountHypoTool(chainDict["chainName"])
    if "hmt" in chainDict["chainName"]:
        hypo.sctSP = int(chainDict["chainParts"][0]["hypoSPInfo"].strip("sp"))
    if "mb_sptrk_" in chainDict["chainName"] or "mb_sp_" in chainDict["chainName"]:
        hypo.pixCL = 2
        hypo.sctSP = 3
    if "mb_excl" in chainDict["chainName"]:
        hypo.pixCLMax = 150 # TODO revisit tightening those
        hypo.sctSPMax = 150 # as above
    if "sp_pix" in chainDict["chainName"]:
        hypo.pixCL = int(chainDict["chainParts"][0]["hypoSPInfo"].removeprefix("pix"))
    if "sp_vpix" in chainDict["chainName"]:
        hypo.pixCL = -1  # Remove any cut on mininum number of Pixel and SCT SpacePoints
        hypo.sctSP = -1
        hypo.pixCLMax = int(chainDict["chainParts"][0]["hypoSPInfo"].removeprefix("vpix"))
    if "nototpix" in chainDict["chainName"]:
        hypo.pixCL = -1  # Remove any cut on mininum number of Pixel and SCT SpacePoints
        hypo.sctSP = -1
        hypo.pixCLnoToT = int(chainDict["chainParts"][0]["hypoSPInfo"].removeprefix("nototpix"))
    if "q2" in chainDict["chainName"]:
        hypo.pixQ2mod = 0.4 # to be adjusted
    if "mb_pixsptrk_" in chainDict["chainName"]: #to be adjusted
        hypo.pixCL = 2
        hypo.sctSP = 0

    return hypo


def TrackCountHypoToolGen(flags, chainDict):
    def ptToGeV(v):        
        if "p" in v:
            sv = v.split("p")
            return int(sv[0])+0.1*int(sv[1])
        else:
            return int(v)

    hypo = CompFactory.TrackCountHypoTool(chainDict["chainName"])
    if "hmt" in chainDict["chainName"]:
        hypo.minNtrks = int(chainDict["chainParts"][0]["hypoTrkInfo"].strip("trk"))
        hypo.minPt = 200*Units.MeV
        if "pusup" in chainDict["chainName"]:
            hypo.maxVertexZ = 10*Units.mm
    if "mb_sptrk" in chainDict["chainName"]:
        hypo.minPt = 100*Units.MeV
        hypo.maxZ0 = 401*Units.millimeter
        if "_pt" in chainDict["chainName"]:
            hypo.minPt = ptToGeV(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
        if "_trk" in chainDict["chainName"]:
            hypo.minNtrks = int(chainDict["chainParts"][0]["hypoTrkInfo"].strip("trk"))
    if "excl" in chainDict["chainName"]:
        hypo.exclusive = True
        hypo.minPt = ptToGeV(chainDict["chainParts"][0]["hypoPtInfo"].strip("pt"))*Units.GeV
        trk = chainDict["chainParts"][0]["hypoTrkInfo"]
        # this is string of the form 4trk6 - 'number'trk'number'
        limits =  trk[0:trk.index("trk")], trk[trk.index("trk")+3:]
        hypo.minNtrks = int(limits[0])
        hypo.maxNtrks = int(limits[1])
    if "mb_pixsptrk" in chainDict["chainName"]: # to be adjusted
        hypo.minPt = 100*Units.MeV
        hypo.maxZ0 = 401*Units.millimeter

        # will set here cuts
    return hypo

def MbtsHypoToolGen(flags, chainDict):
    hypo = CompFactory.MbtsHypoTool(chainDict["chainName"]) # to now no additional settings
    if chainDict["chainParts"][0]["extra"] in ["vetombts2in", "vetospmbts2in"]:
        hypo.MbtsCounters=2
        hypo.MBTSMode=1
        hypo.Veto=True
    if '_all_' in chainDict["chainName"]:
        hypo.AcceptAll = True
        if "mbMon:online" in chainDict["monGroups"]:
            hypo.MonTool = MbtsHypoToolMonitoring(flags)
    else:  #default, one counter on each side
        hypo.MbtsCounters=1
    return hypo



def TrigZVertexHypoToolGen(flags, chainDict):
    hypo = CompFactory.TrigZVertexHypoTool(chainDict["chainName"])
    if "pusup" in chainDict["chainName"]:
        hypo.minWeight = int(chainDict["chainParts"][0]["pileupInfo"].removeprefix("pusup"))
    else:
        hypo.minWeight = -1 # pass always

    return hypo

@AccumulatorCache
def MinBiasSPSel(flags):

    reco = InViewRecoCA("SPCountingReco")
    minBiasFlags = flags.cloneAndReplace("Tracking.ActiveConfig","Trigger.InDetTracking.minBias")

    from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
    seq = InnerTrackingTrigSequence.create(minBiasFlags,
                                           minBiasFlags.Tracking.ActiveConfig.input_name, # this is already in the flags, maybe we would not need to pass it in the future?
                                           rois   = str(reco.inputMaker().InViewRoIs),
                                           inView = str(reco.inputMaker().Views))
    spMakingCA = seq.sequence("spacePointFormation")

    reco.mergeReco(spMakingCA)
    
    # TODO, this is a dependancy of SPcounting
    vdv = CompFactory.AthViews.ViewDataVerifier( "VDVSPCountingInputs",
                                                  DataObjects = {( 'PixelID' , 'DetectorStore+PixelID' ),
                                                                 ( 'SCT_ID'  , 'DetectorStore+SCT_ID' )} )
    reco.addRecoAlgo(vdv)

    from TrigMinBias.MinBiasCountersConfig import SPCounterRecoAlgCfg
    reco.mergeReco(SPCounterRecoAlgCfg(flags))

    selAcc = SelectionCA("MinBiasSPCounting")
    selAcc.mergeReco(reco)
    spCountHypo = CompFactory.SPCountHypoAlg(SpacePointsKey=recordable("HLT_SpacePointCounts"))
    selAcc.addHypoAlgo(spCountHypo)
    return selAcc

def MinBiasSPSequenceGenCfg(flags):
    selAcc = MinBiasSPSel(flags)
    return MenuSequence(flags, selAcc, HypoToolGen = SPCountHypoToolGen)

def MinBiasZVertexFinderSequenceGenCfg(flags):
    recoAcc = InViewRecoCA(name="ZVertFinderReco", InViewRoIs="InputRoI", RequireParentView=True)
    vdv = CompFactory.AthViews.ViewDataVerifier( "VDVZFinderInputs",
                                                  DataObjects = {( 'SpacePointContainer' , 'StoreGateSvc+PixelTrigSpacePoints'),
                                                                 ( 'PixelID' , 'DetectorStore+PixelID' ) })

    recoAcc.addRecoAlgo(vdv)
    from IDScanZFinder.ZFinderAlgConfig import  MinBiasZFinderCfg
    recoAcc.mergeReco( MinBiasZFinderCfg(flags) )
    selAcc = SelectionCA("ZVertexFinderSel")
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo( CompFactory.TrigZVertexHypoAlg("TrigZVertexHypoAlg", ZVertexKey=recordable("HLT_vtx_z")))
    return MenuSequence(flags, selAcc, HypoToolGen = TrigZVertexHypoToolGen)


def MinBiasTrkSequenceGenCfg(flags):
    recoAcc = InViewRecoCA(name="MBTrackReco", InViewRoIs="InputRoI", RequireParentView=True)

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, "minBias", log)

    from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
    trkSeq = InnerTrackingTrigSequence.create(flagsWithTrk, 
                                              flagsWithTrk.Tracking.ActiveConfig.input_name, 
                                              rois = "InputRoI", 
                                              inView = "VDVMinBiasIDTracking") # here

    recoAcc.mergeReco(trkSeq.sequence("OfflineNoDataPrep"))

    selAcc = SelectionCA("MBTrackCountSel")
    selAcc.mergeReco(recoAcc)
    from TrigMinBias.MinBiasCountersConfig import TrackCounterHypoAlgCfg
    trackCountHypoAlgo = TrackCounterHypoAlgCfg(flagsWithTrk)
    selAcc.mergeHypo(trackCountHypoAlgo)
    return MenuSequence(flagsWithTrk, selAcc, HypoToolGen = TrackCountHypoToolGen)


def MinBiasPixelTrkSequenceGenCfg(flags):
    recoAcc = InViewRecoCA(name="MBPixelTrackReco", InViewRoIs="InputRoI", RequireParentView=True)

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, "minBiasPixel", log)

    from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
    trkSeq = InnerTrackingTrigSequence.create(flagsWithTrk, 
                                              flagsWithTrk.Tracking.ActiveConfig.input_name, 
                                              rois = "InputRoI", 
                                              inView = "VDVMinBiasIDTracking") # here

    recoAcc.mergeReco(trkSeq.sequence("OfflineNoDataPrep"))

    selAcc = SelectionCA("MBPixelTrackCountSel")
    selAcc.mergeReco(recoAcc)
    from TrigMinBias.MinBiasCountersConfig import PixelTrackCounterHypoAlgCfg
    trackCountHypoAlgo = PixelTrackCounterHypoAlgCfg(flagsWithTrk)
    selAcc.mergeHypo(trackCountHypoAlgo)
    return MenuSequence(flagsWithTrk, selAcc, HypoToolGen = TrackCountHypoToolGen)



def MinBiasMbtsSequenceGenCfg(flags):
    recoAcc = InEventRecoCA(name="Mbts")

    from TrigMinBias.MbtsConfig import MbtsFexCfg, MbtsSGInputCfg
    recoAcc.mergeReco(MbtsSGInputCfg(flags))

    fex = MbtsFexCfg(flags, MbtsBitsKey = recordable("HLT_MbtsBitsContainer"))
    recoAcc.mergeReco(fex)

    selAcc = SelectionCA("MbtsSel")
    selAcc.mergeReco(recoAcc)

    hypo = CompFactory.MbtsHypoAlg("MbtsHypoAlg", MbtsBitsKey = fex.getPrimary().MbtsBitsKey)
    selAcc.addHypoAlgo(hypo)

    return MenuSequence(flags,
                        selAcc,
                        HypoToolGen = MbtsHypoToolGen)


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()
    zf = MinBiasZVertexFinderSequenceGenCfg(flags)
    zf.ca.printConfig(withDetails=True)

    mb = MinBiasMbtsSequenceGenCfg(flags)
    mb.ca.printConfig()


