#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigDecisionTool.TrigDecisionToolConfig import getRun3NavigationContainerFromInput

def getTrigDecisionToolLite(flags):
    '''
    @brief Configures and returns a TrigDecisionToolLite private tool instance

    '''
    return CompFactory.Trig.TrigDecisionToolLite('TrigDecisionToolLite', HLTSummary = getRun3NavigationContainerFromInput(flags))
