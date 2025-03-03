# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def ListRemaps(cfg, streams=[]):
    #function to get all of the remapped names needed in PFRun3Config.py
    from SGComps.AddressRemappingConfig import InputRenameCfg
    from OutputStreamAthenaPool.OutputStreamConfig import outputStreamName

    if not isinstance (streams, list):
        streams = [streams]

    # Helper to rename a decoration.
    # When we rename a decoration, we also need to ensure that it
    # won't be written.
    def renameDecor (typ, sgname, dname):
        for sname in streams:
            s = cfg.getEventAlgo (outputStreamName (sname))
            for i in range(len(s.ItemList)):
                item = s.ItemList[i]
                if item.find ('#' + sgname + 'Aux.') > 0:
                    if item[-1] != '.':
                        item = item + '.'
                    item = item + '-' + dname + '_renamed'
                    s.ItemList[i] = item
                    break

        decorname = sgname + '.' + dname
        return InputRenameCfg (typ, decorname, decorname + '_renamed')

    list_remaps=[        
        #Remap input containers, that we rebuild from the ESD
        #Remap the calibrated and origin corrected topoclusters
        InputRenameCfg ('xAOD::CaloClusterContainer','CaloCalTopoClusters','CaloCalTopoClusters_renamed'),
        InputRenameCfg ('xAOD::CaloClusterAuxContainer','CaloCalTopoClustersAux.','CaloCalTopoClusters_renamedAux.'),
        InputRenameCfg ('CaloClusterCellLinkContainer', 'CaloCalTopoClusters_links', 'CaloCalTopoClusters_links_renamed'),
        #rename topotowers
        InputRenameCfg ('xAOD::CaloTowerContainer','CaloCalFwdTopoTowers','CaloCalFwdTopoTowers_renamed'),
        InputRenameCfg ('xAOD::CaloTowerAuxContainer','CaloCalFwdTopoTowersAux.','CaloCalFwdTopoTowers_renamedAux.'),
        InputRenameCfg ('xAOD::CaloClusterContainer','LCOriginTopoClusters','LCOriginTopoClusters_renamed'),
        InputRenameCfg ('xAOD::ShallowAuxContainer', 'LCOriginTopoClustersAux.', 'LCOriginTopoClusters_renamedAux.'),
        InputRenameCfg ('xAOD::CaloClusterContainer','EMOriginTopoClusters','EMOriginTopoClusters_renamed'),
        InputRenameCfg( 'xAOD::ShallowAuxContainer', 'EMOriginTopoClustersAux.', 'EMOriginTopoClusters_renamedAux.'),

        #Remap containers that pflow will rebuild
        InputRenameCfg('xAOD::FlowElementContainer','JetETMissChargedParticleFlowObjects','JetETMissChargedParticleFlowObjects_renamed'),
        InputRenameCfg('xAOD::FlowElementAuxContainer','JetETMissChargedParticleFlowObjectsAux.','JetETMissChargedParticleFlowObjects_renamedAux.'),
        InputRenameCfg('xAOD::FlowElementContainer','JetETMissNeutralParticleFlowObjects','JetETMissNeutralParticleFlowObjects_renamed'),
        InputRenameCfg('xAOD::FlowElementAuxContainer','JetETMissNeutralParticleFlowObjectsAux.','JetETMissNeutralParticleFlowObjects_renamedAux.'),

        #Remap the decorations on other containers that pflow will recreate
        #EGamma
        renameDecor ('xAOD::ElectronContainer','Electrons','chargedFELinks'),
        renameDecor ('xAOD::ElectronContainer','Electrons','neutralFELinks'),
        renameDecor ('xAOD::PhotonContainer','Photons','chargedFELinks'),
        renameDecor ('xAOD::PhotonContainer','Photons','neutralFELinks'),
        renameDecor ('xAOD::ElectronContainer','Electrons','neutralpfoLinks'),
        renameDecor ('xAOD::ElectronContainer','Electrons','chargedpfoLinks'),
        renameDecor ('xAOD::PhotonContainer','Photons','neutralpfoLinks'),
        renameDecor ('xAOD::PhotonContainer','Photons','chargedpfoLinks'),
        #Muons
        renameDecor ('xAOD::MuonContainer','Muons','chargedFELinks'),
        renameDecor ('xAOD::MuonContainer','Muons','neutralFELinks'),
        renameDecor ('xAOD::MuonContainer','Muons','muon_efrac_matched_FE'),
        renameDecor ('xAOD::MuonContainer','Muons','ClusterInfo_deltaR'),  
        renameDecor ('xAOD::CaloClusterContainer','MuonClusterCollection','constituentClusterLinks'),
        #Taus
        renameDecor ('xAOD::TauJetContainer','TauJets','neutralFELinks'),
        renameDecor ('xAOD::TauJetContainer','TauJets','chargedFELinks'),
    ]
    
    return list_remaps
