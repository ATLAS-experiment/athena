# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def BTagToolCfg(flags, TaggerList, PrimaryVertexCollectionName="", scheme = '', useBTagFlagsDefaults = True):
      """Adds a new myBTagTool instance and registers it.

      input: jetcol:             The name of the jet collections.
             ToolSvc:            The ToolSvc instance.
             options:            Python dictionary of options to be passed to the BTagTool.
             (note the options storeSecondaryVerticesInJet is passed to the removal tool instead)

      The following default options exist:

      BTagLabelingTool                       default: None
      storeSecondaryVerticesInJet            default: BTaggingFlags.writeSecondaryVertices

      output: The btagtool for the desired jet collection."""

      acc=ComponentAccumulator()

      tagToolList = []

      if 'SV1' in TaggerList:
          from JetTagTools.SV1TagConfig import SV1TagCfg
          sv1tool = acc.popToolsAndMerge(SV1TagCfg(flags, 'SV1Tag', scheme))
          tagToolList.append(sv1tool)
      
      if 'SV1Flip' in TaggerList:
          from JetTagTools.SV1TagConfig import SV1TagCfg
          sv1fliptool = acc.popToolsAndMerge(SV1TagCfg(flags, 'SV1FlipTag', scheme))
          tagToolList.append(sv1fliptool)

      if 'MultiSVbb1' in TaggerList:
          from JetTagTools.MultiSVTagConfig import MultiSVTagCfg
          multisvbb1tool = acc.popToolsAndMerge(MultiSVTagCfg(flags,'MultiSVbb1Tag','MultiSVbb1', scheme))
          tagToolList.append(multisvbb1tool)

      if 'MultiSVbb2' in TaggerList:
          from JetTagTools.MultiSVTagConfig import MultiSVTagCfg
          multisvbb2tool = acc.popToolsAndMerge(MultiSVTagCfg(flags, 'MultiSVbb2Tag','MultiSVbb2', scheme))
          tagToolList.append(multisvbb2tool)

      # list of taggers that use MultivariateTagManager
      mvtm_taggers = ['MV2c00','MV2c10','MV2c20','MV2c10mu','MV2m','DL1','DL1mu']
      mvtm_active_taggers = list(set(mvtm_taggers) & set(TaggerList))
      if len(mvtm_active_taggers) > 0:
          from JetTagTools.MultivariateTagManagerConfig import MultivariateTagManagerCfg
          mvtm = acc.popToolsAndMerge(MultivariateTagManagerCfg(flags, 'mvtm', TaggerList = mvtm_active_taggers, scheme = scheme))
          tagToolList.append(mvtm)

      options = {}
      if useBTagFlagsDefaults:
        defaults = { 'vxPrimaryCollectionName'      : PrimaryVertexCollectionName,
                     'TagToolList'                  : tagToolList,
                   }
        for option in defaults:
            options.setdefault(option, defaults[option])
      options['name'] = 'btag'
      btagtool = CompFactory.Analysis.BTagTool(**options)
      acc.setPrivateTools(btagtool)

      return acc
