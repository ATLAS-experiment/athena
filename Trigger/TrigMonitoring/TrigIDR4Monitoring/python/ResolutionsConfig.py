#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for the Resolutions plots - this will get called
# once for each tool for configured chain - we can set the HistPath upstream
# master tool
        

from TrigIDR4Monitoring.HistUtils import defineHisto
from TrigIDR4Monitoring.HistUtils import resplot


def Resolutions_builder( monTool, mypath, chain, cs ) :

    # what is this "inclusive" nonsense ? If we want the "inclusive" anything, we just
    # need to take the integral over any of the differential variables, it literally serves
    # no purpuse, since yuou can not have a differential resolution vs "inclusive", you can
    # have lietarly only the basic integrated residuals, and the intergrated efficiency
    # those are the only things that you can have "inclusive" so for N track parameters,
    # there are literally N residuals, and 1 efficiency, and those will have their own
    # plots in any case - so will probably drop the "inclusive" as it is more trouble than
    
    parNames = [ "inclusive", "pt", "eta", "d0", "z0", "qoverp", "ptqopt", "theta", "phi", "z0sin" ]
    sigma    = [           1,   20,  0.02,  0.2,    1,       50,      0.1,    0.02,  0.02,      1  ] 
    ranges   = [           1,  100,   4.0,   50,  200,        5,        5,     3.2,   3.2,    200  ]       
    
    mypath = mypath + "/Resolutions"

    from TrigIDR4Monitoring.HistUtils import ptbins_mb
    from TrigIDR4Monitoring.HistUtils import ptbins_default
    
    if "mb" in chain : ptbins = ptbins_mb
    else:              ptbins = ptbins_default
    
    #    from HistUtils import vnbins_full
    #    from HistUtils import vnbins_short

    # vnbins = vnbins_full
    
    from TrigIDR4Monitoring.HistUtils import d0bins_default
    from TrigIDR4Monitoring.HistUtils import d0bins_lrt

    if  "LRT" in chain :  d0bins = d0bins_lrt
    else:                 d0bins = d0bins_default

    
    for parName, sig, rng  in zip(parNames, sigma, ranges):

        ## what are all these plost ???
        ## why do we need a "significance" for pt ? or eta ??

        # what we really want are plots of the residuals of each variable ve pt, or vs eta
        # or vs z, or even d0
        
        defineHisto( monTool,  "pull_"+parName,             path=mypath, type="TH1F", title="pull_"+parName,         xbins=100,  xmin=-10,   xmax=10   )
        defineHisto( monTool,  "res_"+parName,              path=mypath, type="TH1F", title="res_"+parName,          xbins=100,  xmin=-sig,  xmax=sig   )
        defineHisto( monTool,  "sigma_"+parName,            path=mypath, type="TH1F", title="sigma_"+parName,        xbins=100,  xmin=0,     xmax=sig   )
        defineHisto( monTool,  "significance_"+parName,     path=mypath, type="TH1F", title="significance_"+parName, xbins=100,  xmin=-3,    xmax=3   )

        if      parName == "pt" : 
            defineHisto( monTool,  "corr_test_vs_ref_"+parName, path=mypath, type="TH2F", title="corr_"+parName, xbins=ptbins, ybins=ptbins )
        elif parName == "d0" :
            defineHisto( monTool,  "corr_test_vs_ref_"+parName, path=mypath, type="TH2F", title="corr_"+parName, xbins=d0bins, ybins=d0bins )
        else: 
            defineHisto( monTool,  "corr_test_vs_ref_"+parName, path=mypath, type="TH2F", title="corr_"+parName, xbins=100,  xmin=-rng, xmax=rng, ybins=100,  ymin=-rng, ymax=rng )   

            
        # differential resolutions
            
        if parName != "inclusive" : 

            ptname  = f"resolution_{parName}_vs_pt"
            etaname = f"resolution_{parName}_vs_eta"
            z0name  = f"resolution_{parName}_vs_z0"

            resplot( monTool, namespace=ptname,  path=mypath, label=f"{parName} vs pt",  xbins=ptbins,                  ybins=100, ymin=-sig, ymax=sig ) 
            resplot( monTool, namespace=etaname, path=mypath, label=f"{parName} vs eta", xbins=32, xmin=-4,   xmax=4,   ybins=100, ymin=-sig, ymax=sig ) 
            resplot( monTool, namespace=z0name,  path=mypath, label=f"{parName} vs z0",  xbins=25, xmin=-200, xmax=200, ybins=100, ymin=-sig, ymax=sig ) 

#            # resolutions vs pt
#            defineHisto( monTool, "2D",    namespace=ptname,  path=f"{mypath}/{ptname}",  type="TH2F", title=f"2D ({parName} vs pt)",  xbins=ptbins,  ybins=100, ymin=-sig,  ymax=sig )
#            defineHisto( monTool, "1D",      path=f"{mypath}/{ptname}",  type="TH1F", title=f"1D ({parName})",        xbins=100, xmin=-sig,  xmax=sig )
#            defineHisto( monTool, "mean",    path=f"{mypath}/{ptname}",  type="TH1F", title=f"mean {parName} vs pt",  xbins=ptbins )
#            defineHisto( monTool, "sigma",   path=f"{mypath}/{ptname}",  type="TH1F", title=f"sigma {parName} vs pt", xbins=ptbins )

#            # resolutions vs eta
#            defineHisto( monTool, "2D",    namespace=etaname, path=f"{mypath}/{etaname}", type="TH2F", title=f"2D ({parName} vs eta)", xbins=32,   xmin=-4,   xmax=4, ybins=100, ymin=-sig,  ymax=sig )
#            defineHisto( monTool, "1D",     path=f"{mypath}/{etaname}", type="TH1F", title=f"1D ({parName})",         xbins=100,  xmin=-sig, xmax=sig )
#            defineHisto( monTool, "mean",   path=f"{mypath}/{etaname}", type="TH1F", title=f"mean {parName} vs eta",  xbins=32,   xmin=-4,   xmax=4 )
#            defineHisto( monTool, "sigma",  path=f"{mypath}/{etaname}", type="TH1F", title=f"sigma {parName} vs eta", xbins=32,   xmin=-4,   xmax=4 )


#            #resolutions vs zo
#            defineHisto( monTool, "2D",    namespace=z0name,  path=f"{mypath}/{z0name}",  type="TH2F", title=f"2D ({parName} vs z0)",  xbins=25, xmin=-200, xmax=200, ybins=100, ymin=-sig,  ymax=sig )
#            defineHisto( monTool, "1D",     path=f"{mypath}/{z0name}",  type="TH1F", title=f"1D ({parName})",        xbins=100, xmin=-sig, xmax=sig )
#            defineHisto( monTool, "mean",   path=f"{mypath}/{z0name}",  type="TH1F", title=f"mean {parName} vs z0",  xbins=25,  xmin=-200, xmax=200 )
#            defineHisto( monTool, "sigma",  path=f"{mypath}/{z0name}",  type="TH1F", title=f"sigma {parName} vs z0", xbins=25,  xmin=-200, xmax=200 )

    
