#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# create all the histograms for each analysis - this will get called once for each 
# configured chain - we can set the HistPath either already here, or from the c++ 
# code 
        

# actually create the monTool to go along with a specific chain
# stores the histogram, binning, creates all the histograms etc

from TrigIDR4Monitoring.HistUtils import defineHisto

def Tier0_builder( monTool, mypath, chain, cs ) :

    from TrigIDR4Monitoring.HistUtils import ptbins_mb
    from TrigIDR4Monitoring.HistUtils import ptbins_default
    
    if "mb" in chain : ptbins = ptbins_mb
    else:              ptbins = ptbins_default
    
    from TrigIDR4Monitoring.HistUtils import vnbins_full
    from TrigIDR4Monitoring.HistUtils import vnbins_short

    vnbins = vnbins_full
    

    from TrigIDR4Monitoring.HistUtils import d0bins_default
    from TrigIDR4Monitoring.HistUtils import d0bins_lrt

    if  "LRT" in chain :  d0bins = d0bins_lrt
    else:                 d0bins = d0bins_default


#   use a rounded up float(M_PI) values to overcome edge effects of using float 
#   values - will math.pi be appropriate ? double(float(M_PI)) > M_PI so it would
#   depend on whether floats or double are used for the histogram limits in the
#   histograms in defineHistogram  
#   pi = 3.14159265359 
    pi = 3.14159275 

    binlabels = [ "Nrois", "Nevents", "N ref tracks", "N matched tracks", "N total test tracks" ]

    mypath = mypath + "/Tier0"
    
    defineHisto( monTool,  "Chain",    weight="Chain_weight",   path=mypath, type="TH1F", title=chain,  xlabels=binlabels,  xbins=5, xmin=0, xmax=5 )


    defineHisto( monTool,  "roi_eta",      path=mypath, type="TH1F", title="Roi eta",  xbins=25, xmin=-2.5, xmax=2.5 )
    
    if ( "FS_FTF" in cs.tail or "JetSuper_FTF" in cs.tail or "mb" in chain ): 
        defineHisto( monTool,  "reftrk_N",     path=mypath, type="TH1F", title="Reference tracks", xbins=vnbins )
    else:
        defineHisto( monTool,  "reftrk_N",     path=mypath, type="TH1F", title="Reference tracks", xbins=61, xmin=-0.5, xmax=60.5  )
    
    defineHisto( monTool,  "reftrk_pT",    path=mypath, type="TH1F", title="Reference track pT",                xbins=ptbins   )
    defineHisto( monTool,  "reftrk_phi",   path=mypath, type="TH1F", title="Reference track Phi",               xbins=25,   xmin=-pi,    xmax=pi  )
    defineHisto( monTool,  "reftrk_eta",   path=mypath, type="TH1F", title="Reference track Eta",               xbins=25,   xmin=-2.5,     xmax=2.5   )
    
    if "LRT" in chain:
            defineHisto( monTool,  "reftrk_d0",    path=mypath, type="TH1F", title="Reference track d0",     xbins=201,  xmin=-100.0,   xmax=100.0 ) 
    else:
            defineHisto( monTool,  "reftrk_d0",    path=mypath, type="TH1F", title="Reference track d0",     xbins=201,  xmin=-0.5,     xmax=0.5   ) 

    defineHisto( monTool,  "reftrk_z0",    path=mypath, type="TH1F", title="Reference track z0",                xbins=50,   xmin=-225.,    xmax=225.  )
    defineHisto( monTool,  "reftrk_dd0",   path=mypath, type="TH1F", title="Reference track sigma(d0)",         xbins=50,   xmin=0,        xmax=0.5 )
    defineHisto( monTool,  "reftrk_dz0",   path=mypath, type="TH1F", title="Reference track sigma(z0)",         xbins=50,   xmin=0,        xmax=2.5 )
    defineHisto( monTool,  "reftrk_d0sig", path=mypath, type="TH1F", title="Reference track d0 significance",   xbins= 101, xmin=-5.,      xmax=5. )

    if ( "FS_FTF" in cs.tail or "JetSuper_FTF" in cs.tail or "mb" in chain ): 
        defineHisto( monTool,  "testtrk_N",     path=mypath, type="TH1F", title="Test tracks", xbins=vnbins )
    else:
        defineHisto( monTool,  "testtrk_N",     path=mypath, type="TH1F", title="Test tracks", xbins=61, xmin=-0.5, xmax=60.5  )
    
    defineHisto( monTool,  "testtrk_pT",    path=mypath, type="TH1F", title="Test track pT",              xbins=ptbins   )
    defineHisto( monTool,  "testtrk_phi",   path=mypath, type="TH1F", title="Test track Phi",             xbins=25,   xmin=-pi,  xmax=pi  )
    defineHisto( monTool,  "testtrk_eta",   path=mypath, type="TH1F", title="Test track Eta",             xbins=25,   xmin=-2.5,   xmax=2.5   )

    if "LRT" in chain:
            defineHisto( monTool,  "testtrk_d0",    path=mypath, type="TH1F", title="Test track d0",                  xbins=201,  xmin=-100.0, xmax=100.0 )
            defineHisto( monTool,  "reftrk_d0sig", path=mypath, type="TH1F", title="Reference track d0 significance", xbins= 101, xmin=-100.,      xmax=100. )
    else:
            defineHisto( monTool,  "testtrk_d0",    path=mypath, type="TH1F", title="Test track d0",                  xbins=201,  xmin=-0.5,   xmax=0.5   )
            defineHisto( monTool,  "reftrk_d0sig", path=mypath, type="TH1F", title="Reference track d0 significance", xbins= 101, xmin=-5.,      xmax=5. )

    defineHisto( monTool,  "testtrk_z0",    path=mypath, type="TH1F", title="Test track z0",              xbins=50,   xmin=-225.,  xmax=225.  )
    defineHisto( monTool,  "testtrk_dd0",   path=mypath, type="TH1F", title="Test track sigma(d0)",       xbins=50,   xmin=0,      xmax=0.5 )
    defineHisto( monTool,  "testtrk_dz0",   path=mypath, type="TH1F", title="Test track sigma(z0)",       xbins=50,   xmin=0,      xmax=2.5 )
    defineHisto( monTool,  "testtrk_d0sig", path=mypath, type="TH1F", title="Test track d0 significance", xbins=101,  xmin=-5.0,   xmax=5.0  )
    
    defineHisto( monTool,  "layer_rec",     path=mypath, type="TH1F", title="hit layers",                 xbins=32,   xmin=-0.5,   xmax=31.5 )
    defineHisto( monTool,  "layer",         path=mypath, type="TH1F", title="hit layers",                 xbins=32,   xmin=-0.5,   xmax=31.5  )
    
    defineHisto( monTool,  "residual_pT",   path=mypath, type="TH1F", title="track pT residual",         xbins=201,   xmin=-50.0, xmax=50.0 )
    defineHisto( monTool,  "residual_ipT",  path=mypath, type="TH1F", title="track ipT residual",        xbins=55,    xmin=-1.5,   xmax=1.5   )
    
    if "LRT" in chain:
        defineHisto( monTool,  "residual_phi",  path=mypath, type="TH1F", title="track Phi residual",        xbins=50,    xmin=-0.02,  xmax=0.02  )
        defineHisto( monTool,  "residual_eta",  path=mypath, type="TH1F", title="track Eta residual",        xbins=50,    xmin=-0.02,  xmax=0.02  )
        defineHisto( monTool,  "residual_d0",   path=mypath, type="TH1F", title="track d0 residual ",        xbins=251,   xmin=-1.0,   xmax=1.0   )
        defineHisto( monTool,  "residual_z0",   path=mypath, type="TH1F", title="track z0 residual",         xbins=401,   xmin=-2.5,   xmax=2.5  )
        defineHisto( monTool,  "residual_dd0",  path=mypath, type="TH1F", title="track sigma d0 residual ",  xbins=251,   xmin=-0.5,   xmax=0.5  )
        defineHisto( monTool,  "residual_dz0",  path=mypath, type="TH1F", title="track sigma z0 residual",   xbins=401,   xmin=-1.0,   xmax=1.0  )
    else:
        defineHisto( monTool,  "residual_phi",  path=mypath, type="TH1F", title="track Phi residual",        xbins=50,    xmin=-0.01,  xmax=0.01  )
        defineHisto( monTool,  "residual_eta",  path=mypath, type="TH1F", title="track Eta residual",        xbins=50,    xmin=-0.01,  xmax=0.01  )
        defineHisto( monTool,  "residual_d0",   path=mypath, type="TH1F", title="track d0 residual ",        xbins=251,   xmin=-0.6,   xmax=0.6   )
        defineHisto( monTool,  "residual_z0",   path=mypath, type="TH1F", title="track z0 residual",         xbins=401,   xmin=-2.0,   xmax=2.0  )
        defineHisto( monTool,  "residual_dd0",  path=mypath, type="TH1F", title="track sigma d0 residual ",  xbins=251,   xmin=-0.25,   xmax=0.25  )
        defineHisto( monTool,  "residual_dz0",  path=mypath, type="TH1F", title="track sigma z0 residual",   xbins=401,   xmin=-0.5,   xmax=0.5  )
    
    defineHisto( monTool,  "npix",          path=mypath, type="TH1F", title="npix",                 xbins=26,    xmin=-0.5,   xmax=25.5  )
    defineHisto( monTool,  "npix_rec",      path=mypath, type="TH1F", title="npix_rec",             xbins=26,    xmin=-0.5,   xmax=25.5  )
    
    defineHisto( monTool,  "nsct",          path=mypath, type="TH1F", title="nsct",                 xbins=31,    xmin=-0.5,   xmax=30.5  )
    defineHisto( monTool,  "nsct_rec",      path=mypath, type="TH1F", title="nsct_rec",             xbins=31,    xmin=-0.5,   xmax=30.5  )
    
    defineHisto( monTool,  "nsiHits",       path=mypath, type="TH1F", title="nsiHits",              xbins=41,    xmin=-0.5,   xmax=40.5  )
    defineHisto( monTool,  "nsiHits_rec",   path=mypath, type="TH1F", title="nsiHits_rec",          xbins=41,    xmin=-0.5,   xmax=40.5  )
    
    defineHisto( monTool,  "ntrt",          path=mypath, type="TH1F", title="ntrt",                 xbins=91,    xmin=-0.5,   xmax=91.5  )
    defineHisto( monTool,  "ntrt_rec",      path=mypath, type="TH1F", title="ntrt_rec",             xbins=91,    xmin=-0.5,   xmax=91.5  )

    defineHisto( monTool,  "chi2dof",       path=mypath, type="TH1F", title="chi2dof",              xbins=75,    xmin=-0,   xmax=15  )
    defineHisto( monTool,  "chi2dof_rec",   path=mypath, type="TH1F", title="chi2dof_rec",          xbins=75,    xmin=-0,   xmax=15  )
    
    defineHisto( monTool,  "mu",            path=mypath, type="TH1F", title="mu;#mu;Entries",       xbins=80,    xmin=0,    xmax=80  )
        
    defineHisto( monTool,  "Eff_overall",   path=mypath, type="TProfile", title="total efficiency", xbins=1,     xmin=0.,   xmax=1.    )
    
    defineHisto( monTool,  "Eff_pT",     path=mypath, type="TProfile", title="pT efficiency",       xbins=ptbins   )
    defineHisto( monTool,  "Eff_Eta",    path=mypath, type="TProfile", title="eta efficiency",      xbins=25,   xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Eff_Phi",    path=mypath, type="TProfile", title="phi  efficiency",     xbins=25,   xmin=-pi, xmax=pi )
    defineHisto( monTool,  "Eff_d0",     path=mypath, type="TProfile", title="d0 efficiency",       xbins=d0bins     ) 
    defineHisto( monTool,  "Eff_z0",     path=mypath, type="TProfile", title="z0 efficiency",       xbins=50,   xmin=-225., xmax=225. )
    defineHisto( monTool,  "Eff_nVtx",   path=mypath, type="TProfile", title="nVtx efficiency",     xbins=101,   xmin=-0.5,  xmax=100.5 )
    defineHisto( monTool,  "Eff_lb",     path=mypath, type="TProfile", title="efficiency vs lumiblock",      xbins=301, xmin=-0.5, xmax=3009.5 )

    if ( "FS_FTF" in cs.tail or "JetSuper_FTF" in cs.tail or "mb" in chain ): 
        defineHisto( monTool,  "Eff_ntrax",     path=mypath, type="TProfile", title="ntrack efficiency",       xbins=vnbins   )
    else:
        defineHisto( monTool,  "Eff_ntrax",     path=mypath, type="TProfile", title="ntrack efficiency", xbins=61, xmin=-0.5, xmax=60.5  )
    
    defineHisto( monTool,  "trkvtx_x_vs_lb", path=mypath, type="TProfile", title="track vertex x vs lumiblock", xbins=301, xmin=-0.5, xmax=3009.5 )
    defineHisto( monTool,  "trkvtx_y_vs_lb", path=mypath, type="TProfile", title="track vertex y vs lumiblock", xbins=301, xmin=-0.5, xmax=3009.5 )
    defineHisto( monTool,  "trkvtx_z_vs_lb", path=mypath, type="TProfile", title="track vertex z vs lumiblock", xbins=3010, xmin=-0.5, xmax=3009.5 )

    defineHisto( monTool,  "npix_vs_eta",     path=mypath, type="TProfile", title="offline npix vs eta;offline #eta; mean number of offline pixel hits", xbins=30,   xmin=-2.5,  xmax=2.5 )
    defineHisto( monTool,  "npix_vs_eta_rec", path=mypath, type="TProfile", title="trigger npix vs eta;offline #eta; mean number of trigger pixel hits", xbins=30,   xmin=-2.5,  xmax=2.5 )
    defineHisto( monTool,  "nsct_vs_eta",     path=mypath, type="TProfile", title="offline nsct vs eta;offline #eta; mean number of offline sct hits",   xbins=30,   xmin=-2.5,  xmax=2.5 )
    defineHisto( monTool,  "nsct_vs_eta_rec", path=mypath, type="TProfile", title="trigger nsct vs eta;offline #eta; mean number of trigger sct hits",   xbins=30,   xmin=-2.5,  xmax=2.5 )
    defineHisto( monTool,  "ntrt_vs_eta",     path=mypath, type="TProfile", title="offline ntrt vs eta;offline #eta; mean number of offline trt hits",   xbins=30,   xmin=-2.5,  xmax=2.5 )
    defineHisto( monTool,  "ntrt_vs_eta_rec", path=mypath, type="TProfile", title="trigger ntrt vs eta;offline #eta; mean number of trigger trt hits",   xbins=30,   xmin=-2.5,  xmax=2.5 )
    
    defineHisto( monTool,  "npix_vs_phi",     path=mypath, type="TProfile", title="offline npix vs phi;offline #phi; mean number of offline pixel hits", xbins=30,   xmin=-pi, xmax=pi ) 
    defineHisto( monTool,  "npix_vs_phi_rec", path=mypath, type="TProfile", title="trigger npix vs phi;offline #phi; mean number of trigger pixel hits", xbins=30,   xmin=-pi, xmax=pi )
    defineHisto( monTool,  "nsct_vs_phi",     path=mypath, type="TProfile", title="offline nsct vs phi;offline #phi; mean number of offline sct hits",   xbins=30,   xmin=-pi, xmax=pi )
    defineHisto( monTool,  "nsct_vs_phi_rec", path=mypath, type="TProfile", title="trigger nsct vs phi;offline #phi; mean number of trigger sct hits",   xbins=30,   xmin=-pi, xmax=pi )
    defineHisto( monTool,  "ntrt_vs_phi",     path=mypath, type="TProfile", title="offline ntrt vs phi;offline #phi; mean number of offline trt hits",   xbins=30,   xmin=-pi, xmax=pi )
    defineHisto( monTool,  "ntrt_vs_phi_rec", path=mypath, type="TProfile", title="trigger ntrt vs phi;offline #phi; mean number of trigger trt hits",   xbins=30,   xmin=-pi, xmax=pi )
    
    defineHisto( monTool,  "npix_vs_d0",        path=mypath, type="TProfile", title="offline npix vs d0;offline #d0; mean number of offline pixel hits",   xbins=d0bins )
    defineHisto( monTool,  "npix_vs_d0_rec",    path=mypath, type="TProfile", title="trigger npix vs d0;offline #d0; mean number of trigger pixel hits",   xbins=d0bins )
    defineHisto( monTool,  "nsct_vs_d0",        path=mypath, type="TProfile", title="offline nsct vs d0;offline #d0; mean number of offline sct hits",     xbins=d0bins ) 
    defineHisto( monTool,  "nsct_vs_d0_rec",    path=mypath, type="TProfile", title="trigger nsct vs d0;offline #d0; mean number of trigger sct hits",     xbins=d0bins )
    defineHisto( monTool,  "npix_vs_pT",        path=mypath, type="TProfile", title="offline npix vs pT;offline #p_{T}; mean number of offline pixel hits",   xbins=ptbins )
    defineHisto( monTool,  "npix_vs_pT_rec",    path=mypath, type="TProfile", title="trigger npix vs pT;offline #p_{T}; mean number of trigger pixel hits",   xbins=ptbins )
    defineHisto( monTool,  "nsct_vs_pT",        path=mypath, type="TProfile", title="offline nsct vs pT;offline #p_{T} [GeV]; mean number of offline pixel hits", xbins=ptbins )
    defineHisto( monTool,  "nsct_vs_pT_rec",    path=mypath, type="TProfile", title="trigger nsct vs pT;offline #p_{T} [GeV]; mean number of trigger pixel hits", xbins=ptbins )
    defineHisto( monTool,  "nsihits_lb",        path=mypath, type="TProfile", title="offline n sihits vs lumiblock",   xbins=301, xmin=-0.5, xmax=3009.5 )
    defineHisto( monTool,  "nsihits_lb_rec",    path=mypath, type="TProfile", title="trigger n sihits vs lumiblock",   xbins=301, xmin=-0.5, xmax=3009.5 )
    
    defineHisto( monTool,  "Res_pT",    path=mypath, type="TProfile", title="mean pT residual;#eta_{ref};pT_{trig}-pT_{ref} [GeV]",           xbins=25,  xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Res_ipT",   path=mypath, type="TProfile", title="mean ipT residual;#eta_{ref};1/pT_{trig}-1/pT_{ref} [GeV^{-1}]", xbins=25,  xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Res_eta",   path=mypath, type="TProfile", title="mean Eta residual;#eta_{ref};eta_{trig}-#eta_{ref}",             xbins=25,  xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Res_phi",   path=mypath, type="TProfile", title="mean Phi residual;#eta_{ref};phi_{trig}-#phi_{ref}",             xbins=25,  xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Res_d0",    path=mypath, type="TProfile", title="mean d0 residual;#eta_{ref};Delta d0 [mm]",                      xbins=25,  xmin=-2.5,  xmax=2.5  )
    defineHisto( monTool,  "Res_z0",    path=mypath, type="TProfile", title="mean z0 residual;#eta_{ref};Delta z0 [mm]",                       xbins=25,  xmin=-2.5,  xmax=2.5  )

    
    defineHisto( monTool,  "d0_vs_phi_prof",     path=mypath, type="TProfile", title="d0 vs phi_prof;#phi;d0 [mm]",      xbins=25,     xmin=-pi,  xmax=pi )
    defineHisto( monTool,  "d0_vs_phi_rec_prof", path=mypath, type="TProfile", title="d0 vs phi_rec_prof;#phi; d0 [mm]",  xbins=25,     xmin=-pi,  xmax=pi )

    if "probe" in cs.extra: 
        if "Jpsi" in chain:
            defineHisto( monTool,  "invmass",      path=mypath, type="TH1F", title="invariant mass;mass [GeV]", xbins=50, xmin=0, xmax=10 )
            defineHisto( monTool,  "invmass_obj",  path=mypath, type="TH1F", title="invariant mass;mass [GeV]", xbins=50, xmin=0, xmax=10 )
        else: 
            defineHisto( monTool,  "invmass",      path=mypath, type="TH1F", title="invariant mass;mass [GeV]", xbins=320, xmin=0, xmax=200 )
            defineHisto( monTool,  "invmass_obj",  path=mypath, type="TH1F", title="invariant mass;mass [GeV]", xbins=320, xmin=0, xmax=200 )
            

    if cs.vtx :

            defineHisto( monTool, "vx_nvtx",  path=mypath, type="TH1F", title="Offline vertex multiplicity;number of vertices",  xbins=101, xmin=-0.5,  xmax=100.5 )
            defineHisto( monTool, "vx_zed",   path=mypath, type="TH1F", title="Offline vertex z;vtx z [mm]",          xbins=100, xmin=-250,  xmax=250   )
            defineHisto( monTool, "vx_x",     path=mypath, type="TH1F", title="Offline vertex x;vtx x [mm]",          xbins=200, xmin=-1.2,    xmax=1.2   )
            defineHisto( monTool, "vx_y",     path=mypath, type="TH1F", title="Offline vertex y;vtx y [mm]",          xbins=200, xmin=-1.2,    xmax=1.2   )
            defineHisto( monTool, "vx_ntrax", path=mypath, type="TH1F", title="Offline vertex track multiplicity;number of tracks",    xbins=vnbins )
            
            defineHisto( monTool, "vx_nvtx_rec",  path=mypath, type="TH1F", title="Trigger vertex multiplicity;number of vertices",   xbins=101, xmin=-0.5,  xmax=100.5 )
            defineHisto( monTool, "vx_zed_rec",   path=mypath, type="TH1F", title="Trigger vertex Z;vtx z [mm]",           xbins=100, xmin=-250,  xmax=250   )
            defineHisto( monTool, "vx_x_rec",     path=mypath, type="TH1F", title="Trigger vertex X;vtx x [mm]",           xbins=200, xmin=-1.2,  xmax=1.2   )
            defineHisto( monTool, "vx_y_rec",     path=mypath, type="TH1F", title="Trigger vertex Y;vtx y [mm]",           xbins=200, xmin=-1.2,  xmax=1.2   )
            defineHisto( monTool, "vx_ntrax_rec", path=mypath, type="TH1F", title="Trigger vertex track multiplicity;number of tracks",     xbins=vnbins )
            
            defineHisto( monTool, "vx_zed_res",   path=mypath, type="TH1F", title="Delta z [mm]", xbins=400, xmin=-1.0,  xmax=1.0  )
            defineHisto( monTool, "vx_x_res",     path=mypath, type="TH1F", title="Delta x [mm]", xbins=400, xmin=-0.1, xmax=0.1 )
            defineHisto( monTool, "vx_y_res",     path=mypath, type="TH1F", title="Delta y [mm]", xbins=400, xmin=-0.1, xmax=0.1 )
            
            
            defineHisto( monTool, "vx_rdz_vs_zed",   path=mypath, type="TProfile", title="rdz_vs_zed; vtx z [mm];mean z residual [mm]",         xbins=100, xmin=-250,  xmax=250 ) 
            defineHisto( monTool, "vx_rdz_vs_ntrax", path=mypath, type="TProfile", title="rdz_vs_ntrax;number of tracks;mean z residual [mm]",  xbins=vnbins_short )
            defineHisto( monTool, "vx_rdz_vs_nvtx",  path=mypath, type="TProfile", title="rdz_vs_nvtx;number of vertices;mean z residual [mm]", xbins=51,  xmin=-0.125, xmax=50.125 )
            
            # defineHistogram( monTool, "rdz_vs_mu",    30,     0,    30,    400, -20, 20 ); 
            
            defineHisto( monTool, "vx_zed_eff",    path=mypath, type="TProfile", title="Vertex efficicy vs z;efficiency;offline vtx z [mm]",   xbins=50,  xmin=-250,  xmax=250    )
            defineHisto( monTool, "vx_ntrax_eff",  path=mypath, type="TProfile", title="Vertex efficiency vs N tracks;number of tracks;efficiency",   xbins=vnbins )
            defineHisto( monTool, "vx_nvtx_eff",   path=mypath, type="TProfile", title="Vertex efficincy vs N vertices;number of vertices;efficiency",  xbins=101, xmin=-0.5,  xmax=100.5  )
            defineHisto( monTool, "vx_mu_eff",     path=mypath, type="TProfile", title="Vertex efficiency vs mu;<#mu>;efficiency",                 xbins=101,  xmin=-0.5,  xmax=100.5   )
            defineHisto( monTool, "vx_lb_eff",     path=mypath, type="TProfile", title="Vertex efficiency vs lumiblock;lumi block;efficiency",            xbins=151, xmin=-0.5,  xmax=3019.5 )
 

    
    return monTool

