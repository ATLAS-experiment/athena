# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SingleToolToPlot(tool_name, prefix):
    return (tool_name, prefix)

def ComparedToolsToPlot(tool_ref, tool_test, prefix, match_in_energy = False, match_without_shared = False, match_perfectly = False):
    return (tool_ref, tool_test, prefix, match_in_energy, match_without_shared, match_perfectly)

def MatchingOptions(min_similarity = 0.50, terminal_weight = 250., grow_weight = 500., seed_weight = 1000.):
    return (min_similarity, terminal_weight, grow_weight, seed_weight)

def BasicConstantDataExporterToolCfg(flags, name = "ConstantDataExporter"):
    result=ComponentAccumulator()
    ConstantDataExporter = CompFactory.BasicConstantGPUDataExporter(name)
    ConstantDataExporter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    ConstantDataExporter.TimeFileOutput = "ConstantDataExporterTimes.txt"
    result.setPrivateTools(ConstantDataExporter)
    return result

def BasicEventDataExporterToolCfg(flags, name = "EventDataExporter"):
    result=ComponentAccumulator()
    EventDataExporter = CompFactory.BasicEventDataGPUExporter(name)
    EventDataExporter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    EventDataExporter.TimeFileOutput = "EventDataExporterTimes.txt"
    EventDataExporter.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName
    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        EventDataExporter.MissingCellsToFill = flags.CaloRecGPU.ActiveConfig.MissingCellsToFill
    result.setPrivateTools(EventDataExporter)
    return result

def BasicAthenaClusterImporterToolCfg(flags, name = "AthenaClusterImporter"):
    result=ComponentAccumulator()
    AthenaClusterImporter = CompFactory.BasicGPUToAthenaImporter(name)
    AthenaClusterImporter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    AthenaClusterImporter.TimeFileOutput = "ClusterImporterTimes.txt"
    AthenaClusterImporter.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName
    AthenaClusterImporter.ClusterSize = flags.CaloRecGPU.ActiveConfig.ClusterSize
    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        AthenaClusterImporter.MissingCellsToFill = flags.CaloRecGPU.ActiveConfig.MissingCellsToFill
    result.setPrivateTools(AthenaClusterImporter)
    return result

def CaloClusterDeleterToolCfg(flags, name = "ClusterDeleter"):
    result=ComponentAccumulator()
    ClusterDeleter = CompFactory.CaloClusterDeleter(name)
    result.setPrivateTools(ClusterDeleter)
    return result

def CPUOutputToolCfg(flags, name = "CPUOutput",  folder = "output", prefix = "", suffix = ""):
    result=ComponentAccumulator()
    CPUOutput = CompFactory.CaloCPUOutput(name)
    CPUOutput.SavePath = folder
    CPUOutput.FilePrefix = prefix
    CPUOutput.FileSuffix = suffix
    CPUOutput.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName
    result.setPrivateTools(CPUOutput)
    return result

def GPUOutputToolCfg(flags, name = "GPUOutput",  folder = "output", prefix = "", suffix = "", OnlyOutputCells = None):
    result=ComponentAccumulator()
    GPUOutput = CompFactory.CaloGPUOutput(name)
    GPUOutput.SavePath = folder
    GPUOutput.FilePrefix = prefix
    GPUOutput.FileSuffix = suffix
    GPUOutput.UseSortedAndCutClusters = True
    if OnlyOutputCells is not None:
        GPUOutput.OnlyOutputCellInfo = OnlyOutputCells
    result.setPrivateTools(GPUOutput)
    return result

