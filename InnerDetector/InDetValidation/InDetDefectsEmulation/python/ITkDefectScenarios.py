# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def noDefects(flags, cfg, suffix="") :
    pass

def defects(flags, cfg, suffix="", **kwargs) :
    '''
    Generate Pixel and/or strip defects

    flags:  config flags
    cfg:    component accumulator
    suffix: suffix added to histogram files
    Extra arguments for pixel defects:
      PixelNumberOfCCDefects:           -1 (Poisson distributed number of CC defects matching probability),
                                        or fixed number of CC defects for modules with CC defects.
      PixelFrontEndCCDefectProb:        probability of a circuit to suffer one (or more) CC defect(s) [default 0.1]
      PixelDefectProb:                  probability of an individual pixel to be defect [default 0.01]
      PixelModuleDefectProb:            probability of a module to be defect [default = 0.01]
      PixelCornerDefectProb:            probability of 3D modules to have "corner defects [default 0.15]
      PixelFrontEndCCDefectProbBarrel:  None or special probability for CC defects on barrel modules [default None]
      PixelFrontEndCCDefectProbEndcap:  None or special probability for CC defects on endcap modules [default None]
      PixelFrontEndCCDefectProbBarrelL0:None or special probability for CC defects on L0 modules [default None]
      PixelDefectsInputFiles:           []   or a list of root or json files describing certain defects (which would be merged)
      PixelDefectsOutputFile:           None or a file name (ending in ".root" or ".json") to write the defects to.

    Extra arguments for strip defects:
      StripColdNoiseMPV:               mpv of number of strips to be disabled in case of cold noise [default 6]
      StripColdMoiseNMax:              maximum number of strips to be disabled in case of cold noise [default 12]
      StripDefectProb:                 probability of an individual strip to be defect [default 0.01]
      StripModuleDefectProb:           probability of a module to be defect [default 0.01]
      StripColdNoiseDefectProb:        probability of petals to suffer from cold noise [default 0.1]
      StripColdNoiseEvenOnly:          if True only modules with EVEN phi index will suffer from cold noise (?) [default True]
      StripDefectsInputFiles:          []   or a list of root or json files describing certain defects (which would be merged)
      StripDefectsOutputFile:          None or a file name (ending in ".root" or ".json") to write the defects to.
    '''
    if kwargs.pop("Pixel",True) :
        cc_defects = kwargs.pop("PixelNumberOfCCDefects", -1)
        if cc_defects <= 0 :
            from InDetDefectsEmulation.PixelDefectsEmulatorPostInclude import emulateITkPixelDefectsDefault
            emulateITkPixelDefectsDefault(flags,
                                          cfg,
                                          FrontEndCCDefectProb=kwargs.pop("PixelFrontEndCCDefectProb",0.1),
                                          PixelDefectProb=kwargs.pop("PixelDefectProb",1e-2),
                                          ModuleDefectProb=kwargs.pop("PixelModuleDefectProb",1e-2),
                                          CornerDefectProb=kwargs.pop("PixelCornerDefectProb",15e-2),
                                          NoiseProb=0.,
                                          PropagateDefectsToStatus=True,
                                          HistogramFileName=f"EmulatedITkPixelDefects{suffix}.root" if suffix is not None else None
                                          ,FrontEndCCDefectProbBarrel=kwargs.pop("PixelFrontEndCCDefectProbBarrel",None)
                                          ,FrontEndCCDefectProbEndcap=kwargs.pop("PixelFrontEndCCDefectProbEndcap",None)
                                          ,FrontEndCCDefectProbBarrelL0=kwargs.pop("PixelFrontEndCCDefectProbBarrelL0",None)
                                          ,DefectsInputFiles=kwargs.pop("PixelDefectsInputFiles",[])
                                          ,DefectsOutputFile=kwargs.pop("PixelDefectsOutputFile",None)
                                          )
        else :
            from InDetDefectsEmulation.PixelDefectsEmulatorPostInclude import emulateITkPixelDefectsOneCC
            emulateITkPixelDefectsOneCC(flags,
                                        cfg,
                                        FrontEndCCDefectProb=kwargs.pop("PixelFrontEndCCDefectProb",0.1),
                                        PixelDefectProb=kwargs.pop("PixelDefectProb",1e-2),
                                        NumberOfCCDefects=cc_defects,
                                        ModuleDefectProb=kwargs.pop("PixelModuleDefectProb",1e-2),
                                        CornerDefectProb=kwargs.pop("PixelCornerDefectProb",15e-2),
                                        NoiseProb=0.,
                                        # RngPerDefectType=False,
                                        # DefectsInputFiles=[],
                                        # DefectsOutputFile=None,
                                        PropagateDefectsToStatus=True,
                                        HistogramFileName=f"EmulatedITkPixelDefects{suffix}.root" if suffix is not None else None
                                        ,FrontEndCCDefectProbBarrel=kwargs.pop("PixelFrontEndCCDefectProbBarrel",None)
                                        ,FrontEndCCDefectProbEndcap=kwargs.pop("PixelFrontEndCCDefectProbEndcap",None)
                                        ,FrontEndCCDefectProbBarrelL0=kwargs.pop("PixelFrontEndCCDefectProbBarrelL0",None)
                                        ,DefectsInputFiles=kwargs.pop("PixelDefectsInputFiles",[])
                                        ,DefectsOutputFile=kwargs.pop("PixelDefectsOutputFile",None)
                                        )
    else :
        # Remove Pixel related kwargs.
        import re
        pat=re.compile("^Pixel.*")
        rm_keys=[]
        for k,v in kwargs.items() :
            if pat.match(k) :
                rm_keys.append(k)
        for k in rm_keys :
            kwargs.pop(k)

    if kwargs.pop("Strip",True) :
        from InDetDefectsEmulation.StripDefectsEmulatorPostInclude import emulateITkStripDefects,poissonFractions

        coldNoiseDefectFractions=poissonFractions(kwargs.pop("StripColdNoiseMPV",6),kwargs.pop("StripColdMoiseNMax",12))

        emulateITkStripDefects(flags,
                               cfg,
                               StripDefectProb=kwargs.pop("StripDefectProb",1e-2),
                               ModuleDefectProb=kwargs.pop("StripModuleDefectProb",1e-2),
                               coldNoiseDefectProb=kwargs.pop("StripColdNoiseDefectProb",1e-1),
                               coldNoiseNDefectFractions=coldNoiseDefectFractions,
                               coldNoiseDefectsEvenOnly=kwargs.pop("StripColdNoiseEvenOnly",True),
                               NoiseProb=0.,
                               PropagateDefectsToStatus=True,
                               DefectsInputFiles=kwargs.pop("StripDefectsInputFiles",[]),
                               DefectsOutputFile=kwargs.pop("StripDefectsOutputFile",None),
                               HistogramFileName=f"EmulatedITkStripDefects{suffix}.root" if suffix is not None else None)
    else :
        # remove strip related kwargs
        import re
        pat=re.compile("^Strip.*")
        rm_keys=[]
        for k,v in kwargs.items() :
            if pat.match(k) :
                rm_keys.append(k)
        for k in rm_keys :
            kwargs.pop(k)

    # all kwargs should be consumed now.
    if len(kwargs)>0 :
        import logging
        local_logger = logging.getLogger('defects')
        local_logger.debug(kwargs)
    assert(len(kwargs)==0)


