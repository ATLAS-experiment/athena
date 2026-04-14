# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from TrkConfig.TrkVKalVrtFitterConfig import TrkVKalVrtFitterCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
from FlavorTagDiscriminants.FlavorTagNNConfig import MultifoldGNNCfg
from os.path import commonpath
from pathlib import PurePath
from ParticleJetTools.JetParticleAssociationAlgConfig import (
    JetParticleAssociationAlgCfg,
)
from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg

def GNNVertexFitterToolCfg(flags, name="GNNVertexFitterTool", **kwargs):
    acc = ComponentAccumulator()

    acc.merge(BeamSpotCondAlgCfg(flags))
    kwargs.setdefault("VertexFitterTool", acc.popToolsAndMerge(TrkVKalVrtFitterCfg(flags)))
    kwargs.setdefault("GNNModel",
        "GN2HLv01" if flags.GeoModel.Run >= LHCPeriod.Run4
        else "GN2v01")
    kwargs.setdefault("JetCollection", "AntiKt4EMPFlowJets")
    kwargs.setdefault("includePrimaryVertex", False)
    kwargs.setdefault("removeNonHFVertices", False)
    kwargs.setdefault("doInclusiveVertexing", False)
    kwargs.setdefault("maxChi2", 20)
    kwargs.setdefault("HFRatio", 0)
    kwargs.setdefault("applyCuts", False)
    acc.setPrivateTools(CompFactory.Rec.GNNVertexFitterTool(**kwargs))

    return acc
      
def GNNVertexFitterAlgCfg(flags, jetcol="AntiKt4EMPFlowJets", inclusive=False, **kwargs):
    acc = ComponentAccumulator()
    
    trackCollection = 'InDetTrackParticles'
    JetTrackAssociator = "TracksForBTagging"

    #Track Augmenter
    acc.merge(BTagTrackAugmenterAlgCfg(flags))

    acc.merge(JetParticleAssociationAlgCfg(
        flags,
        JetCollection=jetcol,
        InputParticleCollection=trackCollection,
        OutputParticleDecoration=JetTrackAssociator,
    ))

    # decorate b-tagging directly to the jets
    for networks in flags.BTagging.NNs.get(jetcol, []):
        assert len(networks['folds']) > 1

        nnFilePaths=networks['folds']
        common = commonpath(nnFilePaths)
        nn_name = '_'.join(PurePath(common).with_suffix('').parts)
        algname = f'{nn_name}_Jet'

        remapping=networks.get('remapping', {})
        remapping['BTagTrackToJetAssociator'] = JetTrackAssociator 

        args = dict(
            flags=flags,
            BTaggingCollection=None,
            TrackCollection=trackCollection,
            nnFilePaths=nnFilePaths,
            remapping=remapping,
            JetCollection=jetcol,
        )

        acc.merge(MultifoldGNNCfg(**args))

        tool = acc.popToolsAndMerge(
            GNNVertexFitterToolCfg(
                flags,
                name=f'{algname}_VertexFitterTool',
                GNNModel=algname,
                doInclusiveVertexing=inclusive,
                removeNonHFVertices=inclusive, # if running inclusive vertexing, remove vertices with no HF tracks
            )
        )

        name = f'{algname}_VertexFitterAlg{"Incl" if inclusive else ""}'
        outcol = f'{"Inclusive" if inclusive else ""}GNNVertices'

        acc.addEventAlgo(
            CompFactory.Rec.GNNVertexFitterAlg(
                name = name,
                VtxTool=tool, 
                inputJetContainer=jetcol, 
                outputVertexContainer=outcol,
                **kwargs
            )
        )
        
    return acc