def ClusterInfoCalcToolCfg(flags, name = "GPUClusterInfoCalculator", do_cut = True):
    result=ComponentAccumulator()
    CalcTool = CompFactory.BasicGPUClusterInfoCalculator(name)
    CalcTool.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    CalcTool.TimeFileOutput = name + "Times.txt"
    if do_cut:
        if not flags.hasFlag('CaloRecGPU.ActiveConfig.CutClustersInAbsEt') :
            CalcTool.ClusterCutsInAbsEt = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedCutsInAbsE
        else:
            CalcTool.ClusterCutsInAbsEt = flags.CaloRecGPU.ActiveConfig.CutClustersInAbsEt
        CalcTool.ClusterEtorAbsEtCut = flags.CaloRecGPU.ActiveConfig.ClusterEtorAbsEtCut
    else:
        CalcTool.ClusterCutsInAbsEt = True
        CalcTool.ClusterEtorAbsEtCut = -1
        #Cutting on absolute value with a negative value => not cutting at all.

    result.setPrivateTools(CalcTool)
    return result

def TopoAutomatonClusteringToolCfg(flags, name = "TAClusterMaker"):
    result=ComponentAccumulator()
    # maker tools
    TAClusterMaker = CompFactory.TopoAutomatonClustering(name)

    TAClusterMaker.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    TAClusterMaker.TimeFileOutput = "TopoAutomatonClusteringTimes.txt"

    TAClusterMaker.CalorimeterNames= flags.CaloRecGPU.ActiveConfig.CalorimeterNames

    TAClusterMaker.SeedSamplingNames = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedSamplingNames

    TAClusterMaker.CellThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRCellThreshold
    TAClusterMaker.NeighborThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRGrowThreshold
    TAClusterMaker.SeedThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRSeedThreshold

    TAClusterMaker.SeedCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedCutsInAbsE
    TAClusterMaker.NeighborCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterNeighborCutsInAbsE
    TAClusterMaker.CellCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterCellCutsInAbsE

    TAClusterMaker.TwoGaussianNoise = flags.CaloRecGPU.ActiveConfig.TwoGaussianNoise


    TAClusterMaker.SeedCutsInT = flags.CaloRecGPU.ActiveConfig.SeedCutsInT
    TAClusterMaker.CutOOTseed = flags.CaloRecGPU.ActiveConfig.CutOOTseed
    TAClusterMaker.UseTimeCutUpperLimit = flags.CaloRecGPU.ActiveConfig.UseTimeCutUpperLimit
    TAClusterMaker.TimeCutUpperLimit = flags.CaloRecGPU.ActiveConfig.TimeCutUpperLimit
    TAClusterMaker.SeedThresholdOnTAbs = flags.CaloRecGPU.ActiveConfig.SeedThresholdOnTAbs
    TAClusterMaker.TreatL1PredictedCellsAsGood = flags.CaloRecGPU.ActiveConfig.TreatL1PredictedCellsAsGood

    TAClusterMaker.XTalkEM2 = flags.CaloRecGPU.ActiveConfig.UseEM2CrossTalk
    TAClusterMaker.XTalkDeltaT = flags.CaloRecGPU.ActiveConfig.CrossTalkDeltaT

    TAClusterMaker.NeighborOption = flags.CaloRecGPU.ActiveConfig.NeighborOption
    TAClusterMaker.RestrictHECIWandFCalNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictHECIWandFCalNeighbors
    TAClusterMaker.RestrictPSNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictPSNeighbors

    result.setPrivateTools(TAClusterMaker)
    return result

