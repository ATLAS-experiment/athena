# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from PlotterConfigurator import SingleToolToPlot, ComparedToolsToPlot
from CaloRecGPU.CaloRecGPUConfig import PlotterMonitoringToolCfg, DefaultCaloCellMakerCfg, AthenaClusterAndMomentsImporterToolCfg, BasicAthenaClusterImporterToolCfg, DefaultTopologicalClusteringToolCfg, CellsCounterCPUToolCfg, CPUOutputToolCfg, DefaultClusterSplittingToolCfg, DefaultClusterMomentsCalculatorToolCfg, MomentsDumperToolCfg, CaloClusterDeleterToolCfg, GPUOutputToolCfg, TopoAutomatonSplitterToolCfg, ClusterInfoCalcToolCfg, CellsCounterGPUToolCfg, TopoAutomatonClusteringToolCfg, GPUClusterMomentsCalculatorToolCfg

def PrevAlgorithmsConfigurationCfg(flags):
    result=ComponentAccumulator()
    
    if flags.Input.Format is Format.BS:
        #Data-case: Schedule ByteStream reading for LAr & Tile
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        result.merge(LArRawDataReadingCfg(flags))

        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        result.merge( TileRawDataReadingCfg(flags) )

        if flags.Output.doWriteESD:
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterOutputCfg
            result.merge(TileDigitsFilterOutputCfg(flags))
        else: #Mostly for wrapping in RecExCommon
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
            result.merge(TileDigitsFilterCfg(flags))

        from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
        result.merge(LArRawChannelBuilderAlgCfg(flags))

        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        result.merge(TileRawChannelMakerCfg(flags))

    if not flags.Input.isMC and not flags.Common.isOnline:
        from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
        result.merge(LArTimeVetoAlgCfg(flags))

    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
        result.merge(LArFebErrorSummaryMakerCfg(flags))
        
    if flags.Input.Format is Format.BS or 'StreamRDO' in flags.Input.ProcessingTags:
        result.merge(DefaultCaloCellMakerCfg(flags))
    elif flags.CaloRecGPU.ActiveConfig.FillMissingCells:
        from AthenaCommon.Logging import log
        log.warning("Asked to fill missing cells but will not run cell maker! Slow path might be taken!")

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    
    # Schedule total noise cond alg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))
    
    if not flags.Common.isOnline:
        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
        result.merge(LArElecCalibDBCfg(flags,["HVScaleCorr"]))
    
    return result
    


