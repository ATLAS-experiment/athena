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

    for histName in ["all", "barrel", "endcap"]:

        if len(kwargs["pixelKeys"]) == 2 and kwargs["doDiffHistograms"]:
            key0 = kwargs["pixelKeys"][0]
            key1 = kwargs["pixelKeys"][1]
            name = f"{key0} - {key1}"
            monitoringTool.defineHistogram("nmatched_pixel_clusters_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{name}: number of matched clusters", xbins = 51, xmin=-0.5, xmax = 50.5)
            monitoringTool.defineHistogram("diff_pixel_locx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:locx;Local position x;", xbins = 200, xmin = -0.2, xmax = 0.2)
            monitoringTool.defineHistogram("diff_pixel_locy_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:locy;Local position y;", xbins = 200, xmin = -0.2, xmax = 0.2)
            monitoringTool.defineHistogram("diff_pixel_covxx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:covxx;Local covariance xx;", xbins = 1000, xmin = -0.001, xmax = 0.001)
            monitoringTool.defineHistogram("diff_pixel_covyy_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:covyy;Local covariance yy;", xbins = 1000, xmin = -0.001, xmax = 0.001)
            monitoringTool.defineHistogram("diff_pixel_globalx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globalx;Global position x;", xbins = 200, xmin = -0.05, xmax = 0.05)
            monitoringTool.defineHistogram("diff_pixel_globaly_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globaly;Global position y;", xbins = 200, xmin = -0.05, xmax = 0.05)
            monitoringTool.defineHistogram("diff_pixel_globalz_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globalz;Global position z;", xbins = 200, xmin = -0.05, xmax = 0.05)
            monitoringTool.defineHistogram("diff_pixel_channelsphi_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{name}:channels in phi;Channels in #phi;", xbins = 11, xmin = -5.5, xmax = 5.5)
            monitoringTool.defineHistogram("diff_pixel_channelseta_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{name}:channels in eta;Channels in #eta;", xbins = 11, xmin = -5.5, xmax = 5.5)
            monitoringTool.defineHistogram("diff_pixel_widtheta_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:width in eta;Channels in #phi;", xbins = 100, xmin = -5, xmax = 5)
            monitoringTool.defineHistogram("diff_pixel_tot_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{name}:tot;Total TOT;", xbins = 101, xmin = -50.5, xmax = 50.5)

        if len(kwargs["stripKeys"]) == 2 and kwargs["doDiffHistograms"]:
            key0 = kwargs["stripKeys"][0]
            key1 = kwargs["stripKeys"][1]
            name = f"{key0} - {key1}"
            monitoringTool.defineHistogram("nmatched_strip_clusters_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{name}: number of matched clusters", xbins = 100, xmin=0, xmax = 100)
            monitoringTool.defineHistogram("diff_strip_locx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:locx;Local position x;", xbins = 200, xmin = -0.02, xmax = 0.02)
            monitoringTool.defineHistogram("diff_strip_covxx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:covxx;Local covariance xx;", xbins = 100, xmin = -0.1, xmax = 0.1)
            monitoringTool.defineHistogram("diff_strip_globalx_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globalx;Global position x;", xbins = 200, xmin = -0.1, xmax = 0.1)
            monitoringTool.defineHistogram("diff_strip_globaly_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globaly;Global position y;", xbins = 200, xmin = -0.1, xmax = 0.1)
            monitoringTool.defineHistogram("diff_strip_globalz_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:globalz;Global position z;", xbins = 200, xmin = -0.1, xmax = 0.1)        
            monitoringTool.defineHistogram("diff_strip_channelsphi_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{name}:channels in phi;Channels in #phi;", xbins = 10, xmin = -5, xmax = 5)

        for key in kwargs["pixelKeys"]:
            monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_X_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALPOSITION_X;Local position x;", xbins = 800, xmin = -40, xmax = 40)
            monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_Y_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALPOSITION_Y;Local position y;", xbins = 800, xmin = -40, xmax = 40)

            monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_XX_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALCOVARIANCE_XX;Local covariance xx;", xbins = 1000, xmin = 0, xmax = 0.001)
            monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_YY_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALCOVARIANCE_YY;Local covariance yy;", xbins = 1000, xmin = 0, xmax = 0.001)

            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_X_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_X;Global position x [mm];", xbins = 700, xmin = -350, xmax = 350)
            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Y_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_Y;Global position y [mm];", xbins = 700, xmin = -350, xmax = 350)
            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Z_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_Z;Global position z [mm];", xbins = 6000, xmin = -3000, xmax =3000)

            monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_PHI_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{key}_CHANNELS_IN_PHI;Channels in #phi;", xbins = 51, xmin = -0.5, xmax = 50.5)
            monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_ETA_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{key}_CHANNELS_IN_ETA;Channels in #eta;", xbins = 51, xmin = -0.5, xmax = 50.5)

            monitoringTool.defineHistogram(f"{key}_WIDTH_IN_ETA_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_WIDTH_IN_ETA;Width in #eta;", xbins = 100, xmin = 0, xmax = 1)

            monitoringTool.defineHistogram(f"{key}_TOTAL_TOT_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1I", title = f"{key}_TOTAL_TOT;Total ToT;", xbins = 101, xmin = -0.5, xmax = 100.5)

        for key in kwargs["stripKeys"]:
            monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_X_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALPOSITION_X;Local position x;", xbins =  200, xmin =  -100, xmax =  100)             
            monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_XX_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_LOCALCOVARIANCE_XX;Local covariance xx;", xbins =  100, xmin =  0, xmax =  1) 
            
            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_X_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_X;Global position x [mm];", xbins =  200, xmin =  -1024, xmax =  1024) 
            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Y_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_Y;Global position y [mm];", xbins =  200, xmin =  -1024, xmax =  1024) 
            monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Z_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_GLOBALPOSITION_Z;Global position z [mm];", xbins =  200, xmin =  -3000, xmax =  3000) 
            
            monitoringTool.defineHistogram(f"{key}_CHANNELS_IN_PHI_" + histName, path= "FPGAOutputValidation/"+histName, type = "TH1F", title = f"{key}_CHANNELS_IN_PHI;Channels in #phi;", xbins =  100, xmin =  0, xmax =  100)

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