def DefaultTopologicalClusteringToolCfg(flags, name = "TopoMaker"):
    result=ComponentAccumulator()
    # maker tools
    TopoMaker = CompFactory.CaloTopoClusterMaker(name)

    TopoMaker.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName
    TopoMaker.CalorimeterNames= flags.CaloRecGPU.ActiveConfig.CalorimeterNames
    TopoMaker.SeedSamplingNames = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedSamplingNames
    TopoMaker.NeighborOption = flags.CaloRecGPU.ActiveConfig.NeighborOption
    TopoMaker.RestrictHECIWandFCalNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictHECIWandFCalNeighbors
    TopoMaker.RestrictPSNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictPSNeighbors
    TopoMaker.CellThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRCellThreshold
    TopoMaker.NeighborThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRGrowThreshold
    TopoMaker.SeedThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRSeedThreshold

    TopoMaker.SeedCutsInT = flags.CaloRecGPU.ActiveConfig.SeedCutsInT
    TopoMaker.CutOOTseed = flags.CaloRecGPU.ActiveConfig.CutOOTseed
    TopoMaker.UseTimeCutUpperLimit = flags.CaloRecGPU.ActiveConfig.UseTimeCutUpperLimit
    TopoMaker.TimeCutUpperLimit = flags.CaloRecGPU.ActiveConfig.TimeCutUpperLimit

    TopoMaker.ClusterEtorAbsEtCut  = flags.CaloRecGPU.ActiveConfig.ClusterEtorAbsEtCut
    TopoMaker.TwoGaussianNoise = flags.CaloRecGPU.ActiveConfig.TwoGaussianNoise
    TopoMaker.SeedCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedCutsInAbsE
    TopoMaker.NeighborCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterNeighborCutsInAbsE
    TopoMaker.CellCutsInAbsE = flags.CaloRecGPU.ActiveConfig.TopoClusterCellCutsInAbsE
    TopoMaker.SeedThresholdOnTAbs = flags.CaloRecGPU.ActiveConfig.SeedThresholdOnTAbs

    TopoMaker.TreatL1PredictedCellsAsGood = flags.CaloRecGPU.ActiveConfig.TreatL1PredictedCellsAsGood

    TopoMaker.UseGPUCriteria = not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria

    TopoMaker.XTalkEM2 = flags.CaloRecGPU.ActiveConfig.UseEM2CrossTalk
    TopoMaker.XTalkDeltaT = flags.CaloRecGPU.ActiveConfig.CrossTalkDeltaT

    result.setPrivateTools(TopoMaker)
    return result

def TopoAutomatonSplitterToolCfg(flags, name = "TopoAutomatonSplitter"):
    result=ComponentAccumulator()
    # maker tools
    Splitter = CompFactory.TopoAutomatonSplitting(name)

    Splitter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    Splitter.TimeFileOutput = "ClusterSplitterTimes.txt"

    Splitter.NumberOfCellsCut = flags.CaloRecGPU.ActiveConfig.SplitterNumberOfCellsCut
    Splitter.EnergyCut = flags.CaloRecGPU.ActiveConfig.SplitterEnergyCut
    Splitter.SamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSamplingNames
    Splitter.SecondarySamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSecondarySamplingNames
    Splitter.ShareBorderCells = flags.CaloRecGPU.ActiveConfig.SplitterShareBorderCells
    Splitter.EMShowerScale = flags.CaloRecGPU.ActiveConfig.EMShowerScale
    Splitter.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.SplitterUseNegativeClusters

    Splitter.TreatL1PredictedCellsAsGood = flags.CaloRecGPU.ActiveConfig.TreatL1PredictedCellsAsGood

    Splitter.NeighborOption = flags.CaloRecGPU.ActiveConfig.NeighborOption
    Splitter.RestrictHECIWandFCalNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictHECIWandFCalNeighbors
    Splitter.RestrictPSNeighbors = flags.CaloRecGPU.ActiveConfig.RestrictPSNeighbors and flags.CaloRecGPU.ActiveConfig.AlsoRestrictPSOnGPUSplitter
    #Since the CPU version does not restrict this!

    result.setPrivateTools(Splitter)
    return result

