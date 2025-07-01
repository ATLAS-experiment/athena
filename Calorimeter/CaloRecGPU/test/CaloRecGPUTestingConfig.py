# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
class PlotterConfigurator:
    #DoCells currently changes nothing
    #(originally was intended to show
    #that we had cells with the same energy),
    #keeping it here to remain part of the interface
    #if/when we port the cell maker as well.
    def __init__ (self, StepsToPlot = [], PairsToPlot = [], DoStandard = True, DoMoments = False, DoCells = False):
        self.PlotsToDo = []
        if DoStandard:
            for step in StepsToPlot:
                self.PlotsToDo += [
                                    ( (step + "_cluster_E",),
                                      {'type': 'TH1F',
                                       'title': "Cluster Energy; E [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_Et",),
                                      {'type': 'TH1F',
                                       'title': "Cluster Transverse Energy; E_T [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #eta; #eta; Number of Events",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_phi",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #phi; #phi; Number of Events",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta," + step + "_cluster_phi",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta versus #phi; #eta; #phi",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  61,
                                       'ymin':  -3.25,
                                       'ymax':   3.25,
                                       'path': "EXPERT"}
                                    )
                                  ]
            for pair in PairsToPlot:
                self.PlotsToDo += [
                                    ( (pair + "_num_unmatched_clusters",),
                                      {'type': 'TH1F',
                                       'title': "Number of Unmatched Clusters; # of Unmatched Clusters; Number of Events",
                                       'xbins':  21,
                                       'xmin':  -0.5,
                                       'xmax':   20.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_0",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -25,
                                       'xmax':   1025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_1",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -12.5,
                                       'xmax':   512.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_2",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -2.5,
                                       'xmax':   102.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_3",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -0.5,
                                       'xmax':   20.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_E_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Comparison; E^{(CPU)} [MeV]; E^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_delta_E_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Resolution; E^{(CPU)} [MeV]; #Delta E / #(){E^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_Et_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Comparison; E_T^{(CPU)} [MeV]; E_T^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_delta_Et_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Resolution; E_T^{(CPU)} [MeV]; #Delta E_T / #(){E_T^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_E_test;" + pair + "_E_zoom2",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Comparison; E^{(CPU)} [MeV]; E^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'ybins':  63,
                                       'ymin':  -5020,
                                       'ymax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_delta_E_rel_ref;" + pair + "_delta_E_rel_ref_zoom2",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Resolution; E^{(CPU)} [MeV]; #Delta E / #(){E^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'ybins':  63,
                                       'ymin':  -0.05,
                                       'ymax':   0.05,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_Et_test;" + pair + "_Et_zoom2",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Comparison; E_T^{(CPU)} [MeV]; E_T^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'ybins':  63,
                                       'ymin':  -5020,
                                       'ymax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_delta_Et_rel_ref;" + pair + "_delta_Et_rel_ref_zoom2",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Resolution; E_T^{(CPU)} [MeV]; #Delta E_T / #(){E_T^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'ybins':  63,
                                       'ymin':  -0.05,
                                       'ymax':   0.05,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_eta_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta Comparison; #eta^{(CPU)}; #eta^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -10.5,
                                       'ymax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_delta_eta_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta Resolution; #eta^{(CPU)}; #Delta #eta / #(){#eta^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_phi_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #phi Comparison; #phi^{(CPU)}; #phi^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -3.3,
                                       'xmax':   3.3,
                                       'ybins':  63,
                                       'ymin':  -3.3,
                                       'ymax':   3.3,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #phi Resolution; #phi^{(CPU)}; #Delta #phi",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #phi; #phi; Number of Clusters",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cell_secondary_weight_ref," + pair + "_cell_secondary_weight_test",),
                                      {'type': 'TH2F',
                                       'title': "Shared Cell Secondary Weight Comparison; w_2^{(CPU)}; w_2^{(GPU)}",
                                       'xbins': 51,
                                       'xmin':  -0.05,
                                       'xmax':  0.505,
                                       'ybins': 51,
                                       'ymin':  -0.05,
                                       'ymax':  0.505,
                                       'path': "EXPERT"}
                                    )
                                  ]
        if DoMoments:
            for pair in PairsToPlot:
                for mom, (prettynames, limits) in name_to_moment_map.items():
                    for i in range(0, len(limits)):
                      self.PlotsToDo += [ ( (pair + "_cluster_moments_" + mom + "_ref," + pair + "_cluster_moments_" + mom + "_test;" + pair + "_" + mom + "_versus_zoom_" + str(i + 1),),
                                            {'type':  'TH2F',
                                             'title': prettynames[0] + " (" + str(i+1) + "); CPU " + prettynames[1] + "; GPU " + prettynames[1],
                                             'xbins': 63,
                                             'xmin':  limits[i][0],
                                             'xmax':  limits[i][1],
                                             'ybins': 63,
                                             'ymin':  limits[i][0],
                                             'ymax':  limits[i][1],
                                             'path':  "EXPERT"},
                                          ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_moments_" + mom + "_ref," + pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_" + mom + "_error_zoom_" + str(i + 1),),
                                            {'type':   'TH2F',
                                             'title': prettynames[0] + " (" + str(i+1) + "); CPU " + prettynames[1] + "; #Delta " + prettynames[1] + " / #(){CPU " + prettynames[1] + "}",
                                             'xbins':  63,
                                             'xmin':   limits[i][0],
                                             'xmax':   limits[i][1],
                                             'ybins':  63,
                                             'ymin':  -0.025,
                                             'ymax':   0.025,
                                             'path':   "EXPERT"}
                                          ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_cluster_delta_moments_" + mom + "_rel_ref_zoom_0",),
                                            {'type': 'TH1F',
                                             'title': prettynames[0] + "; #Delta " + prettynames[1] + "; Number of Clusters",
                                             'xbins': 51,
                                             'xmin':  -1,
                                             'xmax':  1,
                                             'path': "EXPERT"}
                                           ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_cluster_delta_moments_" + mom + "_rel_ref_zoom_1",),
                                            {'type': 'TH1F',
                                             'title': prettynames[0] + "; #Delta " + prettynames[1] + "; Number of Clusters",
                                             'xbins': 51,
                                             'xmin':  -0.1,
                                             'xmax':  0.1,
                                             'path': "EXPERT"}
                                           ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_cluster_delta_moments_" + mom + "_rel_ref_zoom_2",),
                                            {'type': 'TH1F',
                                             'title': prettynames[0] + "; #Delta " + prettynames[1] + "; Number of Clusters",
                                             'xbins': 51,
                                             'xmin':  -0.05,
                                             'xmax':  0.05,
                                             'path': "EXPERT"}
                                           ),
                                        ]
    def __call__(self, Plotter):
        for plotdef in self.PlotsToDo:
            Plotter.MonitoringTool.defineHistogram(*plotdef[0], **plotdef[1])
        return Plotter

