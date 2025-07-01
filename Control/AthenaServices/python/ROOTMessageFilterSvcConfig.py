#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

# Athena import(s).
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ROOTMessageFilterSvcCfg(flags, SuppressionRules=[]):
    '''Component accumulator for Athena::ROOTMessageFilterSvc

    To be used like:

    .. code-block:: python
       from AthenaServices.ROOTMessageFilterSvcConfig import ROOTMessageFilterSvcConfig
       cfg.merge(ROOTMessageFilterSvcCfg(flags,
                    SuppressionRules=[('TClass::Init', '.*DataHeader_p2.*', ROOT.kWarning)]))

    :param flags: The configuration flags
    :param suppress: A list of tuples, each containing a regular expression for the
                     message type, a regular expression for the message text, and
                     the ROOT message level to suppress.
    '''

    # Set up the component accumulator object.
    result = ComponentAccumulator()

    # Create the service appropriately.
    result.addService(CompFactory.Athena.ROOTMessageFilterSvc(
        SuppressionRules=SuppressionRules), create=True)

    # Return the configured CA object.
    return result