def GPUClusterSplitterToolCfg(flags, name = "GPUClusterSplitter"):
    result=ComponentAccumulator()
    # maker tools
    Splitter = CompFactory.CaloTopoClusterSplitterGPU(name)

    Splitter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    Splitter.TimeFileOutput = "ClusterSplitterTimes.txt"

    Splitter.NumberOfCellsCut = flags.CaloRecGPU.ActiveConfig.SplitterNumberOfCellsCut
    Splitter.EnergyCut = flags.CaloRecGPU.ActiveConfig.SplitterEnergyCut
    Splitter.SamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSamplingNames
    Splitter.SecondarySamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSecondarySamplingNames
    Splitter.ShareBorderCells = flags.CaloRecGPU.ActiveConfig.SplitterShareBorderCells
    Splitter.EMShowerScale = flags.CaloRecGPU.ActiveConfig.EMShowerScale
    Splitter.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.SplitterUseNegativeClusters

    Splitter.TreatL1PredictedCellsAsGood = flags.CaloRecGPU.ActiveConfig.TreatL1PredictedCellsAsGood

    result.setPrivateTools(Splitter)
    return result

def DefaultClusterSplittingToolCfg(flags, name = "TopoSplitter"):
    result=ComponentAccumulator()
    # maker tools
    TopoSplitter = CompFactory.CaloTopoClusterSplitter(name)


    TopoSplitter.NeighborOption = flags.CaloRecGPU.ActiveConfig.NeighborOption
    TopoSplitter.RestrictHECIWandFCalNeighbors  = flags.CaloRecGPU.ActiveConfig.RestrictHECIWandFCalNeighbors

    TopoSplitter.NumberOfCellsCut = flags.CaloRecGPU.ActiveConfig.SplitterNumberOfCellsCut
    TopoSplitter.EnergyCut = flags.CaloRecGPU.ActiveConfig.SplitterEnergyCut

    TopoSplitter.SamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSamplingNames
    TopoSplitter.SecondarySamplingNames = flags.CaloRecGPU.ActiveConfig.SplitterSecondarySamplingNames

    TopoSplitter.ShareBorderCells = flags.CaloRecGPU.ActiveConfig.SplitterShareBorderCells
    TopoSplitter.EMShowerScale = flags.CaloRecGPU.ActiveConfig.EMShowerScale

    TopoSplitter.TreatL1PredictedCellsAsGood = flags.CaloRecGPU.ActiveConfig.TreatL1PredictedCellsAsGood

    TopoSplitter.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.SplitterUseNegativeClusters

    TopoSplitter.UseGPUCriteria = not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria

    result.setPrivateTools(TopoSplitter)
    return result

def GPUClusterMomentsCalculatorToolCfg(flags, name = "GPUTopoMoments"):

    result=ComponentAccumulator()
    GPUTopoMoments = CompFactory.GPUClusterInfoAndMomentsCalculator(name)

    GPUTopoMoments.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes

    if flags.CaloRecGPU.ActiveConfig.UseAbsEnergyMoments is None:
        GPUTopoMoments.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedCutsInAbsE
    else:
        GPUTopoMoments.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.UseAbsEnergyMoments

    GPUTopoMoments.MaxAxisAngle = flags.CaloRecGPU.ActiveConfig.MomentsMaxAxisAngle

    GPUTopoMoments.TwoGaussianNoise = flags.CaloRecGPU.ActiveConfig.TwoGaussianNoise

    GPUTopoMoments.MinBadLArQuality = flags.CaloRecGPU.ActiveConfig.MomentsMinBadLArQuality

    GPUTopoMoments.MinRLateral = flags.CaloRecGPU.ActiveConfig.MomentsMinRLateral
    GPUTopoMoments.MinLLongitudinal = flags.CaloRecGPU.ActiveConfig.MomentsMinLLongitudinal

    result.setPrivateTools(GPUTopoMoments)
    return result