#For pretty printing things in axes when it comes to moments:
#<MOMENT_NAME>: <PLOT TITLE> <AXIS TITLE> <UNITS>
name_to_moment_map =  {
    "time"                        :  (("time",                "time",                ""),[(0, 1e-3)]),
    "FIRST_PHI"                   :  (("firstPhi",            "firstPhi",            ""),[(-3.2, 3.2)]),
    "FIRST_ETA"                   :  (("firstEta",            "firstEta",            ""),[(-10.1, 10.1)]),
    "SECOND_R"                    :  (("secondR",             "secondR",             ""),[(0, 1.25e6)]),
    "SECOND_LAMBDA"               :  (("secondLambda",        "secondLambda",        ""),[(0, 2.5e6)]),
    "DELTA_PHI"                   :  (("deltaPhi",            "deltaPhi",            ""),[(-3.2, 3.2)]),
    "DELTA_THETA"                 :  (("deltaTheta",          "deltaTheta",          ""),[(-1.6, 1.6)]),
    "DELTA_ALPHA"                 :  (("deltaAlpha",          "deltaAlpha",          ""),[(-0.1, 1.6)]),
    "CENTER_X"                    :  (("centerX",             "centerX",             ""),[(-4000, 4000)]),
    "CENTER_Y"                    :  (("centerY",             "centerY",             ""),[(-4000, 4000)]),
    "CENTER_Z"                    :  (("centerZ",             "centerZ",             ""),[(-7000, 7000)]),
    "CENTER_MAG"                  :  (("centerMag",           "centerMag",           ""),[(1000, 7500)]),
    "CENTER_LAMBDA"               :  (("centerLambda",        "centerLambda",        ""),[(0., 25000.),(0., 5000.)]),
    "LATERAL"                     :  (("lateral",             "lateral",             ""),[(-0.05, 1.05)]),
    "LONGITUDINAL"                :  (("longitudinal",        "longitudinal",        ""),[(-0.05, 1.05)]),
    "ENG_FRAC_EM"                 :  (("engFracEM",           "engFracEM",           ""),[(-0.1, 1.1)]),
    "ENG_FRAC_MAX"                :  (("engFracMax",          "engFracMax",          ""),[(-0.1, 1.1)]),
    "ENG_FRAC_CORE"               :  (("engFracCore",         "engFracCore",         ""),[(-0.1, 1.1)]),
    "FIRST_ENG_DENS"              :  (("firstEngDens",        "firstEngDens",        ""),[(-0.1, 5.1)]),
    "SECOND_ENG_DENS"             :  (("secondEngDens",       "secondEngDens",       ""),[(-0.5, 50.5)]),
    "ISOLATION"                   :  (("isolation",           "isolation",           ""),[(-0.05, 2.05)]),
    "ENG_BAD_CELLS"               :  (("engBadCells",         "engBadCells",         ""),[(0., 100000.)]),
    "N_BAD_CELLS"                 :  (("nBadCells",           "nBadCells",           ""),[(-0.5, 25.5)]),
    "N_BAD_CELLS_CORR"            :  (("nBadCellsCorr",       "nBadCellsCorr",       ""),[(-0.5, 25.5)]),
    "BAD_CELLS_CORR_E"            :  (("badCellsCorrE",       "badCellsCorrE",       ""),[(-0.1, 25000.)]),
    "BADLARQ_FRAC"                :  (("badLArQFrac",         "badLArQFrac",         ""),[(-2500., 2500.),(-5., 5.)]),
    "ENG_POS"                     :  (("engPos",              "engPos",              ""),[(-0.1, 250000.)]),
    "SIGNIFICANCE"                :  (("significance",        "significance",        ""),[(-500., 500.)]),
    "CELL_SIGNIFICANCE"           :  (("cellSignificance",    "cellSignificance",    ""),[(-0.1, 100.)]),
    "CELL_SIG_SAMPLING"           :  (("cellSigSampling",     "cellSigSampling",     ""),[(-0.1, 30.)]),
    "AVG_LAR_Q"                   :  (("avgLArQ",             "avgLArQ",             ""),[(-0.1, 70000.)]),
    "AVG_TILE_Q"                  :  (("avgTileQ",            "avgTileQ",            ""),[(-0.1, 300.)]),
   #"ENG_BAD_HV_CELLS"            :  (("engBadHVCells",       "engBadHVCells",       ""),[]),
   #"N_BAD_HV_CELLS"              :  (("nBadHVCells",         "nBadHVCells",         ""),[]),
    "PTD"                         :  (("PTD",                 "PTD",                 ""),[(-0.05, 1.05)]),
    "MASS"                        :  (("mass",                "mass",                ""),[(0., 200000.), (0., 100.)]),
   #"EM_PROBABILITY"              :  (("EMProbability",       "EMProbability",       ""),[]),
   #"HAD_WEIGHT"                  :  (("hadWeight",           "hadWeight",           ""),[]),
   #"OOC_WEIGHT"                  :  (("OOCweight",           "OOCweight",           ""),[]),
   #"DM_WEIGHT"                   :  (("DMweight",            "DMweight",            ""),[]),
   #"TILE_CONFIDENCE_LEVEL"       :  (("tileConfidenceLevel", "tileConfidenceLevel", ""),[]),
    "SECOND_TIME"                 :  (("secondTime",          "secondTime",          ""),[(0., 1e7),(0., 100.)])
   #"number_of_cells"             :  (("numCells",            "numCells",            ""),[]),
   #"VERTEX_FRACTION"             :  (("vertexFraction",      "vertexFraction",      ""),[]),
   #"NVERTEX_FRACTION"            :  (("nVertexFraction",     "nVertexFraction",     ""),[]),
   #"ETACALOFRAME"                :  (("etaCaloFrame",        "etaCaloFrame",        ""),[]),
   #"PHICALOFRAME"                :  (("phiCaloFrame",        "phiCaloFrame",        ""),[]),
   #"ETA1CALOFRAME"               :  (("eta1CaloFrame",       "eta1CaloFrame",       ""),[]),
   #"PHI1CALOFRAME"               :  (("phi1CaloFrame",       "phi1CaloFrame",       ""),[]),
   #"ETA2CALOFRAME"               :  (("eta2CaloFrame",       "eta2CaloFrame",       ""),[]),
   #"PHI2CALOFRAME"               :  (("phi2CaloFrame",       "phi2CaloFrame",       ""),[]),
   #"ENG_CALIB_TOT"               :  (("engCalibTot",         "engCalibTot",         ""),[]),
   #"ENG_CALIB_OUT_L"             :  (("engCalibOutL",        "engCalibOutL",        ""),[]),
   #"ENG_CALIB_OUT_M"             :  (("engCalibOutM",        "engCalibOutM",        ""),[]),
   #"ENG_CALIB_OUT_T"             :  (("engCalibOutT",        "engCalibOutT",        ""),[]),
   #"ENG_CALIB_DEAD_L"            :  (("engCalibDeadL",       "engCalibDeadL",       ""),[]),
   #"ENG_CALIB_DEAD_M"            :  (("engCalibDeadM",       "engCalibDeadM",       ""),[]),
   #"ENG_CALIB_DEAD_T"            :  (("engCalibDeadT",       "engCalibDeadT",       ""),[]),
   #"ENG_CALIB_EMB0"              :  (("engCalibEMB0",        "engCalibEMB0",        ""),[]),
   #"ENG_CALIB_EME0"              :  (("engCalibEME0",        "engCalibEME0",        ""),[]),
   #"ENG_CALIB_TILEG3"            :  (("engCalibTileG3",      "engCalibTileG3",      ""),[]),
   #"ENG_CALIB_DEAD_TOT"          :  (("engCalibDeadTot",     "engCalibDeadTot",     ""),[]),
   #"ENG_CALIB_DEAD_EMB0"         :  (("engCalibDeadEMB0",    "engCalibDeadEMB0",    ""),[]),
   #"ENG_CALIB_DEAD_TILE0"        :  (("engCalibDeadTile0",   "engCalibDeadTile0",   ""),[]),
   #"ENG_CALIB_DEAD_TILEG3"       :  (("engCalibDeadTileG3",  "engCalibDeadTileG3",  ""),[]),
   #"ENG_CALIB_DEAD_EME0"         :  (("engCalibDeadEME0",    "engCalibDeadEME0",    ""),[]),
   #"ENG_CALIB_DEAD_HEC0"         :  (("engCalibDeadHEC0",    "engCalibDeadHEC0",    ""),[]),
   #"ENG_CALIB_DEAD_FCAL"         :  (("engCalibDeadFCAL",    "engCalibDeadFCAL",    ""),[]),
   #"ENG_CALIB_DEAD_LEAKAGE"      :  (("engCalibDeadLeakage", "engCalibDeadLeakage", ""),[]),
   #"ENG_CALIB_DEAD_UNCLASS"      :  (("engCalibDeadUnclass", "engCalibDeadUnclass", ""),[]),
   #"ENG_CALIB_FRAC_EM"           :  (("engCalibFracEM",      "engCalibFracEM",      ""),[]),
   #"ENG_CALIB_FRAC_HAD"          :  (("engCalibFracHad",     "engCalibFracHad",     ""),[]),
   #"ENG_CALIB_FRAC_REST"         :  (("engCalibFracRest"     "engCalibFracRest"     ""),[])
}

