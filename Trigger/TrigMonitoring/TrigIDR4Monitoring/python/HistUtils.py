#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 
        

# actually create the monTool to go along with a specific chain
# stores the histogram, binning, creates all the histograms etc

# wrapper around montool.defineHistogram to simplify the required histogram names
# eg for a TProfile, etc automatically add the second variable, and create the histogram
# alias to avoid having to write pages of tedious boiler plate functions

def _th1(m,n):   return f"{m};{n}"
def _tprof(m,n): return f"{m},{m}_weight;{n}"
def _txy(m,n):   return f"{m}__x,{m}__y;{n}"
def _tyx(m,n):   return f"{m}__y,{m}__x;{n}"

_FORMATTERS = {
    "TH1F":        _th1,
    "TProfile":    _tprof,
    "TH2F":        _txy,
    "TEfficiency": _tyx
}



# wrap the GeneriMonitoringTool calls, so we can properly mangle the
# names automatically

def defineHisto( montool, name, namespace="", **args ) : 

    # here we auto-generate the x and y variable names, so that we can
    # also auto-generated them for a fill without needing to parse
    # anything

    mname = name
    if namespace : mname = f"{namespace}_{name}"
    
    htype = str(args.get("type", "")).strip()

    # NB, we swap the default x and y variables for a TEfficiency,
    #     in order to avoid the egregious bool-before-float root
    #     TEfficiency::Fill() convention
    
    formatter = _FORMATTERS.get(htype, _th1)
    pname     =  formatter(mname, name)

    # make sure that the histogram is created, even if it is never filled
    args["opt"] = "kAlwaysCreate"

    montool.defineHistogram(pname, **args)

    

    # create the required histograms for the differential resolutions
# NB: should probably also include the chi2 and the uncertainty histograms,
#     and maybe the slices, although they can be left up to the c++ code

def resplot( montool, namespace, path, label, xbins=0, xmin=None, xmax=None, ybins=0, ymin=None, ymax=None):
    
    vxbins = _getbins( namespace, xbins, xmin, xmax ) 
    vybins = _getbins( namespace, ybins, ymin, ymax ) 

    subpath = f"{path}/{namespace}"

    defineHisto( montool, "2D",    namespace=namespace,  path=subpath,  type="TH2F", title=f"2D ({label})",  xbins=vxbins,  ybins=vybins )

    defineHisto( montool, "1D",      path=subpath,  type="TH1F", title=f"1D ({label})",  xbins=vybins )
    defineHisto( montool, "mean",    path=subpath,  type="TH1F", title=f"mean {label}",  xbins=vxbins )
    defineHisto( montool, "sigma",   path=subpath,  type="TH1F", title=f"sigma {label}", xbins=vxbins )



    
def _getbins( name, xbins, xmin=None, xmax=None ):

    if isinstance(xbins,list): return xbins
    else:
        if xmin >= xmax : print( name, "BINS OUT OF RANGE !", xbins, xmin, xmax )
        width = (xmax - xmin) / xbins
        limits = []
        for i in range(xbins):
            limits.append( xmin + i*width )

        #    return [xmin + i * width for i in range(bins + 1)]

        return limits
    
    

    
    
# generate the mongroup hist path from the analysis string 

def monGroup( analysis_chain ) :

        from TrigInDetAnalysisExample.chainString import chainString

        chain = chainString( analysis_chain )

        mg = ""

        if chain.head == "" : mg = "/Fullscan"
        else :                mg = "/"+chain.head

        mg += "/"+chain.tail

        if chain.roi :     mg += "_"+chain.roi
        if chain.vtx :     mg += "_"+chain.vtx
        if chain.element : mg += "_" + chain.element
        if chain.extra :   mg += "_" + chain.extra
        if chain.passed :  mg += "/DTE"

        return mg


    
# superior binning for numerous histograms ...
# NB: these should probably go into their own file, rather than
#     hijack the HistUtils, but they are useful everywhere so
#     leave them here for the time being


ptbins_mb = [ 0.2, 0.4, 0.6, 0.8, 
              1, 1.20226, 1.44544, 1.7378, 2.0893, 2.51189, 3.01995, 3.63078, 4.36516, 5.24807, 6.30957, 
              7.58578, 9.12011, 10.9648, 13.1826, 15.8489, 19.0546, 22.9087, 27.5423, 33.1131, 39.8107, 
              47.863, 57.544, 69.1831, 83.1764, 100 ]

ptbins_default = [ 1, 1.20226, 1.44544, 1.7378, 2.0893, 2.51189, 3.01995, 3.63078, 4.36516, 5.24807, 6.30957, 
                   7.58578, 9.12011, 10.9648, 13.1826, 15.8489, 19.0546, 22.9087, 27.5423, 33.1131, 39.8107, 
                   47.863, 57.544, 69.1831, 83.1764, 100 ]


