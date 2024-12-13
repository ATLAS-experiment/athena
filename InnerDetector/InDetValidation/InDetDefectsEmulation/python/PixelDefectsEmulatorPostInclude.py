# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "PixelDefectsEmulatorPostInclude.emulateITkPixelDefects"
# or
#  --postExec "from InDetDefectsEmulation.PixelDefectsEmulatorPostInclude import emulateITkPixelDefects;
#              emulateITkPixelDefects(flags,cfg,DefectProbability=1e-3,CoreColumnDefectProbability=0.01);"
def emulateITkPixelDefects(flags,
                           cfg,
                           CoreColumnDefectProbability: float=0.01,
                           DefectProbability: float=1e-4,
                           HistogramGroupName: str="ITkPixelDefects",
                           HistogramFileName: str="itk_pixel_defects.root") :
    from InDetDefectsEmulation.PixelDefectsEmulatorConfig import (
        ITkPixelDefectsEmulatorCondAlgCfg,
        ITkPixelDefectsEmulatorAlgCfg,
        DefectsHistSvcCfg
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # schedule custom defects generating conditions alg
    cfg.merge( ITkPixelDefectsEmulatorCondAlgCfg(flags,
                                                 # to enable histogramming:
                                                 HistogramGroupName=f"/{HistogramGroupName}/EmulatedDefects/" if HistogramGroupName is not None else "",
                                                 CoreColumnDefectProbability=CoreColumnDefectProbability, # probability for a core column to have a defect
                                                 # (this is not the probability of a module to have a core column defect. The latter
                                                 #  is presumably a probability like computed by probColGroupDefect(0.1, 400/8.)
                                                 #   def probColGroupDefect(prob_col_group_defect_per_mod, n_col_groups) :
                                                 #       # prob of at least one defect per module : prob of not no-defect per module
                                                 #       return 1. - pow( (1-prob_col_group_defect_per_mod), 1/(n_col_groups))
                                                 # probability of a pixel to be defect:
                                                 DefectProbability=DefectProbability,
                                                 WriteKey="ITkPixelEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( ITkPixelDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="ITkPixelEmulatedDefects",
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/RejectedRDOs/" if HistogramGroupName is not None else "",
                                             # (prefix "/PixelDefects/" is assumed by THistSvc config)
                                             OutputLevel=INFO))


def emulatePixelDefects(flags,
                        cfg,
                        DefectProbability: float=1e-4,
                        HistogramGroupName: str="PixelDefects",
                        HistogramFileName: str="pixel_defects.root") :
    from InDetDefectsEmulation.PixelDefectsEmulatorConfig import (
        PixelDefectsEmulatorCondAlgCfg,
        PixelDefectsEmulatorAlgCfg,
        DefectsHistSvcCfg
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # schedule custom defects generating conditions alg
    cfg.merge( PixelDefectsEmulatorCondAlgCfg(flags,
                                                 # to enable histogramming:
                                                 HistogramGroupName=f"/{HistogramGroupName}/EmulatedDefects/" if HistogramGroupName is not None else "",
                                                 CoreColumnDefectProbability=0., # Does not make sense for run3 pixels
                                                 DefectProbability=DefectProbability,
                                                 WriteKey="PixelEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( PixelDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="PixelEmulatedDefects",
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/RejectedRDOs/" if HistogramGroupName is not None else "",
                                             # (prefix "/PixelDefects/" is assumed by THistSvc config)
                                             OutputLevel=INFO))

