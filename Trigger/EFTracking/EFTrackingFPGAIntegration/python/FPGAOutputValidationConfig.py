# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def FPGAOutputValidationCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("pixelKeys", [])
    kwargs.setdefault("stripKeys", [])

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monitoringTool = GenericMonitoringTool(flags, 'FPGAOutputValidationMonitoringTool')
    monitoringTool.HistPath = "/"
    
    for key in kwargs["pixelKeys"]:
        monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_X;Local position x;", xbins = 200, xmin = -40, xmax = 40)
        monitoringTool.defineHistogram(f"{key}_LOCALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALPOSITION_Y;Local position y;", xbins = 200, xmin = -40, xmax = 40)

        monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_XX", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALCOVARIANCE_XX;Local covariance xx;", xbins = 100, xmin = 0, xmax = 1)
        monitoringTool.defineHistogram(f"{key}_LOCALCOVARIANCE_YY", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_LOCALCOVARIANCE_YY;Local covariance yy;", xbins = 100, xmin = 0, xmax = 1)

        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_X;Global position x [mm];", xbins = 200, xmin = -350, xmax = 350)
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Y;Global position y [mm];", xbins = 200, xmin = -350, xmax = 350)
        monitoringTool.defineHistogram(f"{key}_GLOBALPOSITION_Z", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_GLOBALPOSITION_Z;Global position z [mm];", xbins = 200, xmin = -3000, xmax =3000)

        monitoringTool.defineHistogram(f"{key}_OMEGA_X", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_OMEGA_X;Omega X;", xbins = 100, xmin = 0, xmax = 1)
        monitoringTool.defineHistogram(f"{key}_OMEGA_Y", path = "FPGAOutputValidation", type = "TH1F", title = f"{key}_OMEGA_Y;Omega Y;", xbins = 100, xmin = 0, xmax = 1)

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

