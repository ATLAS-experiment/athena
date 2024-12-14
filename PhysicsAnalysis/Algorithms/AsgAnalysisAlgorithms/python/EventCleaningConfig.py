# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class EventCleaningBlock (ConfigBlock):
    """the ConfigBlock for event cleaning"""

    def __init__ (self) :
        super (EventCleaningBlock, self).__init__ ()
        self.addOption ('runPrimaryVertexSelection', True, type=bool,
            info="whether to run primary vertex selection. The default is True.")
        self.addOption ('runEventCleaning', False, type=bool,
            info="whether to run event cleaning (sets up an instance of "
            "CP::EventFlagSelectionAlg). The default is False.")
        self.addOption ('runGRL', True, type=bool,
            info="whether to run GRL decoration/selection. The default is True.")
        self.addOption ('userGRLFiles', [], type=None,
            info="a list of GRL files (list of strings) to select data from. "
            "The default is [] (empty list).")
        self.addOption ('minTracksPerVertex', 2, type=int,
            info="minimum number (integer) of tracks per vertex. The default is 2.")
        self.addOption ('selectionFlags', ['DFCommonJets_eventClean_LooseBad'], type=None,
            info="lags (list of strings) to use for jet cleaning. The default is "
            "['DFCommonJets_eventClean_LooseBad'].")
        # This is a vector<bool>, so parsing True/False is not handled
        # in AnalysisBase, but we can evade this with numerical values
        self.addOption ('invertFlags', [0], type=None,
            info="list of booleans determining whether to invert the cut of the "
            "above selectionFlags. The default is [0].")
        self.addOption ('GRLDict', {}, type=None)
        self.addOption ('noFilter', False, type=bool,
            info="do apply event decoration, but do not filter. The default is False, i.e. 'We decorate events but do not filter' ")
        self.addOption ('useRandomRunNumber', False, type=bool,
            info="use RandomRunNumber to compute GRL info. Only supported for MC. The default is False")

        if self.runGRL and self.userGRLFiles:
            raise ValueError("No userGRLFiles should be specified if runGRL=False")

    def getDefaultGRLs (self, data_year) :
        """ returns a reasonable set of GRLs that should be suited for most analyses """
        from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
        GRLDict = getGoodRunsLists()

        if data_year == 2015:
            return GRLDict['GRL2015']
        elif data_year == 2016:
            return GRLDict['GRL2016']
        elif data_year == 2017:
            return GRLDict['GRL2017_Triggerno17e33prim']
        elif data_year == 2018:
            return GRLDict['GRL2018_Triggerno17e33prim']
        elif data_year == 2022:
            return GRLDict['GRL2022']
        elif data_year == 2023:
            return GRLDict['GRL2023']
        else:
            raise ValueError (f"Data year {data_year} is not recognised for automatic GRL retrieval!")

    def makeAlgs (self, config) :
        
        # Apply GRL
        if self.runGRL and (config.dataType() is DataType.Data or self.useRandomRunNumber):
            if config.dataType() is DataType.Data and self.useRandomRunNumber:
                raise ValueError ("UseRandomRunNumber is only supported for MC!")

            if self.noFilter:
                # here we only decorate the PHYSLITE events with a boolean and don't do any cleaning
                # Set up the GRL Decoration
                for GRLDecoratorName,GRLFile in (self.GRLDict).items():
                    alg = config.createAlgorithm( 'GRLSelectorAlg', GRLDecoratorName )
                    config.addPrivateTool( 'Tool', 'GoodRunsListSelectionTool' )
                    alg.Tool.UseRandomRunNumber = self.useRandomRunNumber
                    alg.Tool.GoodRunsListVec = GRLFile
                    alg.noFilter = True
                    alg.grlKey = "EventInfo." + GRLDecoratorName
                    # Using WriteDecorHandle thus no need for addOutputVar
            else:
                # Set up the GRL selection:
                alg = config.createAlgorithm( 'GRLSelectorAlg', 'GRLSelectorAlg' )
                config.addPrivateTool( 'Tool', 'GoodRunsListSelectionTool' )
                alg.Tool.UseRandomRunNumber = self.useRandomRunNumber
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
                             runGRL = None,
                             userGRLFiles = None,
                             GRLDict = None,
                             noFilter = None,
                             ):
    """Create a basic event cleaning analysis algorithm sequence

    Keyword arguments:
      runPrimaryVertexSelection -- whether to run primary vertex selection
      runEventCleaning -- whether to run event cleaning
      runGRL -- whether to run GRL selection
      userGRLFiles -- a list of GRL files to select data from
      GRLDict -- a dictionary of GRL files to determine decoration names
      noFilter -- whether to apply event decoration or not
    """

    config = EventCleaningBlock ()
    config.setOptionValue ('runPrimaryVertexSelection', runPrimaryVertexSelection)
    config.setOptionValue ('runEventCleaning', runEventCleaning)
    config.setOptionValue ('runGRL', runGRL)
    config.setOptionValue ('userGRLFiles', userGRLFiles)
    config.setOptionValue ('GRLDict', GRLDict)
    config.setOptionValue ('noFilter', noFilter)
    seq.append (config)
