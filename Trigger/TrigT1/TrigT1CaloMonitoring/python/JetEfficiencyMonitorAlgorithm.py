#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JetEfficiencyMonitoringConfig(flags):
    '''Function to configure LVL1 JetEfficiency algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format

    result = ComponentAccumulator()
    ###########################################################################
    # Jet and particle flow config required for data POOL files
    if flags.Input.Format is Format.POOL and not flags.Input.isMC:
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
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JetEfficiencyMonitorAlgorithm,'JetEfficiencyMonAlg')
    groupName = 'JetEfficiencyMonitor' # the monitoring group name is also used for the package name
    JetEfficiencyMonAlg = helper.alg
    JetEfficiencyMonAlg.PackageName = groupName


    #################################################################
    #################################################################
    #################################################################
    #################################################################

    # Do we want to emulate the Phase 1 triggers?
    emulated = False
    JetEfficiencyMonAlg.Emulated = emulated

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

    muon_reference_triggers = ["L1_MU14FCH", "L1_MU18VFCH",
    "L1_MU8F_TAU20IM", "L1_2MU8F", "L1_MU8VF_2MU5VF", "L1_3MU3VF",
    "L1_MU5VF_3MU3VF", "L1_4MU3V", "L1_2MU5VF_3MU3V",
    "L1_RD0_FILLED"]
    JetEfficiencyMonAlg.MuonReferenceTriggers = muon_reference_triggers

    JetEfficiencyMonAlg.BootstrapReferenceTrigger='L1_J15' 
    bootstrap_trigger = JetEfficiencyMonAlg.BootstrapReferenceTrigger

    JetEfficiencyMonAlg.HLTBootstrapReferenceTrigger='HLT_noalg_L1J20' 
    HLTbootstrap_trigger = JetEfficiencyMonAlg.HLTBootstrapReferenceTrigger

    trigPath = 'Developer/JetEfficiency/'
    distributionPath = 'Distributions/'
    noRefPath = 'NoReferenceTrigger/'
    muonRefPath = 'MuonReferenceTrigger/'
    randomRefPath = 'RandomHLTReferenceTrigger/'
    bsRefPath = 'BootstrapReferenceTrigger/'
    bsHLTRefPath = 'BootstrapHLTReferenceTrigger/'
    GeV = 1000

    # add monitoring algorithm to group, with group name and main directory
    single_triggers = ['L1_J20', 'L1_J25', 'L1_J30', 'L1_J40', 'L1_J50', 'L1_J75',
                       'L1_J85', 'L1_J100', 'L1_J120',  'L1_J400']
    multijet_triggers = ['L1_J85_3J30', 'L1_3J50', 'L1_4J15', 'L1_4J20']
    LR_triggers = ['L1_SC111-CJ15']
    
    gfex_SR_triggers = ['L1_gJ20','L1_gJ20p0ETA25','L1_gJ20p25ETA49','L1_gJ20p0ETA25_EMPTY', 'L1_gJ30','L1_gJ40','L1_gJ50', 'L1_gJ50p0ETA25','L1_gJ100', 'L1_gJ100p0ETA25', 'L1_gJ400p0ETA25' ]
    gfex_LR_triggers = ['L1_gLJ80', 'L1_gLJ100', 'L1_gLJ140', 'L1_gLJ140p0ETA25']

    jfex_SR_triggers = ['L1_jJ30','L1_jJ40','L1_jJ50', 'L1_jJ60',
                    'L1_jJ80','L1_jJ90', 'L1_jJ125','L1_jJ140','L1_jJ160', 'L1_jJ180']
    jfex_LR_triggers = ['L1_SC111-CjJ40']
    

    all_SR_singletriggers = single_triggers + gfex_SR_triggers + jfex_SR_triggers
    all_LR_singletriggers = LR_triggers + gfex_LR_triggers + jfex_LR_triggers
    
    JetEfficiencyMonAlg.SmallRadiusJetTriggers_phase1_and_legacy = all_SR_singletriggers
    JetEfficiencyMonAlg.LargeRadiusJetTriggers_phase1_and_legacy = all_LR_singletriggers
    JetEfficiencyMonAlg.multiJet_LegacySmallRadiusTriggers = multijet_triggers

    if passedb4Prescale: 
        prescale_title_add = " (PassBeforePrescale) "
    else: 
        prescale_title_add = " "

    reference_titles = {"Muon" : ' wrt muon triggers',
                        "RandomHLT": ' wrt HLT random chain ' + hltRandom_reference_triggers[0] + ' and ' + hltRandom_reference_triggers[1], 
                        "No": '', 
                        "Bootstrap": ' wrt bootstrap trigger ' + bootstrap_trigger,
                        "BootstrapHLT": ' wrt HLT bootstrap chain ' + HLTbootstrap_trigger }
    reference_paths = {"Muon" : muonRefPath, "RandomHLT": randomRefPath, 
                       "No": noRefPath,  "Bootstrap":  bsRefPath, "BootstrapHLT": bsHLTRefPath }
    references = list(reference_titles.keys())


    trigger_group_list = {"single_triggers" : single_triggers,
                          "multijet_triggers" : multijet_triggers,
                          "LR_triggers" : LR_triggers,
                          "gfex_SR_triggers" : gfex_SR_triggers,
                          "gfex_LR_triggers" : gfex_LR_triggers,
                          "jfex_SR_triggers" : jfex_SR_triggers,
                          "jfex_LR_triggers" : jfex_LR_triggers }
    trigger_title_modifiers = {"single_triggers" : "leading offline jet",
                               "multijet_triggers" : "last offline jet of multijet",
                               "LR_triggers" : "leading LR offline jet",
                               "gfex_SR_triggers" : "leading offline jet", 
                               "gfex_LR_triggers" : "leading LR offline jet",
                               "jfex_SR_triggers" : "leading offline jet",
                               "jfex_LR_triggers" : "leading LR offline jet" }
    trigger_groups = list(trigger_group_list.keys())


    title_for_prop = { "pt" :'pT',  "eta" : '#eta'}
    xlabel_for_prop = { "pt" :'pT [MeV]',  "eta" : '#eta'}
    nbins = {"pt": 200, "eta" :32}
    binmin = {"pt": -50, "eta" :-3.3}
    binmax = {"pt": 1400*GeV, "eta" :3.3}
    properties = list(title_for_prop.keys())


    ######### define all the histograms 

    helper.defineHistogram('run',title='Run Number;run;Events',
                           fillGroup=groupName,
                            path=trigPath,xbins=1000000,xmin=-0.5,xmax=999999.5)

    helper.defineHistogram('raw_pt',title='pT for all leading offline jets (with no trigger requirments);PT [MeV];Events',
                           fillGroup=groupName,
                            path=trigPath + distributionPath,xbins=nbins["pt"],xmin=binmin["pt"], xmax=binmax["pt"])

    helper.defineHistogram('eta',  title='Eta Distribution of offline jets for HLT random chain ' + hltRandom_reference_triggers[0] + ' and ' + hltRandom_reference_triggers[1] + ';#eta; Count',
                           fillGroup=groupName,
                                path=trigPath + distributionPath,xbins=nbins["eta"],xmin=binmin["eta"], xmax=binmax["eta"])
    

    for tgroup in trigger_groups: #iterate through the trigger groups
        for t in trigger_group_list[tgroup]: #pull out trigger of interest
            if ("gJ" in t) or ("gLJ" in t) or ("jLJ" in t) or ("jJ" in t):  pathAdd = "phase1/"
            else: pathAdd = "legacy/"
            for r in references: #iteratate through the refernce trigger options
                for p in properties: 
                    
                    if emulated and (("gJ" in t) or ("gLJ" in t)): 
                        eff_plot_title = title_for_prop[p] + ' Efficiency' + prescale_title_add + 'of ' + trigger_title_modifiers[tgroup] + ' for EMULATED trigger ' + t + reference_titles[r]+';'+xlabel_for_prop[p]+'; Efficiency '
                        dist_plot_title = title_for_prop[p] + ' distribution' + prescale_title_add + 'of '+ trigger_title_modifiers[tgroup] +' for EMULATED trigger ' + t +';'+xlabel_for_prop[p]+'; Count '
                    else: 
                        eff_plot_title = title_for_prop[p] + ' Efficiency' + prescale_title_add + 'of ' + trigger_title_modifiers[tgroup] + ' for trigger ' + t + reference_titles[r]+';'+xlabel_for_prop[p]+'; Efficiency '
                        dist_plot_title = title_for_prop[p] + ' distribution' + prescale_title_add + 'of '+ trigger_title_modifiers[tgroup] +' for trigger ' + t +';'+xlabel_for_prop[p]+'; Count '
                        
                    helper.defineHistogram(p+'_'+r+'_'+t+','+p+'_'+r, type='TEfficiency',  title=eff_plot_title,
                                           fillGroup=groupName,
                                    path=trigPath + pathAdd+ reference_paths[r], xbins=nbins[p], xmin=binmin[p], xmax=binmax[p])
                    
                    helper.defineHistogram(p+':'+r+'_'+t,  title=dist_plot_title,
                                           fillGroup=groupName,
                                    path=trigPath + distributionPath, xbins=nbins[p], xmin=binmin[p], xmax=binmax[p])

    

    acc = helper.result()
    result.merge(acc)
    print("flags.DQ.Environment = " +flags.DQ.Environment )
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

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JetEfficiencyMonitorCfg = JetEfficiencyMonitoringConfig(flags)
    cfg.merge(JetEfficiencyMonitorCfg)


    # message level for algorithm
    JetEfficiencyMonitorCfg.getEventAlgo('JetEfficiencyMonAlg').OutputLevel = 1 # 1/2 INFO/DEBUG
    # options - print all
