#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JetEfficiencyMonitoringConfig(flags):
    '''Function to configure LVL1 JetEfficiency algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = ComponentAccumulator()


    from AthenaConfiguration.Enums import Format
    ###########################################################################
    # Jet and particle flow config required for data POOL files except if is already an AOD
    if flags.Input.Format is Format.POOL and not flags.Input.isMC and not any(["AOD" in a for a in flags.Input.ProcessingTags]):
        from JetRecConfig.JetRecConfig import JetRecCfg
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
        from JetRecConfig.JetConfigFlags import jetInternalFlags
        jetInternalFlags.isRecoJob = True
        result.merge( JetRecCfg(flags,AntiKt4EMPFlow) )
        
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        if flags.DQ.Environment == "AOD":
          result.merge(PFGlobalFlowElementLinkingCfg(flags, useMuonTopoClusters=True))
        else:
          result.merge(PFGlobalFlowElementLinkingCfg(flags))
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        result.merge(PFGlobalFlowElementLinkingCfg(flags))
        from METReconstruction.METAssociatorCfg import METAssociatorCfg
        result.merge(METAssociatorCfg(flags, 'AntiKt4EMPFlow'))
        from METUtilities.METMakerConfig import getMETMakerAlg
        metCA=ComponentAccumulator()
        metCA.addEventAlgo(getMETMakerAlg('AntiKt4EMPFlow'))
        result.merge(metCA)
    ###########################################################################
    
    # make the athena monitoring helper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JetEfficiencyMonitorAlgorithm,'JetEfficiencyMonAlg')
    groupName = 'JetEfficiencyMonitor' # the monitoring group name is also used for the package name
    JetEfficiencyMonAlg = helper.alg
    JetEfficiencyMonAlg.PackageName = groupName


    #################################################################
    #################################################################
    #################################################################
    #################################################################

    # We can choose if we want to use pass before prescale, or not when defining our trigger efficiency
    # generally only want to use pass before prescale when considering the efficiency of a trigger for 
    # internal evaluation of how triggers are behaving
    # the prescaling is an important feature of real utility if a 
    passedb4Prescale = True
    JetEfficiencyMonAlg.PassedBeforePrescale = passedb4Prescale
    
    #################################################################
    #################################################################
    #################################################################
    #################################################################

    #define the various reference triggers
    hltRandom_reference_triggers = ['HLT_j0_perf_L1RD0_FILLED', 'HLT_j0_perf_pf_ftf_L1RD0_FILLED']
    JetEfficiencyMonAlg.HLTRandomReferenceTriggers = hltRandom_reference_triggers

    muon_reference_triggers = ["L1_MU14FCH", "L1_MU18VFCH", "L1_MU8F_TAU20IM", "L1_2MU8F", "L1_MU8VF_2MU5VF", "L1_3MU3VF", "L1_MU5VF_3MU3VF", "L1_4MU3V", "L1_2MU5VF_3MU3V", "L1_RD0_FILLED"]
    JetEfficiencyMonAlg.MuonReferenceTriggers = muon_reference_triggers

    JetEfficiencyMonAlg.BootstrapReferenceTrigger='L1_J15' 

    trigPath = 'Developer/JetEfficiency/'
    ExpertTrigPath = 'Expert/Efficiency/'
    distributionPath = 'Distributions/'
    noRefPath = 'NoReferenceTrigger/'
    muonRefPath = 'MuonReferenceTrigger/'
    randomRefPath = 'RandomHLTReferenceTrigger/'
    bsRefPath = 'BootstrapReferenceTrigger/'
    GeV = 1000

    # add monitoring algorithm to group, with group name and main directory
    single_triggers = []
    multijet_triggers = []
    LR_triggers = []
    
    gfex_SR_triggers = ['L1_gJ20p0ETA25', 'L1_gJ50p0ETA25', 'L1_gJ100p0ETA25', 'L1_gJ400p0ETA25' ]
    gfex_LR_triggers = ['L1_gLJ80p0ETA25', 'L1_gLJ100p0ETA25', 'L1_gLJ140p0ETA25', 'L1_gLJ160p0ETA25']

    jfex_SR_triggers = ['L1_jJ30','L1_jJ40','L1_jJ50', 'L1_jJ60', 'L1_jJ80','L1_jJ90', 'L1_jJ125','L1_jJ140','L1_jJ160', 'L1_jJ180']
    jfex_LR_triggers = ['L1_SC111-CjJ40']
    

    all_SR_singletriggers = single_triggers + gfex_SR_triggers + jfex_SR_triggers
    all_LR_singletriggers = LR_triggers + gfex_LR_triggers + jfex_LR_triggers
    
    JetEfficiencyMonAlg.SmallRadiusJetTriggers_phase1 = all_SR_singletriggers
    JetEfficiencyMonAlg.LargeRadiusJetTriggers_phase1 = all_LR_singletriggers
    JetEfficiencyMonAlg.multiJet_LegacySmallRadiusTriggers = multijet_triggers

    reference_paths = {"Muon" : muonRefPath, "RandomHLT": randomRefPath, "No": noRefPath,  "Bootstrap":  bsRefPath}
    references = ["Muon",  "No", "Bootstrap"] #"RandomHLT"


    trigger_group_list = {"gfex_SR_triggers" : gfex_SR_triggers,
                          "gfex_LR_triggers" : gfex_LR_triggers,
                          "jfex_SR_triggers" : jfex_SR_triggers,
                          "jfex_LR_triggers" : jfex_LR_triggers }
    trigger_title_modifiers = {"gfex_SR_triggers" : "leading offline jet", 
                               "gfex_LR_triggers" : "leading LR offline jet",
                               "jfex_SR_triggers" : "leading offline jet",
                               "jfex_LR_triggers" : "leading LR offline jet" }
    trigger_groups = list(trigger_group_list.keys())

    xlabel_for_prop = { "SRpt" :'pT [MeV]',  "SReta" : '#eta',  "LRpt" :'pT [MeV]',  "LReta" : '#eta'}
    nbins = {"SRpt": 220, "SReta" :32, "LRpt": 220, "LReta" :32}
    binmin = {"SRpt": -50, "SReta" :-3.3, "LRpt": -50, "LReta" :-3.3}
    binmax = {"SRpt": 1800*GeV, "SReta" :3.3, "LRpt": 1800*GeV, "LReta" :3.3}
    properties = ["SRpt","LRpt"] 

    ######### turn off plotting distrubiton histograms so they dont show up on web dispaly 
    plotDistrubutions = False
    if plotDistrubutions: 
        helper.defineHistogram('raw_pt',title='pT for all leading offline jets (with no trigger requirments);PT [MeV];Events',  fillGroup=groupName,  path=trigPath + distributionPath, xbins=nbins["SRpt"], xmin=binmin["SRpt"], xmax=binmax["SRpt"])

        helper.defineHistogram('raw_eta',  title='Eta Distribution for all leading offline jets (with no trigger requirments);#eta; Count', fillGroup=groupName, path=trigPath + distributionPath, xbins=nbins["SReta"], xmin=binmin["SReta"], xmax=binmax["SReta"])
    
    plateau_dict = {} #in case there are low stats for some of the triggers, we could modify the threshold ranges for some triggers
    #gives warning if we dont reach platau by first value, gives error if we dont reach it by second value
    threshold_dict = {"L1_gJ20p0ETA25" : [55e3, 75e3], #hits 50% at 50 for a good run, 100% around 75
                      "L1_gJ50p0ETA25" : [110e3, 180e3], #hits 50% at 109 for a good run, 100% around 180
                      "L1_gJ100p0ETA25" : [195e3, 3000e3],#hits 50% at 185 for a good run, 100% around 300
                      "L1_gJ400p0ETA25" : [620e3, 900e3],#hits 50% at 611 for a good run, 100% around 900
                      "L1_gLJ80p0ETA25" : [125e3, 200e3],#hits 50% at 113 for a good run, 100% around 200
                      "L1_gLJ100p0ETA25" : [150e3, 240e3],#hits 50% at 137 for a good run, 100% around 240
                      "L1_gLJ140p0ETA25" : [210e3, 315e3],#hits 50% at 197 for a good run, 100% around 315
                      "L1_gLJ160p0ETA25" : [240e3, 260e3],#hits 50% at 225 for a good run, 100% around 350
                      "L1_jJ30" : [50e3, 85e3],#hits 50% at 46 for a good run, 100% around 85
                      "L1_jJ40" : [55e3, 90e3],#hits 50% at 47 for a good run, 100% around 90
                      "L1_jJ50" : [60e3, 100e3],#hits 50% at 53 for a good run, 100% around 100
                      "L1_jJ60" : [90e3, 130e3],#hits 50% at 85 for a good run, 100% around 130
                      "L1_jJ80" : [90e3, 155e3],#hits 50% at 86 for a good run, 100% around 155
                      "L1_jJ90" : [110e3, 185e3],#hits 50% at 105 for a good run, 100% around 185
                      "L1_jJ125" : [145e3, 230e3],#hits 50% at 140 for a good run, 100% around 230
                      "L1_jJ140" : [160e3, 225e3],#hits 50% at 150 for a good run, 100% around 255
                      "L1_jJ160" : [180e3, 260e3],#hits 50% at 170 for a good run, 100% around 260
                      "L1_jJ180" : [235e3, 320e3],#hits 50% at 223 for a good run, 100% around 320
                      "L1_SC111-CjJ40": [195e3, 300e3],#hits 50% at 190 for a good run, 100% around 300
    }
    
    ######### define all the histograms 
    for tgroup in trigger_groups: #iterate through the trigger groups
        for t in trigger_group_list[tgroup]: #iterate through the triggers within subgroups 
            if "g" in t: pathAdd = "gFEX/"
            elif "j" in t: pathAdd = "jFEX/"
            #add algorithm that flags if the efficiency is not reaching 100% 
            # assemble thresholdConfig dict
            thresholdConfig = {"Plateau":plateau_dict.get(t,[0.99,0.95])}
            xMaxConfig = min(threshold_dict[t][1]*2.5, binmax["SRpt"]) #set the x maximum of the fit to be 6 times the upper limit, to help the fit work better
            if t in threshold_dict: thresholdConfig["Threshold"] = threshold_dict[t]
            helper.defineDQAlgorithm("JetEfficiency_"+t, 
                                    hanConfig={"libname":"libdqm_algorithms.so","name":"Simple_fermi_Fit_TEff", "xmax":xMaxConfig, "ImproveFit":1}, # this line is always the same
                                    thresholdConfig=thresholdConfig
                                )
            for p in properties: 
                for r in references: #iteratate through the refernce trigger options
                    eff_plot_title =  trigger_title_modifiers[tgroup]+';'+xlabel_for_prop[p]+'; Efficiency '

                    #Using the muon reference trigger selection, as our least biased trigger selection inside the web displkay. Others still exist in the HIST file for now
                    if r == "Muon" and p in ["SRpt", "LRpt"]:
                        helper.defineHistogram(f"bool_{r}_{t}, val_{p};{p}_{t}", type='TEfficiency',  title=eff_plot_title, fillGroup=groupName, path=ExpertTrigPath + pathAdd+ reference_paths[r], xbins=nbins[p], xmin=binmin[p], xmax=binmax[p], hanConfig={"algorithm":"JetEfficiency_"+t})  
                    else:
                        helper.defineHistogram(f"bool_{r}_{t}, val_{p};{p}_{t}", type='TEfficiency',  title=eff_plot_title, fillGroup=groupName, path=trigPath + pathAdd+ reference_paths[r], xbins=nbins[p], xmin=binmin[p], xmax=binmax[p])

    

    acc = helper.result()
    result.merge(acc)
    print("flags.DQ.Environment = " + flags.DQ.Environment )
    return result
 


if __name__=='__main__':
    # set debug level for whole job
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO #DEBUG
    log.setLevel(INFO)

    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    import glob

    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')


    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    flags.lock()

    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JetEfficiencyMonitorCfg = JetEfficiencyMonitoringConfig(flags)
    cfg.merge(JetEfficiencyMonitorCfg)


    # message level for algorithm
    JetEfficiencyMonitorCfg.getEventAlgo('JetEfficiencyMonAlg').OutputLevel = 1 # 1/2 INFO/DEBUG
    # options - print all
