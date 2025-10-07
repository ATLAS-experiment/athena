#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 


# actual code to configure al;l the different algorithm instances for 
# the different slices 


def TrigIDR4Monitoring( flags=None, name=None, monlevel=None ) :  

        tools = []

        from AthenaCommon.Logging import logging
        log = logging.getLogger("TIDAMonitoring")

        log.info( "Creating  TIDA monitoring: " + name )

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

        print( "  R4Mon: toolkey: ", toolkey )                
        print( "  R4Mon:     key: ",     key )                
                        
        # each signature has it's own function now so it makes it easier to disable 
        # any of them is need be

#        TIDR4MonElectron(    flags, key, toolkey, tools, monlevel )
#        TIDR4MonElectronLRT( flags, key, toolkey, tools, monlevel )
        TIDR4MonMuon(        flags, key, toolkey+name, tools, monlevel )
#        TIDR4MonMuonLRT(    flags, key, toolkey, tools, monlevel )
#        TIDR4MonTau(      flags, key, toolkey, tools, monlevel )
#        TIDR4MonBjet(     flags, key, toolkey, tools, monlevel )
#        TIDR4MonMinbias(  flags, key, toolkey, tools, monlevel )
#        TIDR4MonCosmic(   flags, key, toolkey, tools, monlevel )
#        TIDR4MonBphys(    flags, key, toolkey, tools, monlevel )
#        TIDR4MonUtt(      flags, key, toolkey, tools, monlevel )

        return tools


# could we maybe eventually replace by a single creator function, as below, if we can find a way to
# pass in all the custom variables, such as d0 etc, with out resorting to an egregious kwargs pattern
# like this ...

# TIDAsignature( flags, "Egamma", key, toolkey, tools, monlevel, 
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
               

def TrigIDR4Monsignature( flags, signature, key, toolkey, tools, monlevel, chain_regex ) :

        tida = TrigIDR4Mon_builder( flags, name = "ID"+signature+toolkey+"Tool", useHighestPT=True )
        tida.SliceTag       = "HLT/TRIDT/"+signature+"/"+key

        tida.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, chain_regex, monlevel )
                            

        if  len(chains)>0 : 

                tida.ntupleChainNames  = chains

                tida.MonTools = createMonTools( flags, tida.SliceTag, chains )

                tools += [ tida ]







def TIDR4MonElectron( flags, key, toolkey, tools, monlevel ) :
        
        #### Egamma ####

        tidaegamma = TrigIDR4Mon_builder( flags, name = "IDEgamma"+toolkey+"Tool", useHighestPT=True )
        tidaegamma.SliceTag       = "HLT/TRIDT/Egamma/"+key

        tidaegamma.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",  
                              "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*).*:key=HLT_IDTrack_Electron_IDTrig",
                              "HLT_e.(?!.*lrtloose.*).*idperf(?!.*lrtloose.*)(?!.*nogsf.*).*:key=HLT_IDTrack_Electron_GSF",                              
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


                
def TIDR4MonElectronLRT( flags, key, toolkey, tools, monlevel ) :


        #### LRT Egamma ####
        
        tidaegammalrt = TrigIDR4Mon_builder( flags, name = "IDEgammaLRT"+toolkey+"Tool", useHighestPT=True )
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
                


                
def TIDR4MonMuon( flags, key, toolkey, tools, monlevel ) :
                        
        #### muon ####


        name = "IDMuon"+toolkey+"Tool"

        print( "  name: ", name )
        print( "  key:  ", key )

               
        tidamuon = TrigIDR4Mon_builder( flags, name = name, useHighestPT=True )
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


def TIDR4MonMuonLRT( flags, key, toolkey, tools, monlevel ) :

        #### muon LRT ####

        tidamuonlrt = TrigIDR4Mon_builder( flags, name = "IDMuonLRT"+toolkey+"Tool", useHighestPT=True )
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
 




def TIDR4MonTau( flags, key, toolkey, tools, monlevel ) :

        #### tau ####

        tidatau = TrigIDR4Mon_builder( flags, name = "IDTau"+toolkey+"Tool", useHighestPT=True )
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

        tidataulrt = TrigIDR4Mon_builder( flags, name = "IDTauLRT"+toolkey+"Tool", useHighestPT=True )
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





def TIDR4MonBjet( flags, key, toolkey, tools, monlevel ) :

        #### bjets ####

        tidabjet = TrigIDR4Mon_builder( flags, name = "IDBjet"+toolkey+"Tool" )
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
                              "HLT_j.*boffperf.*_ftf.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS"
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





def TIDR4MonMinbias( flags, key, toolkey, tools, monlevel ) :

        ####### minbias ####

        tidaminbias = TrigIDR4Mon_builder( flags, name = "IDMinbias"+toolkey+"Tool" )
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




