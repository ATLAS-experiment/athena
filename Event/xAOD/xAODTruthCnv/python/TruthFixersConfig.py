# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Framework include(s).
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SGComps.AddressRemappingConfig import InputRenameCfg


def TruthParticleFixerAlgCfg(flags, name='xAODTruthParticleFixer',
                             container='TruthParticles', **kwargs):
    '''Configuration for fixing an xAOD::TruthParticleContainer from a DAOD
    '''
    acc = ComponentAccumulator()
    acc.merge(InputRenameCfg('xAOD::TruthParticleContainer',
                             container, f'InFile{container}'))
    acc.merge(InputRenameCfg('xAOD::AuxContainerBase',
                             f'{container}Aux.', f'InFile{container}Aux.'))
    kwargs.setdefault('InputContainer', f'InFile{container}')
    kwargs.setdefault('OutputContainer', container)
    kwargs.setdefault('LinkPrefixToRemove', 'InFile')
    acc.addEventAlgo(
        CompFactory.xAODMaker.TruthParticleFixerAlg(name, **kwargs))
    return acc


def TruthVertexFixerAlgCfg(flags, name='xAODTruthVertexFixer',
                           container='TruthVertices', **kwargs):
    '''Configuration for fixing an xAOD::TruthVertexContainer from a DAOD
    '''
    acc = ComponentAccumulator()
    acc.merge(InputRenameCfg('xAOD::TruthVertexContainer',
                             container, f'InFile{container}'))
    acc.merge(InputRenameCfg('xAOD::AuxContainerBase',
                             f'{container}Aux.', f'InFile{container}Aux.'))
    kwargs.setdefault('InputContainer', f'InFile{container}')
    kwargs.setdefault('OutputContainer', container)
    kwargs.setdefault('LinkPrefixToRemove', 'InFile')
    acc.addEventAlgo(CompFactory.xAODMaker.TruthVertexFixerAlg(name, **kwargs))
    return acc