# create defects from input files
# PixelDefectsInputFiles: [] or list of file names (.root or .json) with persistified pixel defects
# StripDefectsInputFiles: [] or list of file names (.root or .json) with persistified strip defects
# common_path:            '' or a common path prefix (including trailing '/' if there should be one)
#                            prepanded to the file names with pixel or strip defects.
def composedDefects(flags, cfg, suffix, PixelDefectsInputFiles=[], StripDefectsInputFiles=[], common_path='') :
    defects(flags,cfg,suffix,
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            PixelDefectsInputFiles=[common_path+elm for elm in PixelDefectsInputFiles],
            StripDefectsInputFiles=[common_path+elm for elm in StripDefectsInputFiles]
            )

# default defects
# no strip cold noise induced defects
def defaultDefects(flags,cfg) :
    defects(flags,cfg,"defaultDefects",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProb=9164./(9164)*.1)


# default defects but no CC defects in L0 barrel
# to match total CC probability more defects in outer layers
# no strip cold noise induced defects
def defaultDefectsNol0CC(flags,cfg) :
    defects(flags,cfg,"defaultDefectsNol0CC",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProb=9164./(9164-288)*.1,PixelFrontEndCCDefectProbBarrelL0=0.)

# default defects but no CC defects
# no strip cold noise induced defects
def noCCDefects(flags,cfg) :
    defects(flags,cfg,"noCCDefectsNol0CC",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProb=0.,PixelFrontEndCCDefectProbBarrelL0=0.)

