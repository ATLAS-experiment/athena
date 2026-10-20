# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Configure the HitZ reco on its own, without a chain that schedules it.

Until the menu uses it, nothing else in the build touches this configuration,
so a broken flag, alg name or data dependency would otherwise go unnoticed.
"""

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import (
    defaultTestFiles, defaultGeometryTags, defaultConditionsTags,
)
from AthenaCommon.Logging import logging

from TriggerMenuMT.HLT.Jet.JetPresel import getPreselRecoDict
from TriggerMenuMT.HLT.Jet.JetRecoSequencesConfig import JetRecoDataDeps
from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import jetRoIHitZRecoCfg
from TriggerMenuMT.HLT.Bjet import BjetHitZConfig

log = logging.getLogger(__name__)

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RDO_RUN4
flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
flags.lock()

_, preselJetDef = JetRecoDataDeps(flags, **getPreselRecoDict('a4'))['final']
reco, jets = jetRoIHitZRecoCfg(flags, preselJetDef)

hitzJets = 'HLT_AntiKt4EMTopoJets_subjesIS_hitz'
assert jets.startswith(hitzJets), jets

for name in ['ITkPixelPrepDataToxAOD', 'ITkStripPrepDataToxAOD',
             'HitZJetHitAssociationPixel', 'HitZJetHitAssociationStrip',
             'HitZHitsSelector', 'HitZJetTagAlg', 'jetview_'+jets]:
    assert reco.getEventAlgo(name) is not None, f'{name} is not configured'

tagger = reco.getEventAlgo('HitZJetTagAlg')
assert tagger.container == hitzJets
assert tagger.constituentContainer == ''
assert tagger.decorator.nnFile == flags.Trigger.Jet.hitZNetwork
assert dict(tagger.decorator.variableRemapping) == BjetHitZConfig.OUTPUT_REMAP

assert reco.getService('HitZNNSharingOnnxSvc').executionProvider == 'CPU'

reco.wasMerged()
log.info('HitZ reco configured on %s', jets)