from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DefaultCaloCellMakerCfg(flags, cellname):
    from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
    from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
    from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg
    result=ComponentAccumulator()

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
                                       CaloCellsOutputName = cellname)
    result.addEventAlgo(cellAlgo)
    return result

def PrevAlgorithmsConfigurationCfg(flags, testoptions, cellsname):
    result=ComponentAccumulator()
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg

    result.merge(LArGMCfg(flags))
    result.merge(TileGMCfg(flags))
    
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
        result.merge(DefaultCaloCellMakerCfg(flags, cellsname))
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

from enum import Enum

TestTypes = Enum('TestType', ['RunAllGPU',           #Simply run everything that can be run on the GPU
                              'RunAllCPU',           #Simply run everything that can be run on the CPU
                              'Grow',                #Compare CPU and GPU growing
                              'Split',               #Compare CPU and GPU splitting
                              'GrowSplit',           #Compare CPU and GPU (growing + splitting)
                              'CrossTests',          #Compare (CPU or GPU) (growing or splitting)
                              'Moments',             #Compare CPU and GPU moments calculation
                              'GrowSplitMoments',    #Compare CPU and GPU full clusters + moments
                              'PostGPUCalib',        #Compare CPU and GPU full clusters + moments with CPU calibrations on top
                             ])