def DefaultClusterMomentsCalculatorToolCfg(flags, name = "TopoMoments"):
    result=ComponentAccumulator()
    TopoMoments = CompFactory.CaloClusterMomentsMaker(name)

    if flags.CaloRecGPU.ActiveConfig.UseAbsEnergyMoments is None:
        TopoMoments.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.TopoClusterSeedCutsInAbsE
    else:
        TopoMoments.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.UseAbsEnergyMoments

    TopoMoments.MaxAxisAngle = flags.CaloRecGPU.ActiveConfig.MomentsMaxAxisAngle

    TopoMoments.TwoGaussianNoise = flags.CaloRecGPU.ActiveConfig.TwoGaussianNoise

    TopoMoments.MinBadLArQuality = flags.CaloRecGPU.ActiveConfig.MomentsMinBadLArQuality

    TopoMoments.MomentsNames = flags.CaloRecGPU.ActiveConfig.MomentsToCalculate

    TopoMoments.MinRLateral = flags.CaloRecGPU.ActiveConfig.MomentsMinRLateral
    TopoMoments.MinLLongitudinal = flags.CaloRecGPU.ActiveConfig.MomentsMinLLongitudinal

    if not flags.Common.isOnline:
        if flags.Input.isMC:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorr")
        else:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorrRecomputed")

    TopoMoments.UseGPUCriteria = not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria

    result.setPrivateTools(TopoMoments)
    return result

def AthenaClusterAndMomentsImporterToolCfg(flags, name = "AthenaClusterImporter"):
    result=ComponentAccumulator()
    AthenaClusterImporter = CompFactory.GPUToAthenaImporterWithMoments(name)
    AthenaClusterImporter.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName
    AthenaClusterImporter.ClusterSize = flags.CaloRecGPU.ActiveConfig.ClusterSize

    AthenaClusterImporter.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    AthenaClusterImporter.TimeFileOutput = "ClusterAndMomentsImporterTimes.txt"

    #from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
    #theLArHVCellContCorr=LArHVCellContCorrCfg(flags)
    #result.merge(theLArHVCellContCorr)
    #from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
    #result.merge(LArHVScaleCfg(flags))



    if not flags.Common.isOnline:
        if flags.Input.isMC:
            AthenaClusterImporter.HVScaleCorrKey = "LArHVScaleCorr"
        else:
            AthenaClusterImporter.HVScaleCorrKey = "LArHVScaleCorrRecomputed"

    AthenaClusterImporter.MomentsNames = flags.CaloRecGPU.ActiveConfig.MomentsToCalculate

    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        AthenaClusterImporter.MissingCellsToFill = flags.CaloRecGPU.ActiveConfig.MissingCellsToFill

    result.setPrivateTools(AthenaClusterImporter)
    return result

def CellsCounterCPUToolCfg(flags, name = "CPUCounts", folder = "counts", prefix = "CPU", suffix = ""):
    result=ComponentAccumulator()
    CPUCount = CompFactory.CaloCellsCounterCPU(name)
    CPUCount.SavePath = folder
    CPUCount.FilePrefix = prefix
    CPUCount.FileSuffix = suffix
    CPUCount.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName

    CPUCount.CellThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRCellThreshold
    CPUCount.NeighborThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRGrowThreshold
    CPUCount.SeedThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRSeedThreshold

    result.setPrivateTools(CPUCount)
    return result

def CellsCounterGPUToolCfg(flags, name = "GPUCounts", folder = "counts", prefix = "GPU", suffix = ""):
    result=ComponentAccumulator()
    GPUCount = CompFactory.CaloCellsCounterGPU(name)
    GPUCount.SavePath = folder
    GPUCount.FilePrefix = prefix
    GPUCount.FileSuffix = suffix

    GPUCount.CellThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRCellThreshold
    GPUCount.NeighborThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRGrowThreshold
    GPUCount.SeedThresholdOnEorAbsEinSigma = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRSeedThreshold

    result.setPrivateTools(GPUCount)
    return result

def MomentsDumperToolCfg(flags, folder = "moments", name = "MomentsDumper", prefix = "", suffix = ""):
    result=ComponentAccumulator()
    GPUCount = CompFactory.CaloMomentsDumper(name)
    GPUCount.SavePath = folder
    GPUCount.FilePrefix = prefix
    GPUCount.FileSuffix = suffix

    result.setPrivateTools(GPUCount)
    return result

