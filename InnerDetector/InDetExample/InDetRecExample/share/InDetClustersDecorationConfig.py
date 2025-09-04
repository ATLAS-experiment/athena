#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""
Intended use:
Reco_tf.py --postInclude  path/to/athena/InnerDetector/InDetExample/InDetRecExample/share/InDetClustersDecorationConfig.py

This schedules:
  - PixelPrepDataToxAOD
  - SCT_PrepDataToxAOD
  - TrackStateOnSurfaceDecorator

This creates the Pixel & SCT MSOSs containers and the links from the track --> MSOSs --> clusters.
  The decoration runs for InDetTrackParticles and InDetDisappearingTrackParticles.
  Also, the Pixel & SCT cluster & MSOS containers are added to the AOD output stream.

"""

from AthenaCommon.GlobalFlags import globalflags
isMC = (globalflags.DataSource() == 'geant4')
if isMC:
    print("Seting isSimulation == True for DerivationFramework__TrackStateOnSurfaceDecorator")
else:
    print("Seting isSimulation == False for DerivationFramework__TrackStateOnSurfaceDecorator")

from AthenaCommon.AppMgr import ToolSvc

from SiLorentzAngleTool.SiLorentzAngleToolConf import SiLorentzAngleTool
ToolSvc += SiLorentzAngleTool(name="PixelLorentzAngleTool")

from PixelConditionsTools.PixelConditionsToolsConf import PixelConditionsSummaryTool
PixelConditionsSummaryTool = PixelConditionsSummaryTool(name="PixelConditionsSummaryTool")
ToolSvc += PixelConditionsSummaryTool

import InDetRecExample.TrackingCommon as TrackingCommon
from InDetPrepRawDataToxAOD.InDetPrepRawDataToxAODConf import PixelPrepDataToxAOD
xAOD_PixelPrepDataToxAOD = PixelPrepDataToxAOD(
    name="xAOD_PixelPrepDataToxAOD",
    ClusterSplitProbabilityName = TrackingCommon.pixelClusterSplitProbName(),
    PixelConditionsSummaryTool=PixelConditionsSummaryTool,
    LorentzAngleTool=ToolSvc.PixelLorentzAngleTool,
    UseTruthInfo=isMC,
    WriteRDOinformation=True,
    WriteNNinformation=True
)
topSequence += xAOD_PixelPrepDataToxAOD

from InDetPrepRawDataToxAOD.InDetPrepRawDataToxAODConf import SCT_PrepDataToxAOD
xAOD_SCT_PrepDataToxAOD = SCT_PrepDataToxAOD(
    name="xAOD_SCT_PrepDataToxAOD",
    OutputLevel=3, # INFO
    UseTruthInfo=isMC,
    WriteRDOinformation=True
)
topSequence += xAOD_SCT_PrepDataToxAOD

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackStateOnSurfaceDecorator
DFTSOS = DerivationFramework__TrackStateOnSurfaceDecorator(
    name="DFTrackStateOnSurfaceDecorator",
    ContainerName="InDetTrackParticles",
    DecorationPrefix="Reco_",
    StorePixel=True,
    StoreSCT=True,
    StoreTRT=False,
    IsSimulation=isMC
)
ToolSvc += DFTSOS

DFTSOS_DT = DerivationFramework__TrackStateOnSurfaceDecorator(
    name="DFTrackStateOnSurfaceDecorator_DT",
    ContainerName="InDetDisappearingTrackParticles",
    DecorationPrefix="Reco_",
    StorePixel=True,
    StoreSCT=True,
    StoreTRT=False,
    IsSimulation=isMC,
    PixelMsosName="DisappearingPixelMSOSs",
    SctMsosName="DisappearingSCT_MSOSs",
    OutputLevel=3 # INFO
)
ToolSvc += DFTSOS_DT

# Now schedule derivation kernel
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
derivationKernel = DerivationFramework__DerivationKernel(
    name="MyDFKernel",
    AugmentationTools=[DFTSOS, DFTSOS_DT]
)
topSequence += derivationKernel

# Add output containers to AOD stream
StreamAOD.ItemList += [
    "xAOD::TrackMeasurementValidationContainer#PixelClusters",
    "xAOD::TrackMeasurementValidationAuxContainer#PixelClustersAux.",
    "xAOD::TrackStateValidationContainer#PixelMSOSs",
    "xAOD::TrackStateValidationAuxContainer#PixelMSOSsAux.",
    "xAOD::TrackStateValidationContainer#DisappearingPixelMSOSs",
    "xAOD::TrackStateValidationAuxContainer#DisappearingPixelMSOSsAux.",
    "xAOD::TrackMeasurementValidationContainer#SCT_Clusters",
    "xAOD::TrackMeasurementValidationAuxContainer#SCT_ClustersAux.",
    "xAOD::TrackStateValidationContainer#SCT_MSOSs",
    "xAOD::TrackStateValidationAuxContainer#SCT_MSOSsAux.",
    "xAOD::TrackStateValidationContainer#DisappearingSCT_MSOSs",
    "xAOD::TrackStateValidationAuxContainer#DisappearingSCT_MSOSsAux.",
]