class TestOptions:
    def __init__(self):
        self.OutputClusters = False
        self.OutputCounts = False
        self.TestType = TestTypes.RunAllGPU
        self.DoCPULocalCalib = False
        self.OutputCellInfo = False
        self.SkipSyncs = True
        self.UsePerfMon = False
        self.NumEvents = -1

def TestPlotterConfiguration(flags, testoptions, plotter_configurator, cellsname):
    result=ComponentAccumulator()
    
    from CaloRecGPU.CaloRecGPUConfig import PlotterToolCfg, SingleToolToPlot, ComparedToolsToPlot
    
    Plotter = result.popToolsAndMerge(PlotterToolCfg(flags, cellsname))
  
    Plotter = plotter_configurator(Plotter)
    
    #In general: 1) add the name of tools whose resulting clusters we want to plot
    #               as SingleToolToPlot(tool name, prefix we want for the plots)
    #            2) add the names of tools whose resulting clusters we want to compare
    #               as ComparedToolsToPlot(reference tool name, test tool name, prefix for the plots, match in energy or not)
    
    plot_grow = (testoptions.TestType is TestTypes.Grow or testoptions.TestType is TestTypes.GrowSplit or testoptions.TestType is TestTypes.GrowSplitMoments)
    plot_split = (testoptions.TestType is TestTypes.Split or testoptions.TestType is TestTypes.GrowSplit)
    plot_moments = (testoptions.TestType is TestTypes.Moments or testoptions.TestType is TestTypes.GrowSplitMoments)
    
    if plot_grow:
        Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultGrowing", "CPU_growing") ]
        Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostGrowing", "GPU_growing") ]
        Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultGrowing", "PropCalcPostGrowing", "growing") ]
    if plot_split:
        Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultSplitting", "CPU_splitting") ]
        Plotter.ToolsToPlot += [ SingleToolToPlot("PropCalcPostSplitting", "GPU_splitting") ]
        Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultSplitting", "PropCalcPostSplitting", "splitting", True) ]
    if plot_moments:
        Plotter.ToolsToPlot += [ SingleToolToPlot("DefaultMoments", "CPU_moments") ]
        Plotter.ToolsToPlot += [ SingleToolToPlot("AthenaClusterImporter", "GPU_moments") ]
        Plotter.PairsToPlot += [ ComparedToolsToPlot("DefaultMoments", "AthenaClusterImporter", "moments", True, match_perfectly = True) ]
        
    if testoptions.TestType is TestTypes.PostGPUCalib:
        pass
        #No plots for now, to be defined later.
    
    elif testoptions.TestType is TestTypes.CrossTests:
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
    
    result.setPrivateTools(Plotter)
    
    return result


def TestConstantConversionTool(flags, testoptions):
    #Return a component accumulator or None
    
    if testoptions.TestType is TestTypes.RunAllCPU:
        return None
    else:
        from CaloRecGPU.CaloRecGPUConfig import BasicConstantDataExporterToolCfg
        return BasicConstantDataExporterToolCfg(flags)
  
def TestCPUtoGPUExporterTool(flags, testoptions, cellsname):
    #Return a component accumulator or None
    
    if testoptions.TestType is TestTypes.RunAllCPU:
        return None
    else:
        from CaloRecGPU.CaloRecGPUConfig import BasicEventDataExporterToolCfg
        return BasicEventDataExporterToolCfg(flags, cellsname)
        
def TestGPUtoCPUImporterTool(flags, testoptions, cellsname):
    #Return a component accumulator or None
    
    only_simple_properties = ( testoptions.TestType is TestTypes.Grow        or
                               testoptions.TestType is TestTypes.Split       or
                               testoptions.TestType is TestTypes.GrowSplit   or
                               testoptions.TestType is TestTypes.CrossTests     )
    
    if testoptions.TestType is TestTypes.RunAllCPU:
        return None
    elif only_simple_properties:
        from CaloRecGPU.CaloRecGPUConfig import BasicAthenaClusterImporterToolCfg
        return BasicAthenaClusterImporterToolCfg(flags, cellsname,"AthenaClusterImporter")
    else:
        from CaloRecGPU.CaloRecGPUConfig import AthenaClusterAndMomentsImporterToolCfg
        return AthenaClusterAndMomentsImporterToolCfg(flags, cellsname, False, "AthenaClusterImporter")
    
