# Configuration for the CALO-improved GSF re-fit

class GSFCaloImprovementTools:
    def __init__(self, derivation):
        prefix = derivation + "_GSFCaloImp_"

        from AthenaCommon.AppMgr import ToolSvc
        from AthenaCommon.DetFlags import DetFlags

        from CaloDetDescr.CaloDetDescrConf import CaloDepthTool
        self.CaloDepthTool = CaloDepthTool(name = prefix + "CaloDepthTool")
        ToolSvc += self.CaloDepthTool
        print self.CaloDepthTool

        from CaloTrackingGeometry.CaloTrackingGeometryConf import CaloSurfaceBuilder
        self.CaloSurfaceBuilder = CaloSurfaceBuilder(name          = prefix + "CaloSurfaceBuilder",
                                                     CaloDepthTool = self.CaloDepthTool)
        ToolSvc += self.CaloSurfaceBuilder
        print self.CaloSurfaceBuilder

        from egammaTrackTools.egammaTrackToolsConf import CaloCluster_OnTrackBuilder
        self.CCOTBuilder = CaloCluster_OnTrackBuilder(name               = prefix + "CCOTBuilder",
                                                      CaloSurfaceBuilder = self.CaloSurfaceBuilder)
        ToolSvc += self.CCOTBuilder
        print self.CCOTBuilder

        from egammaRec.EMCommonRefitter import GSFTrackFitter
        from egammaTrackTools.egammaTrackToolsConf import egammaTrkRefitterTool
        self.RefitterTool = egammaTrkRefitterTool(name                = prefix + "RefitterTool",
                                                  FitterTool          = GSFTrackFitter,
                                                  CCOTBuilder         = self.CCOTBuilder,
                                                  useBeamSpot         = False,
                                                  useClusterPosition  = True,
                                                  ReintegrateOutliers = True)
        ToolSvc += self.RefitterTool
        print self.RefitterTool

        from egammaTools.InDetTools import egammaExtrapolator
        self.Extrapolator = egammaExtrapolator()

        from InDetAssociationTools.InDetAssociationToolsConf import InDet__InDetPRD_AssociationToolGangedPixels
        self.IDPrdAssociationTool = InDet__InDetPRD_AssociationToolGangedPixels(name                           = prefix + "IDPrdAssociationTool",
                                                                                PixelClusterAmbiguitiesMapName = "PixelClusterAmbiguitiesMap")
        ToolSvc += self.IDPrdAssociationTool
        print self.IDPrdAssociationTool

        from InDetTrackHoleSearch.InDetTrackHoleSearchConf import InDet__InDetTrackHoleSearchTool
        self.HoleSearchTool = InDet__InDetTrackHoleSearchTool(name                         = prefix + "HoleSearchTool",
                                                              Extrapolator                 = self.Extrapolator,
                                                              usePixel                     = DetFlags.haveRIO.pixel_on(),
                                                              useSCT                       = DetFlags.haveRIO.SCT_on(),
                                                              checkBadSCTChip              = InDetFlags.checkDeadElementsOnTrack(),
                                                              CountDeadModulesAfterLastHit = True)

        from AthenaCommon.AppMgr import ServiceMgr
        if DetFlags.haveRIO.SCT_on():
            from SCT_ConditionsServices.SCT_ConditionsServicesConf import SCT_ConditionsSummarySvc
            InDetSCT_ConditionsSummarySvc = SCT_ConditionsSummarySvc(name = "InDetSCT_ConditionsSummarySvc")
            ServiceMgr += InDetSCT_ConditionsSummarySvc
            self.HoleSearchTool.SctSummarySvc = ServiceMgr.InDetSCT_ConditionsSummarySvc
        else:
            self.HoleSearchTool.SctSummarySvc = None

        ToolSvc += self.HoleSearchTool
        print self.HoleSearchTool

        self.TestBLayerTool = None
        if DetFlags.haveRIO.pixel_on():
            from InDetTestBLayer.InDetTestBLayerConf import InDet__InDetTestBLayerTool
            from PixelConditionsServices.PixelConditionsServicesConf import PixelConditionsSummarySvc
            ServiceMgr += PixelConditionsSummarySvc()
            self.TestBLayerTool = InDet__InDetTestBLayerTool(name            = prefix + "TestBLayerTool",
                                                             PixelSummarySvc = ServiceMgr.PixelConditionsSummarySvc,
                                                             Extrapolator    = self.Extrapolator)
            ToolSvc += self.TestBLayerTool
            print self.TestBLayerTool

        self.TRT_ElectronPidTool = None
        if DetFlags.haveRIO.TRT_on() and not InDetFlags.doSLHC() and not InDetFlags.doHighPileup():
            from TRT_ElectronPidTools.TRT_ElectronPidToolsConf import InDet__TRT_LocalOccupancy
            self.TRT_LocalOccupancy = InDet__TRT_LocalOccupancy(name = prefix + "TRT_LocalOccupancy")
            ToolSvc += self.TRT_LocalOccupancy
            print self.TRT_LocalOccupancy

            from TRT_ElectronPidTools.TRT_ElectronPidToolsConf import InDet__TRT_ElectronPidToolRun2
            self.TRT_ElectronPidTool = InDet__TRT_ElectronPidToolRun2(name                   = prefix + "TRT_ElectronPidTool",
                                                                      TRT_LocalOccupancyTool = self.TRT_LocalOccupancy,
                                                                      isData                 = (globalflags.DataSource == "data") )
            ToolSvc += self.TRT_ElectronPidTool
            print self.TRT_ElectronPidTool

        self.PixelToTPIDTool = None
        if DetFlags.haveRIO.pixel_on():
            from PixelToTPIDTool.PixelToTPIDToolConf import InDet__PixelToTPIDTool
            self.PixelToTPIDTool = InDet__PixelToTPIDTool(name = prefix + "PixelToTPIDTool")
            self.PixelToTPIDTool.ReadFromCOOL = True
            ToolSvc += self.PixelToTPIDTool
            print self.PixelToTPIDTool

        from InDetTrackSummaryHelperTool.InDetTrackSummaryHelperToolConf import InDet__InDetTrackSummaryHelperTool
        self.TrackSummaryHelperTool = InDet__InDetTrackSummaryHelperTool(name            = prefix + "TrackSummaryHelperTool",
                                                                         AssoTool        = self.IDPrdAssociationTool,
                                                                         PixelToTPIDTool = self.PixelToTPIDTool,
                                                                         TestBLayerTool  = self.TestBLayerTool,
                                                                         DoSharedHits    = False,
                                                                         HoleSearch      = self.HoleSearchTool,
                                                                         usePixel        = DetFlags.haveRIO.pixel_on(),
                                                                         useSCT          = DetFlags.haveRIO.SCT_on(),
                                                                         useTRT          = DetFlags.haveRIO.TRT_on())
        ToolSvc += self.TrackSummaryHelperTool
        print self.TrackSummaryHelperTool

        from TrkTrackSummaryTool.TrkTrackSummaryToolConf import Trk__TrackSummaryTool
        self.TrackSummaryTool = Trk__TrackSummaryTool(name                   = prefix + "TrackSummaryTool",
                                                      InDetSummaryHelperTool = self.TrackSummaryHelperTool,
                                                      doSharedHits           = False,
                                                      InDetHoleSearchTool    = self.HoleSearchTool,
                                                      TRT_ElectronPidTool    = self.TRT_ElectronPidTool,
                                                      PixelToTPIDTool        = self.PixelToTPIDTool)
        ToolSvc += self.TrackSummaryTool
        print self.TrackSummaryTool

        from TrkParticleCreator.TrkParticleCreatorConf import Trk__TrackParticleCreatorTool
        self.TPCTool = Trk__TrackParticleCreatorTool(name                    = prefix + "TPCTool",
                                                     Extrapolator            = self.Extrapolator,
                                                     TrackSummaryTool        = self.TrackSummaryTool,
                                                     KeepParameters          = True,
                                                     UseTrackSummaryTool     = False,
                                                     ForceTrackSummaryUpdate = False)
        ToolSvc += self.TPCTool
        print self.TPCTool
