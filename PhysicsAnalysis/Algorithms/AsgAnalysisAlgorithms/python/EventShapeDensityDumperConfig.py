# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class EventShapeDensityDumperBlock(ConfigBlock):
    """ConfigBlock for adding EventShape"""

    def __init__(self):
        super().__init__()
        self.addOption(
            "eventShape",
            "Kt4EMPFlowNeutEventShape",
            type=str,
            info="event shape variable to restore. Available options are: `Kt4EMPFlowEventShape`, `Kt4EMPFlowPUSBEventShape`, `Kt4EMTopoOriginEventShape`, `Kt4EMPFlowNeutEventShape`.",
            meta={'choices':(['Kt4EMPFlowEventShape', 'Kt4EMPFlowPUSBEventShape', 'Kt4EMTopoOriginEventShape', 'Kt4EMPFlowNeutEventShape'],1)}
        )

    def instanceName(self):
        """Return the instance name for this block"""
        return self.eventShape + 'Density'
        
    def makeAlgs(self, config):

        config.setSourceName (self.eventShape, self.eventShape)
        config.setContainerMeta (self.eventShape, "nonContainer", True)

        config.addOutputVar(self.eventShape, "Density", "density", noSys=True, auxType="float")
        config.addOutputVar(self.eventShape, "DensitySigma", "density_sigma", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "DensityArea", "density_area", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "Thrust", "thrust", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "ThrustEta", "thrust_eta", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "ThrustPhi", "thrust_phi", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "ThrustMinor", "thrust_minor", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "Sphericity", "sphericity", noSys=True, auxType="float", enabled=False)
        config.addOutputVar(self.eventShape, "FoxWolfram", "foxwolfram", noSys=True, auxType="float", enabled=False)
