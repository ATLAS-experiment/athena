#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@file RPCMonitoringConfig.py
@brief Python configuration of RPC Monitoring for the Run III
'''

def RpcMonitoringConfig(inputFlags):

    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(inputFlags))
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'RpcMonitoringCfg')

    ######################################################################################################
    ## RpcTrackAnaAlgAlg
    ######################################################################################################
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(inputFlags))

    rpcTrackAnaAlg = helper.addAlgorithm(CompFactory.RpcTrackAnaAlg, "RpcTrackAnaAlgAlg", TrackExtrapolator = extrapolator)

    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge( TrackingGeometryCondAlgCfg(inputFlags ) )

    rpcTrackAnaAlg.plotMuonEff = True
    rpcTrackAnaAlg.plotPRD     = True
    rpcTrackAnaAlg.ElementsFileName = "Element.xml"
    rpcTrackAnaAlg.TagAndProbe         = False

    if not inputFlags.DQ.triggerDataAvailable:
        rpcTrackAnaAlg.MuonRoIContainerName = ''
    else:
        # LVL1MuonRoIs are only available after the HLTResultMTByteStreamDecoderAlg has executed
        from TrigDecisionTool.TrigDecisionToolConfig import getRun3NavigationContainerFromInput
        rpcTrackAnaAlg.ExtraInputs.add(('xAOD::TrigCompositeContainer' , 'StoreGateSvc+'+getRun3NavigationContainerFromInput(inputFlags)))

    ######################################################################################################
    ## Occupancy histograms
    ######################################################################################################
    myGroup_track = helper.addGroup(rpcTrackAnaAlg, 'RpcTrackAnaAlg', 'Muon/MuonRawDataMonitoring/RPC/')

    myGroup_track.defineHistogram('run;Run',
                            title='Run Number;run;Events',
                            type='TH1I', 
                            path='RpcOccupancy',
                            xbins=800000,xmin=200000.5,xmax=1000000.5)

    myGroup_track.defineHistogram('evtLB',
                        title='Number of Event;Luminosity Block;N Event',
                        type='TH1I', 
                        path='RpcOccupancy',
                        xbins=2000, xmin=0.5, xmax=2000.5)

    myGroup_track.defineHistogram('prdTime', 
                        title="Number of RPC Hit;Time;NHit",
                        type='TH1D', 
                        path='RpcOccupancy',
                        xbins=36, xmin=-101.5625, xmax=10.9375)

    myGroup_track.defineHistogram('prd_sec,prd_layer;NPRDHit_sectorVSlayer', 
                        title="NPRDHit_sectorVSlayer;Sector(- for C side, + for A side);layer((dbR-1)*2+gasGap);NHit",
                        type='TH2I', 
                        path='RpcOccupancy',
                        xbins=33, xmin=-16.5, xmax=16.5, 
                        ybins=8, ymin=0.5, ymax=8.5)
    myGroup_track.defineHistogram('prd_sec_1214,prd_layer_1214;NPRDHit_sectorVSlayer_Sector1214', 
                        title="NPRDHit_sectorVSlayer_Sector1214;Sector(- for C side, + for A side);layer((dbR-1)*2+gasGap);NHit",
                        type='TH2I', 
                        path='RpcOccupancy',
                        xbins=[-14.5,-13.5,-12.5,-11.5, 11.5, 12.5, 13.5, 14.5],
                        ybins=8, ymin=0.5, ymax=8.5)
    myGroup_track.defineHistogram('prd_sec_eta,prd_layer_eta;NPRDHit_sectorVSlayer_Eta', 
                        title="NPRDHit_sectorVSlayer_eta;Sector(- for C side, + for A side);layer((dbR-1)*2+gasGap);NHit",
                        type='TH2I', 
                        path='RpcOccupancy',
                        xbins=33, xmin=-16.5, xmax=16.5, 
                        ybins=8, ymin=0.5, ymax=8.5)
    myGroup_track.defineHistogram('prd_sec_phi,prd_layer_phi;NPRDHit_sectorVSlayer_Phi', 
                        title="NPRDHit_sectorVSlayer_phi;Sector(- for C side, + for A side);layer((dbR-1)*2+gasGap);NHit",
                        type='TH2I', 
                        path='RpcOccupancy',
                        xbins=33, xmin=-16.5, xmax=16.5, 
                        ybins=8, ymin=0.5, ymax=8.5)

    myGroup_track.defineHistogram('LB,panelInd;NPRDHit_Panels_All', 
                title='Number of RPC Hit;Luminosity Block;Panel Index;NHit',
                type='TH2I', 
                path='RpcOccupancy',
                xbins=2000, xmin=0.5, xmax=2000.5, ybins=8592, ymin=-0.5, ymax=8591.5)
    myGroup_track.defineHistogram('LB;NPRDHitVSLB_All', 
                title="Number of RPC Hit;Luminosity Block;NHit",
                type='TH1I', 
                path='RpcOccupancy',
                xbins=2000, xmin=0.5, xmax=2000.5)

    ######################################################################################################
    ## Rpc Track Analysis
    ######################################################################################################
    trackPath = 'TrackMatch'

    myGroup_track.defineHistogram('LB_nrpchit,PhiSector;NPRDHitFromMuon_PhiSector_vs_LB', 
                            title='Number of RPC hits for muons decayed from Z candidates;Luminosity Block;#phi sector(- for C side, + for A side);NHit',
                            type='TH2I', 
                            path=trackPath,
                            xbins=2000, xmin=0.5, xmax=2000.5, ybins=33, ymin=-16.5, ymax=16.5)

    ## probe muon with Z tag&probe method
    myGroup_track.defineHistogram('muPt_MuonFromZ;Pt_MuonFromZ',
                            title='Pt of muons decayed from Z candidates;Pt[MeV];NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=100,xmin=0,xmax=400e3)
    myGroup_track.defineHistogram('muEta_MuonFromZ;Eta_MuonFromZ',
                            title='#eta of muons decayed from Z candidates;#eta;NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=42, xmin=-1.05,  xmax=1.05)
    myGroup_track.defineHistogram('muPhi_MuonFromZ;Phi_MuonFromZ',
                            title='#phi of muons decayed from Z candidates;#phi;NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=32, xmin=-3.1415926, xmax=3.1415926)
    myGroup_track.defineHistogram('muEta_MuonFromZ,muPhi_MuonFromZ;EtaVSPhi_MuonFromZ', 
                            type='TH2D', 
                            title='Eta vs phi for muons decayed from Z;#eta;#phi;NMuon',
                            path='PlotCand',
                            xbins=42,xmin=-1.05,     xmax=1.05,
                            ybins=32,ymin=-3.1415926,ymax=3.1415926)
    myGroup_track.defineHistogram('muEta_p_MuonFromZ,muPhi_p_MuonFromZ;EtaVSPhi_MuonFromZ_plateau', 
                            type='TH2D', 
                            title='Eta vs phi for muons decayed from Z(pT>25GeV);#eta;#phi;NMuon',
                            path='PlotCand',
                            xbins=42,xmin=-1.05,     xmax=1.05,
                            ybins=32,ymin=-3.1415926,ymax=3.1415926)

    myGroup_track.defineHistogram('hitMulti_eta;HitMultiplicity_eta', 
                            type='TH1I', 
                            title='Hit multiplicity in #eta view for muons decayed from Z candidates;#eta strip hit Multiplicity;muon entries',
                            path=trackPath,
                            xbins=11,xmin=-0.5,   xmax=10.5)

    myGroup_track.defineHistogram('hitMulti_phi;HitMultiplicity_phi', 
                            type='TH1I', 
                            title='Hit multiplicity in #phi view for muons decayed from Z candidates;#phi strip hit Multiplicity;muon entries',
                            path=trackPath,
                            xbins=11,xmin=-0.5,   xmax=10.5)

    myGroup_track.defineHistogram('hitMulti,panelInd_hM;HitMultiplicity_Panels', 
                            title='Hit multiplicity for muons decayed from Z candidates;Hit Multiplicity;Panel Index;NMuon',
                            type='TH2I', 
                            path=trackPath,
                            xbins=11, xmin=-0.5, xmax=10.5, ybins=8592, ymin=-0.5, ymax=8591.5)

    myGroup_track.defineHistogram('clusterSize_eta;ClusterSize_etaView', 
                            type='TH1I', 
                            title='Cluster size in #eta view for muons decayed from Z candidates;Cluster size;NCluster',
                            path=trackPath,
                            xbins=11,xmin=-0.5,   xmax=10.5)

    myGroup_track.defineHistogram('clusterSize_phi;ClusterSize_phiView', 
                            type='TH1I', 
                            title='Cluster size in #phi view for muons decayed from Z candidates;Cluster size;NCluster',
                            path=trackPath,
                            xbins=11,xmin=-0.5,   xmax=10.5)

    myGroup_track.defineHistogram('clusterSize,panelInd_clust;ClusterSize_Panels', 
                            title='Cluster size for muons decayed from Z candidates;Cluster size;Panel Index;NCluster',
                            type='TH2I', 
                            path=trackPath,
                            xbins=11, xmin=-0.5, xmax=10.5, ybins=8592, ymin=-0.5, ymax=8591.5)

    myGroup_track.defineHistogram('muon_passExtrap,panelInd_hM;Detection_Efficiency_MuonFromZ', 
                            title='Panels detection efficiency for muons decayed from Z candidates;Panel Index;Efficiency',
                            type='TEfficiency',
                            path=trackPath,
                            xbins=8592, xmin=-0.5, xmax=8591.5)

    myGroup_track.defineHistogram('muon_passExtrap,LB_detEff;Detection_Efficiency_LB_MuonFromZ', 
                            title='Panels detection efficiency for muons decayed from Z candidates;Luminosity Block;Efficiency',
                            type='TEfficiency',
                            path=trackPath,
                            xbins=2000, xmin=0.5, xmax=2000.5)
                            
    myGroup_track.defineHistogram('isOutTime_prd,panelInd_prd;OuttimeHitFraction_PRDHit', 
                            title='Fraction of out-of-time hits for muons decayed from Z candidates;Panel Index;Fraction of out-of-time hits',
                            type='TEfficiency',
                            path=trackPath,
                            xbins=8592, xmin=-0.5, xmax=8591.5) 

    myGroup_track.defineHistogram('isOutTime_prd_onTrack,panelInd_prd_onTrack;OuttimeHitFraction_PRDHit_onTrack',
                            title='Fraction of out-of-time hits on tracks of muons decayed from Z candidates;Panel Index;Fraction of out-of-time hits',
                            type='TEfficiency',
                            path=trackPath,
                            xbins=8592, xmin=-0.5, xmax=8591.5)

    ## All muon
    myGroup_track.defineHistogram('muPt_allMu;Pt_AllMuons',
                            title='Pt of muons in all events;Pt[MeV];NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=100,xmin=0,xmax=400e3)
    myGroup_track.defineHistogram('muEta_allMu;Eta_AllMuons',
                            title='#eta of muons in all events;#eta;NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=42, xmin=-1.05,  xmax=1.05)
    myGroup_track.defineHistogram('muPhi_allMu;Phi_AllMuons',
                            title='#phi of muons in all events;#phi;NMuon',
                            type='TH1D',
                            path='PlotCand',
                            xbins=32, xmin=-3.1415926, xmax=3.1415926)
    myGroup_track.defineHistogram('muEta_allMu,muPhi_allMu;EtaVSPhi_AllMuons', 
                            type='TH2D', 
                            title='Eta vs phi of all muons;#eta;#phi;NMuon',
                            path='PlotCand',
                            xbins=42,xmin=-1.05,     xmax=1.05,
                            ybins=32,ymin=-3.1415926,ymax=3.1415926)


    myGroup_track.defineHistogram('muon_passExtrap_allMu,panelInd_hM_allMu;Detection_Efficiency_AllMuons', 
                            title='Panels detection efficiency for all muons;Panel Index;Efficiency',
                            type='TEfficiency',
                            path=trackPath,
                            xbins=8592, xmin=-0.5, xmax=8591.5)


    sectors       = ["sector"+str(k) for k in range(1, 16+1)]
    array_sectors = helper.addArray([sectors], rpcTrackAnaAlg, 'RpcTrackAnaAlg', 'Muon/MuonRawDataMonitoring/RPC/')

    array_sectors.defineHistogram('cs_sec;ClusterSize',
                title='Cluster size on {0};Cluster size;NCluster',
                type='TH1I',
                path='TrackMatch/ClusterSize',
                xbins=11, xmin=-0.5, xmax=10.5)

    array_sectors.defineHistogram('hitTime_sec;PRDHitTime_MuonFromZ',
                title='Hit time on {0};Hit time;NHits',
                type='TH1I',
                path='TrackMatch/PRDHitTime',
                xbins=36, xmin=-101.5625, xmax=10.9375)

    ######################################################################################################
    ## Rpc lv1 trigger
    ######################################################################################################
    myGroup_track.defineHistogram('nMu;NMuon',
                            title='Number of Muons;nMuons;Events',
                            type='TH1I',
                            path='PlotCand',
                            xbins=10,xmin=-0.5,xmax=9.5)
    myGroup_track.defineHistogram('nMuBarrel;NMuonBarrel',
                            title='Number of Barrel Muons;nMuons;Events',
                            type='TH1I',
                            path='PlotCand',
                            xbins=5,xmin=-0.5,xmax=4.5)
    myGroup_track.defineHistogram('roiEta;roiEta',
                            title='roi eta;roi #eta;rois',
                            type='TH1D',
                            path='PlotCand',
                            xbins=50,xmin=-2.5,xmax=2.5)

    myGroup_track.defineHistogram('roiBarrelEta;roiBarrelEta',
                            title='Barrel roi eta;roi #eta;rois',
                            type='TH1D',
                            path='PlotCand',
                            xbins=50,xmin=-2.5,xmax=2.5)

    myGroup_track.defineHistogram('roiBarrelThr;roiBarrelThrs',
                            title='Barrel roi threshold;roi threshold;rois',
                            type='TH1I',
                            path='PlotCand',
                            xbins=6,xmin=0.5,xmax=6.5)

    lv1Triggers = ["thr"+str(k) for k in range(1, 6+1)]
    array_triggerThr = helper.addArray([lv1Triggers], rpcTrackAnaAlg, 'RpcTrackAnaAlg', 'Muon/MuonRawDataMonitoring/RPC/')

    array_triggerThr.defineHistogram('passTrigger,muPt_l1;L1TriggerEff_muPt',
                title='L1 Trigger Efficiency for {0};Pt[MeV];#epsilon for {0}',
                type='TEfficiency',
                path='L1Trigger',
                xbins=10, xmin=0.0, xmax=80.0e3)

    array_triggerThr.defineHistogram('passTrigger_plateau,muEta_l1;L1TriggerEff_muEta',
                title='L1 Trigger Efficiency for {0};#eta;#epsilon for {0}',
                type='TEfficiency',
                path='L1Trigger',
                xbins=42,xmin=-1.05, xmax=1.05)

    array_triggerThr.defineHistogram('passTrigger_plateau,muPhi_l1;L1TriggerEff_muPhi',
                title='L1 Trigger Efficiency for {0};#phi;#epsilon for {0}',
                type='TEfficiency',
                path='L1Trigger',
                xbins=32,xmin=-3.1415926,xmax=3.1415926)

    array_triggerThr.defineHistogram('passTrigger_plateau,muEta_l1,muPhi_l1;L1TriggerEff_eta_phi',
                title='L1 Trigger Efficiency for {0};#eta;#phi;#epsilon for {0}',
                type='TEfficiency',
                path='L1Trigger',
                xbins=42,xmin=-1.05,     xmax=1.05,
                ybins=32,ymin=-3.1415926,ymax=3.1415926)

    result.merge(helper.result())
    print(" RpcMonitorAlgorithm END !")

    return result


if __name__=="__main__":
    print(" In RpcMonitorAlgorithm !")
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Config Input/Output 
    import os

    file_list = []
    if os.path.exists('input.txt'):
        infile = open('input.txt', "r")
        file_list = infile.readlines()
        file_list = [ filename.strip() for filename in file_list ]
        print ("read files path from input.txt .")
        print ("files paths: \n", file_list)
    else:
        file_list = ['/eos/atlas/atlascerngroupdisk/det-rpc/data/DESDM_MCP/data18_13TeV.00358615.physics_Main.merge.DESDM_MCP.f961_m2024/data18_13TeV.00358615.physics_Main.merge.DESDM_MCP.f961_m2024._0005.1']

        print ("file input.txt does not exist")
        print ("WIll use files: \n", file_list)

    flags.Input.Files = file_list
    flags.Output.HISTFileName = 'RPCMonitoringOutput.root'
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

    flags.lock()
    flags.dump()

    # Initialize configuration object, add accumulator, merge and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    acc = RpcMonitoringConfig(flags)
    acc.OutputLevel = INFO
    cfg.merge(acc)

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    cfg.merge(AtlasFieldCacheCondAlgCfg(flags))

    if flags.DQ.Steering.Muon.doTrackMon:
        # do not run in RAW->ESD
        if flags.DQ.Environment not in ('tier0Raw',):
            from MuonTrackMonitoring.MuonTrackMonitorAlgorithm import MuonTrackConfig
            cfg.merge(MuonTrackConfig(flags))

    cfg.printConfig(withDetails=True, summariseProps = True)

    cfg.run()
