# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# can be added as a post include to a Reco_tf
#   --postInclude "StripDefectsEmulatorPostInclude.emulateITkStripDefects"
# or
#  --postExec "from InDetDefectsEmulation.StripDefectsEmulatorPostInclude import emulateITkStripDefects;
#              emulateITkStripDefects(flags,cfg,DefectProb=1e-3, ModuleDefectProb=1e-2);"

def emulateITkStripDefects(flags,
                           cfg,
                           DefectProb: float=1e-2,
                           ModuleDefectProb: float=1e-2,
                           MaxRandomPositionAttempts: int=10,
                           FillHistogramsPerPattern: bool=True,
                           FillEtaPhiHistogramsPerPattern: bool=True,
                           HistogramGroupName: str="ITkStripDefects",
                           HistogramFileName: str="itk_strip_defects.root") :
    """
    Schedule algorithms to create ITk strip defect conditions data and to emulate
    defects by dropping ITk strip RDOs overlapping with such defects. ModuleDefectProb controls the number of
    defect strip modules and DefectProb the number of defect strips. If a strip group is overlapping with
    a single strip defect, the strip group will be split.
    """

    from InDetDefectsEmulation.StripDefectsEmulatorConfig import (
        ITkStripDefectsEmulatorCondAlgCfg,
        ITkStripDefectsEmulatorAlgCfg,
        DefectsHistSvcCfg,
        moduleDefect,
        combineModuleDefects
    )
    from AthenaCommon.Constants import INFO

    if HistogramFileName is None or len(HistogramFileName) ==0 :
        HistogramGroupName = None
    if HistogramGroupName is not None :
        cfg.merge( DefectsHistSvcCfg(flags, HistogramGroup=HistogramGroupName, FileName=HistogramFileName))

    # dummy "fractions" for strips, module pattern are just examples, and currently all
    # modules get the same defect probabilities irrespectively of e.g. strip length.
    module_pattern, module_defect_prob, fractions = combineModuleDefects([
                                         moduleDefect(bec=[-2,-2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all endcap A modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   DefectProb        # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[-1,1],layer=[0,1], phi_range=[-99,99],eta_range=[-99,99], # select all barrel, layer 0,1 modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   DefectProb        # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[-1,1],layer=[2,99], phi_range=[-99,99],eta_range=[-99,99], # select all other barrel layers
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   DefectProb        # probability of a strip to be defect
                                                                   ]),
                                         moduleDefect(bec=[2,2],layer=[0,99], phi_range=[-99,99],eta_range=[-99,99], # select all endcap C modules
                                                      side_range=[0,0,1,1], # randomly mark sides as defect. With [0,1] Both sides are marked as defect
                                                      all_rows=False,       # if True all rows of the same un-split module are marked as defect
                                                      probability=[ModuleDefectProb, # probability of a module to be defect
                                                                   DefectProb        # probability of a strip to be defect
                                                                   ])])
    # schedule custom defects generating conditions alg
    cfg.merge( ITkStripDefectsEmulatorCondAlgCfg(flags,
                                                 # to enable histogramming:
                                                 HistogramGroupName=f"/{HistogramGroupName}/StripEmulatedDefects/" if HistogramGroupName is not None else "",
                                                 MaxRandomPositionAttempts=MaxRandomPositionAttempts,
                                                 ModulePatterns=module_pattern,
                                                 DefectProbabilities=module_defect_prob,
                                                 FillHistogramsPerPattern=FillHistogramsPerPattern,
                                                 FillEtaPhiHistogramsPerPattern=FillEtaPhiHistogramsPerPattern,
                                                 WriteKey="ITkStripEmulatedDefects", # the default should match the key below
                                                 ))

    # schedule the algorithm which drops elements from the ITk Pixel RDO collection
    cfg.merge( ITkStripDefectsEmulatorAlgCfg(flags,
                                             # prevent default cond alg from being scheduled :
                                             EmulatedDefectsKey="ITkStripEmulatedDefects",
                                             # to enable histogramming:
                                             HistogramGroupName=f"/{HistogramGroupName}/StripRejectedRDOs/" if HistogramGroupName is not None else "",
                                             OutputLevel=INFO))
