# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from PathResolver import PathResolver
import importlib
import os

log = AthenaLogger(__name__)

#### Now inmport Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig

def getNSubregions(filePath):
    with open(PathResolver.FindCalibFile(filePath), 'r') as f:
        fields = f.readline()
        assert(fields.startswith('towers'))
        n = fields.split()[1]
        return int(n)


# Need to figure out if we have two output writers or somehow only one.
def FPGATrackSimWriteOutputCfg(flags):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool("FPGATrackSimWriteOutput")
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    FPGATrackSimWriteOutput.OutputTreeName = FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimLogicalEventTree")
    if not flags.Trigger.FPGATrackSim.writeAdditionalOutputData:
        FPGATrackSimWriteOutput.EventLimit = 0
    else:
        FPGATrackSimWriteOutput.EventLimit = flags.Trigger.FPGATrackSim.writeOutputEventLimit
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(FPGATrackSimWriteOutput)
    return result

def FPGATrackSimSlicingEngineCfg(flags,name="FPGATrackSimSlicingEngineTool"):
    result = ComponentAccumulator()
    FPGATrackSimSlicingEngineTool = CompFactory.FPGATrackSimSlicingEngineTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    # this is the same as the layer map used by the genscan/inside out tool below.
    FPGATrackSimSlicingEngineTool.LayerMap =  os.path.join(PathResolver.FindCalibDirectory(flags.Trigger.FPGATrackSim.mapsDir),f"{FPGATrackSimDataPrepConfig.getBaseName(flags)}_lyrmap.json")
    FPGATrackSimSlicingEngineTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    # If the GNN is enabled (i.e. this is F-4xx) then we don't want to separate first vs second stage.
    FPGATrackSimSlicingEngineTool.doSecondStage = (not flags.Trigger.FPGATrackSim.ActiveConfig.GNN)
    FPGATrackSimSlicingEngineTool.RootOutput = flags.Trigger.FPGATrackSim.writeAdditionalOutputData
    result.setPrivateTools(FPGATrackSimSlicingEngineTool)
    return result

