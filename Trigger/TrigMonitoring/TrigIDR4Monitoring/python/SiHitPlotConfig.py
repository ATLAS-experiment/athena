#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 
        

# actually create the monTool to go along with a specific chain
# stores the histogram, binning, creates all the histograms etc

from TrigIDR4Monitoring.HistUtils import defineHisto

def SiHitPlot_builder( monTool, mypath, chain, cs ) :

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


    npath = mypath + "/SiHits"
    
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
        defineHisto( monTool, "NInnerMostPixelHits",	           namespace=dir,path=mpath,  type="TH1F", title="NInnerMostPixelHits",               xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NInnerMostPixelEndcapHits",	   namespace=dir,path=mpath,  type="TH1F", title="NInnerMostPixelEndcapHits",         xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NNextToInnerMostPixelHits",	    namespace=dir,  path=mpath,  type="TH1F", title="NNextToInnerMostPixelHits",         xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NNextToInnerMostPixelEndcapHits",   namespace=dir,  path=mpath,  type="TH1F", title="NNextToInnerMostPixelEndcapHits",   xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NInnerMostPixelSharedHits",	   namespace=dir, path=mpath,  type="TH1F", title="NInnerMostPixelSharedHits",         xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NInnerMostPixelSharedEndcapHits",   namespace=dir, path=mpath,  type="TH1F", title="NInnerMostPixelSharedEndcapHits",   xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NPixelHits",	                   namespace=dir, path=mpath,  type="TH1F", title="NPixelHits",                        xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NPixelHoles",	                   namespace=dir, path=mpath,  type="TH1F", title="NPixelHoles",                       xbins=11,  xmin=-0.5,  xmax=10.5  )
        defineHisto( monTool, "NPixelSharedHits",	           namespace=dir, path=mpath,  type="TH1F", title="NPixelSharedHits",                  xbins=21,  xmin=-0.5,  xmax=20.5  )
        #       defineHisto( monTool, "NPixeldEdx",	                    namespace=dir, path=mpath,  type="TH1F", title="NPixeldEdx",                        xbins=20,  xmin=-0.5,  xmax=100 ) # what units ??
        defineHisto( monTool, "NSCTHits",	                   namespace=dir, path=mpath,  type="TH1F", title="NSCTHits",                          xbins=21,  xmin=-0.5,  xmax=20.5  )
        defineHisto( monTool, "NSCTHoles",	                   namespace=dir, path=mpath,  type="TH1F", title="NSCTHoles",                         xbins=11,  xmin=-0.5,  xmax=10.5  )
        defineHisto( monTool, "NSCTSharedHits",	                   namespace=dir, path=mpath,  type="TH1F", title="NSCTSharedHits",                    xbins=21,  xmin=-0.5,  xmax=20.5  )
        #       defineHisto( monTool, "NInnerMostPixelOutliers",	    namespace=dir,  path=mpath,  type="TH1F", title="NInnerMostPixelOutliers",           xbins=21,  xmin=-0.5,  xmax=20.5  )
        #       defineHisto( monTool, "NInnerMostPixelEndcapOutliers",	    namespace=dir,  path=mpath,  type="TH1F", title="NInnerMostPixelEndcapOutliers",     xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NInnerMostPixelSplitHits",	    namespace=dir,  path=mpath,  type="TH1F", title="NInnerMostPixelSplitHits",          xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NInnerMostPixelSplitEndcapHits",    namespace=dir,  path=mpath,  type="TH1F", title="NInnerMostPixelSplitEndcapHits",    xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NExpectedInnerMostPixelHits",	    namespace=dir,  path=mpath,  type="TH1F", title="NExpectedInnerMostPixelHits",       xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NExpectedNextToInnerMostPixelHits", namespace=dir,  path=mpath,  type="TH1F", title="NExpectedNextToInnerMostPixelHits", xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelOutliers",	            namespace=dir,  path=mpath,  type="TH1F", title="NPixelOutliers",                    xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelContribLayers",	            namespace=dir,  path=mpath,  type="TH1F", title="NPixelContribLayers",               xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelSplitHits",	            namespace=dir,  path=mpath,  type="TH1F", title="NPixelSplitHits",                   xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelGangedHits",	            namespace=dir,  path=mpath,  type="TH1F", title="NPixelGangedHits",                  xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelGangedHitsFlaggedFakes",	    namespace=dir,  path=mpath,  type="TH1F", title="NPixelGangedHitsFlaggedFakes",      xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NPixelDeadSensors",	            namespace=dir,  path=mpath,  type="TH1F", title="NPixelDeadSensors",                 xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NSCTOutliers",	                    namespace=dir,  path=mpath,  type="TH1F", title="NSCTOutliers",                      xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NSCTDoubleHoles",	            namespace=dir,  path=mpath,  type="TH1F", title="NSCTDoubleHoles",                   xbins=21,  xmin=-0.5,  xmax=20.5  )
        #        defineHisto( monTool, "NSCTDeadSensors",	            namespace=dir,  path=mpath,  type="TH1F", title="NSCTDeadSensors",                   xbins=21,  xmin=-0.5,  xmax=20.5  )
        
        for var, bins in varConfig.items(): 
            
            stub = "_vs_"+var
            
            defineHisto( monTool, "NInnerMostPixelHits"+stub,               namespace=dir,  path=mpath,  type="TProfile", title="NInnerMostPixelHits",               **bins  )
            defineHisto( monTool, "NInnerMostPixelEndcapHits"+stub,	    namespace=dir,  path=mpath,  type="TProfile", title="NInnerMostPixelEndcapHits",         **bins  )
            #            defineHisto( monTool, "NNextToInnerMostPixelHits"+stub,	 namespace=dir,  path=mpath,  type="TProfile", title="NNextToInnerMostPixelHits",         **bins  )
            #            defineHisto( monTool, "NNextToInnerMostPixelEndcapHits"+stub,   namespace=dir,  path=mpath,  type="TProfile", title="NNextToInnerMostPixelEndcapHits",   **bins  )
            defineHisto( monTool, "NInnerMostPixelSharedHits"+stub,	    namespace=dir,  path=mpath,  type="TProfile", title="NInnerMostPixelSharedHits",         **bins  )
            defineHisto( monTool, "NInnerMostPixelSharedEndcapHits"+stub,   namespace=dir,  path=mpath,  type="TProfile", title="NInnerMostPixelSharedEndcapHits",   **bins  )
            defineHisto( monTool, "NPixelHits"+stub,	                    namespace=dir,  path=mpath,  type="TProfile", title="NPixelHits",                        **bins  )
            defineHisto( monTool, "NPixelHoles"+stub,	                    namespace=dir,  path=mpath,  type="TProfile", title="NPixelHoles",                       **bins  )
            defineHisto( monTool, "NPixelSharedHits"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NPixelSharedHits",                  **bins  )
            #            defineHisto( monTool, "NPixeldEdx"+stub,	                 namespace=dir,  path=mpath,  type="TProfile", title="NPixeldEdx",                        **bins  )
            defineHisto( monTool, "NSCTHits"+stub,	                    namespace=dir,  path=mpath,  type="TProfile", title="NSCTHits",                          **bins  )
            defineHisto( monTool, "NSCTHoles"+stub,	                    namespace=dir,  path=mpath,  type="TProfile", title="NSCTHoles",                         **bins  )
            defineHisto( monTool, "NSCTSharedHits"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NSCTSharedHits",                    **bins  )
            #            defineHisto( monTool, "NInnerMostPixelOutliers"+stub,	         namespace=dir, path=mpath,  type="TProfile", title="NInnerMostPixelOutliers",           **bins  )
            #            defineHisto( monTool, "NInnerMostPixelEndcapOutliers"+stub,     namespace=dir, path=mpath,  type="TProfile", title="NInnerMostPixelEndcapOutliers",     **bins  )
            #            defineHisto( monTool, "NInnerMostPixelSplitHits"+stub,	         namespace=dir, path=mpath,  type="TProfile", title="NInnerMostPixelSplitHits",          **bins  )
            #            defineHisto( monTool, "NInnerMostPixelSplitEndcapHits"+stub,    namespace=dir, path=mpath,  type="TProfile", title="NInnerMostPixelSplitEndcapHits",    **bins  )
            #            defineHisto( monTool, "NExpectedInnerMostPixelHits"+stub,       namespace=dir, path=mpath,  type="TProfile", title="NExpectedInnerMostPixelHits",       **bins  )
            #            defineHisto( monTool, "NExpectedNextToInnerMostPixelHits"+stub, namespace=dir, path=mpath,  type="TProfile", title="NExpectedNextToInnerMostPixelHits", **bins  )
            #            defineHisto( monTool, "NPixelOutliers"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NPixelOutliers",                    **bins  )
            #            defineHisto( monTool, "NPixelContribLayers"+stub,	    namespace=dir,  path=mpath,  type="TProfile", title="NPixelContribLayers",               **bins  )
            #            defineHisto( monTool, "NPixelSplitHits"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NPixelSplitHits",                   **bins  )
            #            defineHisto( monTool, "NPixelGangedHits"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NPixelGangedHits",                  **bins  )
            #            defineHisto( monTool, "NPixelGangedHitsFlaggedFakes"+stub, namespace=dir,  path=mpath,  type="TProfile", title="NPixelGangedHitsFlaggedFakes",      **bins  )
            #            defineHisto( monTool, "NPixelDeadSensors"+stub,	    namespace=dir,  path=mpath,  type="TProfile", title="NPixelDeadSensors",                 **bins  )
            #            defineHisto( monTool, "NSCTOutliers"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NSCTOutliers",                      **bins  )
            #            defineHisto( monTool, "NSCTDoubleHoles"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NSCTDoubleHoles",                   **bins  )
            #            defineHisto( monTool, "NSCTDeadSensors"+stub,	            namespace=dir,  path=mpath,  type="TProfile", title="NSCTDeadSensors",                   **bins  )
            
    return monTool

