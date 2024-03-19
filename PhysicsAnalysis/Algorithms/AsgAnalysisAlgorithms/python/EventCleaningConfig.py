# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class EventCleaningBlock (ConfigBlock):
    """the ConfigBlock for event cleaning"""

    def __init__ (self) :
        super (EventCleaningBlock, self).__init__ ()
        self.addOption ('runPrimaryVertexSelection', True, type=bool)
        self.addOption ('runEventCleaning', False, type=bool)
        self.addOption ('userGRLFiles', [], type=None)
        self.addOption ('minTracksPerVertex', 2, type=int)
        self.addOption ('selectionFlags', ['DFCommonJets_eventClean_LooseBad'], type=None)
        # This is a vector<bool>, so parsing True/False is not handled
        # in AnalysisBase, but we can evade this with numerical values
        self.addOption ('invertFlags', [0], type=None)

    def getDefaultGRLs (self, data_year) :
        """ returns a reasonable set of GRLs that should be suited for most analyses """

        if data_year == 2015:
            return ['GoodRunsLists/data15_13TeV/20170619/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml']
        elif data_year == 2016:
            return ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml']
        elif data_year == 2017:
            return ['GoodRunsLists/data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml']
        elif data_year == 2018:
            return ['GoodRunsLists/data18_13TeV/20190318/data18_13TeV.periodAllYear_DetStatus-v102-pro22-04_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml']
        elif data_year == 2022:
            return ['GoodRunsLists/data22_13p6TeV/20230207/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml']
        elif data_year == 2023:
            return ['GoodRunsLists/data23_13p6TeV/20230712/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-05_MERGED_PHYS_StandardGRL_All_Good_25ns.xml']
        else:
            raise ValueError (f"Data year {data_year} is not recognised for automatic GRL retrieval!")

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data:
            # Set up the GRL selection:
            alg = config.createAlgorithm( 'GRLSelectorAlg', 'GRLSelectorAlg' )
            config.addPrivateTool( 'Tool', 'GoodRunsListSelectionTool' )
            if self.userGRLFiles:
                alg.Tool.GoodRunsListVec = self.userGRLFiles
            else:
                alg.Tool.GoodRunsListVec = self.getDefaultGRLs( config.dataYear() )

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
    config.setOptionValue ('runPrimaryVertexSelection', runPrimaryVertexSelection)
    config.setOptionValue ('runEventCleaning', runEventCleaning)
    config.setOptionValue ('userGRLFiles', userGRLFiles)
    seq.append (config)
