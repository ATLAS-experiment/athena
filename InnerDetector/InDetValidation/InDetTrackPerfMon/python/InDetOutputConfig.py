#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetOutputConfig.py
@author M. Aparo
@date 13-03-20234
@brief CA-based python configurations to add the output stream of the AOD_IDTPM file 
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
#from AthenaConfiguration.ComponentFactory import CompFactory


def InDetOutputCfg( flags, **kwargs ):
    '''
    Write output file in AOD_IDTPM format, on top of the
    histogram output file, for reprocessing
    '''
    acc = ComponentAccumulator()

    ### TODO - uncomment to select which decorations to store
    ## Truth variables
    #truthVarsList = [ 'truthOrigin', 'truthType', 'truthParticleLink', 'truthMatchProbability' ]
    #truthVars = '.'.join( truthVarsList )

    ## object link variables
    #objVarsList = [ 'All', 'Tight', 'Medium', 'Loose', 'VeryLoose', 'LHTight', 'LHMedium', 'LHLoose' ]
    #eleVars = 'LinkedElectron_'+'.LinkedElectron_'.join( objVarsList )
    #muonVars = 'LinkedMuon_'+'.LinkedMuon_'.join( objVarsList )
    #tauBDT1pVars = 'LinkedTauBDT1prong_'+'.LinkedTauBDT1prong_'.join( objVarsList )
    #tauBDT3pVars = 'LinkedTauBDT3prong_'+'.LinkedTauBDT3prong_'.join( objVarsList )
    #tauRNN1pVars = 'LinkedTauRNN1prong_'+'.LinkedTauRNN1prong_'.join( objVarsList )
    #tauRNN3pVars = 'LinkedTauRNN3prong_'+'.LinkedTauRNN3prong_'.join( objVarsList )
    #objVars = '.'.join([ eleVars, muonVars, tauBDT1pVars, tauBDT3pVars, tauRNN1pVars, tauRNN3pVars ])

    ## unique set of items to record
    itemsToRecord = set()

    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames:
        ## adding TrackParticleContainers
        offTracks = getattr( flags.PhysVal.IDTPM, trkAnaName+'.OfflineTrkKey' )
        trigTracks = getattr( flags.PhysVal.IDTPM, trkAnaName+'.TrigTrkKey' )
        itemsToRecord.update({
            'xAOD::TrackParticleContainer#'+offTracks,
            'xAOD::TrackParticleAuxContainer#'+offTracks+'Aux.',#+truthVars+'.'+objVars,
            'xAOD::TrackParticleContainer#'+trigTracks,
            'xAOD::TrackParticleAuxContainer#'+trigTracks+'Aux.',#+truthVars,
        })

        ## adding TruthParticleContainers
        truthTracks = getattr( flags.PhysVal.IDTPM, trkAnaName+'.TruthPartKey' )
        itemsToRecord.update({
            'xAOD::TruthParticleContainer#'+truthTracks,
            'xAOD::TruthParticleAuxContainer#'+truthTracks+'Aux.',
        })

        ## adding object containers
        objStr = getattr( flags.PhysVal.IDTPM, trkAnaName+".SelectOfflineObject" )

        ### electrons
        if objStr == 'Electron':
            itemsToRecord.update({
                'xAOD::ElectronContainer#Electrons',
                # TODO - uncomment to store xAOD::Electron decorations
                #'xAOD::ElectronAuxContainer#ElectronsAux.',
            })

        ### muons
        if objStr == 'Muon':
            itemsToRecord.update({
                'xAOD::MuonContainer#Muons',
                # TODO - uncomment to store xAOD::Muon decorations
                #'xAOD::MuonAuxContainer#MuonsAux.',
            })

        ### taus
        if objStr == 'Tau':
            itemsToRecord.update({
                'xAOD::TauJetContainer#TauJets',
                # TODO - uncomment to store xAOD::TauJets decorations
                #'xAOD::TauJetAuxContainer#TauJetsAux.',
            })

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge( OutputStreamCfg( flags, 'AOD_IDTPM',
                                ItemList = list( itemsToRecord ),
                                disableEventTag = True ) )

    return acc
