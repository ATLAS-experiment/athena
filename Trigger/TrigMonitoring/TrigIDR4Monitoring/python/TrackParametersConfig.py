#
# create all the histograms for the track parameters - this will 
# get called once for each tool for configured chain
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TrigInDetAnalysisExample.TIDAMonTool import defineHisto



def TrackParameters_builder( monTool, mypath, chain, cs ) :

    npath = mypath + "/TrackParameters"

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


    if "_probe" in cs.extra :
        defineHisto( monTool,  "invmass",              path=npath, type="TH1F",  title="invariant mass",  xbins=100, xmin=50, xmax=150  )
    
    
    for dir in [ "reference", "test" ]: 
 
        mpath = npath+"/"+dir

        defineHisto( monTool,  "track_pt",             namespace=dir,    path=mpath, type="TH1F",  title="track_pt",          xbins=ptbins   )
        defineHisto( monTool,  "track_eta",            namespace=dir,    path=mpath, type="TH1F",  title="track_eta",         xbins=40,   xmin=-4,    xmax=4     )
        defineHisto( monTool,  "track_phi",            namespace=dir,    path=mpath, type="TH1F",  title="track_phi",         xbins=20,   xmin=-3.142, xmax=3.142   )
    
        defineHisto( monTool,  "track_d0",             namespace=dir,    path=mpath, type="TH1F",  title="track_d0",          xbins=d0bins )
        defineHisto( monTool,  "track_z0",             namespace=dir,    path=mpath, type="TH1F",  title="track_z0",          xbins=100,  xmin=-200, xmax=200   )
        defineHisto( monTool,  "track_z0sin",          namespace=dir,    path=mpath, type="TH1F",  title="track_z0sin",       xbins=100,  xmin=-200, xmax=200   )
        defineHisto( monTool,  "track_theta",          namespace=dir,    path=mpath, type="TH1F",  title="track_theta",       xbins=20,   xmin=0,    xmax=3.142 )
        
        defineHisto( monTool,  "track_qoverp",         namespace=dir,    path=mpath, type="TH1F",  title="track_qoverp",        xbins=100,  xmin=-4, xmax=4   )
        
        defineHisto( monTool,  "track_nSiHits",        namespace=dir,    path=mpath, type="TH1F",  title="track_nSiHits",        xbins=30,  xmin=0,  xmax=30  )
        defineHisto( monTool,  "track_nSiHits_vs_eta", namespace=dir,    path=mpath, type="TH2F",  title="track_nSiHits_vs_eta", xbins=20,  xmin=-4, xmax=4,  ybins=30,   ymin=0, ymax=30  )

        defineHisto( monTool,  "track_Et",             namespace=dir,    path=mpath, type="TH1F",  title="track_Et",          xbins=ptbins   )
        defineHisto( monTool,  "track_EtovPt",         namespace=dir,    path=mpath, type="TH1F",  title="track_EtovPt",      xbins=40,   xmin=0, xmax=10   )
        
        defineHisto( monTool,  "track_chi2",           namespace=dir,    path=mpath, type="TH1F",  title="track_chi2",         xbins=200, xmin=0, xmax=40   )
        defineHisto( monTool,  "track_ndof",           namespace=dir,    path=mpath, type="TH1F",  title="track_ndof",         xbins=20,  xmin=0, xmax=30   )
        defineHisto( monTool,  "track_chi2OverNdof",   namespace=dir,    path=mpath, type="TH1F",  title="track_chi2OverNdof", xbins=100, xmin=0, xmax=10   )
        
        defineHisto( monTool,  "track_author",         namespace=dir,    path=mpath, type="TH1F",  title="track_author",      xbins=10,   xmin=0, xmax=10   )

        # ???
        #   defineHisto( monTool,  "track_eta_vs_pt",      namespace=dir,    path=mpath, type="TH2F",  title="track_eta_vs_pt",   xbins=0,   xmin=0, xmax=0,  ybins=0,   ymin=0,  ymax=0 )
        #   defineHisto( monTool,  "track_eta_vs_phi",     namespace=dir,    path=mpath, type="TH2F",  title="track_eta_vs_phi",  xbins=0,   xmin=0, xmax=0,  ybins=0,   ymin=0,  ymax=0 ) 

        #???
        #        defineHisto( monTool,  "track_z0_vs_d0",       namespace=dir,    path=mpath, type="TH2F",  title="track_z0_vs_d0",    xbins=50,   xmin=-10, xmax=10,  ybins=0,   ymin=0,  ymax=0 ) 
        #        defineHisto( monTool,  "track_z0sin_vs_d0",    namespace=dir,    path=mpath, type="TH2F",  title="track_z0sin_vs_d0", xbins=50,   xmin=-10, xmax=10,  ybins=0,   ymin=0,  ymax=0 )
        
        defineHisto( monTool,  "sigma_pt",             namespace=dir,    path=mpath, type="TH1F",  title="sigma_pt",          xbins=100,  xmin=-1,   xmax=4   )
        defineHisto( monTool,  "sigma_eta",            namespace=dir,    path=mpath, type="TH1F",  title="sigma_eta",         xbins=100,  xmin=0,    xmax=0.01 )
        defineHisto( monTool,  "sigma_phi",            namespace=dir,    path=mpath, type="TH1F",  title="sigma_phi",         xbins=100,  xmin=0,    xmax=0.01 )
        defineHisto( monTool,  "sigma_d0",             namespace=dir,    path=mpath, type="TH1F",  title="sigma_d0",          xbins=100,  xmin=-0.5, xmax=0.5  )
        defineHisto( monTool,  "sigma_z0",             namespace=dir,    path=mpath, type="TH1F",  title="sigma_z0",          xbins=100,  xmin=0,    xmax=5    )
        defineHisto( monTool,  "sigma_z0sin",          namespace=dir,    path=mpath, type="TH1F",  title="sigma_z0sin",       xbins=100,  xmin=0,    xmax=5    )
        defineHisto( monTool,  "sigma_theta",          namespace=dir,    path=mpath, type="TH1F",  title="sigma_theta",       xbins=100,  xmin=0,    xmax=0.01  )
        
        defineHisto( monTool,  "significance_d0",      namespace=dir,    path=mpath, type="TH1F",  title="significance_d0",   xbins=100,   xmin=-10, xmax=10  )
