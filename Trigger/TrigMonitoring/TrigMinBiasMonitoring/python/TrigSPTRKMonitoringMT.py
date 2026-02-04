#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
"""
@brief configuration for the min bias monitoring
"""

import math

from .utils import getMinBiasChains
from AthenaCommon.Logging import logging

log = logging.getLogger('TrigSPTRKMonitoringMT')


def TrigSPTRK(configFlags, highGranularity=False):

    from AthenaMonitoring import AthMonitorCfgHelper

    monConfig = AthMonitorCfgHelper(configFlags, "HLTMBSPTRKMonAlg")

    from AthenaConfiguration.ComponentFactory import CompFactory

    alg = monConfig.addAlgorithm(CompFactory.HLTMinBiasTrkMonAlg, "HLTMBSPTRKMonAlg")

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_LoosePrimary_Cfg)
    trkSel = monConfig.resobj.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(configFlags))
    alg.TrackSelectionTool = trkSel

    ZFinderCollection = 'HLT_vtx_z'
    if ZFinderCollection in configFlags.Input.Collections:
        log.info("Enabled z finder data reading")
        alg.zFinderDataKey = ZFinderCollection

    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    monAccess = getHLTMonitoringAccess(configFlags)

    spChains = getMinBiasChains(monAccess, '(pix|hmt)')
    spTrkChains = getMinBiasChains(monAccess, '(sptrk|excl|hmt|_mb_sp_)')

    log.info(f'Monitoring {len(spChains)} SpacePoints chains')
    log.debug([name for name, _ in spChains])
    log.info(f'Monitoring {len(spTrkChains)} Tracking chains')
    log.debug([name for name, _ in spTrkChains])

    alg.triggerListTrackingMon = [name for name, _ in spTrkChains]
    alg.triggerListSpacePointsMon = [name for name, _ in spChains]

    # Set track and cluster multiplicity histograms binning
    nbins = 400 if highGranularity else 50
    nTrkMax = 400
    nSctCl = 5000
    nPixBar = 2000
    nPixEC = 500

    for chain, group in spChains:
        mbSpGroup = monConfig.addGroup(alg, f'{chain}_SpacePoints', topPath=f"HLT/MinBiasMon/{group}/SpacePoints/{chain}/")

        mbSpGroup.defineHistogram("PixelCL;PixelCLNarrowRange", title="Number of SP in whole Pixels detector for all events", xbins=100, xmin=0, xmax=100)
        mbSpGroup.defineHistogram("PixelCL;PixelCLWideRange", title="Number of SP in whole Pixels detector for all events", xbins=250, xmin=0, xmax=nPixBar)
        mbSpGroup.defineHistogram("PixBarr_SP", title="Number of SP for all events in Barrel", xbins=250, xmin=0, xmax=nPixBar)
        mbSpGroup.defineHistogram("PixECA_SP", title="Number of SP for all events in ECA", xbins=250, xmin=0, xmax=nPixEC)
        mbSpGroup.defineHistogram("PixECC_SP", title="Number of SP for all events in ECC", xbins=250, xmin=0, xmax=nPixEC)
        mbSpGroup.defineHistogram("SctTot;SctTotNarrowRange", title="Number of SP in whole SCT detector for all events", xbins=100, xmin=0, xmax=100)
        mbSpGroup.defineHistogram("SctTot;SctTotWideRange", title="Number of SP in whole SCT detector for all events", xbins=250, xmin=0, xmax=nSctCl)
        mbSpGroup.defineHistogram("SctBarr_SP", title="Number of SCT_SP for all events in Barrel", xbins=250, xmin=0, xmax=nSctCl)
        mbSpGroup.defineHistogram("SctECA_SP", title="Number of SCT_SP for all events in ECA", xbins=250, xmin=0, xmax=nSctCl)
        mbSpGroup.defineHistogram("SctECC_SP", title="Number of SCT_SP for all events in ECC", xbins=250, xmin=0, xmax=nSctCl)
        mbSpGroup.defineHistogram("SctECA_SP,SctECC_SP", type="TH2F", title=";SctECA_SP;SctECC_SP", xbins=nbins, xmin=0, xmax=nSctCl, ybins=nbins, ymin=0, ymax=nSctCl)
        mbSpGroup.defineHistogram("PixECA_SP,PixECC_SP", type="TH2F", title=";PixECA_SP;PixECC_SP", xbins=nbins, xmin=0, xmax=nPixEC, ybins=nbins, ymin=0, ymax=nPixEC)
        mbSpGroup.defineHistogram("SctBarr_SP,PixBarr_SP", type="TH2F", title=";SctBarr_SP;PixBarr_SP", xbins=nbins, xmin=0, xmax=nSctCl, ybins=nbins, ymin=0, ymax=nPixBar)
        mbSpGroup.defineHistogram("SctECA_SP,PixECA_SP", type="TH2F", title=";SctECA_SP;PixECA_SP", xbins=nbins, xmin=0, xmax=nSctCl, ybins=nbins, ymin=0, ymax=nPixEC)
        mbSpGroup.defineHistogram("SctECC_SP,PixECC_SP", type="TH2F", title=";SctECC_SP;PixECC_SP", xbins=nbins, xmin=0, xmax=nSctCl, ybins=nbins, ymin=0, ymax=nPixEC)
        mbSpGroup.defineHistogram("SctTot,PixelCL", type="TH2F", title=";Number of SP in whole SCT detector for all events;Number of SP in whole Pixels detector for all events", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=4000)

    for chain, group in spTrkChains:
        mbSpTrkGroup = monConfig.addGroup(alg, f'{chain}_Tracking', topPath=f"HLT/MinBiasMon/{group}/Tracking/{chain}/")

        # 1D distributions
        mbSpTrkGroup.defineHistogram("nTrkOffline", title="Number of tracks reconstructed offline;track counts", xbins=200, xmin=-1, xmax=400)
        mbSpTrkGroup.defineHistogram("nTrkOffline;nTrkOfflineLowMult", title="Number of tracks reconstructed offline;track counts", xbins=50, xmin=-1, xmax=50)
        mbSpTrkGroup.defineHistogram("nTrkOnline;nTrkOnlineLowMult", title="Number of tracks reconstructed online;track counts", xbins=50, xmin=-1, xmax=50)

        mbSpTrkGroup.defineHistogram("trkPt", cutmask="trkMask", title="Offline selected tracks pt;p_{T} [GeV]", xbins=100, xmin=0, xmax=5)
        mbSpTrkGroup.defineHistogram("trkEta", cutmask="trkMask", title="Offline selected tracks eta;#eta", xbins=50, xmin=-2.5, xmax=2.5)
        mbSpTrkGroup.defineHistogram("trkPhi", cutmask="trkMask", title="Offline selected tracks phi;#phi", xbins=64, xmin=-math.pi, xmax=math.pi)
        mbSpTrkGroup.defineHistogram("trkHits", title="Offline selected tracks, hits per track;number of hits", xbins=15, xmin=-0.5, xmax=15 - 0.5)

        mbSpTrkGroup.defineHistogram("onlTrkPt", title="Online tracks pt;p_{T} [GeV]", xbins=100, xmin=0, xmax=5)
        mbSpTrkGroup.defineHistogram("onlTrkEta", title="Online tracks eta;#eta", xbins=50, xmin=-2.5, xmax=2.5)
        mbSpTrkGroup.defineHistogram("onlTrkPhi", title="Online tracks phi;#phi", xbins=64, xmin=-math.pi, xmax=math.pi)
        mbSpTrkGroup.defineHistogram("onlTrkHits", title="Online hits per track;number of hits", xbins=15, xmin=-0.5, xmax=15 - 0.5)
        mbSpTrkGroup.defineHistogram("onlTrkZ0", title="Online track z_{0};z_{0}[mm]", xbins=40, xmin=-200, xmax=200)
        mbSpTrkGroup.defineHistogram("onlTrkD0", title="Online track d_{0};d_{0}[mm]", xbins=40, xmin=-20, xmax=20)

        mbSpTrkGroup.defineHistogram("trkD0", cutmask="trkMask", title="Offline selected tracks D0;d_{0} [mm]", xbins=40, xmin=-20, xmax=20)
        mbSpTrkGroup.defineHistogram("trkZ0wrtPV", cutmask="trkMask", title="Offline selected tracks Z0 wrt PV;z_{0}[mm]", xbins=40, xmin=-20, xmax=20)
        mbSpTrkGroup.defineHistogram("trkZ0", cutmask="trkMask", title="Offline selected tracks Z0;z_{0}[mm]", xbins=40, xmin=-200, xmax=200)

        mbSpTrkGroup.defineHistogram("onlineOfflineVtxDelta", title=";(offline - online) vertex z[mm]", xbins=200, xmin=-200, xmax=200)

        # 2D maps
        mbSpTrkGroup.defineHistogram('trkEta,trkPt', cutmask='trkMask', type='TH2F', title='Offline selected tracks pT/eta correlation;#eta;p_{T} [GeV]', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=0, ymax=5)
        mbSpTrkGroup.defineHistogram('onlTrkEta,onlTrkPt', type='TH2F', title='Online tracks pT/eta correlation;#eta;p_{T} [GeV]', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=0, ymax=5)

        mbSpTrkGroup.defineHistogram('trkEta,trkPhi', cutmask='trkMask', type='TH2F', title='Offline selected tracks eta/phi correlation;#eta;#varphi', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=-math.pi, ymax=math.pi)
        mbSpTrkGroup.defineHistogram('onlTrkEta,onlTrkPhi', type='TH2F', title='Online tracks eta/phi correlation;#eta;#varphi', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=-math.pi, ymax=math.pi)

        mbSpTrkGroup.defineHistogram("nTrkOnline,nTrkOffline", type="TH2F", title=";N online tracks;N offline tracks", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)
        mbSpTrkGroup.defineHistogram("nTrkOfflineVtx,nTrkOffline", type="TH2F", title=";N offline tracks with Vtx cut;N offline tracks", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)
        mbSpTrkGroup.defineHistogram("nTrkOnline,zFinderWeight", type="TH2F", title=";N online tracks;ZFinder weight", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)
        mbSpTrkGroup.defineHistogram("nTrkOffline,zFinderWeight", type="TH2F", title=";N online tracks;ZFinder weight", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)
        mbSpTrkGroup.defineHistogram("nTrkOfflineVtx,zFinderWeight", type="TH2F", title=";N offline tracks with Vtx cut;ZFinder weight", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)
        mbSpTrkGroup.defineHistogram("nTrkOfflineVtx,nTrkOnlineVtx", type="TH2F", title=";N offline tracks with Vtx cut;N online tracks with Vtx cut;", xbins=nbins, xmin=0, xmax=nTrkMax, ybins=nbins, ymin=0, ymax=nTrkMax)

        # Ratios
        mbSpTrkGroup.defineHistogram("nTrkOnline", title="Number of tracks reconstructed online;track counts", xbins=200, xmin=-1, xmax=400)
        mbSpTrkGroup.defineHistogram("nTrkRatio", title="Number of tracks reconstructed online/offline;track counts online/offline", xbins=200, xmin=-1, xmax=4)
        mbSpTrkGroup.defineHistogram("trkSelOfflineRatio", title="Number of tracks reconstructed offline(selected)/offline; N sel/all", xbins=200, xmin=0.1, xmax=1.9)

    return monConfig.result()


if __name__ == "__main__":
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.DQ.Environment = "AOD"
    flags.Concurrency.NumConcurrentEvents = 5

    flags.Output.HISTFileName = "TestMonitorOutput.root"
    flags.fillFromArgs()
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    log.info("Input %s", str(flags.Input.Files))
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigSPTRK(flags, highGranularity=True)) # for local testing have very granular histograms

    # If you want to turn on more detailed messages ...
    #    from AthenaCommon.Constants import DEBUG
    #    cfg.getEventAlgo("HLTMBSPTRKMonAlg").OutputLevel = DEBUG
    cfg.printConfig(withDetails=True)  # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()  # use cfg.run(20) to only run on first 20 events
    # to run:
    # python -m TrigMinBiasMonitoring.TrigMinBiasMonitoringMT --filesInput=