def FPGATrackSimBankSvcCfg(flags,name="FPGATrackSimBankSvc"):
    result=ComponentAccumulator()
    FPGATrackSimBankSvc = CompFactory.FPGATrackSimBankSvc(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    FPGATrackSimBankSvc.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    pathBankSvc = flags.Trigger.FPGATrackSim.bankDir if flags.Trigger.FPGATrackSim.bankDir != '' else f'/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/{flags.GeoModel.AtlasVersion}/21.9.16/'+FPGATrackSimDataPrepConfig.getBaseName(flags)+'/SectorBanks/'
    pathBankSvc=PathResolver.FindCalibDirectory(pathBankSvc)
    FPGATrackSimBankSvc.constantsNoGuess_1st = [
        f'{pathBankSvc}corrgen_raw_8L_skipPlane0.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane1.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane2.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane3.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane4.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane5.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane6.gcon',
        f'{pathBankSvc}corrgen_raw_8L_skipPlane7.gcon']
    FPGATrackSimBankSvc.constantsNoGuess_2nd = [
        f'{pathBankSvc}corrgen_raw_13L_skipPlane0.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane1.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane2.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane3.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane4.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane5.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane6.gcon',
        f'{pathBankSvc}corrgen_raw_13L_skipPlane7.gcon']
    layers="5L" if flags.Trigger.FPGATrackSim.ActiveConfig.genScan else "9L"
    s2_layers = 13
    pathMapSvc = flags.Trigger.FPGATrackSim.mapsDir if flags.Trigger.FPGATrackSim.mapsDir != '' else f'/eos/atlas/atlascerngroupdisk/det-htt/HTTsim/{flags.GeoModel.AtlasVersion}/21.9.16/'+FPGATrackSimDataPrepConfig.getBaseName(flags)+'/SectorMaps/'
    pathMapSvc = PathResolver.FindCalibDirectory(pathMapSvc)
    pmap_file = os.path.join(pathMapSvc, f"region{flags.Trigger.FPGATrackSim.region}.pmap")
    with open(pmap_file) as f:
        for line in f:
            if 'logical_s2' in line:
                s2_layers = int(line.strip().split()[0])
                break
    FPGATrackSimBankSvc.constants_1st = f'{pathBankSvc}corrgen_raw_{layers}_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.constants_2nd = f'{pathBankSvc}corrgen_raw_{s2_layers}L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.gcon'
    FPGATrackSimBankSvc.sectorBank_1st = f'{pathBankSvc}sectorsHW_raw_{layers}_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorBank_2nd = f'{pathBankSvc}sectorsHW_raw_{s2_layers}L_reg{flags.Trigger.FPGATrackSim.region}_checkGood1.patt'
    FPGATrackSimBankSvc.sectorSlices = f'{pathBankSvc}slices_{layers}_reg{flags.Trigger.FPGATrackSim.region}.root'
    FPGATrackSimBankSvc.phiShift = flags.Trigger.FPGATrackSim.phiShift

    # These should be configurable. The tag system needs updating though.
    FPGATrackSimBankSvc.sectorQPtBins = [-0.001, -0.0005, 0, 0.0005, 0.001]
    FPGATrackSimBankSvc.qptAbsBinning = False

    result.addService(FPGATrackSimBankSvc, create=True, primary=True)
    return result


def FPGATrackSimRoadUnionToolCfg(flags,name="FPGATrackSimRoadUnionTool"):
    result=ComponentAccumulator()
    RF = CompFactory.FPGATrackSimRoadUnionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    xBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBins
    xBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
    yBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBins
    yBufferBins = flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
    yMin = flags.Trigger.FPGATrackSim.ActiveConfig.qptMin
    yMax = flags.Trigger.FPGATrackSim.ActiveConfig.qptMax
    xMin = flags.Trigger.FPGATrackSim.ActiveConfig.phiMin
    xMax = flags.Trigger.FPGATrackSim.ActiveConfig.phiMax
    if (not flags.Trigger.FPGATrackSim.oldRegionDefs): ### auto-configure this
        phiRange = FPGATrackSimDataPrepConfig.getPhiRange(flags)
        xMin = phiRange[0]
        xMax = phiRange[1]

    xBuffer = (xMax - xMin) / xBins * xBufferBins
    xMin = xMin - xBuffer
    xMax = xMax +  xBuffer
    yBuffer = (yMax - yMin) / yBins * yBufferBins
    yMin -= yBuffer
    yMax += yBuffer
    tools = []
    houghType = flags.Trigger.FPGATrackSim.ActiveConfig.houghType
    roadMerge = flags.Trigger.FPGATrackSim.ActiveConfig.roadMerge


    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    for number in range(getNSubregions(FPGATrackSimMapping.subrmap)):
        HoughTransform = CompFactory.FPGATrackSimHoughTransformTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimHoughTransformTool")+"_" + str(number))
        HoughTransform.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
        HoughTransform.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
        HoughTransform.FPGATrackSimMappingSvc = FPGATrackSimMapping
        HoughTransform.combine_layers = flags.Trigger.FPGATrackSim.ActiveConfig.combineLayers
        HoughTransform.convSize_x = flags.Trigger.FPGATrackSim.ActiveConfig.convSizeX
        HoughTransform.convSize_y = flags.Trigger.FPGATrackSim.ActiveConfig.convSizeY
        HoughTransform.convolution = flags.Trigger.FPGATrackSim.ActiveConfig.convolution
        HoughTransform.d0_max = 0
        HoughTransform.d0_min = 0
        HoughTransform.fieldCorrection = flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
        HoughTransform.hitExtend_x = flags.Trigger.FPGATrackSim.ActiveConfig.hitExtendX
        HoughTransform.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        HoughTransform.nBins_x = xBins + 2 * xBufferBins
        HoughTransform.nBins_y = yBins + 2 * yBufferBins
        HoughTransform.phi_max = xMax
        HoughTransform.phi_min = xMin
        HoughTransform.qpT_max = yMax
        HoughTransform.qpT_min = yMin
        HoughTransform.scale = flags.Trigger.FPGATrackSim.ActiveConfig.scale
        HoughTransform.subRegion = number
        HoughTransform.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.threshold
        HoughTransform.traceHits = True
        HoughTransform.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
        HoughTransform.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints
        HoughTransform.houghType = houghType
        HoughTransform.roadMerge = roadMerge
        if houghType=='LowResource': ##consider only case of LowResource version
            HoughTransform.requirements = flags.Trigger.FPGATrackSim.ActiveConfig.requirements
        if houghType=='Flexible': ##consider only case of Flexible version
            HoughTransform.r_max=flags.Trigger.FPGATrackSim.ActiveConfig.r_max
            HoughTransform.phi_coord_max=flags.Trigger.FPGATrackSim.ActiveConfig.phi_coord_max
            HoughTransform.phi_range=flags.Trigger.FPGATrackSim.ActiveConfig.phi_range
            HoughTransform.r_max_mm=flags.Trigger.FPGATrackSim.ActiveConfig.r_max_mm
            HoughTransform.bitwise_qApt_conv=flags.Trigger.FPGATrackSim.ActiveConfig.bitwise_qApt_conv
            HoughTransform.bitwise_phi0_conv=flags.Trigger.FPGATrackSim.ActiveConfig.bitwise_phi0_conv
            HoughTransform.phi0_sectors=flags.Trigger.FPGATrackSim.ActiveConfig.phi0_sectors
            HoughTransform.qApt_sectors=flags.Trigger.FPGATrackSim.ActiveConfig.qApt_sectors
            HoughTransform.pipes_qApt=flags.Trigger.FPGATrackSim.ActiveConfig.pipes_qApt
            HoughTransform.pipes_phi0=flags.Trigger.FPGATrackSim.ActiveConfig.pipes_phi0

        tools.append(HoughTransform)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result

def FPGATrackSimRoadUnionTool1DCfg(flags,name="FPGATrackSimRoadUnionTool1D"):
    result=ComponentAccumulator()
    tools = []
    RF = CompFactory.FPGATrackSimRoadUnionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    splitpt=flags.Trigger.FPGATrackSim.Hough1D.splitpt
    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    for ptstep in range(splitpt):
        qpt_min = flags.Trigger.FPGATrackSim.Hough1D.qptMin
        qpt_max = flags.Trigger.FPGATrackSim.Hough1D.qptMax
        lowpt = qpt_min + (qpt_max-qpt_min)/splitpt*ptstep
        highpt = qpt_min + (qpt_max-qpt_min)/splitpt*(ptstep+1)
        nSlice = getNSubregions(FPGATrackSimMapping.subrmap)
        for iSlice in range(nSlice):
            tool = CompFactory.FPGATrackSimHough1DShiftTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,
                                                                                                            "Hough1DShift" + str(iSlice)+(("_pt{}".format(ptstep))  if splitpt>1 else "")))
            tool.subRegion = iSlice if nSlice > 1 else -1
            xMin = flags.Trigger.FPGATrackSim.Hough1D.phiMin
            xMax = flags.Trigger.FPGATrackSim.Hough1D.phiMax
            if (not flags.Trigger.FPGATrackSim.oldRegionDefs): ### auto-configure this
                phiRange = FPGATrackSimDataPrepConfig.getPhiRange(flags)
                xMin = phiRange[0]
                xMax = phiRange[1]
            tool.phiMin = xMin
            tool.phiMax = xMax
            tool.qptMin = lowpt
            tool.qptMax = highpt
            tool.nBins = flags.Trigger.FPGATrackSim.Hough1D.xBins
            tool.useDiff = True
            tool.variableExtend = True
            tool.drawHitMasks = False
            tool.phiRangeCut = flags.Trigger.FPGATrackSim.Hough1D.phiRangeCut
            tool.d0spread=-1.0 # mm
            tool.iterStep = 0 # auto, TODO put in tag
            tool.iterLayer = 7 # TODO put in tag
            tool.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
            tool.hitExtend = flags.Trigger.FPGATrackSim.Hough1D.hitExtendX
            tool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
            tool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
            tool.FPGATrackSimMappingSvc = FPGATrackSimMapping
            tool.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
            tool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints

            tools.append(tool)

    RF.tools = tools
    result.addPublicTool(RF, primary=True)
    return result



