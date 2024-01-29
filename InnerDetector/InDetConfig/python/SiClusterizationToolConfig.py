# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiClusterizationTool package

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline


def ClusterMakerToolCfg(flags, name="InDetClusterMakerTool", **kwargs):
    acc = ComponentAccumulator()

    # This directly needs the following Conditions data:
    # PixelChargeCalibCondData & PixelOfflineCalibData
    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeCalibCondCfg, PixelOfflineCalibCondAlgCfg)
    if ('SCT' in flags.Digitization.DoFastDigi and
        'Pixel' not in flags.Digitization.DoFastDigi):
        acc.merge(PixelChargeCalibCondCfg(flags, ReadKey=""))
    else:
        acc.merge(PixelChargeCalibCondCfg(flags))
    acc.merge(PixelOfflineCalibCondAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
        PixelReadoutManagerCfg)
    acc.merge(PixelReadoutManagerCfg(flags))

    if "PixelLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags)))

    if "SCTLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import (
            SCT_LorentzAngleToolCfg)
        kwargs.setdefault("SCTLorentzAngleTool", acc.popToolsAndMerge(
            SCT_LorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.ClusterMakerTool(name, **kwargs))
    return acc


def ITkClusterMakerToolCfg(flags, name="ITkClusterMakerTool", **kwargs):
    acc = ComponentAccumulator()

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
        ITkPixelChargeCalibCondAlgCfg)
    from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
        ITkPixelReadoutManagerCfg)

    # This directly needs the following Conditions data:
    # PixelModuleData & PixelChargeCalibCondData
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    acc.merge(ITkPixelReadoutManagerCfg(flags))
    kwargs.setdefault("PixelReadoutManager", acc.getService(
        "ITkPixelReadoutManager"))

    if "PixelLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
            ITkPixelLorentzAngleToolCfg)
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(
            ITkPixelLorentzAngleToolCfg(flags)))

    if "SCTLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import (
            ITkStripLorentzAngleToolCfg)
        kwargs.setdefault("SCTLorentzAngleTool", acc.popToolsAndMerge(
            ITkStripLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.ClusterMakerTool(name, **kwargs))
    return acc


def InDetPixelRDOToolCfg(flags, name="InDetPixelRDOTool", **kwargs):
    # To produce PixelDetectorElementCollection condition data
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags)

    kwargs.setdefault("PixelDetElStatus", "PixelDetectorElementStatus")

    if kwargs["PixelDetElStatus"] != "":
        from PixelConditionsAlgorithms.PixelConditionsConfig import (
            PixelDetectorElementStatusAlgCfg)
        acc.merge(PixelDetectorElementStatusAlgCfg(flags))

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.PixelConditionsSummaryConfig import (
            PixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            PixelConditionsSummaryCfg(flags)))

    # Enable duplicated RDO check for data15 because duplication mechanism
    # was used.
    kwargs.setdefault("CheckDuplicatedRDO",
                      (len(flags.Input.ProjectName) >= 6 and
                       flags.Input.ProjectName[:6] == "data15"))

    acc.setPrivateTools(CompFactory.InDet.PixelRDOTool(name, **kwargs))
    return acc


def ITkPixelRDOToolCfg(flags, name="ITkPixelRDOTool", **kwargs):
    # To produce ITkPixelDetectorElementCollection condition data
    from PixelGeoModelXml.ITkPixelGeoModelConfig import (
        ITkPixelReadoutGeometryCfg)
    acc = ITkPixelReadoutGeometryCfg(flags)

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import (
            ITkPixelConditionsSummaryCfg)
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(
            ITkPixelConditionsSummaryCfg(flags)))

    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("CheckGanged", False)

    acc.setPrivateTools(CompFactory.InDet.PixelRDOTool(name, **kwargs))
    return acc


def TrigPixelRDOToolCfg(flags, name="InDetTrigPixelRDOTool"):
    return InDetPixelRDOToolCfg(flags, name, PixelDetElStatus="")


