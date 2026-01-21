# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SingleToolToPlot(tool_name, prefix):
    return (tool_name, prefix)

def ComparedToolsToPlot(tool_ref, tool_test, prefix, match_in_energy = False, match_without_shared = False, match_perfectly = False):
    return (tool_ref, tool_test, prefix, match_in_energy, match_without_shared, match_perfectly)

def MatchingOptions(min_similarity = 0.50, terminal_weight = 250., grow_weight = 500., seed_weight = 1000.):
    return (min_similarity, terminal_weight, grow_weight, seed_weight)

def BasicConstantDataExporterToolCfg(flags, name = "ConstantDataExporter", **kwargs):
    result=ComponentAccumulator()
    
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")
        
    ConstantDataExporter = CompFactory.BasicConstantGPUDataExporter(name, **kwargs)
    
    result.setPrivateTools(ConstantDataExporter)
    return result

def BasicEventDataExporterToolCfg(flags, cellsname, name = "EventDataExporter", **kwargs):
    result=ComponentAccumulator()
    
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")
    
    kwargs.setdefault("CellsName", cellsname)
      
    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        kwargs.setdefault("MissingCellsToFill", flags.CaloRecGPU.ActiveConfig.MissingCellsToFill)
                
    EventDataExporter = CompFactory.BasicEventDataGPUExporter(name, **kwargs)
    
    result.setPrivateTools(EventDataExporter)
    return result