vnbins_full = [ -0.5, 
            0.5,      1.5,      2.5,      3.5,      4.5,      5.5,     6.5,     7.5,     8.5,     9.5,     10.5,     11.5,     12.5,    
           13.5,     14.5,     15.5,     17.5,     18.5,     19.5,    21.5,    23.5,    24.5,    26.5,     28.5,     30.5,     32.5,    
           35.5,     37.5,     40.5,     43.5,     46.5,     50.5,    53.5,    57.5,    61.5,    66.5,     71.5,     76.5,     81.5,   
           87.5,     93.5,    100.5,    107.5,    114.5,    123.5,    131.5,   141.5,   150.5,   161.5,   172.5,    185.5,    198.5,    
          211.5,    226.5,    242.5,    259.5,    277.5,    297.5, 
          317.5,    340.5,    363.5,    389.5,    416.5,    445.5,   476.5,   509.5,
          544.5,    582.5,    623.5,    666.5,    713.5,    762.5,   815.5,   872.5,   933.5,   998.5,   1067.5,
         1141.5,   1221.5,   1305.5,   1396.5,   1493.5,   1597.5,
         1708.5,   1827.5,   1953.5,   2089.5 ]
    

vnbins_short = [ 
                    -0.5,
                     0.5,     1.5,    2.5,    3.5,    4.5,    5.5,    6.5,    7.5,    8.5,   9.5,   
                    10.5,    11.5,   12.5,   13.5,   14.5,   15.5,   17.5,   18.5,   19.5,   
                    21.5,    23.5,   24.5,   26.5,   28.5,   
                    30.5,    32.5,   35.5,   37.5,   
                    40.5,    43.5,   46.5,   
                    50.5,    53.5,   57.5,   
                    61.5,    66.5,   
                    71.5,    76.5,   
                    81.5,    87.5,
                    93.5,   
                    100.5,  107.5,  114.5,  123.5,  131.5,  141.5,  150.5,   161.5,   172.5,   185.5,   198.5,   
                    211.5,  226.5,  242.5,  259.5,  277.5,  297.5,   
                    317.5,  340.5,  363.5,  389.5,   
                    416.5,  445.5,  476.5,   
                    509.5,  544.5,  582.5,   
                    623.5,  666.5,   
                    713.5,  762.5,   
                    815.5,  872.5,   
                    933.5,  998.5,   
                    1067.5
            ]


d0bins_default = [ -5.0,  -4.0,  -3.0,  -2.5,   
                   -2.0,  -1.8,  -1.6,  -1.4,  -1.2,  
                   -1.05, -0.95, -0.85, -0.75, -0.65, -0.55, -0.45, -0.35, -0.25, -0.15, -0.05, 
                   0.05,  0.15,  0.25,  0.35,  0.45,  0.55,  0.65,  0.75,  0.85,  0.95,  1.05,  
                   1.2,   1.4,   1.6,   1.8,   2.0,  
                   2.5,   3.0,   4.0,   5.0 ]

d0bins_lrt = [ -100.5, -97.5, -93.5, -90.5,                              
                -86.5, -83.5, -80.5, 
                -77.5, -74.5, -71.5,  
                -69.5, -66.5, -63.5, -61.5, 
                -59.5, -57.5, -54.5, -52.5, -50.5, 
                -49.5, -47.5, -45.5, -43.5, -42.5, -40.5, 
                -39.5, -37.5, -36.5, -34.5, -33.5, -32.5, -31.5, 
                -29.5, -28.5, -27.5, -26.5, -25.5, -24.5, -23.5, -22.5, -21.5, -20.5, 
                -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, 
                 -9.5,  -8.5,  -7.5,  -6.5,  -5.5,  -4.5,  -3.5,  -2.5,  -1.5,  -0.5,
                  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5,  
                 10.5,  11.5,  12.5,  13.5,  14.5,  15.5,  16.5,  17.5,  18.5,  19.5, 
                 20.5,  21.5,  22.5,  23.5,  24.5,  25.5,  26.5,  27.5,  28.5,  29.5, 
                 31.5,  32.5,  33.5,  34.5,  36.5,  37.5,  39.5, 
                 40.5,  42.5,  43.5,  45.5,  47.5,  49.5, 
                 50.5,  52.5,  54.5,  57.5,  59.5, 
                 61.5,  63.5,  66.5,  69.5, 
                 71.5,  74.5,  77.5,    
                 80.5,  83.5,  86.5,   
                 90.5,  93.5,  97.5,    
                100.5 ]
    