def FPGATrackSimRoadUnionToolGenScanCfg(flags,name="FPGATrackSimRoadUnionToolGenScan"):
    result=ComponentAccumulator()
    
    print("logLevel",flags.Trigger.FPGATrackSim.loglevel)

    # read the cuts from a seperate python file specified by FPGATrackSim.GenScan.genScanCuts
    cutset=None
    if flags.Trigger.FPGATrackSim.oldRegionDefs:
        toload=flags.Trigger.FPGATrackSim.GenScan.genScanCuts
        if toload == 'FPGATrackSimGenScanCuts': # its on the newRegion default so it hasn't been set
            toload = 'FPGATrackSimHough.FPGATrackSimGenScanCuts_incr'
        cutset = importlib.import_module(toload).cuts[flags.Trigger.FPGATrackSim.region]
    else:
        # this allows the cut file defined in python to be loaded from the map directory
        cutpath = os.path.join(
             PathResolver.FindCalibDirectory(flags.Trigger.FPGATrackSim.mapsDir),
             f"{flags.Trigger.FPGATrackSim.GenScan.genScanCuts}.py")
        print("Cut File = ", cutpath)
        spec=importlib.util.spec_from_file_location(flags.Trigger.FPGATrackSim.GenScan.genScanCuts,cutpath)
        if spec is None:
            print("Failed to find Cut File")
        cutmodule = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cutmodule)
        cutset=cutmodule.cuts[flags.Trigger.FPGATrackSim.region]

    # make the binned hits class
    BinnnedHits = CompFactory.FPGATrackSimBinnedHits("BinnedHits_1stStage")
    BinnnedHits.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    BinnnedHits.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))

    # set layer map
    if not flags.Trigger.FPGATrackSim.GenScan.layerStudy:
        if flags.Trigger.FPGATrackSim.oldRegionDefs:
            BinnnedHits.layerMapFile = flags.Trigger.FPGATrackSim.GenScan.layerMapFile
        else:
            # now assumed to be in the map directory with name = basename for region + _lyrmap.json
            BinnnedHits.layerMapFile =os.path.join(
             PathResolver.FindCalibDirectory(flags.Trigger.FPGATrackSim.mapsDir),
             f"{FPGATrackSimDataPrepConfig.getBaseName(flags)}_lyrmap.json")


    # make the bintool class
    BinTool = CompFactory.FPGATrackSimBinTool("BinTool_1stStage")
    BinTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel

     # Inputs for the BinTool
    binsteps=[]
    BinDesc=None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars") :
        BinDesc = CompFactory.FPGATrackSimKeyLayerBinDesc("KeyLayerBinDesc")
        BinDesc.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        BinDesc.rin=cutset["rin"]
        BinDesc.rout=cutset["rout"]

        # parameters for key layer bindesc are :"zR1", "zR2", "phiR1", "phiR2", "xm"
        step1 = CompFactory.FPGATrackSimBinStep("PhiBinning")
        step1.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        step1.parBins = [1,1,cutset["parBins"][2],cutset["parBins"][3],cutset["parBins"][4]]
        step2 = CompFactory.FPGATrackSimBinStep("FullBinning")
        step2.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        step2.parBins = cutset["parBins"]
        binsteps = [step1,step2]
    else:
        log.error("Unknown Binning Setup: ",cutset["parSet"])

    BinTool.BinDesc = BinDesc
    BinTool.Steps=binsteps


    # configure the padding around the nominal region
    BinTool.d0FractionalPadding =0.05
    BinTool.z0FractionalPadding =0.05
    BinTool.etaFractionalPadding =0.05
    BinTool.phiFractionalPadding =0.05
    BinTool.qOverPtFractionalPadding =0.05
    BinTool.parMin = cutset["parMin"]
    BinTool.parMax = cutset["parMax"]
    BinnnedHits.BinTool = BinTool


    # make the monitoring class
    Monitor = CompFactory.FPGATrackSimGenScanMonitoring(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"GenScanMonitoring"))
    Monitor.dir = "/GENSCAN/"
    Monitor.THistSvc = CompFactory.THistSvc()
    Monitor.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    Monitor.phiScale = 10.0
    Monitor.etaScale = 100.0
    Monitor.drScale = 20.0

    # make the main tool
    tool = CompFactory.FPGATrackSimGenScanTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"GenScanTool"))
    tool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    tool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    tool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    tool.Monitoring = Monitor
    tool.BinnedHits = BinnnedHits
    tool.rin=cutset["rin"]
    tool.rout=cutset["rout"]

    # For the 'track fitter' part of GenScanTool.
    tool.inBinFiltering = flags.Trigger.FPGATrackSim.GenScan.filterInBin
    tool.phiChi2Weight = flags.Trigger.FPGATrackSim.GenScan.phiChi2Weight
    tool.etaChi2Weight = flags.Trigger.FPGATrackSim.GenScan.etaChi2Weight

    # configure which filers and thresholds to apply
    tool.binFilter=flags.Trigger.FPGATrackSim.GenScan.binFilter
    tool.reversePairDir=flags.Trigger.FPGATrackSim.GenScan.reverse
    tool.applyPairFilter= not flags.Trigger.FPGATrackSim.GenScan.noCuts
    tool.applyPairSetFilter= not flags.Trigger.FPGATrackSim.GenScan.noCuts
    tool.threshold = 4

    # set cuts
    for (cut,val) in cutset.items():
        if cut in ["parBins","parSet","parMin","parMax"]:
            continue
        setattr(tool,cut,val)

    # even though we are not actually doing a Union, we need the
    # RoadUnionTool because mapping is now there
    RoadUnion = CompFactory.FPGATrackSimRoadUnionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    RoadUnion.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    RoadUnion.tools = [tool,]
    result.addPublicTool(RoadUnion, primary=True)

    # special configuration for studing layer definitions
    # pass through all hits, but turn off pairing because
    # it won't be able to run
    if flags.Trigger.FPGATrackSim.GenScan.layerStudy:
        RoadUnion.noHitFilter=True
        tool.binningOnly=True
    return result

def FPGATrackSimRoadUnionToolGNNCfg(flags,name="FPGATrackSimRoadUnionToolGNN"):
    result = ComponentAccumulator()
    RF = CompFactory.FPGATrackSimRoadUnionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    RF.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    patternRecoTool = CompFactory.FPGATrackSimGNNPatternRecoTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimGNNPatternRecoTool"))
    patternRecoTool.GNNGraphHitSelector = CompFactory.FPGATrackSimGNNGraphHitSelectorTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimGNNGraphHitSelectorTool"))
    patternRecoTool.GNNGraphConstruction = result.popToolsAndMerge(FPGATrackSimGNNGraphConstructionToolCfg(flags))
    edgeResult, edgeClassifierTools = FPGATrackSimGNNEdgeClassifierToolCfg(flags)
    result.merge(edgeResult)
    patternRecoTool.GNNEdgeClassifiers = edgeClassifierTools
    patternRecoTool.GNNRoadMaker = result.popToolsAndMerge(FPGATrackSimGNNRoadMakerToolCfg(flags))
    patternRecoTool.GNNRootOutput = result.popToolsAndMerge(FPGATrackSimGNNRootOutputToolCfg(flags))
    patternRecoTool.doGNNRootOutput = flags.Trigger.FPGATrackSim.GNN.doGNNRootOutput
    patternRecoTool.regionNum = int(flags.Trigger.FPGATrackSim.region)

    RF.tools = [patternRecoTool]
    result.addPublicTool(RF, primary=True)

    return result