def PlotterMonitoringToolCfg(flags, name = "PlotterMonitoring"):
    result=ComponentAccumulator()
    PloTool = CompFactory.CaloGPUClusterAndCellDataMonitor(name)

    PloTool.CellThreshold = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRCellThreshold
    PloTool.NeighborThreshold = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRGrowThreshold
    PloTool.SeedThreshold = flags.CaloRecGPU.ActiveConfig.TopoClusterSNRSeedThreshold

    PloTool.CellsName = flags.CaloRecGPU.ActiveConfig.CellsName

    PloTool.ClusterMatchingParameters = MatchingOptions()

    #Tools and Combinations to plot
    #should be set by the end user.

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

    PloTool.MonitoringTool = GenericMonitoringTool(flags, "PlotterMonitoringTool")

    result.setPrivateTools(PloTool)
    return result

def MonitorizationTool(flags, name = "MonTool"):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

    monTool = GenericMonitoringTool(flags, name)

    maxNumberOfClusters=2500.0

    monTool.defineHistogram('container_size', path='EXPERT', type='TH1F',  title="Container Size; Number of Clusters; Number of Events", xbins=50, xmin=0.0, xmax=maxNumberOfClusters)
    monTool.defineHistogram('Et', path='EXPERT', type='TH1F',  title="Cluster E_T; E_T [ MeV ] ; Number of Clusters", xbins=135, xmin=-200.0, xmax=2500.0)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F', title="Cluster #eta; #eta ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F', title="Cluster #phi; #phi ; Number of Clusters", xbins=64, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta,Phi', path='EXPERT', type='TH2F', title="Number of Clusters; #eta ; #phi ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    monTool.defineHistogram('clusterSize', path='EXPERT', type='TH1F', title="Cluster Type; Type ; Number of Clusters", xbins=13, xmin=0.5, xmax=13.5)
    monTool.defineHistogram('signalState', path='EXPERT', type='TH1F', title="Signal State; Signal State ; Number of Clusters", xbins=4, xmin=-1.5, xmax=2.5)
    monTool.defineHistogram('size', path='EXPERT', type='TH1F', title="Cluster Size; Size [Cells] ; Number of Clusters", xbins=125, xmin=0.0, xmax=250.0)
    monTool.defineHistogram('N_BAD_CELLS', path='EXPERT', type='TH1F', title="N_BAD_CELLS; N_BAD_CELLS ; Number of Clusters", xbins=250, xmin=0.5, xmax=250.5)
    monTool.defineHistogram('ENG_FRAC_MAX', path='EXPERT', type='TH1F', title="ENG_FRAC_MAX; ENG_FRAC_MAX ; Number of Clusters", xbins=50, xmin=0.0, xmax=1.1)
    monTool.defineHistogram('mu', path='EXPERT', type='TH1F',  title="mu; mu; Number of Events", xbins=50, xmin=0.0, xmax=100)
    monTool.defineHistogram('mu,container_size', path='EXPERT', type='TH2F',  title="Container Size versus #mu; #mu; cluster container size", xbins=50, xmin=20.0, xmax=70, ybins=50, ymin=0.0, ymax=maxNumberOfClusters)
    return monTool

