# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Categories currently used by offline Egamma TO monitoringMT tool
# Mechanism to read mongroups directly from trigger menu

from AthenaMonitoring.DQConfigFlags import DQDataType

def mongroupsCfg(moniAccess, data_type):

        shifter_eg = moniAccess.monitoredChains(signatures="egammaMon", monLevels=["shifter"])
        shifter_tp = moniAccess.monitoredChains(signatures="egammaMon", monLevels=["shifter_tp"])
        shifter_topo = moniAccess.monitoredChains(signatures="egammaMon", monLevels=["shifter_topo"])

        monitoring_electron = list(filter(lambda x: ('HLT_e' in x), shifter_eg)) # etcut chains
        monitoring_tags = []
        monitoringTP_electron = shifter_tp
        
        # need request new mongroup in the future for bootstrap photon (like "shifter_bsphoton")
        monitoring_photon = list(filter(lambda x: ('HLT_g' in x), shifter_eg))
        monitoring_bootstrap = {
                'HLT_g25_medium_L1EM20VH' : 'HLT_g25_loose_L1EM20VH',
                'HLT_g35_medium_L1EM20VH' : 'HLT_g25_loose_L1EM20VH',
                'HLT_g22_tight_L1EM15VHI' : 'HLT_g20_tight_L1EM15VHI',
                'HLT_g50_loose_L1EM20VH'  : 'HLT_g25_loose_L1EM20VH',
                'HLT_g22_tight_L1eEM18M'  : 'HLT_g20_tight_L1eEM18M',
                'HLT_g25_medium_L1eEM24L' : 'HLT_g25_loose_L1eEM24L',
                'HLT_g35_medium_L1eEM24L' : 'HLT_g25_loose_L1eEM24L',
                'HLT_g50_loose_L1eEM24L'  : 'HLT_g25_loose_L1eEM24L'
        }
        # removing duplicate chains
        for bsphchain in monitoring_bootstrap:
                if bsphchain in monitoring_photon:
                        monitoring_photon.remove(bsphchain)

        t0_tp = moniAccess.monitoredChains(signatures="egammaMon", monLevels=["t0_tp"])
        validationTP_electron_eEM = list(filter(lambda x: ('L1eEM' in x), t0_tp ))
        validation_electron = [ 'HLT_e5_etcut_L1EM3', 'HLT_e5_lhtight_noringer_L1EM3', 'HLT_e60_etcut_L1EM22VHI', 'HLT_e26_etcut_L1EM22VHI','HLT_e300_etcut_L1EM22VHI']
        validation_jpsi = list(filter(lambda x: ('_L1JPSI' in x), shifter_topo ))
        validationTP_jpsiee = ['HLT_e5_lhtight_L1EM3']

        monitoring_topo = []
        mongroups = { 
                'monitoring_electron'           : monitoring_electron,
                'monitoring_photon'             : monitoring_photon,
                'monitoring_bootstrap'          : monitoring_bootstrap,
                'monitoringTP_electron'         : monitoringTP_electron,
                'monitoring_tags'               : monitoring_tags,
                'monitoring_topo'               : monitoring_topo,
                'validationTP_electron_eEM'     : validationTP_electron_eEM,
        }

        if data_type is DQDataType.MC:
                
                mongroups['validation_electron']        = validation_electron
                mongroups['validation_photon']          = monitoring_photon
                mongroups['validation_jpsi']            = validation_jpsi
                mongroups['validationTP_jpsiee']        = validationTP_jpsiee

        elif data_type is DQDataType.HeavyIon:
                # Using hard-code lists until fix for _ion chains in the eg monitoring 
                monitoring_electron_hi=['HLT_e13_etcut_ion_L1eEM12L','HLT_e15_lhmedium_nogsf_ion_L1eEM15']
                monitoring_photon_hi=['HLT_g13_etcut_ion_L1eEM12L','HLT_g15_loose_ion_L1eEM15']
                monitoring_bootstrap_hi = {'HLT_g18_etcut_L1EM10' : 'HLT_g18_etcut_L1EM10'}

                mongroups['monitoring_electron_hi']     = monitoring_electron_hi + monitoring_electron
                mongroups['monitoring_photon_hi']       = monitoring_photon_hi + monitoring_photon
                mongroups['monitoring_bootstrap_hi']    = monitoring_bootstrap_hi

        elif data_type is DQDataType.Cosmics:
                monitoring_electron_cosmic=['HLT_e5_etcut_L1EM3']
                monitoring_photon_cosmic=['HLT_g3_etcut_LArPEB_L1EM3']
                monitoring_bootstrap_cosmic = {'HLT_g3_etcut_LArPEB_L1EM3' : 'HLT_g3_etcut_LArPEB_L1EM3'}

                mongroups['monitoring_electron_cosmic']  = monitoring_electron_cosmic
                mongroups['monitoring_photon_cosmic']    = monitoring_photon_cosmic
                mongroups['monitoring_bootstrap_cosmic'] = monitoring_bootstrap_cosmic
        
        return mongroups

# Topolological chains - monitoring configuration
topo_config = {
                    'Zee'   : {'mass':(50 , 130) , 'dphi':(1.5,   5) },
                    'Jpsiee': {'mass':( 1 ,   5) , 'dphi':(1.5,   5) },
                    'Heg'   : {'mass':(90 , 140) , 'dphi':(1.5,   5) },
              }

######  For Offine EGamma DQ purposes: Offline EGamma Trigger aware implementation ######
# Chains retrived in the egammaPerformance/SetupEgammaMonitoring
# Chains inside the ​Physics_pp_run3_v1.py menu:
######  

primary_single_ele = [
        'HLT_e26_lhtight_ivarloose_L1eEM26M',
        'HLT_e26_lhtight_ivarloose_L1eEM26T',
        'HLT_e28_lhtight_ivarloose_L1eEM28M',
        'HLT_e60_lhmedium_L1eEM26M',
        'HLT_e140_lhloose_L1eEM26M']

primary_double_pho = [
        'HLT_2g22_tight_L12eEM18M',
        'HLT_g35_medium_g25_medium_L12eEM24L',
        'HLT_2g50_loose_L12eEM24L',
        ]

monitoring_Zee = ['HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M',
                'HLT_e26_lhtight_e14_etcut_L1eEM26M',
                'HLT_e26_lhtight_e14_etcut_probe_50invmAB130_L1eEM26M',
                'HLT_e26_lhtight_e14_etcut_L1eEM26M'
                ] + primary_single_ele

monitoring_Jpsiee = [
        'HLT_e5_lhtight_e9_etcut_1invmAB5_L1JPSI-1M5-eEM9',
        'HLT_e5_lhtight_e14_etcut_1invmAB5_L1JPSI-1M5-eEM15',
        'HLT_e9_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM9',
        'HLT_e14_lhtight_e4_etcut_1invmAB5_L1JPSI-1M5-eEM15',
        'HLT_e10_lhvloose_L1eEM9',
        'HLT_e14_lhvloose_L1eEM12L'
        ]