def FPGATrackSimGNNGraphConstructionToolCfg(flags,name="FPGATrackSimGNNGraphConstructionTool"):
    result = ComponentAccumulator()

    GNNGraphConstructionTool = CompFactory.FPGATrackSimGNNGraphConstructionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    GNNGraphConstructionTool.graphTool = flags.Trigger.FPGATrackSim.GNN.graphTool.value
    GNNGraphConstructionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))


    # Module Map Configuration
    GNNGraphConstructionTool.moduleMapType=flags.Trigger.FPGATrackSim.GNN.moduleMapType.value
    GNNGraphConstructionTool.moduleMapFunc=flags.Trigger.FPGATrackSim.GNN.moduleMapFunc.value
    GNNGraphConstructionTool.moduleMapTol=flags.Trigger.FPGATrackSim.GNN.moduleMapTol

    # Metric Learning Configuration
    GNNGraphConstructionTool.metricLearningR=flags.Trigger.FPGATrackSim.GNN.metricLearningR
    GNNGraphConstructionTool.metricLearningMaxN=flags.Trigger.FPGATrackSim.GNN.metricLearningMaxN

    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType

    GNNGraphConstructionTool.MLInferenceTool = result.popToolsAndMerge(OnnxRuntimeInferenceToolCfg(
        flags, flags.Trigger.FPGATrackSim.GNN.MLModelPath, OnnxRuntimeType.CPU))

    result.setPrivateTools(GNNGraphConstructionTool)

    return result

def FPGATrackSimGNNEdgeClassifierToolCfg(flags, name="FPGATrackSimGNNEdgeClassifierTool"):
    result = ComponentAccumulator()

    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
    from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType

    region = int(flags.Trigger.FPGATrackSim.region)
    model_path = f"{flags.Trigger.FPGATrackSim.GNN.GNNModelPath}_{region}.onnx"

    GNNEdgeClassifierTool = CompFactory.FPGATrackSimGNNEdgeClassifierTool(
        FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags, name))
    GNNEdgeClassifierTool.GNNInferenceTool = result.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, model_path, OnnxRuntimeType.CPU, name=f"OnnxInferenceTool_{region}")
    )
    GNNEdgeClassifierTool.regionNum = region

    return result, [GNNEdgeClassifierTool]

def FPGATrackSimGNNRoadMakerToolCfg(flags,name="FPGATrackSimGNNRoadMakerTool"):
    result = ComponentAccumulator()

    GNNRoadMakerTool = CompFactory.FPGATrackSimGNNRoadMakerTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    GNNRoadMakerTool.roadMakerTool = flags.Trigger.FPGATrackSim.GNN.roadMakerTool.value
    GNNRoadMakerTool.edgeScoreCut = flags.Trigger.FPGATrackSim.GNN.edgeScoreCut
    GNNRoadMakerTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    result.setPrivateTools(GNNRoadMakerTool)

    return result

def FPGATrackSimGNNRootOutputToolCfg(flags,name="FPGATrackSimGNNRootOutputTool"):
    result = ComponentAccumulator()

    GNNRootOutputTool = CompFactory.FPGATrackSimGNNRootOutputTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    GNNRootOutputTool.OutputRegion = str(flags.Trigger.FPGATrackSim.region)

    if(flags.Trigger.FPGATrackSim.GNN.doGNNRootOutput):
        result.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimGNNOUTPUT DATAFILE='GNNRootOutput.root', OPT='RECREATE'"]))
    result.setPrivateTools(GNNRootOutputTool)

    return result

def FPGATrackSimDataFlowToolCfg(flags,name="FPGATrackSimDataFlowTool"):
    result=ComponentAccumulator()
    DataFlowTool = CompFactory.FPGATrackSimDataFlowTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    DataFlowTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    DataFlowTool.FPGATrackSimMappingSvc =  result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    DataFlowTool.Chi2ndofCut = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut
    DataFlowTool.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(DataFlowTool)
    return result

def FPGATrackSimHoughRootOutputToolCfg(flags,name="FPGATrackSimHoughRootOutputTool"):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    HoughRootOutputTool.OutputRegion = str(flags.Trigger.FPGATrackSim.region)
    result.setPrivateTools(HoughRootOutputTool)
    return result

def LRTRoadFinderCfg(flags,name="LRTRoadFinder"):

    result=ComponentAccumulator()
    LRTRoadFinder =CompFactory.FPGATrackSimHoughTransform_d0phi0_Tool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    LRTRoadFinder.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    LRTRoadFinder.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    LRTRoadFinder.combine_layers = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackCombineLayers
    LRTRoadFinder.convolution = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackConvolution
    LRTRoadFinder.hitExtend_x = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackHitExtendX
    LRTRoadFinder.scale = flags.Trigger.FPGATrackSim.ActiveConfig.scale
    LRTRoadFinder.threshold = flags.Trigger.FPGATrackSim.ActiveConfig.lrtStraighttrackThreshold
    result.setPrivateTools(LRTRoadFinder)
    return result

def NNTrackToolCfg(flags,name="FPGATrackSimNNTrackTool"):
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.FPGATrackSimNNTrackTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    NNTrackTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    NNTrackTool.IdealGeoRoads = False
    NNTrackTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan and not flags.Trigger.FPGATrackSim.ActiveConfig.GNN
    NNTrackTool.SPRoadFilterTool = result.popToolsAndMerge(SPRoadFilterToolCfg(flags))
    NNTrackTool.MinNumberOfRealHitsInATrack = 5 if flags.Trigger.FPGATrackSim.ActiveConfig.genScan else 7 if flags.Trigger.FPGATrackSim.ActiveConfig.GNN else 9
    NNTrackTool.useSectors = False
    NNTrackTool.doGNNTracking = flags.Trigger.FPGATrackSim.GNN.doGNNTracking
    NNTrackTool.nInputsGNN = flags.Trigger.FPGATrackSim.GNN.nInputsGNN
    NNTrackTool.useCartesian = flags.Trigger.FPGATrackSim.NNCartesianCoordinates
    result.setPrivateTools(NNTrackTool)
    return result

