#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for the Resolutions plots - this will get called
# once for each tool for configured chain - we can set the HistPath upstream
# master tool
        

from TrigIDR4Monitoring.HistUtils import defineHisto

def Efficiencies_builder( monTool, mypath, chain, cs ) :
    
    mypath = mypath + "/Efficiencies"

    from TrigIDR4Monitoring.HistUtils import ptbins_mb
    from TrigIDR4Monitoring.HistUtils import ptbins_default
    
    if "mb" in chain : ptbins = ptbins_mb
    else:              ptbins = ptbins_default
    
    from TrigIDR4Monitoring.HistUtils import vnbins_full
    #    from TrigIDR4Monitoring.HistUtils import vnbins_short

    vnbins = vnbins_full
    
    from TrigIDR4Monitoring.HistUtils import d0bins_default
    from TrigIDR4Monitoring.HistUtils import d0bins_lrt

    if  "LRT" in chain :  d0bins = d0bins_lrt
    else:                 d0bins = d0bins_default

    
    defineHisto( monTool,  "eff_vs_inclusive",   path=mypath, type="TEfficiency", title="eff_vs_inclusive",   xbins=1,   xmin=0, xmax=1   )

    defineHisto( monTool,  "eff_vs_ntracks",     path=mypath, type="TEfficiency", title="eff_vs_ntracks",     xbins=vnbins )

    # bring in the proper log bins later on ...
    defineHisto( monTool,  "eff_vs_pt",          path=mypath, type="TEfficiency", title="eff_vs_pt",          xbins=ptbins   )

    # what do these even mean ??? Just because the chosen binning is rubbish ??? 
    #    defineHisto( monTool,  "eff_vs_logPt",       path=mypath, type="TEfficiency", title="eff_vs_logPt",       xbins=ptbins )
    #    defineHisto( monTool,  "eff_vs_lowPt",       path=mypath, type="TEfficiency", title="eff_vs_lowPt",       xbins=ptbins )

    defineHisto( monTool,  "eff_vs_Et",          path=mypath, type="TEfficiency", title="eff_vs_Et",          xbins=ptbins )
    defineHisto( monTool,  "eff_vs_EtovPt",      path=mypath, type="TEfficiency", title="eff_vs_EtovPt",      xbins=100, xmin=0, xmax=10    )

    defineHisto( monTool,  "eff_vs_eta",         path=mypath, type="TEfficiency", title="eff_vs_eta",         xbins=100,   xmin=-4, xmax=4   )

    # should bring in a standard pi value here ...
    defineHisto( monTool,  "eff_vs_phi",         path=mypath, type="TEfficiency", title="eff_vs_phi",         xbins=50,    xmin=-3.1416, xmax=3.1416   )
    defineHisto( monTool,  "eff_vs_d0",          path=mypath, type="TEfficiency", title="eff_vs_d0",          xbins=d0bins )
    defineHisto( monTool,  "eff_vs_z0",          path=mypath, type="TEfficiency", title="eff_vs_z0",          xbins=100,   xmin=-180, xmax=180   )

    defineHisto( monTool,  "eff_vs_actualMu",    path=mypath, type="TEfficiency", title="eff_vs_actualMu",    xbins=100,   xmin=0, xmax=200   )

    # defineHisto( monTool,  "eff_vs_truthMu",     path=mypath, type="TEfficiancy", title="eff_vs_truthMu",    xbins=0,   xmin=0, xmax=0   )

    # defineHisto( monTool,  "eff_vs_eta_vs_pt",   path=mypath, type="TEfficiency", title="eff_vs_eta_vs_pt",   xbins=0,   xmin=0, xmax=0   )
    # defineHisto( monTool,  "eff_vs_eta_vs_phi",  path=mypath, type="TEfficiency", title="eff_vs_eta_vs_phi",  xbins=0,   xmin=0, xmax=0   )
    # defineHisto( monTool,  "eff_vs_z0_vs_d0",    path=mypath, type="TEfficiency", title="eff_vs_z0_vs_d0",    xbins=0,   xmin=0, xmax=0   )
    # defineHisto( monTool,  "eff_vs_z0sin_vs_d0", path=mypath, type="TEfficiency", title="eff_vs_z0sin_vs_d0", xbins=0,   xmin=0, xmax=0   )
    
    return