def MergedPixelsToolCfg(flags, name="InDetMergedPixelsTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("globalPosAlg", acc.popToolsAndMerge(
        ClusterMakerToolCfg(flags)))
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(
        InDetPixelRDOToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def TrigMergedPixelsToolCfg(flags, name="InDetTrigMergedPixelsTool"):
    acc = ComponentAccumulator()
    kwargs = {}

    kwargs["globalPosAlg"] = acc.popToolsAndMerge(ClusterMakerToolCfg(flags))
    kwargs["PixelRDOTool"] = acc.popToolsAndMerge(TrigPixelRDOToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def ITkMergedPixelsToolCfg(flags, name="ITkMergedPixelsTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("globalPosAlg", acc.popToolsAndMerge(
        ITkClusterMakerToolCfg(flags)))
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(
        ITkPixelRDOToolCfg(flags)))

    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelOfflineCalibData", "")

    acc.setPrivateTools(CompFactory.InDet.MergedPixelsTool(name, **kwargs))
    return acc


def PixelClusterNnCondAlgCfg(
        flags, name="PixelClusterNnCondAlg", trackNetwork=False, **kwargs):
    acc = ComponentAccumulator()

    nn_names = [
        "NumberParticles_NoTrack/",
        "ImpactPoints1P_NoTrack/",
        "ImpactPoints2P_NoTrack/",
        "ImpactPoints3P_NoTrack/",
        "ImpactPointErrorsX1_NoTrack/",
        "ImpactPointErrorsX2_NoTrack/",
        "ImpactPointErrorsX3_NoTrack/",
        "ImpactPointErrorsY1_NoTrack/",
        "ImpactPointErrorsY2_NoTrack/",
        "ImpactPointErrorsY3_NoTrack/"]

    if trackNetwork:
        nn_names = [elm.replace('_NoTrack', '') for elm in nn_names]

    kwargs.setdefault("NetworkNames", nn_names)
    kwargs.setdefault("GetInputsInfo", True)
    kwargs.setdefault("WriteKey", ("PixelClusterNN" if not trackNetwork
                                   else "PixelClusterNNWithTrack"))

    acc.merge(addFoldersSplitOnline(
        flags, "PIXEL",
        "/PIXEL/Onl/PixelClustering/PixelClusNNCalib",
        "/PIXEL/PixelClustering/PixelClusNNCalib",
        className='CondAttrListCollection'))

    if "NetworkToHistoTool" not in kwargs:
        from TrkConfig.TrkNeuralNetworkUtilsConfig import (
            NeuralNetworkToHistoToolCfg)
        kwargs.setdefault("NetworkToHistoTool", acc.popToolsAndMerge(
            NeuralNetworkToHistoToolCfg(flags)))

    acc.addCondAlgo(CompFactory.InDet.TTrainedNetworkCondAlg(name, **kwargs))
    return acc


def PixelClusterNnWithTrackCondAlgCfg(
        flags, name="PixelClusterNnWithTrackCondAlg"):
    return PixelClusterNnCondAlgCfg(flags, name, trackNetwork=True)


def LWTNNCondAlgCfg(flags, name="LWTNNCondAlg", **kwargs):
    acc = ComponentAccumulator()

    acc.merge(addFoldersSplitOnline(
        flags, "PIXEL",
        "/PIXEL/Onl/PixelClustering/PixelNNCalibJSON",
        "/PIXEL/PixelClustering/PixelNNCalibJSON",
        className="CondAttrListCollection"))

    kwargs.setdefault("WriteKey", "PixelClusterNNJSON")

    acc.addCondAlgo(CompFactory.InDet.LWTNNCondAlg(name, **kwargs))
    return acc