def TestBeforeGPUToolsConfiguration(flags, testoptions, cellsname, clustersname):
    
    do_growing = ( testoptions.TestType is TestTypes.Grow              or
                   testoptions.TestType is TestTypes.Split             or
                   testoptions.TestType is TestTypes.GrowSplit         or
                   testoptions.TestType is TestTypes.CrossTests        or 
                   testoptions.TestType is TestTypes.Moments           or 
                   testoptions.TestType is TestTypes.GrowSplitMoments  or 
                   testoptions.TestType is TestTypes.RunAllCPU         or
                   testoptions.TestType is TestTypes.PostGPUCalib         )
                   
    do_splitting = ( testoptions.TestType is TestTypes.Split             or
                     testoptions.TestType is TestTypes.GrowSplit         or
                     testoptions.TestType is TestTypes.CrossTests        or 
                     testoptions.TestType is TestTypes.Moments           or 
                     testoptions.TestType is TestTypes.GrowSplitMoments  or 
                     testoptions.TestType is TestTypes.RunAllCPU         or
                     testoptions.TestType is TestTypes.PostGPUCalib         )
                 
    do_moments = ( testoptions.TestType is TestTypes.Moments           or 
                   testoptions.TestType is TestTypes.GrowSplitMoments  or 
                   testoptions.TestType is TestTypes.RunAllCPU         or
                   testoptions.TestType is TestTypes.PostGPUCalib         )
                   
    do_calib = ( testoptions.TestType is TestTypes.PostGPUCalib )
    
    clear_clusters = ( testoptions.TestType is TestTypes.Grow              or
                       testoptions.TestType is TestTypes.Split             or
                       testoptions.TestType is TestTypes.GrowSplit         or
                       testoptions.TestType is TestTypes.CrossTests        or 
                       testoptions.TestType is TestTypes.Moments           or 
                       testoptions.TestType is TestTypes.GrowSplitMoments  or 
                       testoptions.TestType is TestTypes.PostGPUCalib         )
    
    do_post_clear_grow = ( testoptions.TestType is TestTypes.Split        or
                           testoptions.TestType is TestTypes.CrossTests   or 
                           testoptions.TestType is TestTypes.Moments         )
                           
    do_post_clear_split = ( testoptions.TestType is TestTypes.Moments )
    
    do_post_clear_moments = False
    #Potentially useful for future work on calibrations
    
    from CaloRecGPU.CaloRecGPUConfig import ( DefaultTopologicalClusteringToolCfg,
                                              DefaultClusterSplittingToolCfg,
                                              DefaultClusterMomentsCalculatorToolCfg,
                                              MomentsDumperToolCfg,
                                              CellsCounterCPUToolCfg,
                                              CPUOutputToolCfg,
                                              DefaultTopoClusterLocalCalibToolsCfg )
    
    
    return_tools = []
    
    result = ComponentAccumulator()
    
    if do_growing:
        DefaultClustering = result.popToolsAndMerge( DefaultTopologicalClusteringToolCfg(flags, cellsname,"DefaultGrowing") )
        
        return_tools += [DefaultClustering]
        
        if testoptions.OutputCounts:
            CPUCount1 = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,cellsname,"DefaultGrowCounter", SavePath = "./counts", FilePrefix = "default_grow") )
            return_tools += [CPUCount1]
        if testoptions.OutputClusters:
            CPUOut1 = result.popToolsAndMerge( CPUOutputToolCfg(flags,cellsname,"DefaultGrowOutput", SavePath = "./out_default_grow") )
            return_tools += [CPUOut1]
    
    if do_splitting:
        DefaultSplitter = result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"DefaultSplitting") )
        return_tools += [DefaultSplitter]
        
        if testoptions.OutputCounts:
            CPUCount2 = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,cellsname,"DefaultGrowAndSplitCounter", SavePath = "./counts", FilePrefix = "default_grow_split") )
            return_tools += [CPUCount2]
        if testoptions.OutputClusters:
            CPUOut2 = result.popToolsAndMerge( CPUOutputToolCfg(flags, cellsname,"DefaultGrowAndSplitOutput", SavePath = "./out_default_grow_split") )
            return_tools += [CPUOut2]
        
    if do_moments:
        DefaultMoments = result.popToolsAndMerge( DefaultClusterMomentsCalculatorToolCfg(flags, False,"DefaultMoments") )
        return_tools += [DefaultMoments]
        if testoptions.OutputCounts:
            CPUDumper = result.popToolsAndMerge( MomentsDumperToolCfg(flags,"DefaultMomentsDumper", SavePath = "./moments", FilePrefix = "CPU") )
            return_tools += [CPUDumper]
                   
    if do_calib:
        from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
        caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(flags, name = "DefaultCaloBadChanTool") )
        return_tools += [CompFactory.CaloClusterBadChannelList("DefaultBadChannelList", badChannelTool = caloBadChanTool)]
        
        calibTools = result.popToolsAndMerge(DefaultTopoClusterLocalCalibToolsCfg(flags, False, "Default"))
        return_tools += calibTools
        #This is already a tool array.
        
        from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
        result.merge(caloTopoCoolFolderCfg(flags))
      
    if clear_clusters:
          return_tools += [CompFactory.CaloClusterDeleter("ClusterDeleter")]
    
    if do_post_clear_grow:
        SecondDefaultClustering = result.popToolsAndMerge( DefaultTopologicalClusteringToolCfg(flags,cellsname,"SecondDefaultGrowing") )
        return_tools += [SecondDefaultClustering]
        
    if do_post_clear_split:
        SecondDefaultSplitting = result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"SecondDefaultSpltting") )
        return_tools += [SecondDefaultSplitting]
    
    if do_post_clear_moments:
        SecondDefaultMoments = result.popToolsAndMerge( DefaultClusterMomentsCalculatorToolCfg(flags, False,"SecondDefaultMoments") )
        return_tools += [SecondDefaultMoments]
    
    result.setPrivateTools(return_tools)
    
    return result


