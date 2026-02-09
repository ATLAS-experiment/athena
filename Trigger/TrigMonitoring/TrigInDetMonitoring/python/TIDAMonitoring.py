#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 


# actual code to configure al;l the different algorithm instances for 
# the different slices 


def TIDAMonitoring( flags=None, name=None, monlevel=None, mcTruth=False ) :  

        tools = []

        from AthenaCommon.Logging import logging
        log = logging.getLogger("TIDAMonitoring")

        log.info( "Creating  TIDA monitoring: " + name )
        log.info( "                  mcTruth: " + str(mcTruth) )

        key     = "All"
        toolkey = ""

        if monlevel is not None:
                log.info( "TIDA monitoring not None: monlevel: " + monlevel )        
                if "t0" in monlevel:
                        key     = "Expert"
                        toolkey = "Expert"
                elif "shifter" in monlevel:
                        key     = "Shifter"
                        toolkey = "Shifter"

        # each signature has it's own function now so it makes it easier to disable 
        # any of them is need be

        TIDAelectron( flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAmuon(     flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAtau(      flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAbjet(     flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAminbias(  flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAcosmic(   flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAbphys(    flags, key, toolkey, tools, monlevel, mcTruth )
        TIDAutt(      flags, key, toolkey, tools, monlevel, mcTruth )

        return tools


# could we maybe eventually replace by a single creator function, as below, if we can find a way to
# pass in all the custom variables, such as d0 etc, with out resorting to an egregious kwargs pattern
# like this ...

# TIDAsignature( flags, "Egamma", key, toolkey, tools, monlevel, mcTruth, 
#               [ "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",  
#                 "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_IDTrig",
#                 "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*)(?!.*nogsf.*).*:key=HLT_IDTrack_Electron_GSF",                              
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron:te=1",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:extra=el_tag:roi=HLT_Roi_FastElectron:te=0",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:extra=el_probe:roi=HLT_Roi_FastElectron:te=1",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:te=1",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:te=1",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:extra=el_tag:te=0",
#                 "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:extra=el_probe:te=1" ] ) 
               

def TIDAsignature( flags, signature, key, toolkey, tools, monlevel, mcTruth, chain_regex ) :

        if mcTruth: 
                tida = TrigR3Mon_builder( flags, name = "ID"+signature+"Truth"+toolkey+"Tool", mcTruth=True, pdgID=11 )
                tida.SliceTag       = "HLT/TRIDT/"+signature+"Truth/"+key
        else:
                tida = TrigR3Mon_builder( flags, name = "ID"+signature+toolkey+"Tool", useHighestPT=True )
                tida.SliceTag       = "HLT/TRIDT/"+signature+"/"+key

        tida.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, chain_regex, monlevel )
                            

        if  len(chains)>0 : 

                tida.ntupleChainNames  = chains

                tida.MonTools = createMonTools( flags, tida.SliceTag, chains )

                tools += [ tida ]







def TIDAelectron( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #### Egamma ####

        if mcTruth: 
                tidaegamma = TrigR3Mon_builder( flags, name = "IDEgammaTruth"+toolkey+"Tool", mcTruth=True, pdgID=11 )
                tidaegamma.SliceTag       = "HLT/TRIDT/EgammaTruth/"+key
        else:
                tidaegamma = TrigR3Mon_builder( flags, name = "IDEgamma"+toolkey+"Tool", useHighestPT=True )
                tidaegamma.SliceTag       = "HLT/TRIDT/Egamma/"+key

        tidaegamma.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",  
                              "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_IDTrig",
                              "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*)(?!.*nogsf.*).*:key=HLT_IDTrack_Electron_GSF",
                              "HLT_e.*idperf.*ion.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",                                
                              "HLT_e.*idperf.*ion.*:key=HLT_IDTrack_Electron_FTF",
                              "HLT_e.*idperf.*ion.*:key=HLT_IDTrack_Electron_IDTrig",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron:te=1",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:extra=el_tag:roi=HLT_Roi_FastElectron:te=0",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_FTF:extra=el_probe:roi=HLT_Roi_FastElectron:te=1",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:te=1",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_tag:te=0",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_nogsf_probe_.*inv.*:key=HLT_IDTrack_Electron_IDTrig:extra=el_probe:te=1",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:te=1",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:extra=el_tag:te=0",
                              "HLT_e.*_lhtight.*_e.*_idperf_tight_probe_.*inv.*:key=HLT_IDTrack_Electron_GSF:extra=el_probe:te=1" ], monlevel )

        if  len(chains)>0 : 

                tidaegamma.ntupleChainNames  = chains

                tidaegamma.MonTools = createMonTools( flags, tidaegamma.SliceTag, chains )

                tools += [ tidaegamma ]



        #### LRT Egamma ####
        
        if mcTruth: 
                tidaegammalrt = TrigR3Mon_builder( flags, name = "IDEgammaLRTTruth"+toolkey+"Tool", mcTruth=True, pdgID=11 )
                tidaegammalrt.SliceTag       = "HLT/TRIDT/EgammaLRTTruth/"+key
        else:
                tidaegammalrt = TrigR3Mon_builder( flags, name = "IDEgammaLRT"+toolkey+"Tool", useHighestPT=True )
                tidaegammalrt.SliceTag       = "HLT/TRIDT/EgammaLRT/"+key
                                  
        tidaegammalrt.AnalysisConfig = "Tier0"
        tidaegammalrt.mind0CutOffline = 2.

        from TrigInDetMonitoring.TIDAChains import getchains

        chains = getchains( flags, 
                            [ "HLT_e.*idperf_loose_lrtloose.*:key=HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_LRT",
                              "HLT_e.*idperf_loose_lrtloose.*:key=HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT",
                              "HLT_e.*lrtloose_idperf.*:key=HLT_IDTrack_ElecLRT_FTF:roi=HLT_Roi_FastElectron_FTF",
                              "HLT_e.*lrtloose_idperf.*:key=HLT_IDTrack_ElecLRT_IDTrig:roi=HLT_Roi_FastElectron_LRT",
                            ], monlevel )
        
        if len(chains)>0 : 

                tidaegammalrt.ntupleChainNames  = chains
                tidaegammalrt.ntupleChainNames += [ "Offline", "Offline:+InDetLargeD0TrackParticles" ]

                tidaegammalrt.MonTools = createMonTools( flags, tidaegammalrt.SliceTag, chains )
        
                tools += [ tidaegammalrt ]
                


                
def TIDAmuon( flags, key, toolkey, tools, monlevel, mcTruth ) :
                        
        #### muon ####

        if mcTruth: 
                tidamuon = TrigR3Mon_builder( flags, name = "IDMuonTruth"+toolkey+"Tool", mcTruth=True, pdgID=13 )
                tidamuon.SliceTag = "HLT/TRIDT/MuonTruth/"+key
        else:
                tidamuon = TrigR3Mon_builder( flags, name = "IDMuon"+toolkey+"Tool", useHighestPT=True )
                tidamuon.SliceTag = "HLT/TRIDT/Muon/"+key

        tidamuon.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_mu(?!.*LRT.*)(?!.*tau.*).*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu(?!.*LRT.*)(?!.*tau.*).*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu.*ivarperf.*:key=HLT_IDTrack_MuonIso_FTF:roi=HLT_Roi_MuonIso",
                              "HLT_mu.*ivarperf.*:key=HLT_IDTrack_MuonIso_IDTrig:roi=HLT_Roi_MuonIso",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:te=1",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:te=1",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
                              "HLT_mu.*_mu.*idtp.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:te=1",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:te=1",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_tag:te=0",
                              "HLT_mu.*_mu.*_idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=mu_probe:te=1" ], monlevel )
                              
        if len(chains)>0 : 

                tidamuon.ntupleChainNames = chains
        
                tidamuon.MonTools = createMonTools( flags,  tidamuon.SliceTag, chains )
        
                tools += [ tidamuon ]



        #### muon LRT ####

        if mcTruth:
                tidamuonlrt = TrigR3Mon_builder(flags, name = "IDMuonLRTTruth"+toolkey+"Tool", mcTruth=True, pdgID=13 )
                tidamuonlrt.SliceTag = "HLT/TRIDT/MuonLRTTruth/"+key
        else:
                tidamuonlrt = TrigR3Mon_builder( flags, name = "IDMuonLRT"+toolkey+"Tool", useHighestPT=True )
                tidamuonlrt.SliceTag = "HLT/TRIDT/MuonLRT/"+key

        tidamuonlrt.AnalysisConfig = "Tier0"
        tidamuonlrt.mind0CutOffline = 2.

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_mu.*_LRT_idperf.*:key=HLT_IDTrack_MuonLRT_FTF:roi=HLT_Roi_L2SAMuon_LRT",
                              "HLT_mu.*_LRT_idperf.*:key=HLT_IDTrack_MuonLRT_IDTrig:roi=HLT_Roi_L2SAMuon_LRT"], monlevel )

        if len(chains)>0 :

                tidamuonlrt.ntupleChainNames  = chains
                tidamuonlrt.ntupleChainNames += [ "Offline", "Offline:+InDetLargeD0TrackParticles" ]

                tidamuonlrt.MonTools = createMonTools( flags, tidamuonlrt.SliceTag, chains )
                
                tools += [ tidamuonlrt ]
 




