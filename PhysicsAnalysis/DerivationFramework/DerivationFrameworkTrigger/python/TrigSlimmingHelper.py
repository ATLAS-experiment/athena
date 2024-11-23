# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Helper functions for adding trigger EDM content to a derivation"""

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from TrigEDMConfig.TriggerEDM import getTriggerEDMList


def addTrigEDMSetToOutput(flags, helper: SlimmingHelper, edmSet: str, edmVersion: int = None):
    """Add a full trigger EDM set to the output slimming helper"""

    returnList = []

    if edmVersion is None:
        edmVersion = flags.Trigger.EDMVersion

    # Do nothing if there is no trigger payload in the input file
    if edmVersion == -1:
        return returnList

    edmList = getTriggerEDMList(flags, key=edmSet, runVersion=edmVersion)
    # This list is a mapping from container type to a list of required container names
    # This includes the Aux containers and their lists of aux variables.
    # The SlimmingHelper however requires the list of *interface* (non-Aux) containers.
    # At this point we want all xAOD EDM members and all decorations which have propagated
    # to the AOD to propagate down to the DAOD as well, so we fill the containers into AllVariables.
    # We additionally pick up non-xAOD items in StaticContent
    # Collate that information here
    for cont_type, cont_list in edmList.items():
        for container in cont_list:
            # For the next part we need to know all the container names and the associated aux items
            # If we can assume the standard relation between interface and aux names (which is
            # probably safe) then we just need to look at the aux names for this.
            interface_name, aux, auxitems = container.partition("Aux.")
            if aux:
                returnList += [interface_name]

            if "xAOD::" not in cont_type:
                helper.StaticContent += [f"{cont_type}#{container}"]

    # The list of interface names is returned to the slimming helper within the lookup of smart-collections
    return returnList