def TestGPUToolsConfiguration(flags, testoptions, cellsname, clustersname):
                   
    from CaloRecGPU.CaloRecGPUConfig import ( GPUOutputToolCfg,
                                              TopoAutomatonSplitterToolCfg,
                                              ClusterInfoCalcToolCfg,
                                              CellsCounterGPUToolCfg,
                                              TopoAutomatonClusteringToolCfg,
                                              GPUClusterMomentsCalculatorToolCfg )
                            
    return_tools = []
    
    result = ComponentAccumulator()    

    if testoptions.OutputCellInfo:
        CellOut = result.popToolsAndMerge( GPUOutputToolCfg(flags,"CellInfoOutput", SavePath = "./out_cell", OnlyOutputCellInfo = True) )
        return_tools += [CellOut]
    
    if testoptions.TestType is TestTypes.CrossTests:
        GPUClusterSplitting1 = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitterCPUGrowing") )
        GPUClusterSplitting1.TimeFileOutput = "" #This means there's no output.
        GPUClusterSplitting1.MeasureTimes = False
        return_tools += [GPUClusterSplitting1]
        PropCalc1 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcDefaultGrowGPUSplit", do_cut = False) )
        PropCalc1.TimeFileOutput = "" #This means there's no output.
        PropCalc1.MeasureTimes = False
        return_tools += [PropCalc1]
        if testoptions.OutputCounts:
            GPUCount1 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"DefaultGrowModifiedSplitCounter", SavePath = "./counts", FilePrefix = "default_grow_modified_split") )
            return_tools += [GPUCount1]
        if testoptions.OutputClusters:
            GPUOut1 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"DefaultGrowModifiedSplitOutput", SavePath = "./out_default_grow_modified_split") )
            return_tools += [GPUOut1]
    
    do_growing = ( testoptions.TestType is TestTypes.Grow              or
                   testoptions.TestType is TestTypes.Split             or
                   testoptions.TestType is TestTypes.GrowSplit         or
                   testoptions.TestType is TestTypes.CrossTests        or 
                   testoptions.TestType is TestTypes.GrowSplitMoments  or 
                   testoptions.TestType is TestTypes.RunAllGPU         or
                   testoptions.TestType is TestTypes.PostGPUCalib         )
    
    do_splitting = ( testoptions.TestType is TestTypes.Split             or
                     testoptions.TestType is TestTypes.GrowSplit         or
                     testoptions.TestType is TestTypes.CrossTests        or 
                     testoptions.TestType is TestTypes.GrowSplitMoments  or 
                     testoptions.TestType is TestTypes.RunAllGPU         or
                     testoptions.TestType is TestTypes.PostGPUCalib         )
    
    do_moments = ( testoptions.TestType is TestTypes.Moments           or
                   testoptions.TestType is TestTypes.GrowSplitMoments  or 
                   testoptions.TestType is TestTypes.RunAllGPU         or
                   testoptions.TestType is TestTypes.PostGPUCalib         )
    
    if do_growing:
        TopoAutomatonClustering1 = result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"GPUGrowing") )
        if testoptions.SkipSyncs:
            TopoAutomatonClustering1.MeasureTimes = False
        return_tools += [TopoAutomatonClustering1]
        PropCalc2 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostGrowing") )
        if testoptions.SkipSyncs:
            PropCalc2.MeasureTimes = False
        return_tools += [PropCalc2]
        if testoptions.OutputCounts:
            GPUCount2 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"ModifiedGrowCounter", SavePath = "./counts", FilePrefix = "modified_grow") )
            return_tools += [GPUCount2]
        if testoptions.OutputClusters:
            GPUOut2 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"ModifiedGrowOutput", SavePath = "./out_modified_grow") )
            return_tools += [GPUOut2]
    
    if do_splitting:
        GPUClusterSplitting2 = result.popToolsAndMerge( TopoAutomatonSplitterToolCfg(flags,"GPUSplitter") )
        if testoptions.SkipSyncs:
            GPUClusterSplitting2.MeasureTimes = False
        return_tools += [GPUClusterSplitting2]
        if not do_moments:
            PropCalc3 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalcPostSplitting", False) )
            if testoptions.SkipSyncs:
                PropCalc3.MeasureTimes = False
            return_tools += [PropCalc3]
        if testoptions.OutputCounts:
            GPUCount3 = result.popToolsAndMerge( CellsCounterGPUToolCfg(flags,"ModifiedGrowSplitCounter", SavePath = "./counts", FilePrefix = "modified_grow_split") )
            return_tools += [GPUCount3]
        if testoptions.OutputClusters:
            GPUOut3 = result.popToolsAndMerge( GPUOutputToolCfg(flags,"ModifiedGrowSplitOutput", SavePath = "./out_modified_grow_split") )
            return_tools += [GPUOut3]
    
    if do_moments:
        GPUMomentsDef = result.popToolsAndMerge( GPUClusterMomentsCalculatorToolCfg(flags,"GPUTopoMoments") )
        if testoptions.SkipSyncs:
            GPUMomentsDef.MeasureTimes = False
        return_tools += [GPUMomentsDef]
        
    if testoptions.TestType is TestTypes.CrossTests:
        TopoAutomatonClustering2 = result.popToolsAndMerge( TopoAutomatonClusteringToolCfg(flags,"SecondGPUGrowing") )
        TopoAutomatonClustering2.MeasureTimes = False
        TopoAutomatonClustering2.TimeFileOutput = "" #This means there's no output.
        return_tools += [TopoAutomatonClustering2]
        
        PropCalc4 = result.popToolsAndMerge( ClusterInfoCalcToolCfg(flags,"PropCalc4", True) )
        PropCalc4.TimeFileOutput = "" #This means there's no output.
        PropCalc4.MeasureTimes = False
        return_tools += [PropCalc4]
    
    result.setPrivateTools(return_tools)
    
    return result
    