def FPGATrackSimTrackFitterToolCfg(flags,name="FPGATrackSimTrackFitterTool"):
    result=ComponentAccumulator()
    TF_1st = CompFactory.FPGATrackSimTrackFitterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    TF_1st.GuessHits = flags.Trigger.FPGATrackSim.ActiveConfig.guessHits
    TF_1st.IdealCoordFitType = flags.Trigger.FPGATrackSim.ActiveConfig.idealCoordFitType
    TF_1st.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    TF_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    TF_1st.chi2DofRecoveryMax = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMax
    TF_1st.chi2DofRecoveryMin = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMin
    TF_1st.doMajority = flags.Trigger.FPGATrackSim.ActiveConfig.doMajority
    TF_1st.nHits_noRecovery = flags.Trigger.FPGATrackSim.ActiveConfig.nHitsNoRecovery
    TF_1st.DoDeltaGPhis = flags.Trigger.FPGATrackSim.ActiveConfig.doDeltaGPhis
    TF_1st.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    TF_1st.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    TF_1st.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    TF_1st.SPRoadFilterTool = result.popToolsAndMerge(SPRoadFilterToolCfg(flags))
    TF_1st.fitFromRoad = flags.Trigger.FPGATrackSim.ActiveConfig.fitFromRoad
    result.addPublicTool(TF_1st, primary=True)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags,name="FPGATrackSimOverlapRemovalTool"):
    result=ComponentAccumulator()
    OR_1st = CompFactory.FPGATrackSimOverlapRemovalTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    OR_1st.ORAlgo = "Normal"
    OR_1st.doFastOR = flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR
    OR_1st.NumOfHitPerGrouping = 3
    OR_1st.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and not flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        OR_1st.MinChi2 = getChi2Cut(flags.Trigger.FPGATrackSim.region)
    elif flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        OR_1st.MinChi2 = getChi2CutNN(flags.Trigger.FPGATrackSim.region)
    else:
        OR_1st.MinChi2 = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut
    if flags.Trigger.FPGATrackSim.ActiveConfig.hough or flags.Trigger.FPGATrackSim.ActiveConfig.hough1D:
        OR_1st.nBins_x = flags.Trigger.FPGATrackSim.ActiveConfig.xBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
        OR_1st.nBins_y = flags.Trigger.FPGATrackSim.ActiveConfig.yBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
        OR_1st.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        OR_1st.roadSliceOR = flags.Trigger.FPGATrackSim.ActiveConfig.roadSliceOR

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimOverlapRemovalToolMonitoringCfg
    OR_1st.MonTool = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolMonitoringCfg(flags))

    result.addPublicTool(OR_1st, primary=True)
    return result

def prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=True)
    return newFlags

def SPRoadFilterToolCfg(flags,secondStage=False,name="FPGATrackSimSpacepointRoadFilterTool"):
    result=ComponentAccumulator()
    name="FPGATrackSimSpacepointRoadFilterTool_1st"
    if secondStage:
        name="FPGATrackSimSpacepointRoadFilterTool_2st"
    SPRoadFilter = CompFactory.FPGATrackSimSpacepointRoadFilterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    SPRoadFilter.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    SPRoadFilter.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))
    SPRoadFilter.filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
    SPRoadFilter.minSpacePlusPixel = flags.Trigger.FPGATrackSim.minSpacePlusPixel
    SPRoadFilter.isSecondStage = secondStage
    SPRoadFilter.dropUnpairedIfSP = flags.Trigger.FPGATrackSim.dropUnpairedIfSP

    # This threshold is the number of *missing* hits allowed. For now, assume that if 1st stage this is always 1.
    # We don't actually run this tool in the first stage anymore, so this is more to preserve backwards compatibility
    if secondStage:
        SPRoadFilter.threshold = flags.Trigger.FPGATrackSim.hitThreshold
    else:
        SPRoadFilter.threshold = 1

    SPRoadFilter.setSectors = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    result.setPrivateTools(SPRoadFilter)
    return result