# default defects but no CC defects in end-caps, and no CC defects in L0 barrel
# no strip cold noise induced defects
def noECCCDefectsNol0CC(flags,cfg) :
    defects(flags,cfg,"noECCCDefectsNol0CC",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProbBarrel=(6260+2904)/(2904-288)*.1,PixelFrontEndCCDefectProbEndcap=0.,
            PixelFrontEndCCDefectProbBarrelL0=0.)

# default defects but no CC defects in barrel
# no strip cold noise induced defects
def noBarrelCCDefectsNol0CC(flags,cfg) :
    defects(flags,cfg,"noBarrelCCDefectsNol0CC",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProbBarrel=0.,PixelFrontEndCCDefectProbEndcap=(6260+2904)/6260.*.1,
            PixelFrontEndCCDefectProbBarrelL0=0.)


# default defects but no CC defects in end-caps
# no strip cold noise induced defects
def noECCCDefects(flags,cfg) :
    defects(flags,cfg,"noECCCDefects",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProbBarrel=(6260+2904)/2904.*.1,
            PixelFrontEndCCDefectProbEndcap=0.)

# default defects but no CC defects in barrel
# no strip cold noise induced defects
def noBarrelCCDefects(flags,cfg) :
    defects(flags,cfg,"noBarrelCCDefects",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProbBarrel=0.,PixelFrontEndCCDefectProbEndcap=(6260+2904)/6260.*.1)

# default defects but 20% CC defect prob rather than 10%
# no strip cold noise induced defects
def defaultDefectsMoreCC(flags,cfg) :
    defects(flags,cfg,"defaultDefectsMoreCC",StripColdNoiseDefectProb=0.,
            PixelFrontEndCCDefectProb=9164./(9164)*.2
            )

def defaultDefectsMoreModules(flags,cfg) :
    defects(flags,cfg,"defaultDefectsMoreModules",StripColdNoiseDefectProb=0.,
            PixelFrontEndCCDefectProb=9164./(9164)*.1,
            PixelFrontEndCCDefectProbBarrel=9164./(9164)*.2,
            PixelFrontEndCCDefectProbEndcap=9164./(9164)*.2
            )
# default defects but 20% CC defect prob rather than 10%, no L0 CC defects in barrel
# no strip cold noise induced defects
def defaultDefectsMoreCCNol0CC(flags,cfg) :
    defects(flags,cfg,"defaultDefectsMoreCCNol0CC",StripColdNoiseDefectProb=0.,
            PixelFrontEndCCDefectProb=9164./(9164-288-900)*.2,
            PixelFrontEndCCDefectProbBarrelL0=0.
            )