def TIDR4MonCosmic( flags, key, toolkey, tools, monlevel ) :

        #########  cosmic  ####

        tidacosmic = TrigIDR4Mon_builder( flags, name = "IDCosmic"+toolkey+"Tool" )
        tidacosmic.SliceTag = "HLT/TRIDT/Cosmic/"+key

        tidacosmic.AnalysisConfig = "Tier0"
        
        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_.*cosmic.*:key=HLT_IDTrack_Cosmic_IDTrig" ], monlevel )

        if len(chains)>0 : 
                        
                tidacosmic.ntupleChainNames += chains

                tidacosmic.MonTools = createMonTools( flags,  tidacosmic.SliceTag, chains )
                
                tools += [ tidacosmic ]





def TIDR4MonBphys( flags, key, toolkey, tools, monlevel ) :

        #### bphysics #### - note pdgID=531 for B_s (can we use a list of pdgID?)

        tidabphysics = TrigIDR4Mon_builder( flags, name = "IDBphysics"+toolkey+"Tool", useHighestPT=True )
        tidabphysics.SliceTag = "HLT/TRIDT/Bphysics/"+key

        tidabphysics.AnalysisConfig = "Tier0"

        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_mu.*_bBmumux_BsmumuPhi.*:key=HLT_IDTrack_Bmumux_FTF",
                              "HLT_mu.*_bBmumux_BsmumuPhi.*:key=HLT_IDTrack_Bmumux_IDTrig",
                              "HLT_mu.*_bBmumux_Bidperf.*:key=HLT_IDTrack_Bmumux_FTF",
                              "HLT_mu.*_bBmumux_Bidperf.*:key=HLT_IDTrack_Bmumux_IDTrig"], monlevel )

        if len(chains)>0 :

                tidabphysics.ntupleChainNames += chains

                tidabphysics.MonTools = createMonTools( flags,  tidabphysics.SliceTag, chains )

                tools += [ tidabphysics ]




def TIDR4MonUtt( flags, key, toolkey, tools, monlevel ) :

        #### UTT ####

        tidautt = TrigIDR4Mon_builder( flags, name = "IDUtt"+toolkey+"Tool" )
        tidautt.SliceTag = "HLT/TRIDT/UTT/"+key

        tidautt.AnalysisConfig = "Tier0"
        
        from TrigInDetMonitoring.TIDAChains import getchains
        
        chains = getchains( flags, 
                            [ "HLT_j180_.*dispjet.*_L1J100:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ", "HLT_j180_.*dispjet.*_L1jJ160:key=HLT_IDTrack_DJLRT_FTF:roi=HLT_Roi_DJ" ], monlevel )

        if len(chains)>0 : 
                        
                tidautt.ntupleChainNames += chains

                tidautt.MonTools = createMonTools( flags,  tidautt.SliceTag, chains )
                
                tools += [ tidautt ]







# get this from the TIDAMonitoring

# # create a separate specific monTool for each analysis chain
# # - simplifies the overall analysis configuration

# def createMonTools( flags, label, chains, excludeTagChains=True ):
#         tools = []
#         from TrigInDetAnalysisExample.chainString import chainString
#         from TrigInDetAnalysisExample.TIDAMonTool import createMonTool
#         for mt in chains :
#                 if excludeTagChains and "tag" in chainString(mt).extra:
#                         continue
#                 tool = createMonTool( flags, label, mt )
#                 tools += [ tool ]
#         return tools

from TrigInDetMonitoring.TIDAMonitoring import createMonTools


# create the actual algorithm - calling with this wrapper lets us use the same 
# code for the old, or new configuration 
def TrigIDR4Mon_builder( flags=None, name="NoName", useHighestPT=False, pdgID=0 ):

        if flags is None:
                from TrigIDR4Monitoring.TrigIDR4MonitoringConf import TrigIDR4Mon
                alg = TrigIDR4Mon( name = name )
        else:
                from AthenaConfiguration.ComponentFactory import CompFactory
                alg =  CompFactory.TrigIDR4Mon( name=name )

        alg.UseHighestPT = useHighestPT

        return alg




# wrapper function for the central monitoring configuration 

def TrigIDR4MonConfig( flags, monlevels=None ):
        return TrigIDR4MonitoringCA( flags, monlevels )



# component accumulator wrapper around the overall monitoring functiom                

def TrigIDR4MonitoringCA( flags, monlevels=None ):

        from AthenaMonitoring import AthMonitorCfgHelper
        monConfig = AthMonitorCfgHelper(flags, "TrigIDR4Mon")

        print( "monlevel: ", monlevels )
        
        algs  = TrigIDR4Monitoring(flags, "Tier0", monlevel="idMon:t0:shifter" )
        algs += TrigIDR4Monitoring(flags, "Shifter", monlevel="idMon:shifter" ) 

        if flags.Input.isMC:    
            algs += TrigIDR4Monitoring( flags, name="PhysVal", monlevel="idMon:t0" )
            algs += TrigIDR4Monitoring( flags, name="PhysValShifter", monlevel="idMon:shifter" )
        

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
    flags.Input.isMC  = False

    flags.Output.HISTFileName = 'duff.root'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)

    cfg.merge( PoolReadCfg(flags) )

    cfg.merge( histsvc(flags) )

    cfg.merge( TrigIDR4MonConfig( flags ) ) 

    # If you want to turn on more detailed messages ...
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    Nevents = 10
    cfg.run(Nevents)



