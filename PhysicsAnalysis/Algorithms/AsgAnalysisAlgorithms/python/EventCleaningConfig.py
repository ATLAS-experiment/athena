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

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # no instance name needed for singleton block

    def getDefaultGRLs (self, data_year) :
        """ returns a reasonable set of GRLs that should be suited for most analyses """
        from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
        GRLDict = getGoodRunsLists()

        GRLKey = 'GRL' + str(data_year)
        if data_year==2017 or data_year==2018:
            GRLKey = GRLKey + '_Triggerno17e33prim'
        return GRLDict[GRLKey]

    def makeAlgs (self, config) :
        
        # Apply GRL
        if self.runGRL and (config.dataType() is DataType.Data or self.useRandomRunNumber):
            if config.dataType() is DataType.Data and self.useRandomRunNumber:
                raise ValueError ("UseRandomRunNumber is only supported for MC!")

            if self.noFilter:
                # here we only decorate the PHYSLITE events with a boolean and don't do any cleaning
                # Set up the GRL Decoration
                if not self.GRLDict:
                    raise ValueError ("No GRLDict specified for GRL decoration, please specify a GRLDict")

                for GRLDecoratorName, GRLFileList in self.GRLDict.items():
                    if isinstance(GRLFileList, str):
                        GRLFileList = [GRLFileList]

                    alg = config.createAlgorithm("GRLSelectorAlg", GRLDecoratorName)
                    config.addPrivateTool("Tool", "GoodRunsListSelectionTool")
                    alg.Tool.UseRandomRunNumber = self.useRandomRunNumber
                    alg.Tool.GoodRunsListVec = GRLFileList
                    alg.noFilter = True
                    alg.grlKey = f"EventInfo.{GRLDecoratorName}"

                    config.addOutputVar("EventInfo", GRLDecoratorName, GRLDecoratorName, noSys=True, auxType="char")
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
                                          'PrimaryVertexSelectorAlg',
                                           reentrant=True )
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