def TIDAtau( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #### tau ####

        if mcTruth: 
                tidatau = TrigR3Mon_builder( flags, name = "IDTauTruth"+toolkey+"Tool", mcTruth=True, pdgID=15 )
                tidatau.SliceTag = "HLT/TRIDT/TauTruth/"+key
        else:
                tidatau = TrigR3Mon_builder( flags, name = "IDTau"+toolkey+"Tool", useHighestPT=True )
                tidatau.SliceTag = "HLT/TRIDT/Tau/"+key

        tidatau.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
                              "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
                              "HLT_tau.*idperf.*tracktwoMVA_.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau1_tag:te=0",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore:extra=tau1_probe:te=1",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon:extra=tau0_tag:te=0",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso:extra=tau0_probe:te=1",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon:extra=tau_tag:te=0",
                              "HLT_mu.*tau.*idperf.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso:extra=tau_probe:te=1" ],  monlevel )

        if len(chains)>0 : 

                tidatau.ntupleChainNames = chains
        
                tidatau.MonTools = createMonTools( flags,  tidatau.SliceTag, chains )
                
                tools += [ tidatau ]


        #### tau LRT ####

        if mcTruth:
                tidataulrt = TrigR3Mon_builder(flags, name = "IDTauLRTTruth"+toolkey+"Tool", mcTruth=True, pdgID=15 )
                tidataulrt.SliceTag = "HLT/TRIDT/TauLRTTruth/"+key
        else:
                tidataulrt = TrigR3Mon_builder( flags, name = "IDTauLRT"+toolkey+"Tool", useHighestPT=True )
                tidataulrt.SliceTag = "HLT/TRIDT/TauLRT/"+key

        tidataulrt.AnalysisConfig = "Tier0"
        tidataulrt.mind0CutOffline = 2.

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_tau.*_idperf.*_trackLRT.*:key=HLT_IDTrack_TauLRT_FTF:roi=HLT_Roi_LRT",
                              "HLT_tau.*_idperf.*_trackLRT.*:key=HLT_IDTrack_TauLRT_IDTrig:roi=HLT_Roi_TauLRT"], monlevel )

        if len(chains)>0 :

                tidataulrt.ntupleChainNames  = chains
                tidataulrt.ntupleChainNames += [ "Offline", "Offline:+InDetLargeD0TrackParticles" ]

                tidataulrt.MonTools = createMonTools( flags, tidataulrt.SliceTag, chains )

                tools += [ tidataulrt ]