#TestGrow, TestSplit, TestMoments are self-explanatory.
#If `DoCrossTests` is True, outputs and/or plots
#are done considering all possible combinations
#of (GPU, CPU) x (Growing, Splitting)
#PlotterConfigurator takes the CaloGPUClusterAndCellDataMonitor
#and adds the appropriate plots to it.
def FullTestConfiguration(flags, TestGrow=False, TestSplit=False, TestMoments=False, DoCrossTests=False,
                          PlotterConfigurator=None, OutputCellInfo=False, SkipSyncs=True):
    
    
    if not (TestGrow and TestSplit):
        DoCrossTests = False
    
    result= PrevAlgorithmsConfigurationCfg(flags)
        
    GPUKernelSvc = CompFactory.GPUKernelSizeOptimizerSvc()
    result.addService(GPUKernelSvc)

    from CaloRecGPU.CaloRecGPUConfig import HybridClusterProcessorCfg
    result.merge(HybridClusterProcessorCfg(flags))
    hybridClusterProcessor = result.getEventAlgo("HybridClusterProcessor")

    if PlotterConfigurator is None:
        hybridClusterProcessor.DoPlots = False
    else:
        hybridClusterProcessor.DoPlots = True
        Plotter = PlotterConfigurator(result.popToolsAndMerge(PlotterMonitoringToolCfg(flags)))
        hybridClusterProcessor.PlotterTool = Plotter
        if TestGrow and not DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultGrowing", "CPU_growing") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostGrowing", "GPU_growing") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultGrowing", "PropCalcPostGrowing", "growing") ]
        if TestSplit and not DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultSplitting", "CPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostSplitting", "GPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcPostSplitting", "splitting", True) ]
        if TestMoments:
            Plotter.ToolsToPlot += [ SingleToolToPlot("CPUMoments", "CPU_moments") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("AthenaClusterImporter", "GPU_moments") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("CPUMoments", "AthenaClusterImporter", "moments", True) ]
        if DoCrossTests:
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultGrowing", "CPU_growing") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostGrowing", "GPU_growing") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultGrowing", "PropCalcPostGrowing", "growing") ]
            
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultSplitting", "CPUCPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostSplitting", "GPUGPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcPostSplitting", "CPU_to_GPUGPU_splitting", True) ]
            
            Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcDefaultGrowGPUSplit", "CPUGPU_splitting") ]
            Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultPostGPUSplitting", "GPUCPU_splitting") ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcDefaultGrowGPUSplit", "CPU_to_CPUGPU_splitting", True) ]
            Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "DefaultPostGPUSplitting", "CPU_to_GPUCPU_splitting", True) ]
            
        
    if flags.CaloRecGPU.DoMonitoring or PlotterConfigurator is not None:
        histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
        result.addService(histSvc)
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    
    result.merge(LArGMCfg(flags))
    result.merge(TileGMCfg(flags))
    
    AthenaClusterImporter = None
    
    if TestMoments:
         AthenaClusterImporter = result.popToolsAndMerge( AthenaClusterAndMomentsImporterToolCfg(flags,"AthenaClusterImporter") )
    else:
         AthenaClusterImporter = result.popToolsAndMerge( BasicAthenaClusterImporterToolCfg(flags,"AthenaClusterImporter") )
            
    hybridClusterProcessor.GPUToEventDataTool = AthenaClusterImporter
    
    hybridClusterProcessor.BeforeGPUTools = []
           
    if TestGrow or TestSplit or TestMoments:
        DefaultClustering = result.popToolsAndMerge( DefaultTopologicalClusteringToolCfg(flags,"DefaultGrowing") )
        
        hybridClusterProcessor.BeforeGPUTools += [DefaultClustering]
                    
        if TestGrow and (not TestSplit or DoCrossTests):
            if flags.CaloRecGPU.OutputCountsToFile:
                CPUCount1 = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,"DefaultGrowCounter","./counts", "default_grow") )
                hybridClusterProcessor.BeforeGPUTools += [CPUCount1]
            if flags.CaloRecGPU.OutputClustersToFile:
                CPUOut1 = result.popToolsAndMerge( CPUOutputToolCfg(flags,"DefaultGrowOutput","./out_default_grow") )
                hybridClusterProcessor.BeforeGPUTools += [CPUOut1]
        
            
        if TestSplit or TestMoments:
            FirstSplitter = result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"DefaultSplitting") )
            hybridClusterProcessor.BeforeGPUTools += [FirstSplitter]
            
            if flags.CaloRecGPU.OutputCountsToFile:
                CPUCount2 = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,"DefaultGrowAndSplitCounter","./counts", "default_grow_split") )
                hybridClusterProcessor.BeforeGPUTools += [CPUCount2]
            if flags.CaloRecGPU.OutputClustersToFile:
                CPUOut2 = result.popToolsAndMerge( CPUOutputToolCfg(flags,"DefaultSplitOutput", "./out_default_grow_split") )
                hybridClusterProcessor.BeforeGPUTools += [CPUOut2]
        
        if TestMoments:
            CPUMoments = result.popToolsAndMerge( DefaultClusterMomentsCalculatorToolCfg(flags,"CPUMoments") )
            hybridClusterProcessor.BeforeGPUTools += [CPUMoments]
            if flags.CaloRecGPU.OutputCountsToFile:
                CPUDumper = result.popToolsAndMerge( MomentsDumperToolCfg(flags,"./moments", "CPUMomentsDumper", "CPU") )
                hybridClusterProcessor.BeforeGPUTools += [CPUDumper]
        
        Deleter = result.popToolsAndMerge( CaloClusterDeleterToolCfg(flags))
        hybridClusterProcessor.BeforeGPUTools += [Deleter]                
        
        if TestSplit and (not TestGrow or DoCrossTests):
            SecondDefaultClustering = result.popToolsAndMerge( DefaultTopologicalClusteringToolCfg(flags,"SecondDefaultGrowing") )
            hybridClusterProcessor.BeforeGPUTools += [SecondDefaultClustering]
        
    hybridClusterProcessor.GPUTools = []
    
    if OutputCellInfo:
        CellOut = result.popToolsAndMerge( GPUOutputToolCfg(flags,"CellInfoOutput","./out_cell", OnlyOutputCells = True) )
        hybridClusterProcessor.GPUTools += [CellOut]
        
    if TestSplit:
        if not TestGrow:
            GPUClusterSplitting1 = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitter") )
            if SkipSyncs:
                GPUClusterSplitting1.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
            PropCalc1 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostSplitting", False) )
            if SkipSyncs:
                PropCalc1.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [PropCalc1]
        elif DoCrossTests:
            GPUClusterSplitting1 = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"FirstGPUSplitter") )
            GPUClusterSplitting1.TimeFileOutput = "" #This means there's no output.
            GPUClusterSplitting1.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [GPUClusterSplitting1]
            PropCalc1 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcDefaultGrowGPUSplit", False) )
            PropCalc1.TimeFileOutput = "" #This means there's no output.
            PropCalc1.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [PropCalc1]
        
        if ((not TestGrow) or DoCrossTests):
            if flags.CaloRecGPU.OutputCountsToFile:
                GPUCount1 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"DefaultGrowModifiedSplitCounter","./counts", "default_grow_modified_split") )
                hybridClusterProcessor.GPUTools += [GPUCount1]
            if flags.CaloRecGPU.OutputClustersToFile:
                GPUOut1 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"DefaultGrowModifiedSplitOutput","./out_default_grow_modified_split") )
                hybridClusterProcessor.GPUTools += [GPUOut1]
        
    if TestGrow:
        TopoAutomatonClustering1 = result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"GPUGrowing") )
        if SkipSyncs:
            TopoAutomatonClustering1.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [TopoAutomatonClustering1]
        
        PropCalc2 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostGrowing", True))
        if SkipSyncs:
            PropCalc2.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [PropCalc2]
        if ((not TestSplit) or DoCrossTests):
            if flags.CaloRecGPU.OutputCountsToFile:
                GPUCount2 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"ModifiedGrowCounter","./counts", "modified_grow") )
                hybridClusterProcessor.GPUTools += [GPUCount2]
            if flags.CaloRecGPU.OutputClustersToFile:
                GPUOut2 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"ModifiedGrowOutput","./out_modified_grow") )
                hybridClusterProcessor.GPUTools += [GPUOut2]
    
    if TestGrow and TestSplit:
        GPUClusterSplitting2 = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitter") )
        if SkipSyncs:
            GPUClusterSplitting2.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [GPUClusterSplitting2]
        
        if hybridClusterProcessor.DoPlots or not TestMoments:
          PropCalc3 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostSplitting", False) )
          if SkipSyncs:
              PropCalc3.MeasureTimes = False
          hybridClusterProcessor.GPUTools += [PropCalc3]
        
        if flags.CaloRecGPU.OutputCountsToFile:
            GPUCount3 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"ModifiedGrowSplitCounter","./counts", "modified_grow_split") )
            hybridClusterProcessor.GPUTools += [GPUCount3]
        if flags.CaloRecGPU.OutputClustersToFile:
            GPUOut3 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"ModifiedGrowSplitOutput","./out_modified_grow_split") )
            hybridClusterProcessor.GPUTools += [GPUOut3]
        
        if DoCrossTests:
            TopoAutomatonClustering2 = result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"SecondGPUGrowing") )
            TopoAutomatonClustering2.MeasureTimes = False
            TopoAutomatonClustering2.TimeFileOutput = "" #This means there's no output.
            hybridClusterProcessor.GPUTools += [TopoAutomatonClustering2]
            
            PropCalc4 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalc4", True) )
            PropCalc4.TimeFileOutput = "" #This means there's no output.
            PropCalc4.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [PropCalc4]
        
    if not (TestGrow or TestSplit):
        TopoAutomatonClusteringDef = result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"TopoAutomatonClustering") )
        if SkipSyncs:
            TopoAutomatonClusteringDef.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [TopoAutomatonClusteringDef]
        
        FirstPropCalcDef = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostGrowing", True) )
        if SkipSyncs:
            FirstPropCalcDef.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [FirstPropCalcDef]
        
        GPUClusterSplittingDef = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUTopoSplitter") )
        if SkipSyncs:
            GPUClusterSplittingDef.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
        
        if not TestMoments:
            SecondPropCalcDef = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostSplitting", False) )
            if SkipSyncs:
                SecondPropCalcDef.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [SecondPropCalcDef]
    
    if TestMoments and not DoCrossTests:
        if TestGrow and not TestSplit:
            GPUClusterSplittingDef = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUTopoSplitter") )
            if SkipSyncs:
                GPUClusterSplittingDef.MeasureTimes = False
            hybridClusterProcessor.GPUTools += [GPUClusterSplittingDef]
            
        GPUMomentsDef = result.popToolsAndMerge( GPUClusterMomentsCalculatorToolCfg(flags,"GPUTopoMoments") )
        if SkipSyncs:
            GPUMomentsDef.MeasureTimes = False
        hybridClusterProcessor.GPUTools += [GPUMomentsDef]
        
    hybridClusterProcessor.AfterGPUTools = []
    
    if TestMoments and flags.CaloRecGPU.OutputCountsToFile:
        GPUDumper = result.popToolsAndMerge( MomentsDumperToolCfg(flags,"./moments", "GPUMomentsDumper", "GPU") )
        hybridClusterProcessor.AfterGPUTools += [GPUDumper]
        
    if TestGrow and (not TestSplit or DoCrossTests):
    
        TopoSplitter = result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"DefaultPostGPUSplitting") )
        hybridClusterProcessor.AfterGPUTools += [TopoSplitter]
        
        if DoCrossTests:
            if flags.CaloRecGPU.OutputCountsToFile:
                CPUCount3 = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,"ModifiedGrowDefaultSplitCounter","./counts", "modified_grow_default_split") )
                hybridClusterProcessor.AfterGPUTools += [CPUCount3]
            if flags.CaloRecGPU.OutputClustersToFile:
                CPUOut3 = result.popToolsAndMerge( CPUOutputToolCfg(flags,"ModifiedGrowDefaultSplitOutput","./out_modified_grow_default_split") )
                hybridClusterProcessor.AfterGPUTools += [CPUOut3]
            
    result.addEventAlgo(hybridClusterProcessor,primary=True)

    return result
        
    
