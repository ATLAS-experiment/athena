# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class EventCleaningBlock (ConfigBlock):
    """the ConfigBlock for event cleaning"""

    def __init__ (self) :
        super (EventCleaningBlock, self).__init__ ('Trigger')
        self.addOption ('runPrimaryVertexSelection', True, type=bool)
        self.addOption ('runEventCleaning', False, type=bool)
        self.addOption ('userGRLFiles', [], type=None)
        self.addOption ('minTracksPerVertex', 2, type=int)
        self.addOption ('selectionFlags', ['DFCommonJets_eventClean_LooseBad'], type=None)
        # This is a vector<bool>, so parsing True/False is not handled
        # in AnalysisBase, but we can evade this with numerical values
        self.addOption ('invertFlags', [0], type=None)

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data:
            # Set up the GRL selection:
            alg = config.createAlgorithm( 'GRLSelectorAlg', 'GRLSelectorAlg' )
            config.addPrivateTool( 'Tool', 'GoodRunsListSelectionTool' )
            alg.Tool.GoodRunsListVec = self.userGRLFiles

        # Skip events with no primary vertex:
        if self.runPrimaryVertexSelection:
            alg = config.createAlgorithm( 'CP::VertexSelectionAlg',
                                          'PrimaryVertexSelectorAlg' )
            alg.VertexContainer = 'PrimaryVertices'
            alg.MinVertices = 1
            alg.MinTracks = self.minTracksPerVertex

        # Set up the event cleaning selection:
        if self.runEventCleaning:
            if config.dataType() is DataType.Data:
                alg = config.createAlgorithm( 'CP::EventStatusSelectionAlg', 'EventStatusSelectionAlg' )
                alg.FilterKey = 'EventErrorState'
                alg.FilterDescription = 'selecting events without any error state set'

            alg = config.createAlgorithm( 'CP::EventFlagSelectionAlg', 'EventFlagSelectionAlg' )
            alg.FilterKey = 'JetCleaning'
            alg.selectionFlags = [f'{sel},as_char' for sel in self.selectionFlags]
            alg.invertFlags = self.invertFlags
            alg.FilterDescription = f"selecting events passing: {','.join(alg.selectionFlags)}"




def makeEventCleaningConfig( seq,
                             runPrimaryVertexSelection = None,
                             runEventCleaning = None,
                             userGRLFiles = None):
    """Create a basic event cleaning analysis algorithm sequence

    Keyword arguments:
      runPrimaryVertexSelection -- whether to run primary vertex selection
      runEventCleaning -- wether to run event cleaning
      userGRLFiles -- a list of GRL files to select data from
    """

    config = EventCleaningBlock ()
    config.setOptionValue ('runPrimaryVertexSelection', runPrimaryVertexSelection, noneAction='ignore')
    config.setOptionValue ('runEventCleaning', runEventCleaning, noneAction='ignore')
    config.setOptionValue ('userGRLFiles', userGRLFiles, noneAction='ignore')
    seq.append (config)