def TIDAbjet( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #### bjets ####

        if mcTruth:
                tidabjet = TrigR3Mon_builder( flags, name = "IDBjetTruth"+toolkey+"Tool", mcTruth=True )
                tidabjet.SliceTag = "HLT/TRIDT/BjetTruth/"+key
        else:
                tidabjet = TrigR3Mon_builder( flags, name = "IDBjet"+toolkey+"Tool" )
                tidabjet.SliceTag = "HLT/TRIDT/Bjet/"+key

        tidabjet.AnalysisConfig = "Tier0"
        
        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_j.*presel.*b.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
                              "HLT_j.*roiftf.*:key=HLT_IDTrack_JetSuper_FTF:roi=HLT_Roi_JetSuper:vtx=HLT_IDVertex_JetSuper",
                              "HLT_j.*presel.*b.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                              "HLT_j.*boffperf.*:key=HLT_IDTrack_Bjet_FTF:roi=HLT_Roi_Bjet",
                              "HLT_j.*boffperf.*:key=HLT_IDTrack_Bjet_IDTrig:roi=HLT_Roi_Bjet", 
                              "HLT_j45_pf_ftf_preselj20_L1J15:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                              "HLT_j.*_ftf.*boffperf.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                              "HLT_j.*boffperf.*_ftf.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                              "HLT_.*_j.*ion.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                              "HLT_.*_j.*ion.*:key=HLT_IDTrack_Bjet_FTF:roi=HLT_Roi_Bjet",
                              "HLT_.*_j.*ion.*:key=HLT_IDTrack_Bjet_IDTrig:roi=HLT_Roi_Bjet" 
                            ], monlevel )

        # if we find no chains, try to get more generic chains

        if len(chains)==0 : 
                chains = getchains( flags, 
                            [ 
#                             "HLT_j(?!0).*:key=HLT_IDTrack_Bjet_FTF",
#                             "HLT_j(?!0).*:key=HLT_IDTrack_Bjet_IDTrig", 
                              "HLT_j(?!0).*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
                            ], None )
        

        if len(chains)>0 : 
                        
                tidabjet.ntupleChainNames += chains

                tidabjet.MonTools = createMonTools( flags,  tidabjet.SliceTag, chains )
                
                tools += [ tidabjet ]