# default defects but 1.5% defect pixel rather than 1%, no CC defects in L0 barrel
# no strip cold noise induced defects
def defaultDefectsMorePixelNol0CC(flags,cfg) :
    defects(flags,cfg,"defaultDefectsMorePixelNol0CC",StripColdNoiseDefectProb=0.,PixelFrontEndCCDefectProb=9164./(9164-288-900)*.1,
            PixelFrontEndCCDefectProbBarrelL0=0.,
            PixelDefectProb=1.5e-2
            )

# only pixel CC defects without L0 barrel+endcap CC defects and persistify defects to RNTuple
# prob: probability of CC defects
def persOnlyPixelCCNoL0(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'PixelCC{prob_int}NoL0',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelFrontEndCCDefectProb=9164./(9164-288-900)*prob,
            PixelFrontEndCCDefectProbBarrel=9164./(9164-288-900)*prob,
            PixelFrontEndCCDefectProbEndcap=9164./(9164-288-900)*prob,
            PixelFrontEndCCDefectProbBarrelL0=0.,
            PixelDefectsOutputFile=f'PixelDefectsCC{prob_int}NoL0.root'
            )

# only pixel CC defects _with_ L0 barrel+endcap CC defects and persistify defects to RNTuple
# prob: probability of CC defects
def persOnlyPixelCC(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'PixelCC{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=prob,
            PixelDefectsOutputFile=f'PixelDefectsCC{prob_int}.root'
            )

# only individual pixel defects and persistify defects to RNTuple
# prob: probability of individual defects
def persOnlyPixelSingle(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'PixelSingle{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=prob,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            PixelDefectsOutputFile=f'PixelDefectsSingle{prob_int}.root'
            )

# only pixel corner defects for 3D modules and persistify defects to RNTuple
# prob: probability of corner defects
def persOnlyPixelCorner(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'PixelCorner{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=prob,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            PixelDefectsOutputFile=f'PixelDefectsCorner{prob_int}.root'
            )

# only pixel module defects and persistify defects to RNTuple
# prob: probability of module defects
def persOnlyPixelModule(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'PixelModule{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=prob,
            PixelFrontEndCCDefectProb=0.,
            PixelDefectsOutputFile=f'PixelDefectsModule{prob_int}.root'
            )

# only individual strip defects and persistify defects to RNTuple
# prob: probability of individual defects
def persOnlyStripSingle(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'StripSingle{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=prob,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            StripDefectsOutputFile=f'StripDefectsSingle{prob_int}.root'
            )

# only strip module defects and persistify defects to RNTuple
# prob: probability of module defects
def persOnlyStripModule(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'StripModule{prob_int}',
            StripColdNoiseDefectProb=0.,
            StripDefectProb=0.,
            StripModuleDefectProb=prob,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            StripDefectsOutputFile=f'StripDefectsModule{prob_int}.root'
            )

# only strip cold noise induced defects and persistify defects to RNTuple
# prob: probability of cold noise induced defects
def persOnlyStripColdNoise(flags,cfg,prob) :
    prob_int=int(prob*100)
    defects(flags,cfg,f'StripColdNoise{prob_int}',
            StripColdNoiseDefectProb=prob,
            StripDefectProb=0.,
            StripModuleDefectProb=0.,
            PixelDefectProb=0.,
            PixelCornerDefectProb=0.,
            PixelModuleDefectProb=0.,
            PixelFrontEndCCDefectProb=0.,
            StripDefectsOutputFile=f'StripDefectsColdNoise{prob_int}.root'
            )

def defaultDefectsFromFiles(flags, cfg, suffix="") :
    composedDefects(flags,cfg,suffix=f'defaultDefects{suffix}',
                    PixelDefectsInputFiles=['PixelDefectsModule1.root',
                                            'PixelDefectsCorner15.root',
                                            'PixelDefectsSingle1.root',
                                            'PixelDefectsCC10NoL0.root'],
                    StripDefectsInputFiles=['StripDefectsModule1.root',
                                            'StripDefectsSingle1.root',
                                            'StripDefectsColdNoise10.root'])