def BasicAthenaClusterImporterToolCfg(flags, cellsname, name = "ClusterImporter", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")
    kwargs.setdefault("CellsName", cellsname)
    kwargs.setdefault("ClusterSize", flags.CaloRecGPU.ActiveConfig.ClusterSize)
    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        kwargs.setdefault("MissingCellsToFill", flags.CaloRecGPU.ActiveConfig.MissingCellsToFill)
                
    AthenaClusterImporter = CompFactory.BasicGPUToAthenaImporter(name, **kwargs)
    result.setPrivateTools(AthenaClusterImporter)
    return result

def CPUOutputToolCfg(flags, cellsname, name = "CPUOutput", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("CellsName", cellsname)
    kwargs.setdefault("SavePath", "output")
    
    CPUOutput = CompFactory.CaloCPUOutput(name, **kwargs)
    result.setPrivateTools(CPUOutput)
    return result

def GPUOutputToolCfg(flags, name = "GPUOutput", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("UseSortedAndCutClusters", True)
    kwargs.setdefault("SavePath", "output")
        
    GPUOutput = CompFactory.CaloGPUOutput(name, **kwargs)
    result.setPrivateTools(GPUOutput)
    return result

def ClusterInfoCalcToolCfg(flags, name = "GPUClusterInfoCalculator", do_cut = True, **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")
    if do_cut:
        kwargs.setdefault("ClusterCutsInAbsEt", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterCutClustersInAbsEt)
        kwargs.setdefault("ClusterEtorAbsEtCut", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterEnergyCut)
    else:
        kwargs.setdefault("ClusterCutsInAbsEt", True)
        kwargs.setdefault("ClusterEtorAbsEtCut", -1)
        #Cutting on absolute value with a negative value => not cutting at all.

    CalcTool = CompFactory.BasicGPUClusterInfoCalculator(name, **kwargs)
    result.setPrivateTools(CalcTool)
    return result

def ClusterSorterToolCfg(flags, name = "GPUClusterSorter", do_cut = False, **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")
    if do_cut:
        kwargs.setdefault("ClusterCutsInAbsEt", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterCutClustersInAbsEt)
        kwargs.setdefault("ClusterEtorAbsEtCut", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterEnergyCut)
    else:
        kwargs.setdefault("ClusterCutsInAbsEt", True)
        kwargs.setdefault("ClusterEtorAbsEtCut", -1)
        #Cutting on absolute value with a negative value => not cutting at all.

    SorTool = CompFactory.GPUClusterSorter(name, **kwargs)
    result.setPrivateTools(SorTool)
    return result

def TopoAutomatonClusteringToolCfg(flags, name = "TopoAutomatonClustering", **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")

    kwargs.setdefault("CalorimeterNames", flags.CaloRecGPU.ActiveConfig.GrowingCalorimeterNames)

    kwargs.setdefault("SeedSamplingNames", flags.CaloRecGPU.ActiveConfig.GrowingSeedSamplingNames)

    kwargs.setdefault("SeedThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.TermThreshold)

    kwargs.setdefault("SeedCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold)
    kwargs.setdefault("NeighborCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold)
    kwargs.setdefault("CellCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold)

    kwargs.setdefault("TwoGaussianNoise", flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise)

    kwargs.setdefault("SeedCutsInT", flags.CaloRecGPU.ActiveConfig.doTimeCut)
    kwargs.setdefault("CutOOTseed", flags.CaloRecGPU.ActiveConfig.doTimeCut and flags.CaloRecGPU.ActiveConfig.extendTimeCut)
    kwargs.setdefault("UseTimeCutUpperLimit", flags.CaloRecGPU.ActiveConfig.useUpperLimitForTimeCut)
    kwargs.setdefault("TimeCutUpperLimit", flags.CaloRecGPU.ActiveConfig.timeCutUpperLimit)
    
    kwargs.setdefault("SeedThresholdOnTAbs", flags.CaloRecGPU.ActiveConfig.GrowingTimeCutSeedThreshold)
    
    kwargs.setdefault("TreatL1PredictedCellsAsGood", flags.CaloRecGPU.ActiveConfig.GrowingTreatL1PredictedCellsAsGood)

    kwargs.setdefault("XTalkEM2", flags.CaloRecGPU.ActiveConfig.xtalkEM2)
    kwargs.setdefault("XTalkDeltaT", flags.CaloRecGPU.ActiveConfig.xtalkDeltaT)
    
    #The other cross-talk options are not supported yet.

    kwargs.setdefault("NeighborOption", flags.CaloRecGPU.ActiveConfig.GrowingNeighborOption)
    
    kwargs.setdefault("RestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictHECIWandFCalNeighbors)
    
    kwargs.setdefault("RestrictPSNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictPSNeighbors)

    TAClusterMaker = CompFactory.TopoAutomatonClustering(name, **kwargs)
    result.setPrivateTools(TAClusterMaker)
    return result

def DefaultTopologicalClusteringToolCfg(flags, cellsname, name = "TopoClusterMaker", **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("CellsName", cellsname)
    
    kwargs.setdefault("CalorimeterNames", flags.CaloRecGPU.ActiveConfig.GrowingCalorimeterNames)
    kwargs.setdefault("SeedSamplingNames", flags.CaloRecGPU.ActiveConfig.GrowingSeedSamplingNames)
    
    kwargs.setdefault("NeighborOption", flags.CaloRecGPU.ActiveConfig.GrowingNeighborOption)
    kwargs.setdefault("RestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictHECIWandFCalNeighbors)
    kwargs.setdefault("RestrictPSNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictPSNeighbors)
    
    kwargs.setdefault("SeedThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.TermThreshold)
    
    kwargs.setdefault("SeedCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold)
    kwargs.setdefault("NeighborCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold)
    kwargs.setdefault("CellCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold)

    kwargs.setdefault("SeedCutsInT", flags.CaloRecGPU.ActiveConfig.doTimeCut)
    kwargs.setdefault("CutOOTseed", flags.CaloRecGPU.ActiveConfig.doTimeCut and flags.CaloRecGPU.ActiveConfig.extendTimeCut)
    kwargs.setdefault("UseTimeCutUpperLimit", flags.CaloRecGPU.ActiveConfig.useUpperLimitForTimeCut)
    kwargs.setdefault("TimeCutUpperLimit", flags.CaloRecGPU.ActiveConfig.timeCutUpperLimit)
    kwargs.setdefault("XTalkEM2", flags.CaloRecGPU.ActiveConfig.xtalkEM2)
    kwargs.setdefault("XTalkEM2D", flags.CaloRecGPU.ActiveConfig.xtalkEM2D)
    kwargs.setdefault("XTalkEM2n", flags.CaloRecGPU.ActiveConfig.xtalkEM2n)
    kwargs.setdefault("XTalkEM3", flags.CaloRecGPU.ActiveConfig.xtalkEM3)
    kwargs.setdefault("XTalkEMEta", flags.CaloRecGPU.ActiveConfig.xtalkEMEta)
    kwargs.setdefault("XTalkDeltaT", flags.CaloRecGPU.ActiveConfig.xtalkDeltaT)
    kwargs.setdefault("XTalk2Eratio1", flags.CaloRecGPU.ActiveConfig.xtalk2Eratio1)
    kwargs.setdefault("XTalk2Eratio2", flags.CaloRecGPU.ActiveConfig.xtalk2Eratio2)
    kwargs.setdefault("XTalk3Eratio", flags.CaloRecGPU.ActiveConfig.xtalk3Eratio)
    kwargs.setdefault("XTalkEtaEratio", flags.CaloRecGPU.ActiveConfig.xtalkEtaEratio)
    kwargs.setdefault("XTalk2DEratio", flags.CaloRecGPU.ActiveConfig.xtalk2DEratio)
    
    kwargs.setdefault("ClusterCutsInAbsEt", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterCutClustersInAbsEt)
    kwargs.setdefault("ClusterEtorAbsEtCut", flags.CaloRecGPU.ActiveConfig.PostGrowingClusterEnergyCut)
    
    kwargs.setdefault("TwoGaussianNoise", flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise)
    
    kwargs.setdefault("SeedThresholdOnTAbs", flags.CaloRecGPU.ActiveConfig.GrowingTimeCutSeedThreshold)

    kwargs.setdefault("TreatL1PredictedCellsAsGood", flags.CaloRecGPU.ActiveConfig.GrowingTreatL1PredictedCellsAsGood)

    kwargs.setdefault("UseGPUCriteria", not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria)

    TopoMaker = CompFactory.CaloTopoClusterMaker(name, **kwargs)
    result.setPrivateTools(TopoMaker)
    return result

def TopoAutomatonSplitterToolCfg(flags, name = "ClusterSplitter", **kwargs):
    result=ComponentAccumulator()
    
    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")

    kwargs.setdefault("NumberOfCellsCut", flags.CaloRecGPU.ActiveConfig.SplittingNumberOfCellsCut)
    kwargs.setdefault("EnergyCut", flags.CaloRecGPU.ActiveConfig.SplittingEnergyCut)
    kwargs.setdefault("SamplingNames", flags.CaloRecGPU.ActiveConfig.SplittingSamplingNames)
    kwargs.setdefault("SecondarySamplingNames", flags.CaloRecGPU.ActiveConfig.SplittingSecondarySamplingNames)
    kwargs.setdefault("ShareBorderCells", flags.CaloRecGPU.ActiveConfig.SplittingShareBorderCells)
    kwargs.setdefault("EMShowerScale", flags.CaloRecGPU.ActiveConfig.SplittingEMShowerScale)
    kwargs.setdefault("WeightingOfNegClusters", flags.CaloRecGPU.ActiveConfig.SplittingUseNegativeClusters)

    kwargs.setdefault("TreatL1PredictedCellsAsGood", flags.CaloRecGPU.ActiveConfig.SplittingTreatL1PredictedCellsAsGood)

    kwargs.setdefault("NeighborOption", flags.CaloRecGPU.ActiveConfig.SplittingNeighborOption)
    kwargs.setdefault("RestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.SplittingRestrictHECIWandFCalNeighbors)
    kwargs.setdefault("RestrictPSNeighbors", flags.CaloRecGPU.ActiveConfig.GPUSplittingRestrictPSNeighbors)
    #Since the CPU version does not restrict this!

    Splitter = CompFactory.TopoAutomatonSplitting(name, **kwargs)
    result.setPrivateTools(Splitter)
    return result

def DefaultClusterSplittingToolCfg(flags, name = "TopoSplitter", **kwargs):
    result=ComponentAccumulator()
    
    kwargs.setdefault("NeighborOption", flags.CaloRecGPU.ActiveConfig.SplittingNeighborOption)
    kwargs.setdefault("RestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.SplittingRestrictHECIWandFCalNeighbors)

    kwargs.setdefault("NumberOfCellsCut", flags.CaloRecGPU.ActiveConfig.SplittingNumberOfCellsCut)
    kwargs.setdefault("EnergyCut", flags.CaloRecGPU.ActiveConfig.SplittingEnergyCut)

    kwargs.setdefault("SamplingNames", flags.CaloRecGPU.ActiveConfig.SplittingSamplingNames)
    kwargs.setdefault("SecondarySamplingNames", flags.CaloRecGPU.ActiveConfig.SplittingSecondarySamplingNames)

    kwargs.setdefault("ShareBorderCells", flags.CaloRecGPU.ActiveConfig.SplittingShareBorderCells)
    kwargs.setdefault("EMShowerScale", flags.CaloRecGPU.ActiveConfig.SplittingEMShowerScale)

    kwargs.setdefault("TreatL1PredictedCellsAsGood", flags.CaloRecGPU.ActiveConfig.SplittingTreatL1PredictedCellsAsGood)

    kwargs.setdefault("WeightingOfNegClusters", flags.CaloRecGPU.ActiveConfig.SplittingUseNegativeClusters)

    kwargs.setdefault("UseGPUCriteria", not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria)

    TopoSplitter = CompFactory.CaloTopoClusterSplitter(name, **kwargs)
    result.setPrivateTools(TopoSplitter)
    return result

def GPUClusterMomentsCalculatorToolCfg(flags, name = "GPUTopoMoments", **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")

    kwargs.setdefault("WeightingOfNegClusters", flags.CaloRecGPU.ActiveConfig.MomentsUseAbsEnergy)

    kwargs.setdefault("MaxAxisAngle", flags.CaloRecGPU.ActiveConfig.MomentsMaxAxisAngle)

    kwargs.setdefault("TwoGaussianNoise", flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise)

    kwargs.setdefault("MinBadLArQuality", flags.CaloRecGPU.ActiveConfig.MomentsMinBadLArQuality)

    kwargs.setdefault("MinRLateral", flags.CaloRecGPU.ActiveConfig.MomentsMinRLateral)
    kwargs.setdefault("MinLLongitudinal", flags.CaloRecGPU.ActiveConfig.MomentsMinLLongitudinal)

    GPUTopoMoments = CompFactory.GPUClusterInfoAndMomentsCalculator(name, **kwargs)
    result.setPrivateTools(GPUTopoMoments)
    return result

def DefaultClusterMomentsCalculatorToolCfg(flags, instantiateForTrigger, name = "TopoMoments", **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("WeightingOfNegClusters", flags.CaloRecGPU.ActiveConfig.MomentsUseAbsEnergy)

    kwargs.setdefault("MaxAxisAngle", flags.CaloRecGPU.ActiveConfig.MomentsMaxAxisAngle)

    kwargs.setdefault("TwoGaussianNoise", flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise)

    kwargs.setdefault("MinBadLArQuality", flags.CaloRecGPU.ActiveConfig.MomentsMinBadLArQuality)

    kwargs.setdefault("MomentsNames", flags.CaloRecGPU.ActiveConfig.MomentsToCalculate)

    kwargs.setdefault("MinRLateral", flags.CaloRecGPU.ActiveConfig.MomentsMinRLateral)
    kwargs.setdefault("MinLLongitudinal", flags.CaloRecGPU.ActiveConfig.MomentsMinLLongitudinal)

    kwargs.setdefault("UseGPUCriteria", not flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria)
        
    TopoMoments = CompFactory.CaloClusterMomentsMaker(name, **kwargs)
    
    if not flags.Common.isOnline and not instantiateForTrigger and "LArHVFraction" not in kwargs:
        #If it's already in kwargs, don't override
        #(extra flexibility for the user, not that it's needed...)
        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
        result.merge(LArElecCalibDBCfg(flags,["HVScaleCorr"]))
        if flags.Input.isMC:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorr")
        else:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorrRecomputed")
    
    result.setPrivateTools(TopoMoments)
    return result

def AthenaClusterAndMomentsImporterToolCfg(flags, cellsname, instantiateForTrigger, name = "ClusterAndMomentsImporter", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("CellsName", cellsname)
    
    kwargs.setdefault("ClusterSize", flags.CaloRecGPU.ActiveConfig.ClusterSize)

    kwargs.setdefault("MeasureTimes", flags.CaloRecGPU.ActiveConfig.MeasureTimes)
    kwargs.setdefault("TimeFileOutput", name + "Times.txt")

    if not flags.Common.isOnline and not instantiateForTrigger:
        kwargs.setdefault("FillHVMoments", True)
        if flags.Input.isMC:
            kwargs.setdefault("HVScaleCorrKey", "LArHVScaleCorr")
        else:
            kwargs.setdefault("HVScaleCorrKey", "LArHVScaleCorrRecomputed")
    else:
        kwargs.setdefault("FillHVMoments", False)

    kwargs.setdefault("MomentsNames", flags.CaloRecGPU.ActiveConfig.MomentsToCalculate)

    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        kwargs.setdefault("MissingCellsToFill", flags.CaloRecGPU.ActiveConfig.MissingCellsToFill)

    AthenaClusterImporter = CompFactory.GPUToAthenaImporterWithMoments(name, **kwargs)
    result.setPrivateTools(AthenaClusterImporter)
    return result

def CellsCounterCPUToolCfg(flags, cellsname, name = "CPUCounts", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("SavePath", "counts")
    kwargs.setdefault("FilePrefix", "CPU")
    kwargs.setdefault("CellsName", cellsname)

    kwargs.setdefault("SeedThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.TermThreshold)

    CPUCount = CompFactory.CaloCellsCounterCPU(name, **kwargs)
    result.setPrivateTools(CPUCount)
    return result

def CellsCounterGPUToolCfg(flags, name = "GPUCounts", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("SavePath", "counts")
    kwargs.setdefault("FilePrefix", "GPU")

    kwargs.setdefault("SeedThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.TermThreshold)

    GPUCount = CompFactory.CaloCellsCounterGPU(name, **kwargs)
    result.setPrivateTools(GPUCount)
    return result

def PerformanceInformationOutputToolCfg(flags, cellsname, name = "PerfInfoTool", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("FileName", "event_info.txt")
    kwargs.setdefault("CellsName", cellsname)

    kwargs.setdefault("SeedThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThresholdOnEorAbsEinSigma", flags.CaloRecGPU.ActiveConfig.TermThreshold)

    kwargs.setdefault("GrowingNeighborOption", flags.CaloRecGPU.ActiveConfig.GrowingNeighborOption)
    kwargs.setdefault("GrowingRestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictHECIWandFCalNeighbors)
    kwargs.setdefault("GrowingRestrictPSNeighbors", flags.CaloRecGPU.ActiveConfig.GrowingRestrictPSNeighbors)


    kwargs.setdefault("SplittingNeighborOption", flags.CaloRecGPU.ActiveConfig.SplittingNeighborOption)
    kwargs.setdefault("SplittingRestrictHECIWandFCalNeighbors", flags.CaloRecGPU.ActiveConfig.SplittingRestrictHECIWandFCalNeighbors)
    kwargs.setdefault("SplittingRestrictPSNeighbors", flags.CaloRecGPU.ActiveConfig.GPUSplittingRestrictPSNeighbors)

    kwargs.setdefault("SeedCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsSeedThreshold)
    kwargs.setdefault("NeighborCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsGrowThreshold)
    kwargs.setdefault("CellCutsInAbsE", flags.CaloRecGPU.ActiveConfig.UseAbsTermThreshold)
    
    GPUCount = CompFactory.CaloPerformancePropertiesOutput(name, **kwargs)
    result.setPrivateTools(GPUCount)
    return result
    
def MomentsDumperToolCfg(flags, name = "MomentsDumper", **kwargs):
    result=ComponentAccumulator()
    kwargs.setdefault("SavePath", "moments")

    MomentsDumper = CompFactory.CaloMomentsDumper(name, **kwargs)
    result.setPrivateTools(MomentsDumper)
    return result

def PlotterToolCfg(flags, cellsname, name = "PlotterMonitoring", **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("SeedThreshold", flags.CaloRecGPU.ActiveConfig.SeedThreshold)
    kwargs.setdefault("NeighborThreshold", flags.CaloRecGPU.ActiveConfig.GrowThreshold)
    kwargs.setdefault("CellThreshold", flags.CaloRecGPU.ActiveConfig.TermThreshold)

    kwargs.setdefault("CellsName", cellsname)

    kwargs.setdefault("ClusterMatchingParameters", MatchingOptions())

    if flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        kwargs.setdefault("MissingCellsToFill", flags.CaloRecGPU.ActiveConfig.MissingCellsToFill)
        
    #Tools and Combinations to plot
    #should be set by the end user.

    PloTool = CompFactory.CaloGPUClusterAndCellDataMonitor(name, **kwargs)
    
    if "MonitoringTool" not in kwargs:
        from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
        PloTool.MonitoringTool = GenericMonitoringTool(flags, "PlotterMonitoringTool")
    
    result.setPrivateTools(PloTool)
    return result

def DefaultCalibMomentsToolCfg(flags, name = "TopoCalibMoments", **kwargs):
    result=ComponentAccumulator()
    
    kwargs.setdefault("MomentsNames", ["ENG_CALIB_TOT"
                                       ,"ENG_CALIB_OUT_L"
                                       ,"ENG_CALIB_OUT_T"
                                       ,"ENG_CALIB_EMB0"
                                       ,"ENG_CALIB_EME0"
                                       ,"ENG_CALIB_TILEG3"
                                       ,"ENG_CALIB_DEAD_TOT"
                                       ,"ENG_CALIB_DEAD_EMB0"
                                       ,"ENG_CALIB_DEAD_TILE0"
                                       ,"ENG_CALIB_DEAD_TILEG3"
                                       ,"ENG_CALIB_DEAD_EME0"
                                       ,"ENG_CALIB_DEAD_HEC0"
                                       ,"ENG_CALIB_DEAD_FCAL"
                                       ,"ENG_CALIB_DEAD_LEAKAGE"
                                       ,"ENG_CALIB_DEAD_UNCLASS"
                                       ,"ENG_CALIB_FRAC_EM"
                                       ,"ENG_CALIB_FRAC_HAD"
                                       ,"ENG_CALIB_FRAC_REST"])

    kwargs.setdefault("CalibrationHitContainerNames", ["LArCalibrationHitInactive"
                                                       ,"LArCalibrationHitActive"
                                                       ,"TileCalibHitActiveCell"
                                                       ,"TileCalibHitInactiveCell"])
                                                       
    kwargs.setdefault("DMCalibrationHitContainerNames", ["LArCalibrationHitDeadMaterial"
                                                         ,"TileCalibHitDeadMaterial"])
    
    TopoCalibMoments = CompFactory.CaloCalibClusterMomentsMaker2(name, **kwargs)
    result.setPrivateTools(TopoCalibMoments)
    return result

def DefaultTopoClusterLocalCalibToolsCfg(flags, instantiateForTrigger, prefix = ''):
    result=ComponentAccumulator()
        
    CaloClusterLocalCalib=CompFactory.CaloClusterLocalCalib
    
    # Local cell weights    
    LCClassify   = CompFactory.CaloLCClassificationTool(prefix + "LCClassify")
    LCClassify.ClassificationKey   = "EMFracClassify"
    LCClassify.UseSpread = False
    LCClassify.MaxProbability = 0.85 if flags.GeoModel.AtlasVersion.startswith("Rome") and instantiateForTrigger else 0.5
    LCClassify.StoreClassificationProbabilityInAOD = True
    if instantiateForTrigger:
        LCClassify.UseNormalizedEnergyDensity = not flags.GeoModel.AtlasVersion.startswith("Rome")
    if not instantiateForTrigger:
        LCClassify.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    LCWeight = CompFactory.CaloLCWeightTool(prefix + "LCWeight")
    LCWeight.CorrectionKey       = "H1ClusterCellWeights"
    LCWeight.SignalOverNoiseCut  = 2.0
    LCWeight.UseHadProbability   = True

    LocalCalib = CaloClusterLocalCalib (prefix + "LocalCalib")
    LocalCalib.ClusterClassificationTool     = [LCClassify]
    LocalCalib.ClusterRecoStatus             = [1,2]
    LocalCalib.LocalCalibTools               = [LCWeight]
    if not instantiateForTrigger:
        LocalCalib.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    # Out-of-cluster corrections
    LCOut     = CompFactory.CaloLCOutOfClusterTool(prefix + "LCOut")
    LCOut.CorrectionKey       = "OOCCorrection"
    LCOut.UseEmProbability    = False
    LCOut.UseHadProbability   = True

    OOCCalib   = CaloClusterLocalCalib (prefix + "OOCCalib")
    OOCCalib.ClusterRecoStatus   = [1,2]
    OOCCalib.LocalCalibTools     = [LCOut]
    if not instantiateForTrigger:
        OOCCalib.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    LCOutPi0  = CompFactory.CaloLCOutOfClusterTool(prefix + "LCOutPi0")
    LCOutPi0.CorrectionKey    = "OOCPi0Correction"
    LCOutPi0.UseEmProbability  = True
    LCOutPi0.UseHadProbability = False

    OOCPi0Calib   = CaloClusterLocalCalib (prefix + "OOCPi0Calib")
    OOCPi0Calib.ClusterRecoStatus   = [1,2]
    OOCPi0Calib.LocalCalibTools     = [LCOutPi0]
    if not instantiateForTrigger:
        OOCPi0Calib.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    # Dead material corrections
    LCDeadMaterial   = CompFactory.CaloLCDeadMaterialTool(prefix + "LCDeadMaterial")
    LCDeadMaterial.HadDMCoeffKey       = "HadDMCoeff2"
    LCDeadMaterial.ClusterRecoStatus   = 0
    LCDeadMaterial.WeightModeDM        = 2
    LCDeadMaterial.UseHadProbability   = True
    if not instantiateForTrigger:
        LCDeadMaterial.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    DMCalib    = CaloClusterLocalCalib (prefix + "DMCalib")
    DMCalib.ClusterRecoStatus   = [1,2]
    DMCalib.LocalCalibTools      = [LCDeadMaterial]
    if not instantiateForTrigger:
      DMCalib.WeightingOfNegClusters = flags.CaloRecGPU.ActiveConfig.CalibrationUseAbsEnergy

    lccalibtools = [
        LocalCalib,
        OOCCalib,
        OOCPi0Calib,
        DMCalib]
    
    result.setPrivateTools(lccalibtools)
    
    return result
    
#Depending on ReallyUseGPUTools,
#instantiates GPU or CPU tools with consistent options...
def GPUCaloTopoClusterCfg(flags, instantiateForTrigger, cellsname,
                          clustersname = None, clustersnapname="CaloTopoClusters", name="HybridClusterProcessor",
                          PlotterTool = None, addAsPrimary = True, ReallyUseGPUTools = True):

    doLCCalib = flags.CaloRecGPU.ActiveConfig.doTopoClusterLocalCalib

    if clustersname is None:
        clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"

    if clustersname == "CaloTopoClusters" and doLCCalib:
        raise RuntimeError("Inconsistent arguments: clustersname must not be 'CaloTopoClusters' if doTopoClusterLocalCalib is True")
    
    result = ComponentAccumulator()
    
    if not instantiateForTrigger:
    
      from LArGeoAlgsNV.LArGMConfig import LArGMCfg
      from TileGeoModel.TileGMConfig import TileGMCfg
      from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    
      result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
      result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))
    
      result.merge(LArGMCfg(flags))
      result.merge(TileGMCfg(flags))
    

    HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor(name)
    HybridClusterProcessor.ClustersOutputName = clustersname
    
    HybridClusterProcessor.WriteTriggerSpecificInfo = instantiateForTrigger
    
    HybridClusterProcessor.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    
    HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
        
    HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
    HybridClusterProcessor.DoPlots = PlotterTool is not None
    HybridClusterProcessor.PlotterTool = PlotterTool
    
    HybridClusterProcessor.NumPreAllocatedDataHolders = flags.CaloRecGPU.ActiveConfig.NumPreAllocatedDataHolders

    if ReallyUseGPUTools:
    
      HybridClusterProcessor.ConstantDataToGPUTool = result.popToolsAndMerge( BasicConstantDataExporterToolCfg(flags) )
      HybridClusterProcessor.EventDataToGPUTool = result.popToolsAndMerge( BasicEventDataExporterToolCfg(flags, cellsname) )
      HybridClusterProcessor.GPUToEventDataTool = result.popToolsAndMerge( AthenaClusterAndMomentsImporterToolCfg(flags, cellsname, instantiateForTrigger) )
      
      
      HybridClusterProcessor.BeforeGPUTools = []

      HybridClusterProcessor.GPUTools = []

      HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"GPUGrowing"))]

      HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PostGPUGrowingClusterPropertiesCalculator", True))]

      HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitting") )]

      HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( ClusterSorterToolCfg(flags, "GPUClusterSorting", False) )]

      HybridClusterProcessor.GPUTools += [result.popToolsAndMerge( GPUClusterMomentsCalculatorToolCfg(flags,"GPUTopoMoments") )]
      
    else:
      
      HybridClusterProcessor.ConstantDataToGPUTool = None
      HybridClusterProcessor.EventDataToGPUTool = None
      HybridClusterProcessor.GPUToEventDataTool = None
      HybridClusterProcessor.SkipConversions = True
      
      HybridClusterProcessor.GPUTools = []
      
      HybridClusterProcessor.BeforeGPUTools = [] 
      
      HybridClusterProcessor.BeforeGPUTools += [result.popToolsAndMerge( DefaultTopologicalClusteringToolCfg(flags, cellsname,"CPUGrowing"))]

      HybridClusterProcessor.BeforeGPUTools += [result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"CPUSplitting") )]

      HybridClusterProcessor.BeforeGPUTools += [result.popToolsAndMerge( DefaultClusterMomentsCalculatorToolCfg(flags, instantiateForTrigger, "CPUTopoMoments") )]
      
      HybridClusterProcessor.BeforeGPUTools += [ CompFactory.CaloClusterStoreRawProperties("RawPropertiesStorer") ]
      
      

    HybridClusterProcessor.AfterGPUTools = []

    if not instantiateForTrigger:
      from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
      caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(flags) )
      HybridClusterProcessor.AfterGPUTools += [CompFactory.CaloClusterBadChannelList(badChannelTool = caloBadChanTool)]

    if not instantiateForTrigger and flags.CaloRecGPU.ActiveConfig.doCalibHitMoments:
    
        calibHitsMomentsMaker = result.popToolsAndMerge(DefaultCalibMomentsToolCfg(flags))
        HybridClusterProcessor.AfterGPUTools += [calibHitsMomentsMaker]
    
    if doLCCalib:
        if not instantiateForTrigger:
            HybridClusterProcessor.AfterGPUTools += [CompFactory.CaloClusterSnapshot(OutputName=clustersnapname,
                                                                                     SetCrossLinks=True,
                                                                                     FinalClusterContainerName=clustersname)]
        else:
          from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
          from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
          result.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))
          result.merge(caloTopoCoolFolderCfg(flags))
        
        calibTools = result.popToolsAndMerge(DefaultTopoClusterLocalCalibToolsCfg(flags, instantiateForTrigger, 'Trig' if instantiateForTrigger else ''))
        
        HybridClusterProcessor.AfterGPUTools += calibTools
        #This is already a tool array.
        
        if not instantiateForTrigger:
          from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
          result.merge(caloTopoCoolFolderCfg(flags))
    
    if instantiateForTrigger:
      from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
      result.merge(CaloNoiseCondAlgCfg(flags))
    
    result.addEventAlgo(HybridClusterProcessor, primary=addAsPrimary)

    if instantiateForTrigger or clustersname in flags.CaloRecGPU.ActiveConfig.skipWriteList:
        # don't add these clusters to ESD and AOD
        return result
    
    #Output config:
    AODMoments=[ "SECOND_R" 
                 ,"SECOND_LAMBDA"
                 ,"CENTER_MAG"
                 ,"CENTER_LAMBDA"
                 ,"FIRST_ENG_DENS"
                 ,"ENG_FRAC_MAX" 
                 ,"ISOLATION"
                 ,"ENG_BAD_CELLS"
                 ,"N_BAD_CELLS"
                 ,"BADLARQ_FRAC"
                 ,"ENG_POS"
                 ,"SIGNIFICANCE"
                 ,"AVG_LAR_Q"
                 ,"AVG_TILE_Q"
                 ,"EM_PROBABILITY"
                 ,"BadChannelList"
                 ,"SECOND_TIME"
                 ,"NCELL_SAMPLING"]

    if flags.CaloRecGPU.ActiveConfig.writeExtendedClusterMoments:
        AODMoments += ["LATERAL"
                       ,"LONGITUDINAL"
                       ,"CELL_SIGNIFICANCE"
                       ,"PTD"
                       ,"MASS"]

    if flags.Reco.EnableHI:
        AODMoments += ["CELL_SIG_SAMPLING"]

    if flags.CaloRecGPU.ActiveConfig.writeCalibHitClusterMoments:
        AODMoments += ["ENG_CALIB_TOT"
                       ,"ENG_CALIB_OUT_L"
                       ,"ENG_CALIB_OUT_T"
                       ,"ENG_CALIB_EMB0"
                       ,"ENG_CALIB_EME0"
                       ,"ENG_CALIB_TILEG3"
                       ,"ENG_CALIB_DEAD_TOT"
                       ,"ENG_CALIB_DEAD_EMB0"
                       ,"ENG_CALIB_DEAD_TILE0"
                       ,"ENG_CALIB_DEAD_TILEG3"
                       ,"ENG_CALIB_DEAD_EME0"
                       ,"ENG_CALIB_DEAD_HEC0"
                       ,"ENG_CALIB_DEAD_FCAL"
                       ,"ENG_CALIB_DEAD_LEAKAGE"
                       ,"ENG_CALIB_DEAD_UNCLASS"
                       ,"ENG_CALIB_FRAC_EM"
                       ,"ENG_CALIB_FRAC_HAD"
                       ,"ENG_CALIB_FRAC_REST"]


    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    toESD = [f"xAOD::CaloClusterContainer#{clustersname}",
             f"xAOD::CaloClusterAuxContainer#{clustersname}Aux.",
             f"CaloClusterCellLinkContainer#{clustersname}_links"]
    toAOD = [f"xAOD::CaloClusterContainer#{clustersname}",
             f"CaloClusterCellLinkContainer#{clustersname}_links"]

    AODMoments.append("CellLink") #Add data-link to cell-link container
    if flags.CaloRecGPU.ActiveConfig.addCalibrationHitDecoration: #Add calib hit deco if requried 
        AODMoments.append(flags.CaloRecGPU.ActiveConfig.CalibrationHitDecorationName)

    if flags.CaloRecGPU.ActiveConfig.addCPData:
        AODMoments += ["ClusterWidthEta","ClusterWidthPhi"]

    auxItems = f"xAOD::CaloClusterAuxContainer#{clustersname}Aux."
    auxItems+= ".".join(AODMoments)    

    toAOD.append(auxItems)
 
    result.merge(addToESD(flags, toESD))
    result.merge(addToAOD(flags, toAOD))
    
    
    return result
  