def TIDAminbias( flags, key, toolkey, tools, monlevel, mcTruth ) :

        ####### minbias ####

        if mcTruth:
                tidaminbias = TrigR3Mon_builder( flags, name = "IDMinbiasTruth"+toolkey+"Tool", mcTruth=True )
                tidaminbias.SliceTag = "HLT/TRIDT/MinbiasTruth/"+key
        else:
                tidaminbias = TrigR3Mon_builder( flags, name = "IDMinbias"+toolkey+"Tool" )
                tidaminbias.SliceTag = "HLT/TRIDT/Minbias/"+key

        tidaminbias.AnalysisConfig = "Tier0"
        tidaminbias.z0CutOffline = 120
        tidaminbias.pTCutOffline = 200

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_mb_sptrk.*:key=HLT_IDTrack_MinBias_IDTrig",
                              "HLT_mb_.*pix.*:key=HLT_IDTrack_MinBiasPixel_IDTrig" ], monlevel )

        if len(chains)>0 : 
                        
                tidaminbias.ntupleChainNames += chains

                tidaminbias.MonTools = createMonTools( flags,  tidaminbias.SliceTag, chains )
                
                tools += [ tidaminbias ]




def TIDAcosmic( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #########  cosmic  ####

        if mcTruth:
                tidacosmic = TrigR3Mon_builder( flags, name = "CosmicTruth"+toolkey+"Tool", mcTruth=True )
                tidacosmic.SliceTag = "HLT/TRIDT/CosmicTruth/"+key
        else:
                tidacosmic = TrigR3Mon_builder( flags, name = "IDCosmic"+toolkey+"Tool" )
                tidacosmic.SliceTag = "HLT/TRIDT/Cosmic/"+key

        tidacosmic.AnalysisConfig = "Tier0"
        
        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_.*cosmic.*:key=HLT_IDTrack_Cosmic_IDTrig" ], monlevel )

        if len(chains)>0 : 
                        
                tidacosmic.ntupleChainNames += chains

                tidacosmic.MonTools = createMonTools( flags,  tidacosmic.SliceTag, chains )
                
                tools += [ tidacosmic ]





def TIDAbphys( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #### bphysics #### - note pdgID=531 for B_s (can we use a list of pdgID?)

        if mcTruth:
                tidabphysics = TrigR3Mon_builder( flags, name = "IDBphysicsTruth"+toolkey+"Tool", mcTruth=True, pdgID=531 )
                tidabphysics.SliceTag = "HLT/TRIDT/BphysicsTruth/"+key
        else:
                tidabphysics = TrigR3Mon_builder( flags, name = "IDBphysics"+toolkey+"Tool", useHighestPT=True )
                tidabphysics.SliceTag = "HLT/TRIDT/Bphysics/"+key

        tidabphysics.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_mu.*_bBmumux_BsmumuPhi.*:key=HLT_IDTrack_Bmumux_FTF",
                              "HLT_mu.*_bBmumux_BsmumuPhi.*:key=HLT_IDTrack_Bmumux_IDTrig",
                              "HLT_mu.*_bBmumux_Bidperf.*:key=HLT_IDTrack_Bmumux_FTF",
                              "HLT_mu.*_bBmumux_Bidperf.*:key=HLT_IDTrack_Bmumux_IDTrig",
                              "HLT_.*Bhh.*:key=HLT_IDTrack_Bhh_FTF:roi=HLT_Roi_Bhh"], monlevel )

        if len(chains)>0 :

                tidabphysics.ntupleChainNames += chains

                tidabphysics.MonTools = createMonTools( flags,  tidabphysics.SliceTag, chains )

                tools += [ tidabphysics ]