def TestAfterGPUToolsConfiguration(flags, testoptions, cellsname, clustersname):
                   
    from CaloRecGPU.CaloRecGPUConfig import ( MomentsDumperToolCfg,
                                              DefaultClusterSplittingToolCfg,
                                              CellsCounterCPUToolCfg,
                                              CPUOutputToolCfg,
                                              DefaultTopoClusterLocalCalibToolsCfg )
                            
    return_tools = []
    
    result = ComponentAccumulator()
    
    did_GPU_moments = ( testoptions.TestType is TestTypes.Moments  or
                        testoptions.TestType is TestTypes.GrowSplitMoments  or 
                        testoptions.TestType is TestTypes.RunAllGPU         or
                        testoptions.TestType is TestTypes.PostGPUCalib         )
    
    
    if did_GPU_moments and testoptions.OutputCounts:
        GPUDumper = result.popToolsAndMerge( MomentsDumperToolCfg(flags,"GPUMomentsDumper", SavePath = "./moments", FilePrefix = "GPU") )
        return_tools += [GPUDumper]
        
    if testoptions.TestType is TestTypes.CrossTests:
    
        TopoSplitter = result.popToolsAndMerge( DefaultClusterSplittingToolCfg(flags,"DefaultPostGPUSplitting") )
        return_tools += [TopoSplitter]
        if testoptions.OutputCounts:
            CPUCount = result.popToolsAndMerge( CellsCounterCPUToolCfg(flags,cellsname,"ModifiedGrowDefaultSplitCounter", SavePath = "./counts", FilePrefix = "modified_grow_default_split") )
            return_tools += [CPUCount]
        if testoptions.OutputClusters:
            CPUOut = result.popToolsAndMerge( CPUOutputToolCfg(flags,cellsname,"ModifiedGrowDefaultSplitOutput", SavePath = "./out_modified_grow_default_split") )
            return_tools += [CPUOut]
    
    if testoptions.TestType is TestTypes.PostGPUCalib:
        from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
        caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(flags, name = "ModifiedCaloBadChanTool") )
        return_tools += [CompFactory.CaloClusterBadChannelList("ModifiedBadChannelList", badChannelTool = caloBadChanTool)]
        
        calibTools = result.popToolsAndMerge(DefaultTopoClusterLocalCalibToolsCfg(flags, False, "Modified"))
        return_tools += calibTools
        #This is already a tool array.
        
        from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
        result.merge(caloTopoCoolFolderCfg(flags))
    
    result.setPrivateTools(return_tools)
    
    return result

def MainTestConfiguration(flags, testoptions, plotter_configurator, cellsname, clustersname):
    
    result = PrevAlgorithmsConfigurationCfg(flags, testoptions, cellsname)
        
    GPUKernelSvc = CompFactory.GPUKernelSizeOptimizerSvc()
    result.addService(GPUKernelSvc)
    
    HybridClusterProcessor = CompFactory.CaloGPUHybridClusterProcessor("HybridClusterProcessor")
    HybridClusterProcessor.ClustersOutputName = clustersname
    
    HybridClusterProcessor.WriteTriggerSpecificInfo = False
    
    HybridClusterProcessor.MeasureTimes = flags.CaloRecGPU.ActiveConfig.MeasureTimes
    
    HybridClusterProcessor.TimeFileOutput = "GlobalTimes.txt"
    
    HybridClusterProcessor.DeferConstantDataPreparationToFirstEvent = True
    
    if plotter_configurator is None:
        HybridClusterProcessor.DoPlots = False
    else:
        HybridClusterProcessor.DoPlots = True
        
        HybridClusterProcessor.PlotterTool = result.popToolsAndMerge( TestPlotterConfiguration(flags, testoptions, plotter_configurator, cellsname) )
        
    HybridClusterProcessor.NumPreAllocatedDataHolders = flags.CaloRecGPU.ActiveConfig.NumPreAllocatedDataHolders
        
    if plotter_configurator is not None:
        histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
        result.addService(histSvc)
    
    ConstantDataTool = TestConstantConversionTool(flags, testoptions)
    
    ExportingTool = TestCPUtoGPUExporterTool(flags, testoptions, cellsname)
    
    ImportingTool = TestGPUtoCPUImporterTool(flags, testoptions, cellsname)
    
    if ConstantDataTool is not None and ExportingTool is not None and ImportingTool is not None:
        HybridClusterProcessor.ConstantDataToGPUTool = result.popToolsAndMerge( ConstantDataTool )
        
        HybridClusterProcessor.EventDataToGPUTool = result.popToolsAndMerge( ExportingTool )
        
        HybridClusterProcessor.GPUToEventDataTool = result.popToolsAndMerge( ImportingTool )
        
    else:
        if ConstantDataTool is not None or ExportingTool is not None or ImportingTool is not None:
            from AthenaCommon.Logging import log
            log.warning( "At least one of the conversion tools is None, but not all. Logic error in test types!" +
                        f"Valid tools: constant {ConstantDataTool is not None}, export {ExportingTool is not None}, import {ImportingTool is not None}.")
        
        HybridClusterProcessor.ConstantDataToGPUTool = None
        HybridClusterProcessor.EventDataToGPUTool    = None
        HybridClusterProcessor.GPUToEventDataTool    = None
        
        HybridClusterProcessor.SkipConversions = True
        
    
    
    HybridClusterProcessor.BeforeGPUTools = []
    
    HybridClusterProcessor.BeforeGPUTools += result.popToolsAndMerge( TestBeforeGPUToolsConfiguration(flags, testoptions, cellsname, clustersname) )
        
    HybridClusterProcessor.GPUTools = []
    
    HybridClusterProcessor.GPUTools += result.popToolsAndMerge( TestGPUToolsConfiguration(flags, testoptions, cellsname, clustersname) )
    
    HybridClusterProcessor.AfterGPUTools = []
    
    HybridClusterProcessor.AfterGPUTools += result.popToolsAndMerge( TestAfterGPUToolsConfiguration(flags, testoptions, cellsname, clustersname) )
    
    
    result.addEventAlgo(HybridClusterProcessor,primary=True)

    return result

