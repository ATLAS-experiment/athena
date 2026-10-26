#
# create all the histograms for the track parameters - this will 
# get called once for each tool for configured chain
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TrigInDetAnalysisExample.TIDAMonTool import defineHisto

import json

def JsonHist_builder( monTool, mypath, filename ) :
    
    mpath = mypath + "/Json"

    #    from HistUtils import ptbins_default
    #    ptbins = ptbins_default
    
    #    from HistUtils import d0bins_default
    #    d0bins = d0bins_default
    
    # get histograms from json and create them properly ...
    # NB: need to sort out what to do with these variable ?
    
    with open( filename , "r") as file:
        data = json.load(file)

        # for shte, dck in data.items():

        # print( f"{shte} :" )

        # for cnt, sck in dck.items():
        #    print( f"    {cnt}  {sck}" )

        for name, hist in data.items():

            htype = hist.get("type", "")
            
            xaxis = hist.get("xAxis", {})
            yaxis = hist.get("yAxis", {})
            
            xbins  = int(xaxis.get("nBins", 0))
            xmin   = float(xaxis.get("low", 0))
            xmax   = float(xaxis.get("high", 0))
            
            xtitle = xaxis.get("title", "")
            ytitle = yaxis.get("title", "")

            ybins = -1
            
            if htype == "TH2F" : 
                ybins  = int(yaxis.get("nBins", 0))
                ymin   = float(yaxis.get("low", 0))
                ymax   = float(yaxis.get("high", 0))
                if ymin >= ymax : print( f"y axis range error  {name}") 

                print(f"    name   : {name}   type: {htype}  xbins: {xbins}  lo: {xmin}  hi: {xmax}")
                defineHisto( monTool, name, path=mpath, type=htype, title=f"{name};{xtitle};{ytitle}", xbins=xbins, xmin=xmin, xmax=xmax, ybins=ybins, ymin=ymin, ymax=ymax )
            else:
                if xmin >= xmax : print( f"x axis range error {name}") 
                print(f"    name   : {name}   type: {htype}  xbins: {xbins}  lo: {xmin}  hi: {xmax}")
                defineHisto( monTool, name, path=mpath, type=htype, title=f"{name};{xtitle};{ytitle}", xbins=xbins, xmin=xmin, xmax=xmax )
        