def FPGATrackSimLogicalHitsProcessAlgCfg(inputFlags,name="FPGATrackSimLogicalHitsProcessAlg",**kwargs):

    flags = prepareFlagsForFPGATrackSimLogicalHitsProcessAlg(inputFlags)

    result=ComponentAccumulator()
    kwargs.setdefault("name", FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    theFPGATrackSimLogicalHitsProcessAlg=CompFactory.FPGATrackSimLogicalHitsProcessAlg(**kwargs)
    theFPGATrackSimLogicalHitsProcessAlg.writeOutputData = flags.Trigger.FPGATrackSim.writeAdditionalOutputData
    theFPGATrackSimLogicalHitsProcessAlg.tracking = flags.Trigger.FPGATrackSim.tracking
    theFPGATrackSimLogicalHitsProcessAlg.SetTruthParametersForTracks = flags.Trigger.FPGATrackSim.SetTruthParametersForTracks
    theFPGATrackSimLogicalHitsProcessAlg.doOverlapRemoval = flags.Trigger.FPGATrackSim.doOverlapRemoval
    theFPGATrackSimLogicalHitsProcessAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimLogicalHitsProcessAlg.DoHoughRootOutput1st = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput1st
    theFPGATrackSimLogicalHitsProcessAlg.NumOfHitPerGrouping = flags.Trigger.FPGATrackSim.ActiveConfig.NumOfHitPerGrouping
    theFPGATrackSimLogicalHitsProcessAlg.DoNNTrack_1st = flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis
    theFPGATrackSimLogicalHitsProcessAlg.DoGNNTrack = flags.Trigger.FPGATrackSim.GNN.doGNNTracking
    theFPGATrackSimLogicalHitsProcessAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and not flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        theFPGATrackSimLogicalHitsProcessAlg.TrackScoreCut = getChi2Cut(flags.Trigger.FPGATrackSim.region)
    elif flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        theFPGATrackSimLogicalHitsProcessAlg.TrackScoreCut = getChi2CutNN(flags.Trigger.FPGATrackSim.region)
    else:
        theFPGATrackSimLogicalHitsProcessAlg.TrackScoreCut = flags.Trigger.FPGATrackSim.ActiveConfig.chi2cut
    theFPGATrackSimLogicalHitsProcessAlg.passLowestChi2TrackOnly = flags.Trigger.FPGATrackSim.ActiveConfig.passLowestChi2TrackOnly
    theFPGATrackSimLogicalHitsProcessAlg.secondStageStrips = (not flags.Trigger.FPGATrackSim.ActiveConfig.GNN)
    FPGATrackSimMaping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.FPGATrackSimMapping = FPGATrackSimMaping
    # Adding region so we can set it for tracks, then get the bfield
    theFPGATrackSimLogicalHitsProcessAlg.Region = flags.Trigger.FPGATrackSim.region
    # If tracking is set to False or if we do the NN analysis, don't configure the bank service
    if flags.Trigger.FPGATrackSim.tracking and not flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis:
        result.getPrimaryAndMerge(FPGATrackSimBankSvcCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionTool1DCfg(flags))
    elif (flags.Trigger.FPGATrackSim.ActiveConfig.genScan):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolGenScanCfg(flags))
    elif (flags.Trigger.FPGATrackSim.ActiveConfig.GNN):
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolGNNCfg(flags))
    else:
        theFPGATrackSimLogicalHitsProcessAlg.RoadFinder = result.getPrimaryAndMerge(FPGATrackSimRoadUnionToolCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.etaPatternFilter):
        EtaPatternFilter = CompFactory.FPGATrackSimEtaPatternFilterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimEtaPatternFilterTool"))
        EtaPatternFilter.FPGATrackSimMappingSvc = FPGATrackSimMaping
        EtaPatternFilter.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
        EtaPatternFilter.EtaPatterns = flags.Trigger.FPGATrackSim.mapsDir+"/"+FPGATrackSimDataPrepConfig.getBaseName(flags)+".patt"
        theFPGATrackSimLogicalHitsProcessAlg.RoadFilter = EtaPatternFilter
        theFPGATrackSimLogicalHitsProcessAlg.FilterRoads = True

    if (flags.Trigger.FPGATrackSim.ActiveConfig.phiRoadFilter):
        RoadFilter2 = CompFactory.FPGATrackSimPhiRoadFilterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimPhiRoadFilterTool"))
        RoadFilter2.FPGATrackSimMappingSvc = FPGATrackSimMaping
        RoadFilter2.threshold = flags.Trigger.FPGATrackSim.Hough1D.threshold[0]
        RoadFilter2.fieldCorrection = flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
        ### set the window to be a constant value (could be changed), array should be length of the threshold
        windows = [flags.Trigger.FPGATrackSim.Hough1D.phifilterwindow for i in range(len(flags.Trigger.FPGATrackSim.ActiveConfig.hitExtendX))]
        RoadFilter2.window = windows

        theFPGATrackSimLogicalHitsProcessAlg.RoadFilter2 = RoadFilter2
        theFPGATrackSimLogicalHitsProcessAlg.FilterRoads2 = True

    theFPGATrackSimLogicalHitsProcessAlg.SlicingEngineTool = result.getPrimaryAndMerge(FPGATrackSimSlicingEngineCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg.HoughRootOutputTool = result.getPrimaryAndMerge(FPGATrackSimHoughRootOutputToolCfg(flags))

    LRTRoadFilter = CompFactory.FPGATrackSimLLPRoadFilterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimLLPRoadFilterTool"))
    result.addPublicTool(LRTRoadFilter)
    theFPGATrackSimLogicalHitsProcessAlg.LRTRoadFilter = LRTRoadFilter

    theFPGATrackSimLogicalHitsProcessAlg.LRTRoadFinder = result.getPrimaryAndMerge(LRTRoadFinderCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.NNTrackTool = result.getPrimaryAndMerge(NNTrackToolCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg.OutputTool = result.popToolsAndMerge(FPGATrackSimWriteOutputCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.TrackFitter_1st = result.getPrimaryAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.OverlapRemoval_1st = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))

    theFPGATrackSimLogicalHitsProcessAlg.SpacePointTool = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimSpacePointsToolCfg(flags))
    theFPGATrackSimLogicalHitsProcessAlg.Spacepoints = flags.Trigger.FPGATrackSim.spacePoints

    if flags.Trigger.FPGATrackSim.ActiveConfig.lrt:
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseBasicHitFilter != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseMlHitFilter, 'Inconsistent LRT hit filtering setup, need either ML of Basic filtering enabled'
        assert flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseStraightTrackHT != flags.Trigger.FPGATrackSim.ActiveConfig.lrtUseDoubletHT, 'Inconsistent LRT HT setup, need either double or strightTrack enabled'
        theFPGATrackSimLogicalHitsProcessAlg.doLRT = True
        theFPGATrackSimLogicalHitsProcessAlg.LRTHitFiltering = (not flags.Trigger.FPGATrackSim.ActiveConfig.lrtSkipHitFiltering)

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimLogicalHitsProcessAlgMonitoringCfg
    theFPGATrackSimLogicalHitsProcessAlg.MonTool = result.getPrimaryAndMerge(FPGATrackSimLogicalHitsProcessAlgMonitoringCfg(flags))
    result.addEventAlgo(theFPGATrackSimLogicalHitsProcessAlg)

    return result

def getChi2Cut(region):
    chi2cut_l = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 20] #from most recent run on this branch with new maps/banks, all chi2's are under 1
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    eta_to_chi2 = {
        (0.0, 0.2): chi2cut_l[0], (0.2, 0.4): chi2cut_l[1], (0.4, 0.6): chi2cut_l[2],
        (0.6, 0.8): chi2cut_l[3], (0.8, 1.0): chi2cut_l[4], (1.0, 1.2): chi2cut_l[5],
        (1.2, 1.4): chi2cut_l[6], (1.4, 1.6): chi2cut_l[7], (1.6, 1.8): chi2cut_l[8],
        (1.8, 2.0): chi2cut_l[9], (2.0, 2.2): chi2cut_l[10], (2.2, 2.4): chi2cut_l[11],
        (2.4, 2.6): chi2cut_l[12], (2.6, 2.8): chi2cut_l[13], (2.8, 3.0): chi2cut_l[14],
        (3.0, 3.2): chi2cut_l[15], (3.2, 3.4): chi2cut_l[16], (3.4, 3.6): chi2cut_l[17],
        (3.6, 3.8): chi2cut_l[18], (3.8, 4.0): chi2cut_l[19]
    }

    return eta_to_chi2.get(abs_etaRange, 20)

def getChi2CutNN(region):
    chi2cut_l = [0.97,0.97,0.99, ### 0.0-0.6
                 0.7,0.86,0.99, ### 0.6-1.2,
                 0.98,0.92,0.98, ### 1.2-1.8
                 0.95,0.93,0.4, ### 1.8-2.4
                 0.4,0.4,0.7, ### 2.4-3.0
                 0.6,0.5,0.4, ### 3.0-3.6
                 0.4,0.4] ### 3.6-4.0
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    eta_to_chi2 = {
        (0.0, 0.2): chi2cut_l[0], (0.2, 0.4): chi2cut_l[1], (0.4, 0.6): chi2cut_l[2],
        (0.6, 0.8): chi2cut_l[3], (0.8, 1.0): chi2cut_l[4], (1.0, 1.2): chi2cut_l[5],
        (1.2, 1.4): chi2cut_l[6], (1.4, 1.6): chi2cut_l[7], (1.6, 1.8): chi2cut_l[8],
        (1.8, 2.0): chi2cut_l[9], (2.0, 2.2): chi2cut_l[10], (2.2, 2.4): chi2cut_l[11],
        (2.4, 2.6): chi2cut_l[12], (2.6, 2.8): chi2cut_l[13], (2.8, 3.0): chi2cut_l[14],
        (3.0, 3.2): chi2cut_l[15], (3.2, 3.4): chi2cut_l[16], (3.4, 3.6): chi2cut_l[17],
        (3.6, 3.8): chi2cut_l[18], (3.8, 4.0): chi2cut_l[19]
    }
    return eta_to_chi2.get(abs_etaRange, 0.99) 