def RunFullTestConfiguration(flags, testoptions, plotter_configurator = None, cellsname = "AllCalo", clustersname = "CaloCalTopoClustersNew"):
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg=MainServicesCfg(flags)
    
    if flags.Input.Format is Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags))
    elif flags.Input.Format is Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
        if "EventInfo" not in flags.Input.Collections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
            cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True),sequenceName="AthAlgSeq")
    else:
        from AthenaCommon.Logging import log
        log.warning("Unsupported format...")
        
    if testoptions.UsePerfMon:
       from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
       cfg.merge(PerfMonMTSvcCfg(flags))
        
    cfg.merge(MainTestConfiguration(flags, testoptions, plotter_configurator, cellsname, clustersname))
    
    cfg.getService("MessageSvc").infoLimit = 100000000
    
    cfg.run(testoptions.NumEvents)

def GetRealInputFilePaths(files, default_files):
    if files is None or len(files) == 0:
        return default_files
    else:
        ret = []
      
        from AthenaConfiguration.TestDefaults import defaultTestFiles
        standard_tests = {a: b for (a,b) in defaultTestFiles.__dict__.items() if not a.startswith("__") and not callable(b) }
        #Since all the standard tests are specified as member variables,
        #extract them...
        
        from TrigValTools.TrigValSteering.Input import load_input_json
        trigger_tests = load_input_json()

        for f in files:
            if f == 'default':
                ret += default_files
            elif f == 'trigEB':
                ret += trigger_tests['data']['paths']
            elif f == 'ttbar':
                ret += trigger_tests['ttbar']['paths']
            elif f == 'jets':
                ret += ["/eos/atlas/atlascerngroupdisk/data-art/large-input/trig-val/TrigEgammaValidation/valid3.147917.Pythia8_AU2CT10_jetjet_JZ7W.recon.RDO.e3099_s2578_r6596_tid05293007_00/RDO.05293007._000001.pool.root.1"]
            elif f in trigger_tests.keys():
                ret += trigger_tests[f]['paths']
            elif f in standard_tests.keys():
                ret += standard_tests[f]
            else:
                ret += [f]

        # for files on EOS, use xrootd rather than fuse
        ret = [f'root://eosatlas.cern.ch/{path}' if '/eos/' in path else path for path in ret]

        return ret
    
    
def PrepareTest(default_files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"],
                parse_command_arguments = True,
                allocate_as_many_as_threads = True,
                default_argument_for_files = None):

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
        parser.add_argument('-s','--synchronize', action = 'store_true')
       
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
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
                    
    initflags = initConfigFlags()
    testoptions = TestOptions()
    
    flags = initflags.cloneAndReplace("CaloRecGPU.ActiveConfig", "CaloRecGPU.Default", True)
    
    flags.CaloRecGPU.GlobalFlags.UseCaloRecGPU = True 
        
    if args is None:
        flags.Input.Files = GetRealInputFilePaths(default_argument_for_files, default_files)
    else:
        flags.Input.Files = GetRealInputFilePaths(args.files, default_files)
    
    if parse_command_arguments:
        flags.fillFromArgs(listOfArgs=rest)
        #We could instead use our parser to overload here and so on, but...
        flags.Concurrency.NumThreads = int(args.numthreads)
        flags.Concurrency.NumConcurrentEvents = int(args.numthreads)
        #This is to ensure the measurements are multi-threaded in the way we expect, I guess?
        flags.PerfMon.doFastMonMT = args.perfmon
        flags.PerfMon.doFullMonMT = args.fullmon
        # configure GPU
        flags.CaloRecGPU.ActiveConfig.MeasureTimes = args.measuretimes
        testoptions.OutputClusters = args.outputclusters
        testoptions.OutputCounts = args.outputcounts
        flags.CaloRecGPU.ActiveConfig.FillMissingCells = not args.notfillcells
        flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria = args.useoriginalcriteria
        flags.CaloRecGPU.ActiveConfig.doTwoGaussianNoise = not args.nodoublegaussiannoise
        if allocate_as_many_as_threads:
            flags.CaloRecGPU.ActiveConfig.NumPreAllocatedDataHolders = int(args.numthreads)
        testoptions.UsePerfMon = args.perfmon or args.fullmon
        testoptions.NumEvents = int(args.numevents)
        testoptions.SkipSyncs = not args.synchronize
    
    flags.CaloRecGPU.GlobalFlags.UseCaloRecGPU = True
    flags.CaloRecGPU.ActiveConfig.MissingCellsToFill = [186986, 187352]
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    
    from AthenaConfiguration.Enums import LHCPeriod
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA if flags.GeoModel.Run is LHCPeriod.Run3 and not flags.Input.isMC else defaultConditionsTags.autoconfigure(flags)
    #TODO: This is because autoconfigure errors out for run 3 data right now,
    #      and we want to test with the trigEB files...
    
    flags.GeoModel.AtlasVersion=defaultGeometryTags.autoconfigure(flags)
    
    return (flags, testoptions)
