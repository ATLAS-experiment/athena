# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def FPGAOutputValidationCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("pixelKeys", [])
    kwargs.setdefault("stripKeys", [])
    kwargs.setdefault("doDiffHistograms", False)
    kwargs.setdefault("matchByID", False)
    kwargs.setdefault("allowedRdoMisses", 0)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monitoringTool = GenericMonitoringTool(flags, 'FPGAOutputValidationMonitoringTool')
    monitoringTool.HistPath = "/"

    if len(kwargs["pixelKeys"]) == 2 and kwargs["doDiffHistograms"]:
        key0 = kwargs["pixelKeys"][0]
        key1 = kwargs["pixelKeys"][1]
        name = f"{key0} - {key1}"
        monitoringTool.defineHistogram("nmatched_pixel_clusters", path = "FPGAOutputValidation", type = "TH1I", title = f"{name}: number of matched clusters", xbins = 100, xmin=0, xmax = 100)
        monitoringTool.defineHistogram("diff_pixel_locx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:locx;Local position x;", xbins = 200, xmin = -0.2, xmax = 0.2)
        monitoringTool.defineHistogram("diff_pixel_locy",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:locy;Local position y;", xbins = 200, xmin = -0.2, xmax = 0.2)
        monitoringTool.defineHistogram("diff_pixel_covxx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:covxx;Local covariance xx;", xbins = 100, xmin = -0.1, xmax = 0.1)
        monitoringTool.defineHistogram("diff_pixel_covyy",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:covyy;Local covariance yy;", xbins = 100, xmin = -0.1, xmax = 0.1)                        
        monitoringTool.defineHistogram("diff_pixel_globalx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globalx;Global position x;", xbins = 200, xmin = -0.05, xmax = 0.05)
        monitoringTool.defineHistogram("diff_pixel_globaly",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globaly;Global position y;", xbins = 200, xmin = -0.05, xmax = 0.05)
        monitoringTool.defineHistogram("diff_pixel_globalz",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globalz;Global position z;", xbins = 200, xmin = -0.05, xmax = 0.05)        
        monitoringTool.defineHistogram("diff_pixel_channelsphi", path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:channels in phi;Channels in #phi;", xbins = 10, xmin = -5, xmax = 5)
        monitoringTool.defineHistogram("diff_pixel_channelseta", path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:channels in eta;Channels in #eta;", xbins = 10, xmin = -5, xmax = 5)        
        monitoringTool.defineHistogram("diff_pixel_widtheta", path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:width in eta;Channels in #phi;", xbins = 10, xmin = -5, xmax = 5)
        monitoringTool.defineHistogram("diff_pixel_tot", path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:tot;Total TOT;", xbins = 200, xmin = -1, xmax = 1)

    if len(kwargs["stripKeys"]) == 2 and kwargs["doDiffHistograms"]:
        key0 = kwargs["stripKeys"][0]
        key1 = kwargs["stripKeys"][1]
        name = f"{key0} - {key1}"
        monitoringTool.defineHistogram("nmatched_strip_clusters", path = "FPGAOutputValidation", type = "TH1I", title = f"{name}: number of matched clusters", xbins = 100, xmin=0, xmax = 100)
        monitoringTool.defineHistogram("diff_strip_locx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:locx;Local position x;", xbins = 200, xmin = -0.02, xmax = 0.02)
        monitoringTool.defineHistogram("diff_strip_covxx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:covxx;Local covariance xx;", xbins = 100, xmin = -0.1, xmax = 0.1)
        monitoringTool.defineHistogram("diff_strip_globalx",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globalx;Global position x;", xbins = 200, xmin = -0.1, xmax = 0.1)
        monitoringTool.defineHistogram("diff_strip_globaly",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globaly;Global position y;", xbins = 200, xmin = -0.1, xmax = 0.1)
        monitoringTool.defineHistogram("diff_strip_globalz",path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:globalz;Global position z;", xbins = 200, xmin = -0.1, xmax = 0.1)        
        monitoringTool.defineHistogram("diff_strip_channelsphi", path = "FPGAOutputValidation", type = "TH1F", title = f"{name}:channels in phi;Channels in #phi;", xbins = 10, xmin = -5, xmax = 5)

    for key in kwargs["pixelKeys"]:
        monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_X;Local position x;", xbins = 200, xmin = -40, xmax = 40)
        monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_Y;Local position y;", xbins = 200, xmin = -40, xmax = 40)

        monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_XX", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALCOVARIANCE_XX;Local covariance xx;", xbins = 100, xmin = 0, xmax = 1)
        monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_YY", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALCOVARIANCE_YY;Local covariance yy;", xbins = 100, xmin = 0, xmax = 1)

        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_X;Global position x [mm];", xbins = 200, xmin = -350, xmax = 350)
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Y;Global position y [mm];", xbins = 200, xmin = -350, xmax = 350)
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Z", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Z;Global position z [mm];", xbins = 200, xmin = -3000, xmax =3000)

        monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_PHI", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_CHANNELS_IN_PHI;Channels in #phi;", xbins = 100, xmin = 0, xmax = 100)
        monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_ETA", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_CHANNELS_IN_ETA;Channels in #eta;", xbins = 100, xmin = 0, xmax = 100)

        monitoringTool.defineHistogram(f"{key}_WIDTH_IN_ETA", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_WIDTH_IN_ETA;Width in #eta;", xbins = 100, xmin = 0, xmax = 1)

        monitoringTool.defineHistogram(f"{key}_TOTAL_TOT", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_TOTAL_TOT;Total ToT;", xbins = 100, xmin = 0, xmax = 100)

    for key in kwargs["stripKeys"]:
        monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_X;Local position x;", xbins =  200, xmin =  -100, xmax =  100) 
        # monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_Y", xbins =  200, xmin =  -100, xmax =  100) 
        
        monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_XX", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALCOVARIANCE_XX;Local covariance xx;", xbins =  100, xmin =  0, xmax =  1) 
        
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_X;Global position x [mm];", xbins =  200, xmin =  -1024, xmax =  1024) 
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Y;Global position y [mm];", xbins =  200, xmin =  -1024, xmax =  1024) 
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Z", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Z;Global position z [mm];", xbins =  200, xmin =  -3000, xmax =  3000) 
        
        monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_PHI", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_CHANNELS_IN_PHI;Channels in #phi;", xbins =  100, xmin =  0, xmax =  100)

    from AthenaConfiguration.ComponentFactory import CompFactory 
    FPGAOutputValidationAlg = CompFactory.FPGAOutputValidationAlg(
        "FPGAOutputValidationAlg", 
        **kwargs
    )

    FPGAOutputValidationAlg.monitoringTool = monitoringTool
    acc.addEventAlgo(FPGAOutputValidationAlg)

    acc.addService(CompFactory.THistSvc(
        Output=["FPGAOutputValidation DATAFILE='FPGAOutputValidation.root', OPT='RECREATE'"]
    ))

    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument("--inputFiles", default = [], action = "append", required = True)
    argumentParser.add_argument("--outputFile", default = "FPGAOutputValidation.root")
    argumentParser.add_argument("--pixelKeys", default = [], action = "append")
    argumentParser.add_argument("--stripKeys", default = [], action = "append")
    argumentParser.add_argument("--threads", default = 1)

    arguments = argumentParser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = arguments.threads
    flags.Input.Files = arguments.inputFiles
    flags.Output.AODFileName = arguments.outputFile
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge(FPGAOutputValidationCfg(flags, **{
        "pixelKeys": arguments.pixelKeys,
        "stripKeys": arguments.stripKeys,
    }))

    acc.run(-1)