def ConfigureMultiRegionFlags(flags):
    # convert regex to array of regions
    if flags.Trigger.FPGATrackSim.regionList == "": # in case of empty list just use the region set to flags.Trigger.FPGATrackSim.region
        flags.Trigger.FPGATrackSim.regionList = [flags.Trigger.FPGATrackSim.region]
    else: # otherwise use the regionList (this overrides the region flag)
        from FPGATrackSimConfTools.FPGATrackSimHelperFunctions import convertRegionsExpressionToArray
        flags.Trigger.FPGATrackSim.regionList = convertRegionsExpressionToArray(flags.Trigger.FPGATrackSim.regionList)
    print(f"Running for regions: {flags.Trigger.FPGATrackSim.regionList}")


def FPGATrackSimF150FlagCfg(flags):
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    
    flags.Concurrency.NumThreads=4
    flags.Concurrency.NumConcurrentEvents=1
    flags.Concurrency.NumProcs=0
    
    flags.Trigger.FPGATrackSim.readOfflineObjects=False
    flags.Trigger.FPGATrackSim.writeAdditionalOutputData=False
    flags.Trigger.FPGATrackSim.doMultiTruth=False
    
    FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepFlagCfg(flags)
    flags.Trigger.FPGATrackSim.tracking = False
    flags.Trigger.FPGATrackSim.Hough.genScan = True
    flags.Trigger.FPGATrackSim.convertSPs = True
    flags.Trigger.FPGATrackSim.Hough.secondStage = False
    flags.Trigger.FPGATrackSim.regionList="34,98,162,226,290,354,418,482,546,610,674,738,802,866,930,994,1058,1122,1186,1250"
    ConfigureMultiRegionFlags(flags)
    
    return flags