def NnClusterizationFactoryCfg(flags, name="NnClusterizationFactory", **kwargs):
    from PixelReadoutGeometry.PixelReadoutGeometryConfig import (
        PixelReadoutManagerCfg)
    acc = PixelReadoutManagerCfg(flags)

    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeCalibCondCfg)
    acc.merge(PixelChargeCalibCondCfg(flags))

    if flags.GeoModel.Run is LHCPeriod.Run1:
        acc.merge(PixelClusterNnCondAlgCfg(flags))
        acc.merge(PixelClusterNnWithTrackCondAlgCfg(flags))
    else:
        acc.merge(LWTNNCondAlgCfg(flags))

    if "PixelLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags)))

    kwargs.setdefault("doRunI", flags.GeoModel.Run is LHCPeriod.Run1)
    kwargs.setdefault("useToT", False)
    kwargs.setdefault("useRecenteringNNWithoutTracks", (
        flags.GeoModel.Run is LHCPeriod.Run1))
    kwargs.setdefault("useRecenteringNNWithTracks", False)
    kwargs.setdefault("correctLorShiftBarrelWithoutTracks", 0)
    kwargs.setdefault("correctLorShiftBarrelWithTracks", (
        0.03 if flags.GeoModel.Run is LHCPeriod.Run1 else 0.))
    kwargs.setdefault("useTTrainedNetworks",
                      flags.GeoModel.Run is LHCPeriod.Run1)
    kwargs.setdefault("NnCollectionReadKey", (
        "PixelClusterNN" if flags.GeoModel.Run is LHCPeriod.Run1 else ""))
    kwargs.setdefault("NnCollectionWithTrackReadKey", (
        "PixelClusterNNWithTrack" if flags.GeoModel.Run is LHCPeriod.Run1
        else ""))
    kwargs.setdefault("NnCollectionJSONReadKey", (
        "" if flags.GeoModel.Run is LHCPeriod.Run1
        else "PixelClusterNNJSON"))

    acc.setPrivateTools(
        CompFactory.InDet.NnClusterizationFactory(name, **kwargs))
    return acc


def TrigNnClusterizationFactoryCfg(flags, name="TrigNnClusterizationFactory"):
    acc = ComponentAccumulator()
    kwargs = {}

    from PixelConditionsAlgorithms.PixelConditionsConfig import (
        PixelChargeCalibCondCfg)
    acc.merge(PixelChargeCalibCondCfg(flags))

    acc.merge(PixelClusterNnCondAlgCfg(flags))
    acc.merge(PixelClusterNnWithTrackCondAlgCfg(flags))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import (
        PixelLorentzAngleToolCfg)
    kwargs["PixelLorentzAngleTool"] = acc.popToolsAndMerge(
        PixelLorentzAngleToolCfg(flags))

    kwargs["useToT"] = False
    kwargs["NnCollectionReadKey"] = "PixelClusterNN"
    kwargs["NnCollectionWithTrackReadKey"] = "PixelClusterNNWithTrack"

    acc.setPrivateTools(
        CompFactory.InDet.NnClusterizationFactory(name, **kwargs))
    return acc


def NnPixelClusterSplitProbToolCfg(
        flags, name="NnPixelClusterSplitProbTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("NnClusterizationFactory", acc.popToolsAndMerge(
            NnClusterizationFactoryCfg(flags)))
    kwargs.setdefault("PriorMultiplicityContent", [1, 1, 1])
    kwargs.setdefault("useBeamSpotInfo", flags.Tracking.useBeamSpotInfoNN)

    acc.setPrivateTools(
        CompFactory.InDet.NnPixelClusterSplitProbTool(name, **kwargs))
    return acc


def PixelGangedAmbiguitiesFinderCfg(
        flags, name="InDetPixelGangedAmbiguitiesFinder", **kwargs):
    # To produce PixelDetectorElementCollection condition data
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags)
    acc.setPrivateTools(
        CompFactory.InDet.PixelGangedAmbiguitiesFinder(name, **kwargs))
    return acc


def ITkPixelGangedAmbiguitiesFinderCfg(
        flags, name="ITkPixelGangedAmbiguitiesFinder", **kwargs):
    # To produce ITkPixelDetectorElementCollection condition data
    from PixelGeoModelXml.ITkPixelGeoModelConfig import (
        ITkPixelReadoutGeometryCfg)
    acc = ITkPixelReadoutGeometryCfg(flags)
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    acc.setPrivateTools(
        CompFactory.InDet.PixelGangedAmbiguitiesFinder(name, **kwargs))
    return acc