def TIDAutt( flags, key, toolkey, tools, monlevel, mcTruth ) :

        #### UTT ####

        if mcTruth:
                tidautt = TrigR3Mon_builder( flags, name = "IDUttTruth"+toolkey+"Tool", mcTruth=True )
                tidautt.SliceTag = "HLT/TRIDT/UTT/"+key
        else:
                tidautt = TrigR3Mon_builder( flags, name = "IDUtt"+toolkey+"Tool" )
                tidautt.SliceTag = "HLT/TRIDT/UTT/"+key

        tidautt.AnalysisConfig = "Tier0"
        
        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_j180_.*dispjet.*_L1J100:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ", "HLT_j180_.*dispjet.*_L1jJ160:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ" ], monlevel )

        if len(chains)>0 : 
                        
                tidautt.ntupleChainNames += chains

                tidautt.MonTools = createMonTools( flags,  tidautt.SliceTag, chains )
                
                tools += [ tidautt ]








# create a separate specific monTool for each analysis chain
# - simplifies the overall analysis configuration

def createMonTools( flags, label, chains, excludeTagChains=True ):
        tools = []
        from TrigInDetAnalysisExample.chainString import chainString
        from TrigInDetAnalysisExample.TIDAMonTool import createMonTool
        for mt in chains :
                if excludeTagChains and "tag" in chainString(mt).extra:
                        continue
                tool = createMonTool( flags, label, mt )
                tools += [ tool ]
        return tools
        

# create the actual algorithm - calling with this wrapper lets us use the same 
# code for the old, or new configuration 
def TrigR3Mon_builder( flags=None, name="NoName", useHighestPT=False, mcTruth=False, pdgID=0 ):

        if flags is None:
                from TrigInDetAnalysisExample.TrigInDetAnalysisExampleConf import TrigR3Mon
                alg = TrigR3Mon( name = name )
        else:
                from AthenaConfiguration.ComponentFactory import CompFactory
                alg =  CompFactory.TrigR3Mon( name=name )

        alg.UseHighestPT = useHighestPT
        alg.mcTruth      = False

        if mcTruth :
                alg.mcTruth = True
                alg.pixHitsOffline = -1
                alg.sctHitsOffline = -1
                alg.siHitsOffline  = -1

                if pdgID != 0 :
                        if pdgID == 15 :
                                alg.SelectParentTruthPdgId = 15
                        else :
                                alg.SelectTruthPdgId = pdgID
                        
        return alg




# wrapper function for the central monitoring configuration 

def TrigInDetMonConfig( flags, monlevels=None ):
        return TIDAMonitoringCA( flags, monlevels )



# component accumulator wrapper around the overall monitoring functiom                

def TIDAMonitoringCA( flags, monlevels=None ):

        from AthenaMonitoring import AthMonitorCfgHelper
        monConfig = AthMonitorCfgHelper(flags, "TrigIDMon")

        # algs = TIDAMonitoring(flags, "All" )
        algs = TIDAMonitoring(flags, "Tier0", monlevel="idMon:t0:shifter" )
        algs += TIDAMonitoring(flags, "Shifter", monlevel="idMon:shifter", mcTruth=False ) 
      
        if flags.Input.isMC:    
            algs += TIDAMonitoring(flags, name="PhysVal", monlevel="idMon:t0", mcTruth=True )
            algs += TIDAMonitoring(flags, name="PhysValShifter", monlevel="idMon:shifter", mcTruth=True )

        for a in algs:
                monConfig.addAlgorithm(a)

        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
        ca = ComponentAccumulator()
        ca.merge(monConfig.result())
        return ca






def histsvc( flags ):

    from AthenaCommon.Logging import log

    if flags.Output.HISTFileName: 
            log.info( "histsvc: Create THistSvc with file name: "+flags.Output.HISTFileName )
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    ca = ComponentAccumulator()
    
    from AthenaConfiguration.ComponentFactory import CompFactory
    THistSvc = CompFactory.THistSvc
        
    histsvc = THistSvc()
    if flags.Output.HISTFileName:
        histsvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % (flags.DQ.FileKey, flags.Output.HISTFileName)]
        log.info( "histsvc: "+histsvc.Output[-1] )
            
    ca.addService(histsvc)
       
    return ca
            


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    log.info( "test running" )

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    # Input files
    # AOD file to be run w/ MT access and Mon Groups implemented
    file = 'AOD.pool.root'

    flags.Input.Files = [file]
    flags.Input.isMC  = True

    flags.Output.HISTFileName = 'duff.root'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)

    cfg.merge( PoolReadCfg(flags) )

    cfg.merge( histsvc(flags) )

    cfg.merge( TrigInDetMonConfig( flags ) ) 

    # If you want to turn on more detailed messages ...
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    Nevents = 10
    cfg.run(Nevents)