#This simply uses the GPU versions.
#For the tests, we will build our own
#depending on what we want to compare against.
def HybridClusterProcessorCfg(flags, name="HybridClusterProcessor"):
    result = ComponentAccumulator()

    HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor(name)
    HybridClusterProcessor.ClustersOutputName = flags.CaloRecGPU.ActiveConfig.ClustersOutputName
    HybridClusterProcessor.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
    HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
    HybridClusterProcessor.DoPlots = False
    HybridClusterProcessor.PlotterTool = None
    HybridClusterProcessor.DoMonitoring = flags.CaloRecGPU.ActiveConfig.DoMonitoring

    if flags.CaloRecGPU.ActiveConfig.DoMonitoring:
        histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
        result.addService(histSvc)
        HybridClusterProcessor.MonitoringTool = MonitorizationTool(flags)

    HybridClusterProcessor.NumPreAllocatedDataHolders = flags.CaloRecGPU.ActiveConfig.NumPreAllocatedDataHolders


    HybridClusterProcessor.ConstantDataToGPUTool = result.popToolsAndMerge( BasicConstantDataExporterToolCfg(flags) )
    HybridClusterProcessor.EventDataToGPUTool = result.popToolsAndMerge( BasicEventDataExporterToolCfg(flags) )
    HybridClusterProcessor.GPUToEventDataTool = result.popToolsAndMerge( AthenaClusterAndMomentsImporterToolCfg(flags) )

    HybridClusterProcessor.BeforeGPUTools = []

    HybridClusterProcessor.GPUTools = []

    HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"TopoAutomatonClustering"))]

    HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PostGrowGPUClusterPropertiesCalculator", True))]

    HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitter") )]

    HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( GPUClusterMomentsCalculatorToolCfg(flags,"GPUTopoMoments") )]

    HybridClusterProcessor.AfterGPUTools = []

    from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
    caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(flags) )
    HybridClusterProcessor.AfterGPUTools += [CompFactory.CaloClusterBadChannelList(badChannelTool = caloBadChanTool)]

    # add the total Noise
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags))

    #if self.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib:
        #Took out CaloClusterSnapshot that wanted to be a part of a CaloClusterMaker.
        #Possibly change in the future?
    #    from CaloRec.CaloTopoClusterConfig import getTopoClusterLocalCalibTools
    #    HybridClusterProcessor.AfterGPUTools += getTopoClusterLocalCalibTools(flags)

    #    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    #    result.merge(caloTopoCoolFolderCfg(self.ConfigFlags))

    result.addEventAlgo(HybridClusterProcessor)

    return result

def DefaultCaloCellMakerCfg(flags):
        from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
        from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
        from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg
        result=ComponentAccumulator()

        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg

        result.merge(LArGMCfg(flags))
        result.merge(TileGMCfg(flags))

        larCellBuilder     = result.popToolsAndMerge(LArCellBuilderCfg(flags))
        larCellCorrectors  = result.popToolsAndMerge(LArCellCorrectorCfg(flags))
        tileCellBuilder = result.popToolsAndMerge(TileCellBuilderCfg(flags))
        cellFinalizer  = CompFactory.CaloCellContainerFinalizerTool()

        if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
            tileCellBuilder.fakeCrackCells = True

        cellMakerTools=[larCellBuilder,tileCellBuilder,cellFinalizer]+larCellCorrectors

        #Add corrections tools that are not LAr or Tile specific:
        if flags.Calo.Cell.doPileupOffsetBCIDCorr or flags.Cell.doPedestalCorr:
            theCaloCellPedestalCorr=CaloCellPedestalCorrCfg(flags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellPedestalCorr))

        #LAr HV scale corr must come after pedestal corr
        if flags.LAr.doHVCorr:
            from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
            cellMakerTools.append(result.popToolsAndMerge(LArHVCellContCorrCfg(flags)))


        if flags.Calo.Cell.doDeadCellCorr:
            cellMakerTools.append(result.popToolsAndMerge(CaloCellNeighborsAverageCorrCfg(flags)))

        if flags.Calo.Cell.doEnergyCorr:
            cellMakerTools.append(result.popToolsAndMerge(CaloEnergyRescalerCfg(flags)))
        if flags.Calo.Cell.doTimeCorr:
            cellMakerTools.append(result.popToolsAndMerge(CaloCellTimeCorrCfg(flags)))

        cellAlgo=CompFactory.CaloCellMaker(CaloCellMakerToolNames = cellMakerTools,
                                           CaloCellsOutputName = flags.CaloRecGPU.ActiveConfig.CellsName)
        result.addEventAlgo(cellAlgo)
        return result

