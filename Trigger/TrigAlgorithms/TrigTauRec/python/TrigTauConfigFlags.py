# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from tauRec.TauConfigFlags import createTauConfigFlags

def createTrigTauConfigFlags():
    flags = AthConfigFlags()

    # Some additional flags defined here only exists in the Trigger context, but 'cloneAndReplace' in 'addFlagsCategory'
    # assumes a 'Trigger.Offline.Tau' structure, so we need all flags there
    flags.join(createTauConfigFlags(), prefix='Trigger.Offline')

    # All config files will be located at: /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/TrigTauRec/
    # To copy new files, please contact the Tau Trigger coordinators
    flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath = 'TrigTauRec/00-11-02'

    # BRT TES pT calibration for all tau triggers
    flags.Trigger.Offline.Tau.MvaTESConfig = 'OnlineMvaTES_BRT_MC23a_v2.weights.root'


    #####################################################################################
    # DeepSet Nominal ID (xxxxxRNN/perf/idperf_tracktwoMVA chains)
    # LEGACY: MC/Dev trigger menus only!
    #####################################################################################
    # Using LVNN inference

    flags.addFlag('Trigger.Offline.Tau.DeepSet.NetworkConfig', ['DeepSetID_MC23_v2_0p.json',
                                                                'DeepSetID_MC23_v2_1p.json',
                                                                'DeepSetID_MC23_v2_mp.json'])
    flags.addFlag('Trigger.Offline.Tau.DeepSet.MaxTracks', 10)
    flags.addFlag('Trigger.Offline.Tau.DeepSet.MaxClusters', 6)
    flags.addFlag('Trigger.Offline.Tau.DeepSet.ScoreFlatteningConfig', ['DeepSetID_MC23_v2_newPerf_flat_0p.root',
                                                                        'DeepSetID_MC23_v2_newPerf_flat_1p.root',
                                                                        'DeepSetID_MC23_v2_newPerf_flat_mp.root'])
    flags.addFlag('Trigger.Offline.Tau.DeepSet.WPNames', ['VeryLoose', 'Loose', 'Medium', 'Tight'])
    flags.addFlag('Trigger.Offline.Tau.DeepSet.TargetEff', [[0.98,  0.90, 0.65,  0.50],  # 0p WPs: VL, L, M, T
                                                            [0.992, 0.99, 0.97,  0.94],  # 1p WPs: VL, L, M, T
                                                            [0.99,  0.94, 0.895, 0.80]]) # mp WPs: VL, L, M, T


    #####################################################################################
    # RNN LLP ID (xxxxxRNN/perf/idperf_tracktwoLLP chains)
    #####################################################################################
    # Using LVNN inference

    flags.addFlag('Trigger.Offline.Tau.RNNLLP.NetworkConfig', ['llpdev/net_experimental_llz_0p.json',
                                                               'llpdev/net_experimental_llz_1p.json',
                                                               'llpdev/net_experimental_llz_mp.json'])
    flags.addFlag('Trigger.Offline.Tau.RNNLLP.MaxTracks', 10)
    flags.addFlag('Trigger.Offline.Tau.RNNLLP.MaxClusters', 6)
    flags.addFlag('Trigger.Offline.Tau.RNNLLP.ScoreFlatteningConfig', ['llpdev/rnnid_flat_llp_llz0p_050621-v1.root',
                                                                       'llpdev/rnnid_flat_llp_llz1p_050621-v1.root',
                                                                       'llpdev/rnnid_flat_llp_llzmp_050621-v1.root'])
    flags.addFlag('Trigger.Offline.Tau.RNNLLP.WPNames', ['VeryLoose', 'Loose', 'Medium', 'Tight'])
    flags.addFlag('Trigger.Offline.Tau.RNNLLP.TargetEff', [[0.98,  0.90, 0.65,  0.50],  # 0p WPs: VL, L, M, T
                                                           [0.992, 0.99, 0.965, 0.94],  # 1p WPs: VL, L, M, T
                                                           [0.99,  0.98, 0.865, 0.80]]) # mp WPs: VL, L, M, T


    #####################################################################################
    # GNTau Nominal ID (loose/medium/tightGNTau_tracktwoMVA chains)
    #####################################################################################
    # Using ONNX inference

    flags.addFlag('Trigger.Offline.Tau.GNTau.ONNXConfig', ['HLTGNTau_v1p1/GNTau0p_SC2_HP0_fmt.onnx', 'HLTGNTau_v1p1/GNTau1p_SC2_HP0_fmt.onnx', 'HLTGNTau_v1p1/GNTaump_SC2_HP0_fmt.onnx'])
    flags.addFlag('Trigger.Offline.Tau.GNTau.MaxTracks', 10)
    flags.addFlag('Trigger.Offline.Tau.GNTau.MaxClusters', 8)
    flags.addFlag('Trigger.Offline.Tau.GNTau.OutputDiscriminant', 1) # 0: -log(PJet), 1: PTau
    flags.addFlag('Trigger.Offline.Tau.GNTau.ScoreFlatteningConfig', ['HLTGNTau_v1p1/0p_GNTau_map.root', 'HLTGNTau_v1p1/1p_GNTau_map.root', 'HLTGNTau_v1p1/mp_GNTau_map.root'])
    flags.addFlag('Trigger.Offline.Tau.GNTau.TargetWPs', {
        # Target efficiencies (0P, 1P, MP) for each WP
        'VeryLoose': (0.98, 0.992, 0.99), 
        'Loose': (0.90, 0.99, 0.94), 
        'Medium': (0.65, 0.97, 0.92), 
        'Tight': (0.50, 0.94, 0.80),
    })


    #####################################################################################
    # GNTauExt Dev1 ID (loose/mediumGNTauDev1/... chains)
    #####################################################################################
    # Using ONNX inference

    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.ONNXConfig', 'HLTGNTauExt_v0p1/network.onnx')
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.MaxTracks', 10)
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.MaxClusters', 30)
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.NodeNameTau', 'GNTauExt_phadtau')
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.NodeNameJet', 'GNTauExt_pbkg')
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.OutputDiscriminant', 1) # 0: -log(PJet), 1: PTau
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.ScoreFlatteningConfig', ['HLTGNTauExt_v0p1/flattening_map_0p.root', 'HLTGNTauExt_v0p1/flattening_map_1p.root', 'HLTGNTauExt_v0p1/flattening_map_mp.root'])
    flags.addFlag('Trigger.Offline.Tau.GNTauDev1.TargetWPs', {
        # Target efficiencies (0P, 1P, MP) for each WP
        'Loose': (0.86, 0.98, 0.93), 
        'Medium': (0.62, 0.96, 0.91), 

        'LooseVar1': (0.88, 0.97, 0.92), 
        'MediumVar1': (0.64, 0.95, 0.90), 

        'LooseVar2': (0.87, 0.98, 0.94), 
        'MediumVar2': (0.63, 0.96, 0.92), 
    })


    return flags


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.lock()
    flags.Tau.MvaTESConfig
    flags.Trigger.doLVL1
    flags.dump('Tau|Trigger')

    assert flags.Tau.MvaTESConfig != flags.Trigger.Offline.Tau.MvaTESConfig, 'No difference between trigger customization'
    flags.dump('Tau|Trigger')
