# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Functions used by the ComponentAccumulator to de-duplicate components.
#

from AthenaConfiguration.DebuggingContext import raiseWithCurrentContext

class DeduplicationFailed(RuntimeError):
    pass


def deduplicate(newComp, compList):
    """Merge newComp with compList. If a matching component is found it is updated
    in compList. Otherwise a new component is added to compList.

    Returns True if a new component has been added, otherwise False. If merging fails,
    an exception is raised.
    """

    for idx, comp in enumerate(compList):
        if comp.__cpp_type__==newComp.__cpp_type__ and comp.name==newComp.name:
            exception = None
            try:
                newComp.merge(comp)
            except Exception as e:
                exception = e  # remember exception and raise again outside the handler
            if exception:
                raiseWithCurrentContext(exception)

            # We found a service of the same type and name and could reconcile the two instances.
            # Overwrite the component in the list with the new (merged) component.
            compList[idx] = newComp
            return False

    # No component of the same type and name found, simply append
    compList.append(newComp)
    return True


def deduplicateOne(newComp, oldComp):
    """Merge newComp with oldComp and provide diagnostics on failure."""

    exception = None
    try:
        assert oldComp.__cpp_type__ == newComp.__cpp_type__, "Deduplicating components of different type"
        assert oldComp.name == newComp.name, "Deduplicating components of different name"
        oldComp.merge(newComp)
    except Exception as e:
        exception = e  # remember exception and raise again outside the handler
    if exception:
        raiseWithCurrentContext(exception)