def PrepareTest(clustersname=None,default_files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"],
                parse_command_arguments = True,
                allocate_as_many_as_threads = True):

    import argparse
    
    args = None
    rest = None
    
    if parse_command_arguments:
        parser = argparse.ArgumentParser()
        
        parser.add_argument('-events','--numevents', type=int, default = 10)
        parser.add_argument('-threads','--numthreads', type=int, default = 1)
        parser.add_argument('-f','--files', action = 'extend', nargs = '*')
        parser.add_argument('-t','--measuretimes', action = 'store_true')
        parser.add_argument('-o','--outputclusters', action = 'store_true')
        parser.add_argument('-c','--outputcounts', action = 'store_true')
        parser.add_argument('-nfc','--notfillcells', action = 'store_true')
        
        parser.add_argument('-uoc','--useoriginalcriteria', action = 'store_true')
        
        parser.add_argument('-ndgn','--nodoublegaussiannoise', action = 'store_true')
       
        parser.add_argument('-m','--perfmon', action = 'store_true')
        parser.add_argument('-fm','--fullmon', action = 'store_true')
                   
        
        (args, pre_rest) = parser.parse_known_args()
                    
        if pre_rest is None or len(pre_rest) == 0:
            rest = ['--threads', '1']
            #Crude workaround for a condition within ConfigFlags.fillFromArgs
            #that would make it inspect the whole command-line when provided
            #with an empty list of arguments.
            #(I'd personally suggest changing the "listOfArgs or sys.argv[1:]"
            # used there since empty arrays are falsy while being non-null,
            # to a "sys.argv[1:] if listofArgs is None else listofArgs",
            # but I'm not about to suggest potentially code-breaking behaviour changes
            # when I can work around them easily, even if rather inelegantly...)
        else:
            rest = pre_rest
    
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior=1

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    #from AthenaConfiguration.TestDefaults import defaultTestFiles
                    
    flags = initConfigFlags()
    
    if parse_command_arguments:
        flags.fillFromArgs(listOfArgs=rest)
        #We could instead use our parser to overload here and so on, but...
    
    if args is None or args.files is None:
        flags.Input.Files = default_files
    elif len(args.files) == 0:
        flags.Input.Files = default_files
    elif len(args.files) == 1:
        if args.files[0] == 'default':
            flags.Input.Files = default_files
        elif args.files[0] == 'ttbar':
            flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000001.pool.root.1",
                                       "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000002.pool.root.1",
                                       "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigInDetValidation/samples/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.RDO.e3698_s2608_s2183_r7195/RDO.06752780._000003.pool.root.1" ]
            
        elif args.files[0] == 'jets':
            flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"]
        else:
            flags.Input.Files = args.files
    else:
        flags.Input.Files = args.files

    perfmon=False
    if parse_command_arguments:
        if args.perfmon or args.fullmon:
           perfmon=True

    if parse_command_arguments:
        flags.Concurrency.NumThreads = int(args.numthreads)
        flags.Concurrency.NumConcurrentEvents = int(args.numthreads)
        #This is to ensure the measurments are multi-threaded in the way we expect, I guess?
        flags.PerfMon.doFastMonMT = args.perfmon
        flags.PerfMon.doFullMonMT = args.fullmon
        # configure GPU
        flags.CaloRecGPU.Default.MeasureTimes = args.measuretimes
        flags.CaloRecGPU.Default.OutputClustersToFile = args.outputclusters
        flags.CaloRecGPU.Default.OutputCountsToFile = args.outputcounts
        flags.CaloRecGPU.Default.FillMissingCells = not args.notfillcells
        flags.CaloRecGPU.Default.UseOriginalCriteria = args.useoriginalcriteria
        flags.CaloRecGPU.Default.TwoGaussianNoise = not args.nodoublegaussiannoise
        if allocate_as_many_as_threads:
            flags.CaloRecGPU.Default.NumPreAllocatedDataHolders = int(args.numthreads)

    flags.CaloRecGPU.Default.MissingCellsToFill = [186986, 187352]

    if clustersname is not None:
        flags.CaloRecGPU.Default.ClustersOutputName = clustersname
    
    if parse_command_arguments:
        return (flags, perfmon,int(args.numevents))
    else:
        return (flags, perfmon, -1)

def MinimalSetup(flags,perfmon):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    
    if perfmon:
       from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
       cfg.merge(PerfMonMTSvcCfg(flags))
            
    
    if 'StreamRDO' in flags.Input.ProcessingTags:
        cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(),sequenceName="AthAlgSeq")

    return cfg
    
    