def FPGATrackSimSeedingCfg(flags):
    acc=ComponentAccumulator()
    acc.merge(FPGATrackSimDataPrepConfig.FPGATrackSimClusteringCfg(flags))

    from FPGATrackSimConfTools.FPGATrackSimMultiRegionConfig import FPGATrackSimMultiRegionTrackingCfg
    acc.merge(FPGATrackSimMultiRegionTrackingCfg(flags))
    
    from FPGATrackSimSeeding.FPGATrackSimSeedingConfig import FPGATrackSimSeedingCfg
    acc.merge(FPGATrackSimSeedingCfg(flags))
    
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg


    flags = initConfigFlags()

    ############################################
    # Flags used in the prototrack chain
    FinalProtoTrackChainxAODTracksKey="FPGA"
    flags.Detector.EnableCalo = False 

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True
    from ActsConfig.ActsCIFlags import actsLegacyWorkflowFlags
    actsLegacyWorkflowFlags(flags)
    flags.Acts.doRotCorrection = False

    ############################################
    flags.Concurrency.NumThreads=1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Concurrency.NumProcs=0
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Debug.DumpEvtStore=False

    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()

    ConfigureMultiRegionFlags(flags)


    if flags.Trigger.FPGATrackSim.Hough.useVaryingChi2Cut and not flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis:
        flags.Trigger.FPGATrackSim.Hough.chi2cut = getChi2Cut(flags.Trigger.FPGATrackSim.region)
    assert not flags.Trigger.FPGATrackSim.pipeline.startswith('F-5'),"ERROR You are trying to run an F-5* pipeline! This is not yet supported!"

    if (flags.Trigger.FPGATrackSim.pipeline.startswith('F-1')):
        print("You are trying to run an F-100 pipeline! I am going to run the Data Prep chain for you and nothing else!")
        FPGATrackSimDataPrepConfig.runDataPrepChain()
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-2')):
        print("You are trying to run an F-2* pipeline! I am auto-configuring the 1D bitshift for you, including eta pattern filters and phi road filters")
        flags.Trigger.FPGATrackSim.Hough.etaPatternFilter = True
        flags.Trigger.FPGATrackSim.Hough.phiRoadFilter = True
        flags.Trigger.FPGATrackSim.Hough.hough1D = True
        flags.Trigger.FPGATrackSim.Hough.hough = False
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-3')):
        print("You are trying to run an F-3* pipeline! I am auto-configuring the 2D HT for you, and disabling the eta pattern filter and phi road filter. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.etaPatternFilter = False
        flags.Trigger.FPGATrackSim.Hough.phiRoadFilter = False
        flags.Trigger.FPGATrackSim.Hough.hough1D = False
        flags.Trigger.FPGATrackSim.Hough.hough = True
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-4')):
        print("You are trying to run an F-4* pipeline! I am auto-configuring the GNN pattern recognition for you. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.GNN = True
        flags.Trigger.FPGATrackSim.Hough.chi2cut = 40 # All of the track candidates have chi2 values around 20 for some reason. Needs further investigation. For now move the default cut value to 40
    elif (flags.Trigger.FPGATrackSim.pipeline.startswith('F-6')):
        print("You are trying to run an F-6* pipeline! I am auto-configuring the Inside-Out for you. Whether you wanted to or not")
        flags.Trigger.FPGATrackSim.Hough.genScan=True
        flags.Trigger.FPGATrackSim.spacePoints = flags.Trigger.FPGATrackSim.Hough.secondStage
    elif (flags.Trigger.FPGATrackSim.pipeline != ""):
        raise AssertionError("ERROR You are trying to run the pipeline " + flags.Trigger.FPGATrackSim.pipeline + " which is not yet supported!")

    if (not flags.Trigger.FPGATrackSim.pipeline.startswith('F-1')): ### if DP pipeline skip everything else!

        flags.Tracking.writeExtendedSi_PRDInfo = not flags.Trigger.FPGATrackSim.writeOfflPRDInfo
        splitPipeline=flags.Trigger.FPGATrackSim.pipeline.split('-')
        trackingOption=9999999
        if (len(splitPipeline) > 1): trackingOption=int(splitPipeline[1])
        if (trackingOption < 9999999):
            trackingOptionMod = (trackingOption % 100)
            if (trackingOptionMod == 0):
                print("You are trying to run the linearized chi2 fit as part of a pipeline! I am going to enable this for you whether you want to or not")
                flags.Trigger.FPGATrackSim.tracking = True
                flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = False
            elif (trackingOptionMod == 10):
                print("You are trying to run the second stage NN fake rejection as part of a pipeline! I am going to enable this for you whether you want to or not")
                flags.Trigger.FPGATrackSim.tracking = True
                flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = True ### enable the nn tool
                flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis2nd = flags.Trigger.FPGATrackSim.Hough.secondStage
                if (flags.Trigger.FPGATrackSim.pipeline.startswith('F-6')):
                    flags.Trigger.FPGATrackSim.doNNPathFinder = True
                    flags.Trigger.FPGATrackSim.doOverlapRemoval = False ## disable 1st stage overlap removal
                    flags.Trigger.FPGATrackSim.tracking = False
                    flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis = False
                    flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis2nd = flags.Trigger.FPGATrackSim.Hough.secondStage
            else:
                raise AssertionError("ERROR Your tracking option for the pipeline = " + str(trackingOption) + " is not yet supported!")

        if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
            log.info("wrapperFile is string, converting to list")
            flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
            flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]

        if flags.Trigger.FPGATrackSim.Hough.useVaryingChi2Cut and flags.Trigger.FPGATrackSim.Hough.trackNNAnalysis:
            flags.Trigger.FPGATrackSim.Hough.chi2cut = getChi2CutNN(flags.Trigger.FPGATrackSim.region)

        flags.lock()
        flags.dump()
        flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
        acc=MainServicesCfg(flags)

        if flags.Trigger.FPGATrackSim.writeAdditionalOutputData:
            acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))

            if (flags.Trigger.FPGATrackSim.Hough.houghRootoutput1st | flags.Trigger.FPGATrackSim.Hough.houghRootoutput2nd):
                acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimHOUGHOUTPUT DATAFILE='HoughRootOutput.root', OPT='RECREATE'"]))

            if flags.Trigger.FPGATrackSim.Hough.writeTestOutput:
                acc.addService(CompFactory.THistSvc(Output = ["FPGATRACKSIMOUTPUT DATAFILE='test.root', OPT='RECREATE'"]))

            if (flags.Trigger.FPGATrackSim.Hough.genScan):
                acc.addService(CompFactory.THistSvc(Output = ["GENSCAN DATAFILE='genscan.root', OPT='RECREATE'"]))

        if not flags.Trigger.FPGATrackSim.wrapperFileName:
            from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
            acc.merge(PoolReadCfg(flags))

            if flags.Input.isMC:
                from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
                acc.merge(GEN_AOD2xAODCfg(flags))

                from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
                acc.merge(addTruthPileupJetsToOutputCfg(flags))

            if flags.Detector.EnableCalo:
                from CaloRec.CaloRecoConfig import CaloRecoCfg
                acc.merge(CaloRecoCfg(flags))

            if flags.Tracking.recoChain:
                from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
                acc.merge(InDetTrackRecoCfg(flags))
                if flags.Trigger.FPGATrackSim.writeOfflPRDInfo: 
                    from InDetConfig.InDetPrepRawDataToxAODConfig import ITkActsPrepDataToxAODCfg
                    acc.merge( ITkActsPrepDataToxAODCfg( flags,
                                    PixelMeasurementContainer = "ITkPixelMeasurements_offl",
                                    StripMeasurementContainer = "ITkStripMeasurements_offl" ) )
                from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
                acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )
                from InDetConfig.InDetPrepRawDataFormationConfig import ITkXAODToInDetClusterConversionCfg
                acc.merge(ITkXAODToInDetClusterConversionCfg(flags))
    

        # Configure both the dataprep and logical hits algorithms.
        acc.merge(FPGATrackSimDataPrepConfig.FPGATrackSimDataPrepAlgCfg(flags))

        from FPGATrackSimConfTools.FPGATrackSimMultiRegionConfig import FPGATrackSimMultiRegionTrackingCfg
        acc.merge(FPGATrackSimMultiRegionTrackingCfg(flags))

        if flags.Trigger.FPGATrackSim.doEDMConversion:
            stage = "_2nd" if flags.Trigger.FPGATrackSim.Hough.secondStage else "_1st"
            acc.merge(FPGATrackSimDataPrepConfig.FPGAConversionAlgCfg(flags, name = f"FPGAConversionAlg{stage}",
                                                                        stage = f"{stage}",
                                                                        doActsTrk=True,
                                                                        doSP=flags.Trigger.FPGATrackSim.convertSPs))

            from FPGATrackSimPrototrackFitter.FPGATrackSimPrototrackFitterConfig import FPGATruthDecorationCfg, FPGAProtoTrackFitCfg
            acc.merge(FPGAProtoTrackFitCfg(flags,stage=f"{stage}")) # Run ACTS KF
            acc.merge(FPGATruthDecorationCfg(flags,FinalProtoTrackChainxAODTracksKey=FinalProtoTrackChainxAODTracksKey)) # Run Truth Matching/Decoration chain
            if flags.Trigger.FPGATrackSim.runCKF:
                from FPGATrackSimConfTools.FPGATrackExtensionConfig import FPGATrackExtensionAlgCfg
                acc.merge(FPGATrackExtensionAlgCfg(flags, enableTrackStatePrinter=False, name="FPGATrackExtension",
                                                    ProtoTracksLocation=f"ActsProtoTracks{stage}FromFPGATrack")) # run CKF track extension on FPGA tracks

            if flags.Trigger.FPGATrackSim.writeToAOD:
                acc.merge(FPGATrackSimDataPrepConfig.WriteToAOD(flags,
                                                                stage = f"{stage}",
                                                                finalTrackParticles=f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"))

            # Reporting algorithm (used for debugging - can be disabled)
            from FPGATrackSimReporting.FPGATrackSimReportingConfig import FPGATrackSimReportingCfg
            acc.merge(FPGATrackSimReportingCfg(flags, stage=f"{stage}",
                                                perEventReports = ((flags.Trigger.FPGATrackSim.sampleType != 'skipTruth') and flags.Exec.MaxEvents<=10 ) )) # disable perEventReports for pileup samples or many events

        acc.store(open('AnalysisConfig.pkl','wb'))
      
        acc.foreach_component("*FPGATrackSim*").OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        if flags.Trigger.FPGATrackSim.msgLimit!=-1:
            acc.getService("MessageSvc").debugLimit = flags.Trigger.FPGATrackSim.msgLimit
            acc.getService("MessageSvc").infoLimit = flags.Trigger.FPGATrackSim.msgLimit
            acc.getService("MessageSvc").verboseLimit = flags.Trigger.FPGATrackSim.msgLimit

        statusCode = acc.run(flags.Exec.MaxEvents)
        assert statusCode.isSuccess() is True, "Application execution did not succeed"