def SCT_ClusteringToolCfg(
        flags, name="InDetSCT_ClusteringTool", **kwargs):
    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    kwargs.setdefault("SCTDetElStatus",
                      "SCTDetectorElementStatusWithoutFlagged")

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
            SCT_ConditionsSummaryToolCfg)
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False)))

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import (
            SCT_LorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            SCT_LorentzAngleToolCfg(flags)))

    kwargs.setdefault("globalPosAlg", acc.popToolsAndMerge(
        ClusterMakerToolCfg(flags)))

    if flags.InDet.selectSCTIntimeHits:
        coll_25ns = (flags.Beam.BunchSpacing <= 25 and
                     flags.Beam.Type is BeamType.Collisions)
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.InDet.SCT_ClusteringTool(name, **kwargs))
    return acc


def Trig_SCT_ClusteringToolCfg(flags, name="Trig_SCT_ClusteringTool"):
    acc = ComponentAccumulator()

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import (
        SCT_ConditionsSummaryToolCfg)
    conditionsTool = acc.popToolsAndMerge(SCT_ConditionsSummaryToolCfg(
        flags, withFlaggedCondTool=False, withTdaqTool=False))

    acc.setPrivateTools(acc.popToolsAndMerge(SCT_ClusteringToolCfg(
        flags, name,
        conditionsTool=conditionsTool,
        SCTDetElStatus=""
    )))
    return acc


def ITKStrip_SCT_ClusteringToolCfg(
        flags, name="ITkStripClusteringTool", **kwargs):
    # To produce ITkStripDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("useRowInformation", True)  # ITk-specific clustering

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import (
            ITkStripConditionsSummaryToolCfg)
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            ITkStripConditionsSummaryToolCfg(flags)))

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import (
            ITkStripLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            ITkStripLorentzAngleToolCfg(flags)))

    kwargs.setdefault("globalPosAlg", acc.popToolsAndMerge(
        ITkClusterMakerToolCfg(flags)))

    if flags.ITk.selectStripIntimeHits:
        coll_25ns = (flags.Beam.BunchSpacing <= 25 and
                     flags.Beam.Type is BeamType.Collisions)
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.InDet.SCT_ClusteringTool(name, **kwargs))
    return acc


def ITkTruthClusterizationFactoryCfg(
        flags, name='ITkTruthClusterizationFactory', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("InputSDOMap", "ITkPixelSDO_Map")
    kwargs.setdefault("discardPUHits", flags.Digitization.PileUp)
    kwargs.setdefault("truthClusterSplittingEff", flags.Tracking.TruthClusterSplittingEff)
    acc.setPrivateTools(
        CompFactory.InDet.TruthClusterizationFactory(name, **kwargs))
    return acc


def ITkTruthPixelClusterSplitProbToolCfg(
        flags, name="ITkTruthPixelClusterSplitProbTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PriorMultiplicityContent", [1, 1, 1])
    kwargs.setdefault("NnClusterizationFactory", acc.popToolsAndMerge(
        ITkTruthClusterizationFactoryCfg(flags)))
    acc.setPrivateTools(
        CompFactory.InDet.TruthPixelClusterSplitProbTool(name, **kwargs))
    return acc


def HitsToxAODCopierCfg(flags):
    ca = ComponentAccumulator()
    from InDetConfig.SiClusterizationToolConfig import ITkPixelRDOToolCfg
    tool = ca.popToolsAndMerge(ITkPixelRDOToolCfg(flags))
    alg = CompFactory.InDet.HitsToxAODCopier(PixelRDOTool=tool, PixelRDOContainerKey="ITkPixelRDOs")
    ca.addEventAlgo(alg)

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    # for the available content of this collection, consult the HitsToxAODCopier.cxx
    toRecod = [ "xAOD::BaseContainer#PixelHits", "xAOD::AuxContainerBase#PixelHitsAux.col.row.tot.eta_module.phi_module.layer_disk.barrel_ec.detid",
            "xAOD::BaseContainer#StripHits", "xAOD::AuxContainerBase#StripHitsAux.strip.side.eta_module.phi_module.layer_disk.barrel_ec.detid"  ]
    ca.merge(addToAOD(flags, toRecod))
    ca.merge(addToESD(flags, toRecod))

    return ca    
