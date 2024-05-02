#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @author Joseph Lambert

def compareTextBuilder(yamlPath='') :
    """
    Return result of comparing a ConfigSequence produced using the provided
    YAML file and the one produced by the Builder sequence below.
    """
    # create text config object to build text configurations
    from AnalysisAlgorithmsConfig.ConfigText import TextConfig
    config = TextConfig()

    # CommonServices
    config.addBlock('CommonServices')
    config.setOptions(systematicsHistogram='systematicsList')
    config.setOptions(filterSystematics="^(?:(?!PseudoData).)*$")

    # PileupReweighting
    config.addBlock('PileupReweighting')

    # EventCleaning
    config.addBlock('EventCleaning')
    config.setOptions (runEventCleaning=True)

    # Jets
    config.addBlock('Jets')
    config.setOptions (containerName='AnaJets')
    config.setOptions (jetCollection='AntiKt4EMPFlowJets')
    config.setOptions (runJvtUpdate=False)
    config.setOptions (runNNJvtUpdate=True)
    # Jets.FlavourTagging
    config.addBlock( 'Jets.FlavourTagging')
    config.setOptions (containerName='AnaJets')
    config.setOptions (selectionName='ftag')
    config.setOptions (noEffSF=True)
    config.setOptions (btagger='DL1dv01')
    config.setOptions (btagWP='FixedCutBEff_60')
    # Jets.JVT
    config.addBlock('Jets.JVT', containerName='AnaJets')
    # Jets.PtEtaSelection
    config.addBlock ('Jets.PtEtaSelection')
    config.setOptions (containerName='AnaJets')
    config.setOptions (selectionDecoration='selectPtEta')

    # Electrons
    config.addBlock ('Electrons')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (forceFullSimConfig=True)
    # Electrons.WorkingPoint
    config.addBlock ('Electrons.WorkingPoint')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionName='loose')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (noEffSF=True)
    config.setOptions (likelihoodWP='LooseBLayerLH')
    config.setOptions (isolationWP='Loose_VarRad')
    config.setOptions (recomputeLikelihood=False)
    # Electrons.PtEtaSelection
    config.addBlock ('Electrons.PtEtaSelection')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionDecoration='selectPtEta')
    config.setOptions (minPt=10000.0)

    # Photons
    config.addBlock ('Photons', containerName='AnaPhotons')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (recomputeIsEM=False)
    # Photons.WorkingPoint
    config.addBlock ('Photons.WorkingPoint')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionName='tight')
    config.setOptions (forceFullSimConfig=True)
    config.setOptions (noEffSF=True)
    config.setOptions (qualityWP='Tight')
    config.setOptions (isolationWP='FixedCutTight')
    config.setOptions (recomputeIsEM=False)
    # Photons.PtEtaSelection
    config.addBlock ('Photons.PtEtaSelection')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionDecoration='selectPtEta')
    config.setOptions (minPt=10000.0)

    # Muons
    config.addBlock ('Muons', containerName='AnaMuons')
    # Muons.WorkingPoint
    config.addBlock ('Muons.WorkingPoint')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionName='medium')
    config.setOptions (quality='Medium')
    config.setOptions (isolation='Loose_VarRad')
    config.setOptions (onlyRecoEffSF=True)
    # Muons.PtEtaSelection
    config.addBlock ('Muons.PtEtaSelection')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionDecoration='selectPtEta')

    # TauJets
    config.addBlock ('TauJets', containerName='AnaTauJets')
    # TauJets.WorkingPoint
    config.addBlock ('TauJets.WorkingPoint')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionName='tight')
    config.setOptions (quality='Tight')
    # TauJets.PtEtaSelection
    config.addBlock ('TauJets.PtEtaSelection')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionDecoration='selectPtEta')

    # GeneratorLevelAnalysis
    config.addBlock( 'GeneratorLevelAnalysis')

    # MissingET
    config.addBlock ('MissingET')
    config.setOptions (containerName='AnaMET')
    config.setOptions (jets='AnaJets')
    config.setOptions (taus='AnaTauJets.tight')
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (muons='AnaMuons.medium')

    # OverlapRemoval
    config.addBlock( 'OverlapRemoval' )
    config.setOptions (inputLabel='preselectOR')
    config.setOptions (outputLabel='passesOR' )
    config.setOptions (jets='AnaJets')
    config.setOptions (taus='AnaTauJets.tight')
    config.setOptions (electrons='AnaElectrons.loose')
    config.setOptions (photons='AnaPhotons.tight')
    config.setOptions (muons='AnaMuons.medium')

    # Thinning
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaJets')
    config.setOptions (outputName='OutJets')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaElectrons')
    config.setOptions (selectionName='loose')
    config.setOptions (outputName='OutElectrons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaPhotons')
    config.setOptions (selectionName='tight')
    config.setOptions (outputName='OutPhotons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaMuons')
    config.setOptions (selectionName='medium')
    config.setOptions (outputName='OutMuons')
    config.addBlock ('Thinning')
    config.setOptions (containerName='AnaTauJets')
    config.setOptions (selectionName='tight')
    config.setOptions (outputName='OutTauJets')

    config.addBlock ('Output')
    config.setOptions (treeName='analysis')
    config.setOptions (vars=[])
    config.setOptions (metVars=[])
    outputContainers = {
        'mu_': 'OutMuons',
        'el_': 'OutElectrons',
        'ph_' : 'OutPhotons',
        'tau_': 'OutTauJets',
        'jet_': 'OutJets',
        'met_': 'AnaMET',
        '': 'EventInfo'}
    config.setOptions (containers=outputContainers)
    disable_commands = [
        'disable jet_select_baselineJvt.*',
        'disable el_select_loose.*',
        'disable mu_select_medium.*',
        'disable ph_select_tight.*',
        'disable tau_select_tight.*',
    ]
    config.setOptions (commands=disable_commands)

    # configure ConfigSequence
    configSeq = config.configure()


    ## produce ConfigSequecne with yaml file
    textConfig = TextConfig(yamlPath)
    textConfigSeq = textConfig.configure()

    ## Compare
    buildBlocks = configSeq._blocks
    textBlocks = textConfigSeq._blocks 
    if len(buildBlocks) != len(textBlocks):
        raise Exception("Number of blocks are different")
    for i in range(len(buildBlocks)):
        buildBlock = buildBlocks[i]
        textBlock = textBlocks[i]
        buildName = buildBlock.__class__.__name__
        textName = textBlock.__class__.__name__
        if buildName != textName:
            raise Exception(f"In position {i} "
                f"the yaml file results in {textName} "
                f"and the builder results in {buildName}")
        for name in buildBlock.getOptions():
            if name == 'groupName':
                continue
            build = buildBlock.getOptionValue(name)
            text = textBlock.getOptionValue(name)
            if build != text:
                raise Exception(f"For block {buildName}, the block "
                    f"option {name} the yaml file results in {text} "
                    f"and the builder results in {build}")
            

if __name__ == '__main__':
    import os
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('--text-config', dest='text_config',
            default='', action='store',
            help='Perform unit tests using the provided yaml file')
    (options, args) = parser.parse_args()
    textConfig = options.text_config

    if not os.path.isfile(textConfig):
        raise FileNotFoundError(f"{textConfig} is not a file")

    # compare text and builder
    compareTextBuilder(textConfig)
