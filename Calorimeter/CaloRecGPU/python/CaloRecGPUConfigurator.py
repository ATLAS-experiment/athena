# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import MeV, ns, cm, deg

def MatchingOptions(min_similarity = 0.50, terminal_weight = 250., grow_weight = 500., seed_weight = 1000.):
    return (min_similarity, terminal_weight, grow_weight, seed_weight)

class CaloRecGPUConfigurator:
    def __init__ (self, configFlags = None, cellsname ="AllCalo"):
        self.ConfigFlags = configFlags
        self.CellsName = cellsname
        
        self.TopoClusterSNRSeedThreshold = 4.0
        self.TopoClusterSNRGrowThreshold = 2.0
        self.TopoClusterSNRCellThreshold = 0.0
        self.ClusterSize = "Topo_420"
        
        self.TopoClusterSeedCutsInAbsE = True
        self.TopoClusterNeighborCutsInAbsE = True
        self.TopoClusterCellCutsInAbsE = True
        
        self.CalorimeterNames = ["LAREM",
                                 "LARHEC",
                                 "LARFCAL",
                                 "TILE"]
        self.TopoClusterSeedSamplingNames = ["PreSamplerB", "EMB1", "EMB2", "EMB3",
                                  "PreSamplerE", "EME1", "EME2", "EME3",
                                  "HEC0", "HEC1","HEC2", "HEC3",
                                  "TileBar0", "TileBar1", "TileBar2",
                                  "TileExt0", "TileExt1", "TileExt2",
                                  "TileGap1", "TileGap2", "TileGap3",
                                  "FCAL0", "FCAL1", "FCAL2"]
                                  
        self.NeighborOption = "super3D"
        self.RestrictHECIWandFCalNeighbors  = False
        self.RestrictPSNeighbors  = True
        self.AlsoRestrictPSOnGPUSplitter = False
        #This is to override vanilla behaviour with the possibility of also
        #restricting neighbours in out GPU splitter.

        if configFlags is not None:
            configFlags.lock()
        
        if configFlags is not None:
            self.SeedCutsInT = configFlags.Calo.TopoCluster.doTimeCut
            self.CutOOTseed = configFlags.Calo.TopoCluster.extendTimeCut and configFlags.Calo.TopoCluster.doTimeCut
            self.UseTimeCutUpperLimit = configFlags.Calo.TopoCluster.useUpperLimitForTimeCut
        else:
            self.SeedCutsInT = False
            self.CutOOTseed = False
            self.UseTimeCutUpperLimit = False
        
        self.TimeCutUpperLimit = 20.0
        self.TreatL1PredictedCellsAsGood = True
        
        self.UseEM2CrossTalk = False
        self.CrossTalkDeltaT = 15 * ns
        
        self.SeedThresholdOnTAbs = 12.5 * ns
        
        self.CutClustersInAbsEt = None
        self.ClusterEtorAbsEtCut = -1e-16*MeV
        
        #Since we (by default) use absolute values,
        #a cut off of 0 MeV could mean some clusters
        #where the cells have just the right combination
        #of energies could sum down to 0 on one side (CPU/GPU)
        #and not on the other. This is actually valid
        #for any threshold, but this way we accept all clusters...
        
        if configFlags is not None:
            self.TwoGaussianNoise = configFlags.Calo.TopoCluster.doTwoGaussianNoise
        else:
            self.TwoGaussianNoise = True
        
        
        self.SplitterNumberOfCellsCut = 4
        self.SplitterEnergyCut = 500 * MeV
        self.SplitterSamplingNames = ["EMB2", "EMB3",
                                      "EME2", "EME3",
                                      "FCAL0"]
        self.SplitterSecondarySamplingNames = ["EMB1","EME1",
                                               "TileBar0","TileBar1","TileBar2",
                                               "TileExt0","TileExt1","TileExt2",
                                               "HEC0","HEC1","HEC2","HEC3",
                                               "FCAL1","FCAL2"]
        self.SplitterShareBorderCells = True
        
        self.EMShowerScale = 5.0 * cm
        
        if configFlags is not None:
            self.SplitterUseNegativeClusters = configFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
        else:
            self.SplitterUseNegativeClusters = False
         
        if configFlags is not None:
            self.UseAbsEnergyMoments = configFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
        else:
            self.UseAbsEnergyMoments = False
        
        self.MomentsMaxAxisAngle = 20 * deg
        
        self.MomentsMinBadLArQuality = 4000
        
        self.MomentsToCalculate = [ "FIRST_PHI",
                                    "FIRST_ETA",
                                    "SECOND_R",
                                    "SECOND_LAMBDA",
                                    "DELTA_PHI",
                                    "DELTA_THETA",
                                    "DELTA_ALPHA",
                                    "CENTER_X",
                                    "CENTER_Y",
                                    "CENTER_Z",
                                    "CENTER_MAG",
                                    "CENTER_LAMBDA",
                                    "LATERAL",
                                    "LONGITUDINAL",
                                    "ENG_FRAC_EM",
                                    "ENG_FRAC_MAX",
                                    "ENG_FRAC_CORE",
                                    "FIRST_ENG_DENS",
                                    "SECOND_ENG_DENS",
                                    "ISOLATION",
                                    "ENG_BAD_CELLS",
                                    "N_BAD_CELLS",
                                    "N_BAD_CELLS_CORR",
                                    "BAD_CELLS_CORR_E",
                                    "BADLARQ_FRAC",
                                    "ENG_POS",
                                    "SIGNIFICANCE",
                                    "CELL_SIGNIFICANCE",
                                    "CELL_SIG_SAMPLING",
                                    "AVG_LAR_Q",
                                    "AVG_TILE_Q",
                                    "PTD",
                                    "MASS",
                                    "SECOND_TIME",
                                    "NCELL_SAMPLING" ]
        
        self.MomentsMinRLateral = 4 * cm
        self.MomentsMinLLongitudinal = 10 * cm
        
        if configFlags is not None:
            if not configFlags.Common.isOnline:
                self.MomentsToCalculate += ["ENG_BAD_HV_CELLS",
                                            "N_BAD_HV_CELLS"  ]
                
        self.ClustersOutputName = "Clusters"
        
        self.MeasureTimes = False
        self.OutputCountsToFile = False
        self.OutputClustersToFile = False
        
        self.DoMonitoring = False
                
        self.NumPreAllocatedDataHolders = 0

        self.FillMissingCells = True
        #We want a full (and ordered) cell container
        #to get to the fast path.
        
        self.MissingCellsToFill = []
        
        self.UseOriginalCriteria = False
        #If True, use the original criteria
        #(which disagree with the GPU implementation)
                    
    def BasicConstantDataExporterToolConf(self, name = "ConstantDataExporter"):
        result=ComponentAccumulator()
        ConstantDataExporter = CompFactory.BasicConstantGPUDataExporter(name)
        ConstantDataExporter.MeasureTimes = False
        ConstantDataExporter.TimeFileOutput = "ConstantDataExporterTimes.txt"
        result.setPrivateTools(ConstantDataExporter)
        return result

    def BasicEventDataExporterToolConf(self, name = "EventDataExporter"):
        result=ComponentAccumulator()
        EventDataExporter = CompFactory.BasicEventDataGPUExporter(name)
        EventDataExporter.MeasureTimes = self.MeasureTimes
        EventDataExporter.TimeFileOutput = "EventDataExporterTimes.txt"
        EventDataExporter.CellsName = self.CellsName
        if self.FillMissingCells:
            EventDataExporter.MissingCellsToFill = self.MissingCellsToFill
        result.setPrivateTools(EventDataExporter)
        return result

    def BasicAthenaClusterImporterToolConf(self, name = "AthenaClusterImporter"):
        result=ComponentAccumulator()
        AthenaClusterImporter = CompFactory.BasicGPUToAthenaImporter(name)
        AthenaClusterImporter.MeasureTimes = self.MeasureTimes
        AthenaClusterImporter.TimeFileOutput = "ClusterImporterTimes.txt"
        AthenaClusterImporter.CellsName = self.CellsName
        AthenaClusterImporter.ClusterSize = self.ClusterSize
        if self.FillMissingCells:
            AthenaClusterImporter.MissingCellsToFill = self.MissingCellsToFill
        result.setPrivateTools(AthenaClusterImporter)
        return result

    def CaloClusterDeleterToolConf(self, name = "ClusterDeleter"):
        result=ComponentAccumulator()
        ClusterDeleter = CompFactory.CaloClusterDeleter(name)
        result.setPrivateTools(ClusterDeleter)
        return result
        
    def CPUOutputToolConf(self, folder = "output", name = "CPUOutput", prefix = "", suffix = ""):
        result=ComponentAccumulator()
        CPUOutput = CompFactory.CaloCPUOutput(name)
        CPUOutput.SavePath = folder
        CPUOutput.FilePrefix = prefix
        CPUOutput.FileSuffix = suffix
        CPUOutput.CellsName = self.CellsName
        result.setPrivateTools(CPUOutput)
        return result

    def GPUOutputToolConf(self, folder = "output", name = "GPUOutput", prefix = "", suffix = "", OnlyOutputCells = None):
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
        
    def ClusterInfoCalcToolConf(self, name = "GPUClusterInfoCalculator", do_cut = True):
        result=ComponentAccumulator()
        CalcTool = CompFactory.BasicGPUClusterInfoCalculator(name)
        CalcTool.MeasureTimes = self.MeasureTimes
        CalcTool.TimeFileOutput = name + "Times.txt"
        if do_cut:
            if self.CutClustersInAbsEt is None:
                CalcTool.ClusterCutsInAbsEt = self.TopoClusterSeedCutsInAbsE
            else:
                CalcTool.ClusterCutsInAbsEt = self.CutClustersInAbsEt
            CalcTool.ClusterEtorAbsEtCut = self.ClusterEtorAbsEtCut
        else:
            CalcTool.ClusterCutsInAbsEt = True
            CalcTool.ClusterEtorAbsEtCut = -1
            #Cutting on absolute value with a negative value => not cutting at all.
            
        result.setPrivateTools(CalcTool)
        return result
                
    def TopoAutomatonClusteringToolConf(self, name = "TAClusterMaker"):
        result=ComponentAccumulator()
        # maker tools
        TAClusterMaker = CompFactory.TopoAutomatonClustering(name)

        TAClusterMaker.MeasureTimes = self.MeasureTimes
        TAClusterMaker.TimeFileOutput = "TopoAutomatonClusteringTimes.txt"

        TAClusterMaker.CalorimeterNames= self.CalorimeterNames
        
        TAClusterMaker.SeedSamplingNames = self.TopoClusterSeedSamplingNames
        
        TAClusterMaker.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        TAClusterMaker.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        TAClusterMaker.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        TAClusterMaker.SeedCutsInAbsE = self.TopoClusterSeedCutsInAbsE
        TAClusterMaker.NeighborCutsInAbsE = self.TopoClusterNeighborCutsInAbsE
        TAClusterMaker.CellCutsInAbsE = self.TopoClusterCellCutsInAbsE
        
        TAClusterMaker.TwoGaussianNoise = self.TwoGaussianNoise
                
                
        TAClusterMaker.SeedCutsInT = self.SeedCutsInT
        TAClusterMaker.CutOOTseed = self.CutOOTseed
        TAClusterMaker.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        TAClusterMaker.TimeCutUpperLimit = self.TimeCutUpperLimit
        TAClusterMaker.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        TAClusterMaker.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        TAClusterMaker.XTalkEM2 = self.UseEM2CrossTalk
        TAClusterMaker.XTalkDeltaT = self.CrossTalkDeltaT
        
        TAClusterMaker.NeighborOption = self.NeighborOption
        TAClusterMaker.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        TAClusterMaker.RestrictPSNeighbors  = self.RestrictPSNeighbors
        
        result.setPrivateTools(TAClusterMaker)
        return result
        
    def DefaultTopologicalClusteringToolConf(self, name = "TopoMaker"):
        result=ComponentAccumulator()
        # maker tools
        TopoMaker = CompFactory.CaloTopoClusterMaker(name)

        TopoMaker.CellsName = self.CellsName
        TopoMaker.CalorimeterNames= self.CalorimeterNames
        TopoMaker.SeedSamplingNames = self.TopoClusterSeedSamplingNames
        TopoMaker.NeighborOption = self.NeighborOption
        TopoMaker.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        TopoMaker.RestrictPSNeighbors  = self.RestrictPSNeighbors
        TopoMaker.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        TopoMaker.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        TopoMaker.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold

        TopoMaker.SeedCutsInT = self.SeedCutsInT
        TopoMaker.CutOOTseed = self.CutOOTseed
        TopoMaker.UseTimeCutUpperLimit = self.UseTimeCutUpperLimit
        TopoMaker.TimeCutUpperLimit = self.TimeCutUpperLimit

        TopoMaker.ClusterEtorAbsEtCut  = self.ClusterEtorAbsEtCut
        TopoMaker.TwoGaussianNoise = self.TwoGaussianNoise
        TopoMaker.SeedCutsInAbsE = self.TopoClusterSeedCutsInAbsE
        TopoMaker.NeighborCutsInAbsE = self.TopoClusterNeighborCutsInAbsE
        TopoMaker.CellCutsInAbsE = self.TopoClusterCellCutsInAbsE
        TopoMaker.SeedThresholdOnTAbs = self.SeedThresholdOnTAbs
        
        TopoMaker.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        TopoMaker.UseGPUCriteria = not self.UseOriginalCriteria
        
        TopoMaker.XTalkEM2 = self.UseEM2CrossTalk
        TopoMaker.XTalkDeltaT = self.CrossTalkDeltaT
        
        result.setPrivateTools(TopoMaker)
        return result
        
        
    def TopoAutomatonSplitterToolConf(self, name = "TopoAutomatonSplitter"):
        result=ComponentAccumulator()
        # maker tools
        Splitter = CompFactory.TopoAutomatonSplitting(name)

        Splitter.MeasureTimes = self.MeasureTimes
        Splitter.TimeFileOutput = "ClusterSplitterTimes.txt"
        
        Splitter.NumberOfCellsCut = self.SplitterNumberOfCellsCut
        Splitter.EnergyCut = self.SplitterEnergyCut
        Splitter.SamplingNames = self.SplitterSamplingNames
        Splitter.SecondarySamplingNames = self.SplitterSecondarySamplingNames
        Splitter.ShareBorderCells = self.SplitterShareBorderCells
        Splitter.EMShowerScale = self.EMShowerScale
        Splitter.WeightingOfNegClusters = self.SplitterUseNegativeClusters
                
        Splitter.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        Splitter.NeighborOption = self.NeighborOption
        Splitter.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        Splitter.RestrictPSNeighbors = self.RestrictPSNeighbors and self.AlsoRestrictPSOnGPUSplitter
        #Since the CPU version does not restrict this!
        
        result.setPrivateTools(Splitter)
        return result
        
    def GPUClusterSplitterToolConf(self, name = "GPUClusterSplitter"):
        result=ComponentAccumulator()
        # maker tools
        Splitter = CompFactory.CaloTopoClusterSplitterGPU(name)

        Splitter.MeasureTimes = self.MeasureTimes
        Splitter.TimeFileOutput = "ClusterSplitterTimes.txt"
        
        Splitter.NumberOfCellsCut = self.SplitterNumberOfCellsCut
        Splitter.EnergyCut = self.SplitterEnergyCut
        Splitter.SamplingNames = self.SplitterSamplingNames
        Splitter.SecondarySamplingNames = self.SplitterSecondarySamplingNames
        Splitter.ShareBorderCells = self.SplitterShareBorderCells
        Splitter.EMShowerScale = self.EMShowerScale
        Splitter.WeightingOfNegClusters = self.SplitterUseNegativeClusters
                
        Splitter.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        result.setPrivateTools(Splitter)
        return result
        
    def DefaultClusterSplittingToolConf(self, name = "TopoSplitter"):        
        result=ComponentAccumulator()
        # maker tools
        TopoSplitter = CompFactory.CaloTopoClusterSplitter(name)

        
        TopoSplitter.NeighborOption = self.NeighborOption
        TopoSplitter.RestrictHECIWandFCalNeighbors  = self.RestrictHECIWandFCalNeighbors
        
        TopoSplitter.NumberOfCellsCut = self.SplitterNumberOfCellsCut
        TopoSplitter.EnergyCut = self.SplitterEnergyCut
        
        TopoSplitter.SamplingNames = self.SplitterSamplingNames
        TopoSplitter.SecondarySamplingNames = self.SplitterSecondarySamplingNames
        
        TopoSplitter.ShareBorderCells = self.SplitterShareBorderCells
        TopoSplitter.EMShowerScale = self.EMShowerScale
        
        TopoSplitter.TreatL1PredictedCellsAsGood = self.TreatL1PredictedCellsAsGood
        
        TopoSplitter.WeightingOfNegClusters = self.SplitterUseNegativeClusters
        
        TopoSplitter.UseGPUCriteria = not self.UseOriginalCriteria
        
        result.setPrivateTools(TopoSplitter)
        return result
      
    def GPUClusterMomentsCalculatorToolConf(self, name = "GPUTopoMoments"):
        
        result=ComponentAccumulator()
        GPUTopoMoments = CompFactory.GPUClusterInfoAndMomentsCalculator(name)
        
        GPUTopoMoments.MeasureTimes = self.MeasureTimes
        
        if self.UseAbsEnergyMoments is None:
            GPUTopoMoments.WeightingOfNegClusters = self.TopoClusterSeedCutsInAbsE
        else:
            GPUTopoMoments.WeightingOfNegClusters = self.UseAbsEnergyMoments
            
        GPUTopoMoments.MaxAxisAngle = self.MomentsMaxAxisAngle
        
        GPUTopoMoments.TwoGaussianNoise = self.TwoGaussianNoise
        
        GPUTopoMoments.MinBadLArQuality = self.MomentsMinBadLArQuality
        
        GPUTopoMoments.MinRLateral = self.MomentsMinRLateral
        GPUTopoMoments.MinLLongitudinal = self.MomentsMinLLongitudinal
                            
        result.setPrivateTools(GPUTopoMoments)
        return result
        
        
    def DefaultClusterMomentsCalculatorToolConf(self, name = "TopoMoments"):
        result=ComponentAccumulator()
        TopoMoments = CompFactory.CaloClusterMomentsMaker(name)
        
        if self.UseAbsEnergyMoments is None:
            TopoMoments.WeightingOfNegClusters = self.TopoClusterSeedCutsInAbsE
        else:
            TopoMoments.WeightingOfNegClusters = self.UseAbsEnergyMoments
            
        TopoMoments.MaxAxisAngle = self.MomentsMaxAxisAngle
        
        TopoMoments.TwoGaussianNoise = self.TwoGaussianNoise
        
        TopoMoments.MinBadLArQuality = self.MomentsMinBadLArQuality
        
        TopoMoments.MomentsNames = self.MomentsToCalculate
        
        TopoMoments.MinRLateral = self.MomentsMinRLateral
        TopoMoments.MinLLongitudinal = self.MomentsMinLLongitudinal

        if not self.ConfigFlags.Common.isOnline:
            if self.ConfigFlags.Input.isMC:
                TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorr")
            else:
                TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorrRecomputed")
        
        TopoMoments.UseGPUCriteria = not self.UseOriginalCriteria
        
        result.setPrivateTools(TopoMoments)
        return result
        
    def AthenaClusterAndMomentsImporterToolConf(self, name = "AthenaClusterImporter"):
        result=ComponentAccumulator()
        AthenaClusterImporter = CompFactory.GPUToAthenaImporterWithMoments(name)
        AthenaClusterImporter.CellsName = self.CellsName
        AthenaClusterImporter.ClusterSize = self.ClusterSize
        
        AthenaClusterImporter.MeasureTimes = self.MeasureTimes
        AthenaClusterImporter.TimeFileOutput = "ClusterAndMomentsImporterTimes.txt"
        
        
                
        if not self.ConfigFlags.Common.isOnline:
            if self.ConfigFlags.Input.isMC:
                AthenaClusterImporter.HVScaleCorrKey = "LArHVScaleCorr"
            else:
                AthenaClusterImporter.HVScaleCorrKey = "LArHVScaleCorrRecomputed"
        
        AthenaClusterImporter.MomentsNames = self.MomentsToCalculate
        
        if self.FillMissingCells:
            AthenaClusterImporter.MissingCellsToFill = self.MissingCellsToFill
            
        result.setPrivateTools(AthenaClusterImporter)
        return result
        
    def CellsCounterCPUToolConf(self, folder = "counts", name = "CPUCounts", prefix = "CPU", suffix = ""):
        result=ComponentAccumulator()
        CPUCount = CompFactory.CaloCellsCounterCPU(name)
        CPUCount.SavePath = folder
        CPUCount.FilePrefix = prefix
        CPUCount.FileSuffix = suffix
        CPUCount.CellsName = self.CellsName
        
        CPUCount.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        CPUCount.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        CPUCount.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        result.setPrivateTools(CPUCount)
        return result
        
    def CellsCounterGPUToolConf(self, folder = "counts", name = "GPUCounts", prefix = "GPU", suffix = ""):
        result=ComponentAccumulator()
        GPUCount = CompFactory.CaloCellsCounterGPU(name)
        GPUCount.SavePath = folder
        GPUCount.FilePrefix = prefix
        GPUCount.FileSuffix = suffix
        
        GPUCount.CellThresholdOnEorAbsEinSigma = self.TopoClusterSNRCellThreshold
        GPUCount.NeighborThresholdOnEorAbsEinSigma = self.TopoClusterSNRGrowThreshold
        GPUCount.SeedThresholdOnEorAbsEinSigma = self.TopoClusterSNRSeedThreshold
        
        result.setPrivateTools(GPUCount)
        return result
        
    def MomentsDumperToolConf(self, folder = "moments", name = "MomentsDumper", prefix = "", suffix = ""):
        result=ComponentAccumulator()
        GPUCount = CompFactory.CaloMomentsDumper(name)
        GPUCount.SavePath = folder
        GPUCount.FilePrefix = prefix
        GPUCount.FileSuffix = suffix
        
        result.setPrivateTools(GPUCount)
        return result
        
    def PlotterMonitoringToolConf(self, name = "PlotterMonitoring"):
        result=ComponentAccumulator()
        PloTool = CompFactory.CaloGPUClusterAndCellDataMonitor(name)
        
        PloTool.CellThreshold = self.TopoClusterSNRCellThreshold
        PloTool.NeighborThreshold = self.TopoClusterSNRGrowThreshold
        PloTool.SeedThreshold = self.TopoClusterSNRSeedThreshold
        
        PloTool.CellsName = self.CellsName
        
        PloTool.ClusterMatchingParameters = MatchingOptions()
        
        #Tools and Combinations to plot
        #should be set by the end user.
        
        from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
        
        PloTool.MonitoringTool = GenericMonitoringTool(self.ConfigFlags, "PlotterMonitoringTool")
        
        result.setPrivateTools(PloTool)
        return result
    
    def MonitorizationToolConf(self, name = "MonTool"):       
        from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
        
        monTool = GenericMonitoringTool(self.ConfigFlags, name)
        
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
        
    def DefaultCaloCellMakerConf(self, should_be_primary = False):
        from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
        from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
        from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg
        result=ComponentAccumulator()
       
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg
        
        result.merge(LArGMCfg(self.ConfigFlags))
        result.merge(TileGMCfg(self.ConfigFlags))

        larCellBuilder     = result.popToolsAndMerge(LArCellBuilderCfg(self.ConfigFlags))
        larCellCorrectors  = result.popToolsAndMerge(LArCellCorrectorCfg(self.ConfigFlags))
        tileCellBuilder = result.popToolsAndMerge(TileCellBuilderCfg(self.ConfigFlags))
        cellFinalizer  = CompFactory.CaloCellContainerFinalizerTool()
        
        if self.FillMissingCells:
            tileCellBuilder.fakeCrackCells = True
        
        cellMakerTools=[larCellBuilder,tileCellBuilder,cellFinalizer]+larCellCorrectors
        
        #Add corrections tools that are not LAr or Tile specific:
        if self.ConfigFlags.Calo.Cell.doPileupOffsetBCIDCorr or self.ConfigFlags.Cell.doPedestalCorr:
            theCaloCellPedestalCorr=CaloCellPedestalCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellPedestalCorr))

        #LAr HV scale corr must come after pedestal corr
        if self.ConfigFlags.LAr.doHVCorr:
            from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
            theLArHVCellContCorr=LArHVCellContCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theLArHVCellContCorr))


        if self.ConfigFlags.Calo.Cell.doDeadCellCorr:
            theCaloCellNeighborAvg=CaloCellNeighborsAverageCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellNeighborAvg))

        if self.ConfigFlags.Calo.Cell.doEnergyCorr:
            theCaloCellEnergyRescaler=CaloEnergyRescalerCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloCellEnergyRescaler))
        if self.ConfigFlags.Calo.Cell.doTimeCorr:
            theCaloTimeCorr=CaloCellTimeCorrCfg(self.ConfigFlags)
            cellMakerTools.append(result.popToolsAndMerge(theCaloTimeCorr))

        cellAlgo=CompFactory.CaloCellMaker(CaloCellMakerToolNames = cellMakerTools,
                                           CaloCellsOutputName = self.CellsName)
        result.addEventAlgo(cellAlgo, primary = should_be_primary)
        return result
        
    #This simply uses the GPU versions.
    #For the tests, we will build our own
    #depending on what we want to compare against.
    def HybridClusterProcessorConf(self, clustersname = None, should_be_primary = True):
        result = ComponentAccumulator()
        
        HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor("HybridClusterProcessor")
        HybridClusterProcessor.ClustersOutputName = self.ClustersOutputName
        HybridClusterProcessor.MeasureTimes = self.MeasureTimes
        HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
        HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
        HybridClusterProcessor.DoPlots = False
        HybridClusterProcessor.PlotterTool = None
        HybridClusterProcessor.DoMonitoring = self.DoMonitoring
        
        if self.DoMonitoring:
            histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
            result.addService(histSvc)
            HybridClusterProcessor.MonitoringTool = self.MonitorizationToolConf()
        
        HybridClusterProcessor.NumPreAllocatedDataHolders = self.NumPreAllocatedDataHolders
        
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        from TileGeoModel.TileGMConfig import TileGMCfg
        
        result.merge(LArGMCfg(self.ConfigFlags))
        result.merge(TileGMCfg(self.ConfigFlags))
        
        ConstantDataExporter = result.popToolsAndMerge( self.BasicConstantDataExporterToolConf() )
        EventDataExporter = result.popToolsAndMerge( self.BasicEventDataExporterToolConf() )
        AthenaClusterImporter = result.popToolsAndMerge( self.AthenaClusterAndMomentsImporterToolConf() )
        
        HybridClusterProcessor.ConstantDataToGPUTool = ConstantDataExporter
        HybridClusterProcessor.EventDataToGPUTool = EventDataExporter
        HybridClusterProcessor.GPUToEventDataTool = AthenaClusterImporter
        
        HybridClusterProcessor.BeforeGPUTools = []
       
        HybridClusterProcessor.GPUTools = []
        
        TopoAutomatonClusteringDef = result.popToolsAndMerge( self.TopoAutomatonClusteringToolConf("TopoAutomatonClustering"))
        HybridClusterProcessor.GPUTools += [TopoAutomatonClusteringDef]
        
        FirstPropCalcDef = result.popToolsAndMerge( self.ClusterInfoCalcToolConf("PostGrowGPUClusterPropertiesCalculator", True))
        HybridClusterProcessor.GPUTools += [FirstPropCalcDef]
        
        GPUClusterSplittingDef = result.popToolsAndMerge( self.TopoAutomatonSplitterToolConf("GPUSplitter") )
        HybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
        
        GPUMomentsCalcDef = result.popToolsAndMerge( self.GPUClusterMomentsCalculatorToolConf("GPUTopoMoments") )
        HybridClusterProcessor.GPUTools += [GPUMomentsCalcDef]
            
        HybridClusterProcessor.AfterGPUTools = []
        
        from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
        caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(self.ConfigFlags) )
        CaloClusterBadChannelList=CompFactory.CaloClusterBadChannelList
        BadChannelListCorr = CaloClusterBadChannelList(badChannelTool = caloBadChanTool)
        HybridClusterProcessor.AfterGPUTools += [BadChannelListCorr]

            
        if self.ConfigFlags.Calo.TopoCluster.doTopoClusterLocalCalib:    
            #Took out CaloClusterSnapshot that wanted to be a part of a CaloClusterMaker.
            #Possibly change in the future?
            from CaloRec.CaloTopoClusterConfig import getTopoClusterLocalCalibTools
            HybridClusterProcessor.AfterGPUTools += getTopoClusterLocalCalibTools(self.ConfigFlags)
        
            from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
            result.merge(caloTopoCoolFolderCfg(self.ConfigFlags))

        result.addEventAlgo(HybridClusterProcessor, primary = should_be_primary)

        return result
        




