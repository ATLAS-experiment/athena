#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 
        

# actually create the monTool to go along with a specific chain
# stores the histogram, binning, creates all the histograms etc

from TrigIDR4Monitoring.HistUtils import defineHisto

def TRTPlot_builder( monTool, mypath, chain, cs ) :

    from TrigIDR4Monitoring.HistUtils import ptbins_mb
    from TrigIDR4Monitoring.HistUtils import ptbins_default
    
    if "mb" in chain : ptbins = ptbins_mb
    else:              ptbins = ptbins_default
    
    #    from TrigIDR4Monitoring.HistUtils import vnbins_full
    #    from TrigIDR4Monitoring.HistUtils import vnbins_short
    
    #    vnbins = vnbins_full
    

    from TrigIDR4Monitoring.HistUtils import d0bins_default
    from TrigIDR4Monitoring.HistUtils import d0bins_lrt

    if  "LRT" in chain :  d0bins = d0bins_lrt
    else:                 d0bins = d0bins_default


    npath = mypath + "/TRTHits"
    
    varConfig = {
        "pt":  {"xbins": ptbins},
        "eta": {"xbins": 40, "xmin": -4,   "xmax": 4},
        "phi": {"xbins": 20, "xmin": -3.142, "xmax": 3.142},
        "d0":  {"xbins": d0bins}
    }    
    
    for dir in [ "reference", "test" ]: 

        mpath = npath+"/"+dir
        
        # oh dear, just so many histograms, so many ...
        
        # why do we want a histogram of the "innermost" layer hits ?? That should just be 0, 1, or 2 surely ? What is the point
        # of any of these sort of histograms, just have a plot of the "hit layers" then you can have one histogram for
        # *all* the layers, instead of essentially one histogram per layer. And anyway what is the "InnerMostPixelHits" ?
        # the number of hits on the "innerMost Layer" or the Layer with the innermost hit ?
        defineHisto( monTool, "NTRTHits",	       namespace=dir,path=mpath,  type="TH1F", title="NInnerMostPixelHits",        xbins=61,  xmin=-0.5,  xmax=60.5  )
        defineHisto( monTool, "NTRTHighThresholdHits", namespace=dir,path=mpath,  type="TH1F", title="NInnerMostPixelEndcapHits",  xbins=61,  xmin=-0.5,  xmax=60.5  )
        
        for var, bins in varConfig.items(): 
            
            stub = "_vs_"+var

            defineHisto( monTool, f"NTRTHits{stub}",	           namespace=dir,  path=mpath,  type="TProfile", title="NTRTHits",              **bins )
            defineHisto( monTool, f"NTRTHighThresholdHits{stub}", namespace=dir,  path=mpath,  type="TProfile", title="NTRTHighThresholdHits",  **bins )

            
    return monTool

