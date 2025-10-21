"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FlowEnergyDecorator():
    """
    Function to configure FlowEnergyDecorator for Athena.
    FlowEnergyDecorator decorates UFO's (UFOCSSK) with energy cluster's information.
    """

    
    ca = ComponentAccumulator()

    decorator = CompFactory.FlowEnergyDecorator(
        name="FlowEnergyDecorator",
    )

    ca.addEventAlgo(decorator)

    return ca
