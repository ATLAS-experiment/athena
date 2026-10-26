#
# create all the histograms for the Vertex analysis - this will get
# called once for each tool for configured chain - we can set the
# HistPath upstream master tool
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from TrigIDR4Monitoring.HistUtils import defineHisto
from TrigIDR4Monitoring.HistUtils import resplot
from TrigIDR4Monitoring.HistUtils import vnbins_full

def Vertices_builder( monTool, mypath, chain, cs ) :

    mpath = mypath + "/Vertex"

    for dir in [ "reference", "test" ]: 

        npath = mpath+"/"+dir

        defineHisto( monTool,  "nVtx",              namespace=dir,   path=npath, type="TH1F", title="nVtx",   xbins=100,   xmin=0,    xmax=200  )        
        defineHisto( monTool,  "nVtx_vs_actualMu",  namespace=dir,   path=npath, type="TH2F", title="nVtx_vs_actualMu",   xbins=100,   xmin=0,  xmax=200, ybins=100, ymin=0, ymax=200 )

        defineHisto( monTool,  "vtx_x",        namespace=dir,    path=npath, type="TH1F", title="vtx_x",   xbins=300,   xmin=-1.5, xmax=1.5   )
        defineHisto( monTool,  "vtx_y",        namespace=dir,    path=npath, type="TH1F", title="vtx_y",   xbins=300,   xmin=-1.5, xmax=1.5   )
        defineHisto( monTool,  "vtx_z",        namespace=dir,    path=npath, type="TH1F", title="vtx_z",   xbins=180,   xmin=-180, xmax=180   )
        
        defineHisto( monTool,  "vtx_x_err",    namespace=dir,    path=npath, type="TH1F", title="vtx_x_err",   xbins=100,   xmin=-0.1, xmax=0.1  )
        defineHisto( monTool,  "vtx_y_err",    namespace=dir,    path=npath, type="TH1F", title="vtx_y_err",   xbins=100,   xmin=-0.1, xmax=0.1  )
        defineHisto( monTool,  "vtx_z_err",    namespace=dir,    path=npath, type="TH1F", title="vtx_z_err",   xbins=100,   xmin=-1,   xmax=1    )
        
        defineHisto( monTool,  "vtx_chi2OverNdof",  namespace=dir,   path=npath, type="TH1F", title="vtx_chi2OverNdof",   xbins=100,   xmin=0, xmax=10   )
        
        defineHisto( monTool,  "vtx_time",     namespace=dir,   path=npath, type="TH1F", title="vtx_time",       xbins=100,   xmin=0, xmax=1   )
        defineHisto( monTool,  "vtx_time_err", namespace=dir,   path=npath, type="TH1F", title="vtx_time_err",   xbins=100,   xmin=0, xmax=1   )
        
        defineHisto( monTool,  "vtx_type",     namespace=dir,   path=npath, type="TH1F", title="vtx_type",       xbins=200,   xmin=0, xmax=20   )
        
        
        defineHisto( monTool,  "vtx_nTracks",         namespace=dir,   path=npath, type="TH1F", title="vtx_nTracks",        xbins=100,   xmin=0,  xmax=100   )
        defineHisto( monTool,  "vtx_track_weight",    namespace=dir,   path=npath, type="TH1F", title="vtx_track_weight",   xbins=100,   xmin=0,  xmax=100   )
        
        defineHisto( monTool,  "vtx_track_pt",        namespace=dir,   path=npath, type="TH1F", title="vtx_track_pt",       xbins=100,   xmin=0,  xmax=100   )
        defineHisto( monTool,  "vtx_track_eta",       namespace=dir,   path=npath, type="TH1F", title="vtx_track_eta",      xbins=100,   xmin=-4, xmax=4   )
        
        defineHisto( monTool,  "vtx_track_d0",        namespace=dir,   path=npath, type="TH1F", title="vtx_track_d0",       xbins=100,   xmin=-10,   xmax=10   )
        defineHisto( monTool,  "vtx_track_z0",        namespace=dir,   path=npath, type="TH1F", title="vtx_track_z0",       xbins=100,   xmin=-180, xmax=180  )
        defineHisto( monTool,  "vtx_track_d0_err",    namespace=dir,   path=npath, type="TH1F", title="vtx_track_d0_err",   xbins=100,   xmin=0,    xmax=0.1  )
        defineHisto( monTool,  "vtx_track_z0_err",    namespace=dir,   path=npath, type="TH1F", title="vtx_track_z0_err",   xbins=100,   xmin=0,    xmax=0.1  )
        
        defineHisto( monTool,  "vtx_track_nSiHits",   namespace=dir,   path=npath, type="TH1F", title="vtx_track_nSiHits",  xbins=10,    xmin=0,    xmax=10   )
        defineHisto( monTool,  "vtx_track_nSiHoles",  namespace=dir,   path=npath, type="TH1F", title="vtx_track_nSiHoles", xbins=10,    xmin=0,    xmax=10   )
        
        # defineHisto( monTool,  "nVtx_vs_actualMu_2D",    namespace=dir,   path=mypath, type="TH2F", title="nVtx_vs_actualMu_2D",   xbins=0,   xmin=0, xmax=0   )

    # remaining residuals, and efficiencies ...
        
    defineHisto( monTool,  "vtx_ntest_vs_nref",    path=mpath, type="TH2F", title="vtx_ntest_vs_nref",    xbins=50,    xmin=0,    xmax=200,  ybins=50,  ymin=0,  ymax=200  )

    # residuals ...
    defineHisto( monTool,  "vtx_res_x",            path=mpath, type="TH1F", title="vtx_res_x",            xbins=100,   xmin=-0.3, xmax=0.3  )
    defineHisto( monTool,  "vtx_res_y",            path=mpath, type="TH1F", title="vtx_res_y",            xbins=100,   xmin=-0.3, xmax=0.3  )
    defineHisto( monTool,  "vtx_res_z",            path=mpath, type="TH1F", title="vtx_res_z",            xbins=200,   xmin=-2,   xmax=2  )

    # differential residuals ...

    rp = [ "vtx_res_z_vs_z", "vtx_res_z_vs_ntracks", "vtx_res_z_vs_nvtx",  "vtx_res_z_vs_mu" ]   

    resplot( monTool, namespace=rp[0],  path=mpath, label=rp[0],  xbins=20, xmin=-200, xmax=200, ybins=200, ymin=-2, ymax=2 ) 
    resplot( monTool, namespace=rp[1],  path=mpath, label=rp[1],  xbins=vnbins_full,             ybins=200, ymin=-2, ymax=2 ) 
    resplot( monTool, namespace=rp[2],  path=mpath, label=rp[2],  xbins=20, xmin=0,    xmax=200, ybins=200, ymin=-2, ymax=2 ) 
    resplot( monTool, namespace=rp[3],  path=mpath, label=rp[3],  xbins=20, xmin=0,    xmax=200, ybins=200, ymin=-2, ymax=2 ) 

    #    defineHisto( monTool,  "vtx_res_z_vs_z",       path=mpath, type="TH2F", title="vtx_res_z_vs_z",       xbins=20,    xmin=-200, xmax=200, ybins=200, ymin=-2, ymax=2 )
    #    defineHisto( monTool,  "vtx_res_z_vs_ntracks", path=mpath, type="TH2F", title="vtx_res_z_vs_ntracks", xbins=20,    xmin=0,    xmax=200, ybins=200, ymin=-2, ymax=2 )   
    #    defineHisto( monTool,  "vtx_res_z_vs_nvtx",    path=mpath, type="TH2F", title="vtx_res_z_vs_nvtx",    xbins=20,    xmin=0,    xmax=200, ybins=200, ymin=-2, ymax=2 )   
    #    defineHisto( monTool,  "vtx_res_z_vs_mu",      path=mpath, type="TH2F", title="vtx_res_z_vs_mu",      xbins=20,    xmin=0,    xmax=200, ybins=200, ymin=-2, ymax=2 )   

    # efficiencies ...
    defineHisto( monTool,  "vtx_eff_vs_z",         path=mpath, type="TEfficiency", title="vtx_eff_vs_z",       xbins=50,    xmin=-200, xmax=200   )
    defineHisto( monTool,  "vtx_eff_vs_mu",        path=mpath, type="TEfficiency", title="vtx_eff_vs_mu",      xbins=20,    xmin=0,    xmax=200   )
    defineHisto( monTool,  "vtx_eff_vs_nvtx",      path=mpath, type="TEfficiency", title="vtx_eff_vs_nvtx",    xbins=40,    xmin=0,    xmax=200   )
    defineHisto( monTool,  "vtx_eff_vs_ntracks",   path=mpath, type="TEfficiency", title="vtx_eff_vs_ntracks", xbins=20,    xmin=0,    xmax=200   )

    
    return
