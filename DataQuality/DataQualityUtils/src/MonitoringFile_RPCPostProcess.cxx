/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//*************************************************
// Class for the RPC merge histogram and fill the COOL DB
// author Michele Bianco michele.bianco@le.infn.it, Gabriele Chiodini gabriele.chiodini@le.infn.it                          
// and Angelo Guida angelo.guida@le.infn.it
// 08/April/2009
//************************************************

#include "DataQualityUtils/MonitoringFile.h"
#include "DataQualityUtils/CoolRpc.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include "TClass.h"
#include "TKey.h"

namespace{
  void
  writeIfValid(TH1F * pH){
    if(pH) pH->Write("",TObject::kOverwrite);
  }
  
  void
  fillIfValid(TH1F * pH, float val){
	  if(pH) pH->Fill(val);
	  return;
	}
	
	float
	getBinContentIfValid(TH1F * pH, int binIdx){
	     float result = -9999;;
	     if (pH) result = pH->GetBinContent(binIdx);
	     return result;
	}
	
  float
  getBinErrorIfValid(TH1F * pH, int binIdx){
    float result = -1;;
    if (pH) result = pH->GetBinError(binIdx);
    return result;
  }

}

namespace dqutils {

void 
MonitoringFile::RPCPostProcess( const std::string & inFilename, bool /* isIncremental */ ) 
{
 // std::cout << "Running RPC post processing \n" ;
  
  bool  applyEffThreshold  =  true;
  bool  EffThreshold       = false;
  bool  printout           = true ;
  float Minimum_efficiency =  0.5;
  
  
  
  TFile* f = TFile::Open(inFilename.c_str(),"UPDATE");
  if (f == 0) {
    std::cerr << "MonitoringFile::RPCPostProcess(): "
	      << "Input file not opened \n";
    return;
  }
  if(f->GetSize()<1000.) {
    std::cerr << "MonitoringFile::RPCPostProcess(): "
	      << "Input file empty \n";
    return; 
  }
  CoolRpc coolrpc;												    		 
 
  // get run directory name
  std::string run_dir;
  TIter next_run ( f->GetListOfKeys() );
  TKey* key_run(0);
  while ((key_run = dynamic_cast<TKey*>( next_run() )) !=0 ) {
    TObject* obj_run = key_run->ReadObj();
    TDirectory* tdir_run = dynamic_cast<TDirectory*>( obj_run );
    if (tdir_run !=0 ) {
      std::string tdir_run_name( tdir_run->GetName() );
      if (tdir_run_name.find("run")!= std::string::npos ) {
        run_dir=std::move(tdir_run_name);
	
	int run_number;
	run_number = atoi( (run_dir.substr(4, run_dir.size()-4 )).c_str() );
        std::cout << "run_number rpc monitoring " << run_number <<std::endl;
	
	std::string pathRawMon     = run_dir + "/Muon/MuonRawDataMonitoring/RPC/"                           ;
	//std::string pathTrackMon   = run_dir + "/Muon/MuonTrackMonitoring/NoTrigger/RPCStandAloneTrackMon/" ;
	std::string pathTrackMon   = run_dir + "/Muon/MuonRawDataMonitoring/RPCStandAloneTrackMon/" ;
	
	std::string dir_ov_raw     = pathRawMon   + "Overview/" ;
	std::string dir_sum_raw    = pathRawMon   + "Summary/"  ;
	std::string dir_dqmf_raw   = pathRawMon   + "Dqmf/"     ;
	
  	std::string dir_sideA_track    = pathTrackMon + "RPCBA/"             ;
  	std::string dir_sideC_track    = pathTrackMon + "RPCBC/"             ;
  	std::string dir_glob_track     = pathTrackMon + "GLOBAL/"            ;
	std::string dir_sum_track      = pathTrackMon + "Summary/"           ;
	std::string dir_dqmf_track     = pathTrackMon + "Dqmf/"              ;
	std::string dir_trigger_track  = pathTrackMon + "TriggerEfficiency/" ;
	
	double n_hit_f, n_tr_p, panel_eff, panel_err_eff;
	double nEta, nPhi, nEtaPhi, gapEff=0.0, gapErrEff=0.0;
	double res_mean, res2_mean, res_RMS;
	double panel_occ; 
	double panelCS, panelCS2, panelCS_entries , panelCS_mean, panelCS2_mean, panelCS_RMS;
	double Time_mean, Time2_mean,  Time_RMS;
	double noiseCorr, noiseCorrErr ;
	double noiseTot,  noiseTotErr  ;
	double noiseErrNorm ;
	
	// trigger efficiency
	std::string METracks_name   = dir_trigger_track   + "hMEtracks"            ; 
	std::string MuctpiThr0_name = dir_trigger_track   + "hRPCMuctpiThr0"       ; 
	std::string MuctpiThr1_name = dir_trigger_track   + "hRPCMuctpiThr1"       ; 
	std::string MuctpiThr2_name = dir_trigger_track   + "hRPCMuctpiThr2"       ; 
	std::string MuctpiThr3_name = dir_trigger_track   + "hRPCMuctpiThr3"       ; 
	std::string MuctpiThr4_name = dir_trigger_track   + "hRPCMuctpiThr4"       ; 
	std::string MuctpiThr5_name = dir_trigger_track   + "hRPCMuctpiThr5"       ; 
	std::string PadThr0_name = dir_trigger_track   + "hRPCPadThr0"       ; 
	std::string PadThr1_name = dir_trigger_track   + "hRPCPadThr1"       ; 
	std::string PadThr2_name = dir_trigger_track   + "hRPCPadThr2"       ; 
	std::string PadThr3_name = dir_trigger_track   + "hRPCPadThr3"       ; 
	std::string PadThr4_name = dir_trigger_track   + "hRPCPadThr4"       ; 
	std::string PadThr5_name = dir_trigger_track   + "hRPCPadThr5"       ; 
	std::string PhiEtaCoinThr0_name = dir_trigger_track   + "hRPCPhiEtaCoinThr0"       ; 
	std::string PhiEtaCoinThr1_name = dir_trigger_track   + "hRPCPhiEtaCoinThr1"       ; 
	std::string PhiEtaCoinThr2_name = dir_trigger_track   + "hRPCPhiEtaCoinThr2"       ; 
	std::string PhiEtaCoinThr3_name = dir_trigger_track   + "hRPCPhiEtaCoinThr3"       ; 
	std::string PhiEtaCoinThr4_name = dir_trigger_track   + "hRPCPhiEtaCoinThr4"       ; 
	std::string PhiEtaCoinThr5_name = dir_trigger_track   + "hRPCPhiEtaCoinThr5"       ; 
	
	std::string MuctpiThr_eff0_name = dir_trigger_track   + "hRPCMuctpiThr_eff0"       ; 
	std::string MuctpiThr_eff1_name = dir_trigger_track   + "hRPCMuctpiThr_eff1"       ; 
	std::string MuctpiThr_eff2_name = dir_trigger_track   + "hRPCMuctpiThr_eff2"       ; 
	std::string MuctpiThr_eff3_name = dir_trigger_track   + "hRPCMuctpiThr_eff3"       ; 
	std::string MuctpiThr_eff4_name = dir_trigger_track   + "hRPCMuctpiThr_eff4"       ; 
	std::string MuctpiThr_eff5_name = dir_trigger_track   + "hRPCMuctpiThr_eff5"       ; 
	std::string PadThr_eff0_name = dir_trigger_track   + "hRPCPadThr_eff0"       ; 
	std::string PadThr_eff1_name = dir_trigger_track   + "hRPCPadThr_eff1"       ; 
	std::string PadThr_eff2_name = dir_trigger_track   + "hRPCPadThr_eff2"       ; 
	std::string PadThr_eff3_name = dir_trigger_track   + "hRPCPadThr_eff3"       ; 
	std::string PadThr_eff4_name = dir_trigger_track   + "hRPCPadThr_eff4"       ; 
	std::string PadThr_eff5_name = dir_trigger_track   + "hRPCPadThr_eff5"       ; 
	std::string PhiEtaCoinThr_eff0_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff0"       ; 
	std::string PhiEtaCoinThr_eff1_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff1"       ; 
	std::string PhiEtaCoinThr_eff2_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff2"       ; 
	std::string PhiEtaCoinThr_eff3_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff3"       ; 
	std::string PhiEtaCoinThr_eff4_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff4"       ; 
	std::string PhiEtaCoinThr_eff5_name = dir_trigger_track   + "hRPCPhiEtaCoinThr_eff5"       ; 
 
 	
	if ( RPCCheckHistogram(f,METracks_name.c_str()) && RPCCheckHistogram(f,MuctpiThr0_name.c_str()) && RPCCheckHistogram(f,MuctpiThr_eff0_name.c_str())) {
	
 	 
	  TH1I* hist_METracks   = (TH1I*)(f->Get( METracks_name.c_str()));
	  TH1I* hist_MuctpiThr0 = (TH1I*)(f->Get( MuctpiThr0_name.c_str()));
	  TH1I* hist_MuctpiThr1 = (TH1I*)(f->Get( MuctpiThr1_name.c_str()));
	  TH1I* hist_MuctpiThr2 = (TH1I*)(f->Get( MuctpiThr2_name.c_str()));
	  TH1I* hist_MuctpiThr3 = (TH1I*)(f->Get( MuctpiThr3_name.c_str()));
	  TH1I* hist_MuctpiThr4 = (TH1I*)(f->Get( MuctpiThr4_name.c_str()));
	  TH1I* hist_MuctpiThr5 = (TH1I*)(f->Get( MuctpiThr5_name.c_str()));
	  TH1I* hist_PadThr0 = (TH1I*)(f->Get( PadThr0_name.c_str()));
	  TH1I* hist_PadThr1 = (TH1I*)(f->Get( PadThr1_name.c_str()));
	  TH1I* hist_PadThr2 = (TH1I*)(f->Get( PadThr2_name.c_str()));
	  TH1I* hist_PadThr3 = (TH1I*)(f->Get( PadThr3_name.c_str()));
	  TH1I* hist_PadThr4 = (TH1I*)(f->Get( PadThr4_name.c_str()));
	  TH1I* hist_PadThr5 = (TH1I*)(f->Get( PadThr5_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr0 = (TH1I*)(f->Get( PhiEtaCoinThr0_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr1 = (TH1I*)(f->Get( PhiEtaCoinThr1_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr2 = (TH1I*)(f->Get( PhiEtaCoinThr2_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr3 = (TH1I*)(f->Get( PhiEtaCoinThr3_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr4 = (TH1I*)(f->Get( PhiEtaCoinThr4_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr5 = (TH1I*)(f->Get( PhiEtaCoinThr5_name.c_str()));
	  TH1I* hist_MuctpiThr_eff0 = (TH1I*)(f->Get( MuctpiThr_eff0_name.c_str()));
	  TH1I* hist_MuctpiThr_eff1 = (TH1I*)(f->Get( MuctpiThr_eff1_name.c_str()));
	  TH1I* hist_MuctpiThr_eff2 = (TH1I*)(f->Get( MuctpiThr_eff2_name.c_str()));
	  TH1I* hist_MuctpiThr_eff3 = (TH1I*)(f->Get( MuctpiThr_eff3_name.c_str()));
	  TH1I* hist_MuctpiThr_eff4 = (TH1I*)(f->Get( MuctpiThr_eff4_name.c_str()));
	  TH1I* hist_MuctpiThr_eff5 = (TH1I*)(f->Get( MuctpiThr_eff5_name.c_str()));
	  TH1I* hist_PadThr_eff0 = (TH1I*)(f->Get( PadThr_eff0_name.c_str()));
	  TH1I* hist_PadThr_eff1 = (TH1I*)(f->Get( PadThr_eff1_name.c_str()));
	  TH1I* hist_PadThr_eff2 = (TH1I*)(f->Get( PadThr_eff2_name.c_str()));
	  TH1I* hist_PadThr_eff3 = (TH1I*)(f->Get( PadThr_eff3_name.c_str()));
	  TH1I* hist_PadThr_eff4 = (TH1I*)(f->Get( PadThr_eff4_name.c_str()));
	  TH1I* hist_PadThr_eff5 = (TH1I*)(f->Get( PadThr_eff5_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff0 = (TH1I*)(f->Get( PhiEtaCoinThr_eff0_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff1 = (TH1I*)(f->Get( PhiEtaCoinThr_eff1_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff2 = (TH1I*)(f->Get( PhiEtaCoinThr_eff2_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff3 = (TH1I*)(f->Get( PhiEtaCoinThr_eff3_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff4 = (TH1I*)(f->Get( PhiEtaCoinThr_eff4_name.c_str()));
	  TH1I* hist_PhiEtaCoinThr_eff5 = (TH1I*)(f->Get( PhiEtaCoinThr_eff5_name.c_str()));
	
	  int nb = hist_METracks->GetNbinsX() ;
	  double  Ly_eff, Ly_effErr ;
	  auto calculateErr = [](float hitOn, float trPrj)->double{
	    return std::sqrt( std::abs( hitOn) / trPrj ) *
	  		   std::sqrt( 1. - std::abs( hitOn) / trPrj ) /
	  		   std::sqrt( trPrj );
	  };
	  for ( int ib=0; ib!=nb; ib++ ) {
	     float n_Ly_hitOn = hist_MuctpiThr0->GetBinContent(ib+1) ;  
	     float n_Ly_TrPrj = hist_METracks  ->GetBinContent(ib+1) ;
	     
	     
	     if ( n_Ly_TrPrj >0 ) {
	       //MuctpiThr0
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff0->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff0->SetBinError  (ib+1, Ly_effErr) ;
	    
	       //MuctpiThr1
	       n_Ly_hitOn = hist_MuctpiThr1->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff1->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff1->SetBinError  (ib+1, Ly_effErr) ;
	       //MuctpiThr2
	       n_Ly_hitOn = hist_MuctpiThr2->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff2->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff2->SetBinError  (ib+1, Ly_effErr) ;
	       //MuctpiThr3
	       n_Ly_hitOn = hist_MuctpiThr3->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff3->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff3->SetBinError  (ib+1, Ly_effErr) ;
	       //MuctpiThr4
	       n_Ly_hitOn = hist_MuctpiThr4->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff4->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff4->SetBinError  (ib+1, Ly_effErr) ;
	       //MuctpiThr5
	       n_Ly_hitOn = hist_MuctpiThr5->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_MuctpiThr_eff5->SetBinContent(ib+1, Ly_eff)    ;
	       hist_MuctpiThr_eff5->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr0
	       n_Ly_hitOn = hist_PadThr0->GetBinContent(ib+1) ;
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff0->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff0->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr1
	       n_Ly_hitOn = hist_PadThr1->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff1->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff1->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr2
	       n_Ly_hitOn = hist_PadThr2->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff2->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff2->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr3
	       n_Ly_hitOn = hist_PadThr3->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff3->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff3->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr4
	       n_Ly_hitOn = hist_PadThr4->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff4->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff4->SetBinError  (ib+1, Ly_effErr) ;
	       //PadThr5
	       n_Ly_hitOn = hist_PadThr5->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PadThr_eff5->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PadThr_eff5->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr0
	       n_Ly_hitOn = hist_PhiEtaCoinThr0->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff0->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff0->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr1
	       n_Ly_hitOn = hist_PhiEtaCoinThr1->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff1->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff1->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr2
	       n_Ly_hitOn = hist_PhiEtaCoinThr2->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff2->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff2->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr3
	       n_Ly_hitOn = hist_PhiEtaCoinThr3->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff3->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff3->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr4
	       n_Ly_hitOn = hist_PhiEtaCoinThr4->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff4->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff4->SetBinError  (ib+1, Ly_effErr) ;
	       //PhiEtaCoinThr5
	       n_Ly_hitOn = hist_PhiEtaCoinThr5->GetBinContent(ib+1) ;  
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = calculateErr(n_Ly_hitOn, n_Ly_TrPrj) ;
	       hist_PhiEtaCoinThr_eff5->SetBinContent(ib+1, Ly_eff)    ;
	       hist_PhiEtaCoinThr_eff5->SetBinError  (ib+1, Ly_effErr) ;
	     }
	     
	     
	            
	  }
	
	  // write out histogram
	  TDirectory* dir = f->GetDirectory( dir_trigger_track.c_str() ) ;  
	  if ( dir != 0 ) {
	    dir->cd() ;
	    hist_MuctpiThr_eff0->Write("",TObject::kOverwrite);
	    hist_MuctpiThr_eff1->Write("",TObject::kOverwrite);
	    hist_MuctpiThr_eff2->Write("",TObject::kOverwrite);
	    hist_MuctpiThr_eff3->Write("",TObject::kOverwrite);
	    hist_MuctpiThr_eff4->Write("",TObject::kOverwrite);
	    hist_MuctpiThr_eff5->Write("",TObject::kOverwrite);
	    hist_PadThr_eff0->Write("",TObject::kOverwrite);
	    hist_PadThr_eff1->Write("",TObject::kOverwrite);
	    hist_PadThr_eff2->Write("",TObject::kOverwrite);
	    hist_PadThr_eff3->Write("",TObject::kOverwrite);
	    hist_PadThr_eff4->Write("",TObject::kOverwrite);
	    hist_PadThr_eff5->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff0->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff1->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff2->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff3->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff4->Write("",TObject::kOverwrite);
	    hist_PhiEtaCoinThr_eff5->Write("",TObject::kOverwrite);
	  }
	}

	
	// layer efficiency
	std::string LyHit_name, LyPrj_name, LyEff_name ;
	LyHit_name = dir_glob_track   + "Hit_on_Track_Layer"       ; 
	LyPrj_name = dir_glob_track   + "Track_Projected_on_Layer" ;
	LyEff_name = dir_glob_track   + "Layer_Efficiency"         ;  
	
	if ( RPCCheckHistogram(f,LyHit_name.c_str()) && RPCCheckHistogram(f,LyPrj_name.c_str()) && RPCCheckHistogram(f,LyEff_name.c_str()) ) {
	  TH1I* hist_LyHit = (TH1I*)(f->Get( LyHit_name.c_str()));
	  TH1I* hist_LyPrj = (TH1I*)(f->Get( LyPrj_name.c_str()));
	  TH1F* hist_LyEff = (TH1F*)(f->Get( LyEff_name.c_str()));
	
	  int nb = hist_LyEff->GetNbinsX() ;
	  double  Ly_eff, Ly_effErr ;
	  for ( int ib=0; ib!=nb; ib++ ) {
	     float n_Ly_hitOn = hist_LyHit->GetBinContent(ib+1) ;  
	     float n_Ly_TrPrj = hist_LyPrj->GetBinContent(ib+1) ;
	     
	     if ( n_Ly_TrPrj >0 ) {
	       Ly_eff    = float(n_Ly_hitOn)/float(n_Ly_TrPrj);
	       
	       Ly_effErr = sqrt( fabs( n_Ly_hitOn) / n_Ly_TrPrj ) *
	  		   sqrt( 1. - fabs( n_Ly_hitOn) / n_Ly_TrPrj ) /
	  		   sqrt( n_Ly_TrPrj ) ;
	       hist_LyEff->SetBinContent(ib+1, Ly_eff)    ;
	       hist_LyEff->SetBinError  (ib+1, Ly_effErr) ;
	     }       
	  }
	
	  // write out histogram
	  TDirectory* dir = f->GetDirectory( dir_glob_track.c_str() ) ;  
	  if ( dir != 0 ) {
	    dir->cd() ;
	    //hist_LyEff->Write();
	    hist_LyEff->Write("",TObject::kOverwrite);
	  }
	}
	
	
	// layer efficiency Side A
	std::string LyHit_SideA_name, LyPrj_SideA_name, LyEff_SideA_name ;
	LyHit_SideA_name = dir_sideA_track   + "Layer_HitOnTrack_sideA"; 
	LyPrj_SideA_name = dir_sideA_track   + "Layer_TrackProj_sideA" ;
	LyEff_SideA_name = dir_sideA_track   + "Layer_Efficiency_sideA";  
	
	if ( RPCCheckHistogram(f,LyHit_SideA_name.c_str()) && RPCCheckHistogram(f,LyPrj_SideA_name.c_str()) && RPCCheckHistogram(f,LyEff_SideA_name.c_str()) ) {
	  TH1I* hist_LyHit_SideA = (TH1I*)(f->Get( LyHit_SideA_name.c_str()));
	  TH1I* hist_LyPrj_SideA = (TH1I*)(f->Get( LyPrj_SideA_name.c_str()));
	  TH1F* hist_LyEff_SideA = (TH1F*)(f->Get( LyEff_SideA_name.c_str()));
	
	  int nb = hist_LyEff_SideA->GetNbinsX() ;
	  double  Ly_eff_SideA, Ly_effErr_SideA ;
	  for ( int ib=0; ib!=nb; ib++ ) {
	     float n_Ly_hitOn_SideA = hist_LyHit_SideA->GetBinContent(ib+1) ;  
	     float n_Ly_TrPrj_SideA = hist_LyPrj_SideA->GetBinContent(ib+1) ;
	     
	     if ( n_Ly_TrPrj_SideA >0 ) {
	       Ly_eff_SideA    = float(n_Ly_hitOn_SideA)/float(n_Ly_TrPrj_SideA);
	       
	       Ly_effErr_SideA = sqrt( fabs( n_Ly_hitOn_SideA) / n_Ly_TrPrj_SideA ) *
	  		   sqrt( 1. - fabs( n_Ly_hitOn_SideA) / n_Ly_TrPrj_SideA ) /
	  		   sqrt( n_Ly_TrPrj_SideA ) ;
	       hist_LyEff_SideA->SetBinContent(ib+1, Ly_eff_SideA)    ;
	       hist_LyEff_SideA->SetBinError  (ib+1, Ly_effErr_SideA) ;
	     }       
	  }
	
	  // write out histogram
	  TDirectory* dir = f->GetDirectory( dir_sideA_track.c_str() ) ;  
	  if ( dir != 0 ) {
	    dir->cd() ;
	    //hist_LyEff->Write();
	    hist_LyEff_SideA->Write("",TObject::kOverwrite);
	  }
	}
		
	// layer efficiency Side C
	std::string LyHit_SideC_name, LyPrj_SideC_name, LyEff_SideC_name ;
	LyHit_SideC_name = dir_sideC_track   + "Layer_HitOnTrack_sideC"; 
	LyPrj_SideC_name = dir_sideC_track   + "Layer_TrackProj_sideC" ;
	LyEff_SideC_name = dir_sideC_track   + "Layer_Efficiency_sideC";  
	
	if ( RPCCheckHistogram(f,LyHit_SideC_name.c_str()) && RPCCheckHistogram(f,LyPrj_SideC_name.c_str()) && RPCCheckHistogram(f,LyEff_SideC_name.c_str()) ) {
	  TH1I* hist_LyHit_SideC = (TH1I*)(f->Get( LyHit_SideC_name.c_str()));
	  TH1I* hist_LyPrj_SideC = (TH1I*)(f->Get( LyPrj_SideC_name.c_str()));
	  TH1F* hist_LyEff_SideC = (TH1F*)(f->Get( LyEff_SideC_name.c_str()));
	
	  int nb = hist_LyEff_SideC->GetNbinsX() ;
	  double  Ly_eff_SideC, Ly_effErr_SideC ;
	  for ( int ib=0; ib!=nb; ib++ ) {
	     float n_Ly_hitOn_SideC = hist_LyHit_SideC->GetBinContent(ib+1) ;  
	     float n_Ly_TrPrj_SideC = hist_LyPrj_SideC->GetBinContent(ib+1) ;
	     
	     if ( n_Ly_TrPrj_SideC >0 ) {
	       Ly_eff_SideC    = float(n_Ly_hitOn_SideC)/float(n_Ly_TrPrj_SideC);
	       
	       Ly_effErr_SideC = sqrt( fabs( n_Ly_hitOn_SideC) / n_Ly_TrPrj_SideC ) *
	  		   sqrt( 1. - fabs( n_Ly_hitOn_SideC) / n_Ly_TrPrj_SideC ) /
	  		   sqrt( n_Ly_TrPrj_SideC ) ;
	       hist_LyEff_SideC->SetBinContent(ib+1, Ly_eff_SideC)    ;
	       hist_LyEff_SideC->SetBinError  (ib+1, Ly_effErr_SideC) ;
	     }       
	  }	
	  // write out histogram
	  TDirectory* dir = f->GetDirectory( dir_sideC_track.c_str() ) ;  
	  if ( dir != 0 ) {
	    dir->cd() ;
	    //hist_LyEff->Write();
	    hist_LyEff_SideC->Write("",TObject::kOverwrite);
	  }
	}

			
	
	int rpc_eventstotal = 0;
	if ( RPCCheckHistogram(f,(dir_ov_raw + "Number_of_RPC_hits_per_event").c_str() ) ) {
	  TH1I* rpc_hits = (TH1I*)(f->Get((dir_ov_raw + "Number_of_RPC_hits_per_event").c_str() ) ) ;
	  rpc_eventstotal = int(rpc_hits->GetEntries() );
	}
	
	
	// distribution plot all atlas 
	std::string AverageEff_C_name		= dir_sideC_track   + "Efficiency_Distribution_sideC" 		;
	std::string AverageEff_A_name		= dir_sideA_track   + "Efficiency_Distribution_sideA"		;
	std::string AverageGapEff_C_name	= dir_sideC_track   + "GapEfficiency_Distribution_sideC"	;
	std::string AverageGapEff_A_name	= dir_sideA_track   + "GapEfficiency_Distribution_sideA"	;
	std::string AverageNoiseCorr_C_name	= dir_sideC_track   + "NoiseCorr_Distribution_sideC"	   	;
	std::string AverageNoiseCorr_A_name	= dir_sideA_track   + "NoiseCorr_Distribution_sideA"	   	;
	std::string AverageNoiseTot_C_name	= dir_sideC_track   + "NoiseTot_Distribution_sideC"		;
	std::string AverageNoiseTot_A_name	= dir_sideA_track   + "NoiseTot_Distribution_sideA"		;
	std::string AverageCS_C_name		= dir_sideC_track   + "CS_Distribution_sideC"			;
	std::string AverageCS_A_name		= dir_sideA_track   + "CS_Distribution_sideA"			;
	std::string AverageRes_CS1_C_name	= dir_sideC_track   + "Res_CS1_Distribution_sideC"        	;
	std::string AverageRes_CS1_A_name	= dir_sideA_track   + "Res_CS1_Distribution_sideA"        	;
	std::string AverageRes_CS2_C_name	= dir_sideC_track   + "Res_CS2_Distribution_sideC"        	;
	std::string AverageRes_CS2_A_name	= dir_sideA_track   + "Res_CS2_Distribution_sideA"        	;
	std::string AverageRes_CSmore2_C_name	= dir_sideC_track   + "Res_CSmore2_Distribution_sideC"	  	;
	std::string AverageRes_CSmore2_A_name	= dir_sideA_track   + "Res_CSmore2_Distribution_sideA"	  	;
	std::string AverageRes_CS1rms_C_name	= dir_sideC_track   + "Res_CS1rms_Distribution_sideC"	  	;
	std::string AverageRes_CS1rms_A_name	= dir_sideA_track   + "Res_CS1rms_Distribution_sideA"	  	;
	std::string AverageRes_CS2rms_C_name	= dir_sideC_track   + "Res_CS2rms_Distribution_sideC"	  	;
	std::string AverageRes_CS2rms_A_name	= dir_sideA_track   + "Res_CS2rms_Distribution_sideA"	  	;
	std::string AverageRes_CSmore2rms_C_name= dir_sideC_track   + "Res_CSmore2rms_Distribution_sideC"       ;
	std::string AverageRes_CSmore2rms_A_name= dir_sideA_track   + "Res_CSmore2rms_Distribution_sideA"       ;
	std::string AverageOccupancy_C_name	= dir_sideC_track   + "Occupancy_Distribution_sideC"	 	;
	std::string AverageOccupancy_A_name	= dir_sideA_track   + "Occupancy_Distribution_sideA"	 	;
	std::string AverageTime_C_name		= dir_sideC_track   + "Time_Distribution_sideC"	    		;
	std::string AverageTime_A_name		= dir_sideA_track   + "Time_Distribution_sideA"	    		;
	
	
	auto initialiseHisto = [f](const std::string & name)->TH1F* {
	  TH1F* result =  (TH1F*) (f->Get(name.c_str()));
	  return result;
	};
	
	TH1F* h_AverageEff_C		        = initialiseHisto(AverageEff_C_name);
	TH1F* h_AverageEff_A		        =initialiseHisto(AverageEff_A_name);
	TH1F* h_AverageGapEff_C		        =initialiseHisto(AverageGapEff_C_name);
	TH1F* h_AverageGapEff_A		        =initialiseHisto(AverageGapEff_A_name);
	TH1F* h_AverageNoiseCorr_C	        =initialiseHisto(AverageNoiseCorr_C_name);
	TH1F* h_AverageNoiseCorr_A	        = initialiseHisto(AverageNoiseCorr_A_name);
	TH1F* h_AverageNoiseTot_C 	        = initialiseHisto(AverageNoiseTot_C_name);
	TH1F* h_AverageNoiseTot_A	        = initialiseHisto(AverageNoiseTot_A_name);
	TH1F* h_AverageCS_C		        = initialiseHisto(AverageCS_C_name);
	TH1F* h_AverageCS_A		        = initialiseHisto(AverageCS_A_name);
	TH1F* h_AverageRes_CS1_C  	        = initialiseHisto(AverageRes_CS1_C_name);
	TH1F* h_AverageRes_CS1_A  	        = initialiseHisto(AverageRes_CS1_A_name);
	TH1F* h_AverageRes_CS2_C  	        = initialiseHisto(AverageRes_CS2_C_name);
	TH1F* h_AverageRes_CS2_A  	        = initialiseHisto(AverageRes_CS2_A_name);
	TH1F* h_AverageRes_CSmore2_C	        = initialiseHisto(AverageRes_CSmore2_C_name);
	TH1F* h_AverageRes_CSmore2_A	        = initialiseHisto(AverageRes_CSmore2_A_name);
	TH1F* h_AverageRes_CS1rms_C	        = initialiseHisto(AverageRes_CS1rms_C_name);
	TH1F* h_AverageRes_CS1rms_A	        = initialiseHisto(AverageRes_CS1rms_A_name);
	TH1F* h_AverageRes_CS2rms_C	        = initialiseHisto(AverageRes_CS2rms_C_name);
	TH1F* h_AverageRes_CS2rms_A	        = initialiseHisto(AverageRes_CS2rms_A_name);
	TH1F* h_AverageRes_CSmore2rms_C	        = initialiseHisto(AverageRes_CSmore2rms_C_name);
	TH1F* h_AverageRes_CSmore2rms_A	        = initialiseHisto(AverageRes_CSmore2rms_A_name);
	TH1F* h_AverageOccupancy_C	        = initialiseHisto(AverageOccupancy_C_name);
	TH1F* h_AverageOccupancy_A	        = initialiseHisto(AverageOccupancy_A_name);
	TH1F* h_AverageTime_C		        = initialiseHisto(AverageTime_C_name);
	TH1F* h_AverageTime_A		        = initialiseHisto(AverageTime_A_name);
	
	auto resetIfValid = [](TH1F * pH)->void{
	  if (pH) pH->Reset();
	  return;
	};
	
	
	
	resetIfValid(h_AverageEff_C);
	resetIfValid(h_AverageEff_A	       );
	resetIfValid(h_AverageGapEff_C	       );
	resetIfValid(h_AverageGapEff_A	       );
	resetIfValid(h_AverageNoiseCorr_C      );
	resetIfValid(h_AverageNoiseCorr_A      );
	resetIfValid(h_AverageNoiseTot_C       );
	resetIfValid(h_AverageNoiseTot_A       );
	resetIfValid(h_AverageCS_C	       );
	resetIfValid(h_AverageCS_A	       );
	resetIfValid(h_AverageRes_CS1_C	       );
	resetIfValid(h_AverageRes_CS1_A	       );
	resetIfValid(h_AverageRes_CS2_C	       );
	resetIfValid(h_AverageRes_CS2_A	       );
	resetIfValid(h_AverageRes_CSmore2_C    );
	resetIfValid(h_AverageRes_CSmore2_A    );
	resetIfValid(h_AverageRes_CS1rms_C     );
	resetIfValid(h_AverageRes_CS1rms_A     );
	resetIfValid(h_AverageRes_CS2rms_C     );
	resetIfValid(h_AverageRes_CS2rms_A     );
	resetIfValid(h_AverageRes_CSmore2rms_C );
	resetIfValid(h_AverageRes_CSmore2rms_A );
	resetIfValid(h_AverageOccupancy_C      );
	resetIfValid(h_AverageOccupancy_A      );
	resetIfValid(h_AverageTime_C	       );
	resetIfValid(h_AverageTime_A	       );
	 
	// summary plots
	int countpanelindb = 0 ;
	int countpaneleff0 = 0 ;
	int countpaneltrack0 = 0 ;
	for (int i_sec=0; i_sec!=15*1+1; i_sec++) {
	  char sector_char[100]   ;
	  std::string sector_name ;
          sprintf(sector_char,"_Sector%.2d",i_sec+1) ;  // sector number with 2 digits
          sector_name = sector_char ;
	  std::cout << " RPC sector_name processing  "<< sector_name<<std::endl;
	  
	  std::string TrackProj_name		  	= dir_sum_track + "SummaryTrackProj"  		  	+ sector_name ;
	  std::string HitOnTrack_name		  	= dir_sum_track + "SummaryHitOnTrack" 		  	+ sector_name ;
	  std::string HitOnTrackCross_name	  	= dir_sum_track + "SummaryHitOnTrack_withCrossStrip"    + sector_name ;
	  std::string Eff_name			  	= dir_sum_track + "SummaryEfficiency" 		 	+ sector_name ;
	  std::string GapEff_name		  	= dir_sum_track + "SummaryGapEfficiency"		+ sector_name ;
	  std::string NoiseCorr_name		  	= dir_sum_track + "SummaryNoiseCorr"  		  	+ sector_name ;
	  std::string NoiseCorr_s_name		  	= dir_sum_track + "SummaryNoiseCorr_NotNorm"  		+ sector_name ;
	  std::string NoiseTot_name  		  	= dir_sum_track + "SummaryNoiseTot"			+ sector_name ;
	  std::string NoiseTot_s_name  		  	= dir_sum_track + "SummaryNoiseTot_NotNorm"		+ sector_name ;
	  std::string Res_CS1_name            	  	= dir_sum_track + "SummaryRes_CS1"			+ sector_name ;
	  std::string Res_CS1_s_name            	= dir_sum_track + "SummaryRes_CS1_NotNorm"		+ sector_name ;
	  std::string Res_CS1_square_name      	  	= dir_sum_track + "SummaryRes_CS1_square"		+ sector_name ;
	  std::string Res_CS1_entries_name     	  	= dir_sum_track + "SummaryRes_CS1_entries"		+ sector_name ;
	  std::string Res_CS2_name             	  	= dir_sum_track + "SummaryRes_CS2"			+ sector_name ;
	  std::string Res_CS2_s_name             	= dir_sum_track + "SummaryRes_CS2_NotNorm"		+ sector_name ;
	  std::string Res_CS2_square_name      	  	= dir_sum_track + "SummaryRes_CS2_square"		+ sector_name ;
	  std::string Res_CS2_entries_name     	  	= dir_sum_track + "SummaryRes_CS2_entries"		+ sector_name ;
	  std::string Res_CSmore2_name         	  	= dir_sum_track + "SummaryRes_CSmore2"		  	+ sector_name ;
	  std::string Res_CSmore2_s_name       	  	= dir_sum_track + "SummaryRes_CSmore2_NotNorm"	  	+ sector_name ;
	  std::string Res_CSmore2_square_name  	  	= dir_sum_track + "SummaryRes_CSmore2_square" 	  	+ sector_name ;
	  std::string Res_CSmore2_entries_name 	  	= dir_sum_track + "SummaryRes_CSmore2_entries"	  	+ sector_name ;
	  std::string CS_name			  	= dir_sum_track + "SummaryCS" 			  	+ sector_name ;
	  std::string CS_s_name			  	= dir_sum_track + "SummaryCS_NotNorm"		  	+ sector_name ;
	  std::string CS_square_name 		  	= dir_sum_track + "SummaryCS_square"  		  	+ sector_name ;
	  std::string CS_entries_name		  	= dir_sum_track + "SummaryCS_entries" 		  	+ sector_name ;
	  std::string CS1_entries_name		  	= dir_sum_track + "SummaryCS1_entries" 		  	+ sector_name ;
	  std::string CS2_entries_name		  	= dir_sum_track + "SummaryCS2_entries" 		  	+ sector_name ;
	  std::string Time_name			  	= dir_sum_track + "SummaryTime"			  	+ sector_name ;
	  std::string Time_s_name		  	= dir_sum_track + "SummaryTime_NotNorm"			+ sector_name ;
	  std::string Time_square_name		  	= dir_sum_track + "SummaryTime_square"		  	+ sector_name ;
	  std::string Occupancy_name   		  	= dir_sum_track + "SummaryOccupancy"  		  	+ sector_name ;
	  std::string Occupancy_s_name   		= dir_sum_track + "SummaryOccupancy_NotNorm"	  	+ sector_name ;
	  std::string PanelId_name                      = dir_sum_track + "SummaryPanelID"	                + sector_name ;
	  
	  // distribution plot per sector 
	  std::string EffSecDist_name			= dir_sum_track + "SummaryEffDistriPerSector"		+ sector_name ;		
	  std::string GapEffSecDist_name		= dir_sum_track + "SummaryGapEffDistriPerSector"	+ sector_name ;
	  std::string NoiseCorrSecDist_name		= dir_sum_track + "SummaryNoiseCorrDistriPerSector"	+ sector_name ;
	  std::string NoiseTotSecDist_name		= dir_sum_track + "SummaryNoiseTotDistriPerSector"	+ sector_name ;
	  std::string CSSecDist_name			= dir_sum_track + "SummaryCSDistriPerSector"		+ sector_name ;
	  std::string Res_CS1SecDist_name		= dir_sum_track + "SummaryRes_CS1DistriPerSector"	+ sector_name ;
	  std::string Res_CS2SecDist_name		= dir_sum_track + "SummaryRes_CS2DistriPerSector"	+ sector_name ;
	  std::string Res_CSmore2SecDist_name		= dir_sum_track + "SummaryRes_CSmore2DistriPerSector"	+ sector_name ;
	  std::string Res_CS1_rmsSecDist_name		= dir_sum_track + "SummaryRes_CS1rmsDistriPerSector"	+ sector_name ;
	  std::string Res_CS2_rmsSecDist_name		= dir_sum_track + "SummaryRes_CS2rmsDistriPerSector"	+ sector_name ;
	  std::string Res_CSmore2_rmsSecDist_name	= dir_sum_track + "SummaryRes_CSmore2rmsDistriPerSector"+ sector_name ;
	  std::string TimeSecDist_name			= dir_sum_track + "SummaryTimeDistriPerSector"		+ sector_name ;
	  std::string OccupancySecDist_name		= dir_sum_track + "SummaryOccupancyDistriPerSector"	+ sector_name ;
	  //
	  auto initialiseHisto = [f](const std::string & name)->TH1F* {
	    TH1F* result =  (TH1F*) (f->Get(name.c_str()));
	    return result;
	  };
	  TH1F* h_TrackProj	      	  = initialiseHisto(TrackProj_name);
	  TH1F* h_HitOnTrack          	  = initialiseHisto(HitOnTrack_name);
	  TH1F* h_HitOnTrackCross     	  = initialiseHisto(HitOnTrackCross_name);
	  TH1F* h_Eff	      	      	  = initialiseHisto(Eff_name);
	  TH1F* h_GapEff	      	  = initialiseHisto(GapEff_name);
	  TH1F* h_NoiseCorr           	  = initialiseHisto(NoiseCorr_name);
	  TH1F* h_NoiseCorr_s          	  = initialiseHisto(NoiseCorr_s_name);
	  TH1F* h_NoiseTot            	  = initialiseHisto(NoiseTot_name);
	  TH1F* h_NoiseTot_s           	  = initialiseHisto(NoiseTot_s_name);
	  TH1F* h_Res_CS1                 = initialiseHisto(Res_CS1_name);   
	  TH1F* h_Res_CS1_s               = initialiseHisto(Res_CS1_s_name);
	  TH1F* h_Res_CS1_square          = initialiseHisto(Res_CS1_square_name);   
	  TH1F* h_Res_CS1_entries     	  = initialiseHisto(Res_CS1_entries_name);  
	  TH1F* h_Res_CS2  	      	  = initialiseHisto(Res_CS2_name); 
	  TH1F* h_Res_CS2_s  	      	  = initialiseHisto(Res_CS2_s_name); 
	  TH1F* h_Res_CS2_square          = initialiseHisto(Res_CS2_square_name);   
	  TH1F* h_Res_CS2_entries     	  = initialiseHisto(Res_CS2_entries_name);  
	  TH1F* h_Res_CSmore2	      	  = initialiseHisto(Res_CSmore2_name);
	  TH1F* h_Res_CSmore2_s	      	  = initialiseHisto(Res_CSmore2_s_name); 
	  TH1F* h_Res_CSmore2_square  	  = initialiseHisto(Res_CSmore2_square_name);
	  TH1F* h_Res_CSmore2_entries 	  = initialiseHisto(Res_CSmore2_entries_name);
	  TH1F* h_CS                  	  = initialiseHisto(CS_name);
	  TH1F* h_CS_s                 	  = initialiseHisto(CS_s_name);
	  TH1F* h_CS_square           	  = initialiseHisto(CS_square_name);
	  TH1F* h_CS_entries          	  = initialiseHisto(CS_square_name);
	  TH1F* h_CS1_entries          	  = initialiseHisto(CS1_entries_name);
	  TH1F* h_CS2_entries          	  = initialiseHisto(CS2_entries_name);
	  TH1F* h_Time                	  = initialiseHisto(Time_name);
	  TH1F* h_Time_s               	  = initialiseHisto(Time_s_name);
	  TH1F* h_Time_square         	  = initialiseHisto(Time_square_name);
	  TH1F* h_Occupancy           	  = initialiseHisto(Occupancy_name);
	  TH1F* h_Occupancy_s          	  = initialiseHisto(Occupancy_s_name);
	  TH1F* h_PanelId		  = initialiseHisto(PanelId_name);
	  
	  TH1F* h_EffSecDist	          = initialiseHisto(EffSecDist_name);
	  TH1F* h_GapEffSecDist	          = initialiseHisto(GapEffSecDist_name);
	  TH1F* h_NoiseCorrSecDist        = initialiseHisto(NoiseCorrSecDist_name);  
	  TH1F* h_NoiseTotSecDist         = initialiseHisto(NoiseTotSecDist_name); 
	  TH1F* h_CSSecDist	          = initialiseHisto(CSSecDist_name);
	  TH1F* h_Res_CS1SecDist	  = initialiseHisto(Res_CS1SecDist_name);    
	  TH1F* h_Res_CS2SecDist	  = initialiseHisto(Res_CS2SecDist_name);    
	  TH1F* h_Res_CSmore2SecDist      = initialiseHisto(Res_CSmore2SecDist_name);
	  TH1F* h_Res_CS1_rmsSecDist      = initialiseHisto(Res_CS1_rmsSecDist_name); 
	  TH1F* h_Res_CS2_rmsSecDist      = initialiseHisto(Res_CS2_rmsSecDist_name);
	  TH1F* h_Res_CSmore2_rmsSecDist  = initialiseHisto(Res_CSmore2_rmsSecDist_name);
	  TH1F* h_TimeSecDist	          = initialiseHisto(TimeSecDist_name); 
	  TH1F* h_OccupancySecDist        = initialiseHisto(OccupancySecDist_name);
	 
	  auto resetIfValid = [](TH1F * pH)->void{
	    if (pH) pH->Reset();
	    return;
	  };
	  resetIfValid( h_EffSecDist	        );
	  resetIfValid( h_GapEffSecDist          );
	  resetIfValid( h_NoiseCorrSecDist       );
	  resetIfValid( h_NoiseTotSecDist        );
	  resetIfValid( h_CSSecDist	        );
	  resetIfValid( h_Res_CS1SecDist         );
	  resetIfValid( h_Res_CS2SecDist         );
	  resetIfValid( h_Res_CSmore2SecDist     );
	  resetIfValid( h_Res_CS1_rmsSecDist     );
	  resetIfValid( h_Res_CS2_rmsSecDist     );
	  resetIfValid( h_Res_CSmore2_rmsSecDist );
	  resetIfValid( h_TimeSecDist	        );
	  resetIfValid( h_OccupancySecDist       );
	  resetIfValid( h_Eff    	        );
	  
	  // efficiency
	  if ( h_TrackProj && h_HitOnTrack && h_Eff ) {
	    for ( int ib = 0; ib != h_TrackProj->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; } } 
	      
	      n_hit_f = h_HitOnTrack -> GetBinContent(ib+1) ;
	      n_tr_p  = h_TrackProj  -> GetBinContent(ib+1) ;
	      
	     //if ( n_tr_p>50 ) {    
	      if ( n_tr_p!=0) { panel_eff = n_hit_f/n_tr_p ; }
	      else { panel_eff=0. ; }
	      if ( n_tr_p!=0 ) {
	        panel_err_eff = sqrt( fabs(n_hit_f)/n_tr_p ) *
	                        sqrt( 1. - fabs(n_hit_f)/n_tr_p ) /
		                sqrt( n_tr_p ) ;
	      }
	      else { panel_err_eff=0. ; } 
	      h_Eff -> SetBinContent(ib+1, panel_eff) 	        ;
	      h_Eff -> SetBinError  (ib+1, panel_err_eff)	;
	      if ( h_EffSecDist ) h_EffSecDist->Fill(panel_eff) ;
	      if ( ib>( h_TrackProj->GetNbinsX()/2 ) ) { fillIfValid(h_AverageEff_A, panel_eff); } 
	      else                                     { fillIfValid(h_AverageEff_C, panel_eff); }
	    //}
	  }
	    // write out histogram
	    TDirectory* dir = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dir != 0 ) {
	      dir->cd() ;
	      h_Eff -> Write("",TObject::kOverwrite);
	      writeIfValid(h_EffSecDist);
	    }
	  }
	  
	  // gap efficiency
	  if ( h_TrackProj && h_HitOnTrack && h_HitOnTrackCross && h_GapEff ) {
	    h_GapEff->Reset();
	    nEta=0; nPhi=0; nEtaPhi=0;
	    int sign=1;
	    int bmin=0;
	    bmin = int( h_GapEff->GetXaxis()->GetXmin() ) ;
	    for ( int ib = 0; ib !=h_TrackProj->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; } }
	      if ( (bmin + ib )%2 != 0 ) continue; // Phi panel
	      sign = std::copysign(1, (bmin + ib ));
	      
	      nEta    = h_HitOnTrack     ->GetBinContent( ib+1	      ) ;
	      nPhi    = h_HitOnTrack     ->GetBinContent( ib+1 + sign ) ;
	      nEtaPhi = h_HitOnTrackCross->GetBinContent( ib+1	      ) ;
	      n_tr_p  = h_TrackProj      ->GetBinContent( ib+1        ) ;
	      
	      if ( n_tr_p>0 ) { 
	        gapEff    = (nEta+nPhi-nEtaPhi) / n_tr_p  ; 
		      gapErrEff = sqrt( fabs( nEta+nPhi-nEtaPhi)/n_tr_p ) *
	                    sqrt( 1. - fabs( nEta+nPhi-nEtaPhi)/n_tr_p ) /
		                  sqrt( n_tr_p ) ;
        } else  { 
	        gapEff    = 0.; 
	        gapErrEff = 0.;
        }
	      h_GapEff -> SetBinContent(ib+1, gapEff    );
	      h_GapEff -> SetBinError  (ib+1, gapErrEff );
	      fillIfValid(h_GapEffSecDist, gapEff);
	      if ( ib>( h_TrackProj->GetNbinsX()/2 )  ) { fillIfValid(h_AverageGapEff_A, gapEff); }
	      else                                      { fillIfValid(h_AverageGapEff_C,  gapEff); }
	    }
	    TDirectory* dir = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dir != 0 ) {
	      dir->cd() ;
	      h_GapEff -> Write("",TObject::kOverwrite) ;
	      if ( h_GapEffSecDist ) h_GapEffSecDist-> Write("",TObject::kOverwrite) ;
	    }
	  }
	  
	  
	  //residuals CS = 1
	  res_mean=0; res2_mean=0; res_RMS=0; 
	  
	  if ( h_Res_CS1 && h_Res_CS1_s && h_Res_CS1_square && h_Res_CS1_entries ) {
	    for ( int ib = 0; ib != h_Res_CS1->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      if ( (h_Res_CS1_entries->GetBinContent(ib+1) )!=0 ) {
	        res_mean  = (h_Res_CS1_s->GetBinContent(ib+1) )/ (h_Res_CS1_entries->GetBinContent(ib+1) )       ;
	        res2_mean = (h_Res_CS1_square->GetBinContent(ib+1) )/ (h_Res_CS1_entries->GetBinContent(ib+1) );
	        res_RMS   = sqrt( (fabs( res2_mean-res_mean*res_mean ))/( h_Res_CS1_entries->GetBinContent(ib+1) ) ) ;  
	      
	        h_Res_CS1->SetBinContent(ib+1, res_mean);
	        h_Res_CS1->SetBinError(  ib+1, res_RMS );
		      fillIfValid( h_Res_CS1SecDist, res_mean);
		      fillIfValid( h_Res_CS1_rmsSecDist, res_RMS );
          if ( ib>( h_Res_CS1->GetNbinsX()/2 ) ) {
            fillIfValid( h_AverageRes_CS1_A, res_mean    );
            fillIfValid( h_AverageRes_CS1rms_A , res_RMS);
          } else {
            fillIfValid( h_AverageRes_CS1_C,  res_mean    );
            fillIfValid( h_AverageRes_CS1rms_C, res_RMS  );
          }
	      }
	    } // end for bins
	    TDirectory* dirRes1 = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirRes1 != 0 ) {
	      dirRes1->cd() ;
	      h_Res_CS1->Write("",TObject::kOverwrite);
        writeIfValid( h_Res_CS1SecDist );
        writeIfValid(h_Res_CS1_rmsSecDist);
	    }
	  }
	  
	  //residuals CS = 2
	  res_mean=0; res2_mean=0; res_RMS=0; 
	  if ( h_Res_CS2 && h_Res_CS2_s && h_Res_CS2_square && h_Res_CS2_entries ) {
	  
	    for ( int ib = 0; ib != h_Res_CS2->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      if ( (h_Res_CS2_entries->GetBinContent(ib+1) )!=0 ) {
	        res_mean  = (h_Res_CS2_s->GetBinContent(ib+1) )/ (h_Res_CS2_entries->GetBinContent(ib+1) )       ;
	        res2_mean = (h_Res_CS2_square->GetBinContent(ib+1) )/ (h_Res_CS2_entries->GetBinContent(ib+1) );
	        res_RMS   = sqrt( fabs ( ( res2_mean-res_mean*res_mean )/( h_Res_CS2_entries->GetBinContent(ib+1) ) ) ) ;  
	      
	        h_Res_CS2->SetBinContent(ib+1, res_mean);
	        h_Res_CS2->SetBinError(  ib+1, res_RMS );
	        fillIfValid(h_Res_CS2SecDist, res_mean);
	        fillIfValid(h_Res_CS2_rmsSecDist, res_RMS);
          if ( ib >( h_Res_CS2->GetNbinsX()/2 ) ) {
            fillIfValid(h_AverageRes_CS2_A, res_mean);
            fillIfValid(h_AverageRes_CS2rms_A, res_RMS);
		      } else {
		        fillIfValid(h_AverageRes_CS2_C, res_mean);
		        fillIfValid(h_AverageRes_CS2rms_C, res_RMS);
		      }
	      }
	    }
	    TDirectory* dirRes2 = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirRes2 != 0 ) {
	      dirRes2->cd() ;
	      h_Res_CS2->Write("",TObject::kOverwrite);
	      writeIfValid( h_Res_CS2SecDist     );
	      writeIfValid ( h_Res_CS2_rmsSecDist );
	    }
	  }
	  
	  //residuals CS > 2
	  res_mean=0; res2_mean=0; res_RMS=0; 
	  if ( h_Res_CSmore2 && h_Res_CSmore2_s && h_Res_CSmore2_square && h_Res_CSmore2_entries ) {
	  
	    for ( int ib = 0; ib != h_Res_CSmore2->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      if ( (h_Res_CSmore2_entries->GetBinContent(ib+1) )!=0 ) {
	        res_mean  = (h_Res_CSmore2_s->GetBinContent(ib+1) )/ (h_Res_CSmore2_entries->GetBinContent(ib+1) )       ;
	        res2_mean = (h_Res_CSmore2_square->GetBinContent(ib+1) )/ (h_Res_CSmore2_entries->GetBinContent(ib+1) );
	        res_RMS   = sqrt( fabs( ( res2_mean-res_mean*res_mean )/( h_Res_CSmore2_entries->GetBinContent(ib+1) ) ) ) ;  
	      
	        h_Res_CSmore2->SetBinContent(ib+1, res_mean);
	        h_Res_CSmore2->SetBinError(  ib+1, res_RMS );
	        fillIfValid(h_Res_CSmore2SecDist, res_mean);
	        fillIfValid(h_Res_CSmore2_rmsSecDist, res_RMS);
		      if ( ib > (h_Res_CSmore2->GetNbinsX()/2 ) ) {
		        fillIfValid(h_AverageRes_CSmore2_A, res_mean);
		        fillIfValid(h_AverageRes_CSmore2rms_A, res_RMS);
		      } else {
		        fillIfValid(h_AverageRes_CSmore2_C, res_mean);
		        fillIfValid(h_AverageRes_CSmore2rms_C, res_RMS);
		      }
	      }
	    }
	    TDirectory* dirResp2 = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirResp2 != 0 ) {
	      dirResp2->cd() ;
	      h_Res_CSmore2->Write("",TObject::kOverwrite);
	      writeIfValid ( h_Res_CSmore2SecDist     );
	      writeIfValid ( h_Res_CSmore2_rmsSecDist );
	    }
	  }
	  
	  // occupancy
	  if ( (rpc_eventstotal>0) && h_Occupancy && h_Occupancy_s ) {
	    for ( int ib = 0; ib != h_Occupancy->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      panel_occ = h_Occupancy_s->GetBinContent(ib+1) ;
	      panel_occ = panel_occ/float(rpc_eventstotal) ;
	      
	      h_Occupancy->SetBinContent(ib+1, panel_occ       ) ;
	      h_Occupancy->SetBinError(  ib+1, sqrt(panel_occ) ) ;
	      
	      if (panel_occ>0 ) {
	        panel_occ = log10( panel_occ ) ;
              }
	      else {
	        panel_occ = -10 ;
	      }
	      fillIfValid ( h_OccupancySecDist, panel_occ );
	      if ( h_PanelId and (ib>( h_PanelId->GetNbinsX()/2)) ) { fillIfValid ( h_AverageOccupancy_A,  panel_occ); }
	      else 				    { fillIfValid ( h_AverageOccupancy_C, panel_occ); }
	    }
	    // write occupancy histograms
	    TDirectory* dirOcc = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirOcc !=0 ) {
	      dirOcc->cd();
	      h_Occupancy       ->Write("",TObject::kOverwrite);
	      h_OccupancySecDist->Write("",TObject::kOverwrite);
	    }
	  }
	  
	  
	  // CS
	  if ( h_CS && h_CS_s && h_CS_square && h_CS_entries ){
	    for ( int ib = 0; ib != h_CS->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      panelCS_entries = h_CS_entries->GetBinContent(ib+1);
	      panelCS	      = h_CS_s->GetBinContent(ib+1)      ;
	      panelCS2	      = h_CS_square->GetBinContent(ib+1) ;
	      if ( panelCS_entries !=0 ) {
	        panelCS_mean  = panelCS/panelCS_entries  ;
		panelCS2_mean = panelCS2/panelCS_entries ;
		panelCS_RMS   = sqrt( fabs( (panelCS2_mean - panelCS_mean*panelCS_mean ) / panelCS_entries ) ) ;
		
		h_CS->SetBinContent( ib+1, panelCS_mean ) ;
		h_CS->SetBinError  ( ib+1, panelCS_RMS  ) ;
		
		fillIfValid ( h_CSSecDist,panelCS_mean );
		if ( ib > ( h_CS->GetNbinsX()/2 ) ) { fillIfValid ( h_AverageCS_A, panelCS_mean ); }
		else                                { fillIfValid ( h_AverageCS_C,  panelCS_mean); }
	      }
	    }
	    // write CS histograms
	    TDirectory* dirCS = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirCS!=0 ) {
	      dirCS->cd();
	      h_CS->Write("",TObject::kOverwrite) ;
	      writeIfValid ( h_CSSecDist );
	    }
	  }
	  
	  // time
	  if ( h_Time && h_Time_s && h_Time_square && h_CS_entries ) {
	    for ( int ib = 0; ib != h_Time->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      if ( h_CS_entries->GetBinContent(ib+1) !=0 ) {
	        Time_mean  = (h_Time_s->GetBinContent(ib+1) )/ (h_CS_entries->GetBinContent(ib+1) )	           ;
	        Time2_mean = (h_Time_square->GetBinContent(ib+1) )/ (h_CS_entries->GetBinContent(ib+1) )	   ;
	        Time_RMS   = sqrt(fabs( ( Time2_mean-Time_mean*Time_mean )/( h_CS_entries->GetBinContent(ib+1) )) );
	      
	        h_Time->SetBinContent(ib+1, Time_mean);
	        h_Time->SetBinError(  ib+1, Time_RMS );
	      
	        fillIfValid ( h_TimeSecDist,  Time_mean);
		if ( ib>( h_Time->GetNbinsX()/2 )  ) { fillIfValid ( h_AverageTime_A, Time_mean ); }
		else                                 { fillIfValid ( h_AverageTime_C, Time_mean ); }
	      }
	    }
	    // write time histograms
	    TDirectory* dirTime = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirTime != 0 ) {
	      dirTime->cd();
	      h_Time->Write("",TObject::kOverwrite) ;
	      writeIfValid(h_TimeSecDist);
	    }
	  }
	  // noise	      
    noiseErrNorm = 18257.42 ;
	  if ( (rpc_eventstotal>0) && h_NoiseCorr && h_NoiseCorr_s && h_NoiseTot && h_NoiseTot_s && h_CS_entries) {
	    //std::cout << " Taglio Eventi " << std::endl;
	    for ( int ib = 0; ib != h_NoiseCorr->GetNbinsX(); ib++ ) {
	      if ( h_PanelId ) { if ( (h_PanelId->GetBinContent(ib+1) )==0 ) { continue; }  }
	      if ( h_CS_entries->GetBinContent(ib+1) ==0)continue;
	      noiseCorr    = h_NoiseCorr_s->GetBinContent(ib+1) ;
	      noiseCorrErr = sqrt( noiseCorr ) * noiseErrNorm / float(rpc_eventstotal) ;
	      noiseCorr = (noiseCorr*1000000)/float(rpc_eventstotal) ;
	      h_NoiseCorr->SetBinContent(ib+1, noiseCorr);
	      h_NoiseCorr->SetBinError(ib+1, noiseCorrErr);
	      fillIfValid ( h_NoiseCorrSecDist,  noiseCorr);
	      
	      noiseTot    = h_NoiseTot_s->GetBinContent(ib+1)   ;
	      noiseTotErr = sqrt( noiseTot ) * noiseErrNorm / float(rpc_eventstotal) ;
	      noiseTot  = (noiseTot*1000000)/float(rpc_eventstotal) ;
	      h_NoiseTot->SetBinContent(ib+1, noiseTot    ) ;
	      h_NoiseTot->SetBinError(ib+1, noiseTotErr )   ;
	      
	      fillIfValid ( h_NoiseTotSecDist, noiseTot );	      
	      
	      if ( ib > ( h_NoiseCorr->GetNbinsX()/2 ) ) { 
	        fillIfValid ( h_AverageNoiseTot_A, noiseTot); 
		      fillIfValid ( h_AverageNoiseCorr_A, noiseCorr );
        } else { 
	        fillIfValid ( h_AverageNoiseTot_C, noiseTot ); 
		      fillIfValid ( h_AverageNoiseCorr_C, noiseCorr);
	      }
	    }
	    TDirectory* dirNoise = f->GetDirectory( dir_sum_track.c_str() ) ;
	    if ( dirNoise!=0 ) {
	      dirNoise->cd();
	      h_NoiseCorr->Write("",TObject::kOverwrite) ;
	      h_NoiseTot ->Write("",TObject::kOverwrite) ;
	      
	      writeIfValid ( h_NoiseCorrSecDist );
	      writeIfValid ( h_NoiseTotSecDist  );
	    }
	  }
	  
	  
	 ///create and insert entries in SQLite DB 
         //  bool do_asciiCoolFile1  = true;
	   float effeta            = -9999;	  float effphi  	  = -9999;  	 char arr_effeta  	  [14]={};      char arr_effphi	      [14]={};	    
           float erreffeta         = -1; 	  float erreffphi	  =-1;  	 char arr_erreffeta	  [14]={};      char arr_erreffphi	      [14]={};	    
	   float reseta_cs1        = -9999;	  float resphi_cs1	  = -9999;  	 char arr_reseta_cs1	  [14]={};      char arr_resphi_cs1        [14]={};	    
           float errreseta_cs1     = -1; 	  float errresphi_cs1	  =-1;  	 char arr_errreseta_cs1	  [14]={};      char arr_errresphi_cs1     [14]={};	    
	   float reseta_cs2        = -9999;	  float resphi_cs2	  = -9999;  	 char arr_reseta_cs2	  [14]={};      char arr_resphi_cs2        [14]={};	    
           float errreseta_cs2     = -1; 	  float errresphi_cs2	  =-1;  	 char arr_errreseta_cs2	  [14]={};      char arr_errresphi_cs2     [14]={};	    
	   float reseta_csother    = -9999;	  float resphi_csother	  = -9999;  	 char arr_reseta_csother    [14]={};      char arr_resphi_csother    [14]={};	    
           float errreseta_csother = -1; 	  float errresphi_csother =-1;  	 char arr_errreseta_csother [14]={};      char arr_errresphi_csother [14]={};	    
	   float timeeta           = -9999;	  float timephi 	  = -9999;  	 char arr_timeeta 	  [14]={};      char arr_timephi	      [14]={};	    
           float errtimeeta        = -1; 	  float errtimephi	  =-1;  	 char arr_errtimeeta	  [14]={};      char arr_errtimephi        [14]={};	    
	   float noiseeta          = -9999;	  float noisephi	  = -9999;  	 char arr_noiseeta	  [14]={};      char arr_noisephi	      [14]={};	    
           float errnoiseeta       = -1; 	  float errnoisephi	  =-1;  	 char arr_errnoiseeta	  [14]={};      char arr_errnoisephi       [14]={};	    
	   float noiseeta_cor      = -9999;	  float noisephi_cor	  = -9999;  	 char arr_noiseeta_cor	  [14]={};      char arr_noisephi_cor      [14]={};	    
           float errnoiseeta_cor   = -1; 	  float errnoisephi_cor   =-1;  	 char arr_errnoiseeta_cor   [14]={};      char arr_errnoisephi_cor   [14]={};	    
	   float cl_sizeeta        = -9999;	  float cl_sizephi	  = -9999;  	 char arr_cl_sizeeta	  [14]={};      char arr_cl_sizephi        [14]={};	    
           float errcl_sizeeta     = -1; 	  float errcl_sizephi	  =-1;  	 char arr_errcl_sizeeta	  [14]={};      char arr_errcl_sizephi     [14]={};	    
  		
           float eta_effphi        =  0;          float phi_effeta        = 0;      		
	 
	   int   PanelCode   = 0;
	   int   Binposition   = 0;
	   int   TableVersion = 1;     //Version of Cool Table Formatting according with Andrea Di Simone Table
	   int   n_tr_peta    = 0;     //Number of eta tracks reconstructed on a gas volume
	   int   n_tr_pphi    = 0;     //Number of phi tracks reconstructed on a gas volume
	   int   NumberOfInfo = 1;     //Number of information for each variable 1= only pannel, >1 depending on tipy of monitoring setting and strip pannel geometry
	   int   StripCluster = 0;     //Number of strip that realize de reduction bin and the strip pannel geometry 0=no strip info,if panel has 24 strip: 24=strip info, 3   = cluster of 8 strip, 6 = cluster of 4 strip and so on
	   

 
     coolrpc.coolDbFolder("sqlite://;schema=RPCDQMFOFFLINE.db;dbname=RPC_DQA","/OFFLINE/OFFLINE_DQMF");
	   int nbin = 0;
	   if ( h_Eff ) nbin = h_Eff ->GetNbinsX() ;
	   auto writeToCString = [](float val, char* array0)->void{
	     //following two lines reproduce original behaviour
	     sprintf(array0,  "%f ", val);   //unsafe!
	     array0[5] = 0; //ugh!
	   };
	   
    for(int ibin=1 ; ibin!=nbin+1 ; ibin++){
	    if ( h_PanelId )PanelCode = (int)h_PanelId-> GetBinContent(ibin) ;
	    if(PanelCode ==0)continue;
  	     if(ibin%2==0){ 
	     
	      if (h_TrackProj) n_tr_pphi  =(int)h_TrackProj   -> GetBinContent(ibin) ;
	      if(n_tr_pphi <1000 ) continue;

	      effphi = getBinContentIfValid(h_Eff, ibin) ;    
	      writeToCString(effphi, arr_effphi);
	      erreffphi = getBinErrorIfValid(h_Eff, ibin);
	      writeToCString(erreffphi, arr_erreffphi);
	      
	      resphi_cs1 = getBinContentIfValid(h_Res_CS1, ibin);
	      writeToCString(resphi_cs1, arr_resphi_cs1);
	      errresphi_cs1 = getBinErrorIfValid(h_Res_CS1, ibin);
	      writeToCString(errresphi_cs1,arr_errresphi_cs1);
	      
        resphi_cs2 = getBinContentIfValid(h_Res_CS2, ibin);
        writeToCString(resphi_cs2,arr_resphi_cs2);
        errresphi_cs2 = getBinErrorIfValid(h_Res_CS2, ibin);
        writeToCString(errresphi_cs2,arr_errresphi_cs2);
        
        resphi_csother = getBinContentIfValid(h_Res_CSmore2, ibin);
        writeToCString(resphi_csother,arr_resphi_csother);
        errresphi_csother = getBinErrorIfValid(h_Res_CSmore2, ibin);
        writeToCString(errresphi_csother, arr_errresphi_csother);
        
	      timephi = getBinContentIfValid(h_Time, ibin);
	      writeToCString(timephi,arr_timephi);
	      errtimephi = getBinErrorIfValid(h_Time, ibin);
	      writeToCString(errtimephi,arr_errtimephi);
        
        noisephi = getBinContentIfValid(h_NoiseTot, ibin);
        writeToCString(noisephi,arr_noisephi);
        errnoisephi = getBinErrorIfValid(h_NoiseTot, ibin);
        writeToCString(errnoisephi,arr_errnoisephi);
        
        noisephi_cor = getBinContentIfValid(h_NoiseCorr, ibin);
        writeToCString(noisephi_cor, arr_noisephi_cor);
        errnoisephi_cor = getBinErrorIfValid(h_NoiseCorr, ibin);
        writeToCString(errnoisephi_cor, arr_errnoisephi_cor);
        
        cl_sizephi = getBinContentIfValid(h_CS, ibin);
        writeToCString(cl_sizephi, arr_cl_sizephi);
        errcl_sizephi = getBinErrorIfValid(h_CS, ibin);
        writeToCString(errcl_sizephi,arr_errcl_sizephi);
	     
	     }else{
	      if (h_TrackProj) {n_tr_peta  =(int)h_TrackProj   -> GetBinContent(ibin) ;}
              if(n_tr_peta <1000) continue;

 	      effeta 	     = getBinContentIfValid(h_Eff,ibin) ;      
	      writeToCString(effeta,arr_effeta);
	      erreffeta	     = getBinErrorIfValid(h_Eff,ibin) ;      
	      writeToCString(erreffeta,arr_erreffeta);
	      
	      reseta_cs1	     = getBinContentIfValid(h_Res_CS1,ibin) ;      
	      writeToCString(reseta_cs1,arr_reseta_cs1);
	      errreseta_cs1       = getBinErrorIfValid(h_Res_CS1,ibin) ;      
	      writeToCString(errreseta_cs1,arr_errreseta_cs1);
	      
	      reseta_cs2	     = getBinContentIfValid(h_Res_CS2,ibin) ;      
	      writeToCString(reseta_cs2,arr_reseta_cs2);
	      errreseta_cs2       = getBinErrorIfValid(h_Res_CS2,ibin) ;      
	      writeToCString(errreseta_cs2,arr_errreseta_cs2);
	      
        reseta_csother      = getBinContentIfValid(h_Res_CSmore2,ibin) ;      
	      writeToCString(reseta_csother,arr_reseta_csother);
	      errreseta_csother   = getBinErrorIfValid(h_Res_CSmore2,ibin) ;      
	      writeToCString(errreseta_csother,arr_errreseta_csother);
	      
	      timeeta	     = getBinContentIfValid(h_Time,ibin) ;      
	      writeToCString(timeeta,arr_timeeta);
	      errtimeeta	     = getBinErrorIfValid(h_Time,ibin) ;      
	      writeToCString(errtimeeta,arr_errtimeeta);
	      
	      noiseeta	     = getBinContentIfValid(h_NoiseTot,ibin) ;      
	      writeToCString(noiseeta,arr_noiseeta);
	      errnoiseeta	     = getBinErrorIfValid(h_NoiseTot,ibin) ;      
	      writeToCString(errnoiseeta,arr_errnoiseeta);
	      
	      noiseeta_cor	     = getBinContentIfValid(h_NoiseCorr,ibin) ;      
	      writeToCString(noiseeta_cor, arr_noiseeta_cor	);
	      errnoiseeta_cor     = getBinErrorIfValid(h_NoiseCorr,ibin) ;      
	      writeToCString(errnoiseeta_cor, arr_errnoiseeta_cor);
	      
	      cl_sizeeta	     = getBinContentIfValid(h_CS,ibin) ;      
	      writeToCString(cl_sizeeta,arr_cl_sizeeta);
	      errcl_sizeeta       = getBinErrorIfValid(h_CS,ibin) ;      
	      writeToCString(errcl_sizeeta,arr_errcl_sizeeta);	     
	     
	    
              //std::cout<<"PanelCode  "<<PanelCode<<" etaprimo "<<"\n";
 
	      char recEta   [4000]; //eff_eta, res_cs1, res_cs2, res_csother, time, mean and rms
	      char detEta   [4000]; //noise, noise_corr, cs, mean and rms
	      char recPhi1  [4000]; //eff_phi, res_cs1, res_cs2, res_csother, time, only mean
 	      char recPhi2  [4000]; //eff_phi, res_cs1, res_cs2, res_csother, time, only rms
	      char detPhi1  [4000]; //noise, noise_corr, cs, mean and rms
	      char detPhi2  [4000];
	      sprintf(recEta,  "%5d %5d %5d %5d %s %s %s %s %s %s %s %s %s %s ", TableVersion,  
	        n_tr_peta, NumberOfInfo, StripCluster, arr_effeta, arr_erreffeta, arr_reseta_cs1, 
	        arr_errreseta_cs1, arr_reseta_cs2, arr_errreseta_cs2, arr_reseta_csother, 
	        arr_errreseta_csother, arr_timeeta, arr_errtimeeta) ;  
	      sprintf(detEta,  "%s %s %s %s %s %s ", arr_noiseeta, arr_errnoiseeta, 
	        arr_noiseeta_cor, arr_errnoiseeta_cor, arr_cl_sizeeta, arr_errcl_sizeeta) ;  
	      sprintf(recPhi1, "%5d %5d %5d %s %s %s %s %s ", n_tr_pphi, NumberOfInfo, 
	        StripCluster, arr_effphi, arr_resphi_cs1, arr_resphi_cs2, arr_resphi_csother, 
	        arr_timephi) ;  
	      sprintf(recPhi2, "%s %s %s %s %s ", arr_erreffphi, arr_errresphi_cs1, 
	        arr_errresphi_cs2, arr_errresphi_csother, arr_errtimephi) ;  
	      sprintf(detPhi1, "%s %s %s %s %s %s ", arr_noisephi, arr_errnoisephi, 
	        arr_noisephi_cor, arr_errnoisephi_cor, arr_cl_sizephi, arr_errcl_sizephi) ;  
	      sprintf(detPhi2, "0 ") ;  
 	      std::string cool_tag="Reco";		        
              coolrpc.setSince(0U,0U);		
              coolrpc.setUntil(4294967295U,0U);	
 	      coolrpc.insert_withTag(run_number*0+429496729U,PanelCode,recEta,detEta,recPhi1,recPhi2,detPhi1,detPhi2, cool_tag);			    
	     }
	    }
	   
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
	   
            int   TableVersionCondDB  = 2 ;         //RPC conditionDB table versioning
	    float gapeffeta           = -9999;	       char arr_gapeffeta 	 [10];  	   
            float errgapeffeta        = -1;	       char arr_errgapeffeta	 [10];  	   
            float gapeffphi	      = -9999;	       char arr_gapeffphi 	 [10];
	    float errgapeffphi        = -1;            char arr_errgapeffphi	 [10];
	    float gapeff              = -9999;			    
            float errgapeff           = -1;
	    float entriesCSeta        = -1;
	    int   entriesCS1eta	      = -1;
	    int   entriesCS2eta	      = -1;
	    float entriesCSphi        = -1;
	    int   entriesCS1phi	      = -1;
	    int   entriesCS2phi	      = -1;
            float rateCS1eta	      = -1;	       char arr_rateCS1eta	 [10]; 
            float rateCS2eta	      = -1;	       char arr_rateCS2eta	 [10]; 
            float rateCSmore2eta      = -1;	       char arr_rateCSmore2eta	 [10]; 
            float rateCS1phi	      = -1;	       char arr_rateCS1phi	 [10]; 
            float rateCS2phi	      = -1;	       char arr_rateCS2phi	 [10]; 
            float rateCSmore2phi      = -1;	       char arr_rateCSmore2phi	 [10]; 
	    
	    

 	    coolrpc.coolDbFolder("sqlite://;schema=RPCConditionDB.db;dbname=RPC_DQA","/OFFLINE/FINAL");
  	    std::string dir_cool_raw = run_dir + "/Muon/MuonRawDataMonitoring/RPC/CoolDB/";
	    TDirectory* coolFolder = f->GetDirectory( dir_cool_raw.c_str() ) ;
            
	    if ( coolFolder != 0 ) {
	  	  	  
	    std::vector<std::string> layerList ;
	    layerList.push_back( "LowPt0"   );
	    layerList.push_back( "LowPt1"   );
	    layerList.push_back( "Pivot0"   );
	    layerList.push_back( "Pivot1"   );
	    layerList.push_back( "HighPt0"  );
	    layerList.push_back( "HighPt1"  );
	    int NumberLayerStrip      = 0;
	    int PanelStripId          = 0;
	    int StripProfileContenent = 0;
	  
	    for ( std::vector<std::string>::const_iterator iter=layerList.begin(); iter!=layerList.end(); ++iter ) {
	      for ( int i_dblPhi=0; i_dblPhi!=2*1+1; ++i_dblPhi ) {
	        char coolName[40];
		sprintf(coolName, "Sector%.2d_%s_dblPhi%d", i_sec+1, (*iter).c_str(), i_dblPhi+1 );
		std::string stripId_name       = dir_cool_raw + coolName + "_PanelId" ;
	        std::string stripProfile_name  = dir_cool_raw + coolName + "_Profile" ;
		TH1F* h_stripId      = NULL;
	        TH1F* h_stripProfile = NULL;
	        
		if ( RPCCheckHistogram(f, stripId_name.c_str()      ) ) { h_stripId      = (TH1F*) ( f->Get(stripId_name.c_str())      );} 
		if ( RPCCheckHistogram(f, stripProfile_name.c_str() ) ) { h_stripProfile = (TH1F*) ( f->Get(stripProfile_name.c_str()) );} 
		
		if ( h_stripId && h_stripProfile ) {
		 
		  int  SingleStripsValue    = 0;
		  int  StripsOnPanel        = 1; //number of strip on panel
		  char SingleStripsStatus  [80]  ;
		  char SingleStripsStatusOK[80]  ;
		  std::string PanelStripsStatus  ;
		  std::string PanelStripsStatusOK;
		  
		  NumberLayerStrip = h_stripProfile->GetNbinsX() ;
		  for(int Nstrips=1 ; Nstrips!=NumberLayerStrip+1 ; Nstrips++){
		    PanelStripId          = (int)h_stripId      -> GetBinContent(Nstrips) ;
		    StripProfileContenent = (int)h_stripProfile -> GetBinContent(Nstrips) ;
		    float StripOccupancy = (float)(StripProfileContenent)/rpc_eventstotal;
		    if (StripOccupancy==0) SingleStripsValue=0;	   
		    if (StripOccupancy>0   && StripOccupancy<0.9) SingleStripsValue=5;
		    if (StripOccupancy>0.9) SingleStripsValue=9;
		    		    
		    if(h_stripId-> GetBinCenter(Nstrips) > 0){ 
		     sprintf(SingleStripsStatus  , "%d 000.0 0.000|", SingleStripsValue );
		     sprintf(SingleStripsStatusOK,  "5 000.0 0.000|"                    );
		    }else{ 
		     sprintf(SingleStripsStatus, "|000.0 0.000 %d", SingleStripsValue );
		     sprintf(SingleStripsStatusOK, "|000.0 0.000 5"                   );
		    }
		    PanelStripsStatus   = PanelStripsStatus   +   SingleStripsStatus;
		    PanelStripsStatusOK = PanelStripsStatusOK + SingleStripsStatusOK;
		    
		    

		    if((int)h_stripId -> GetBinContent(Nstrips)==(int)h_stripId  -> GetBinContent(Nstrips+1))StripsOnPanel++;                      
		    //std::cout <<Nstrips<<" "<< h_stripId-> GetBinCenter(Nstrips)<< " "<< SingleStripsStatus <<" PanelStripsStatus " << PanelStripsStatus <<" PanelStripsId " << PanelStripId <<std::endl;

		    if((int)h_stripId -> GetBinContent(Nstrips)!=(int)h_stripId  -> GetBinContent(Nstrips+1)){
		    //std::cout <<StripsOnPanel<<" StripsOnPanel "<< std::endl;
  
		    if(h_stripId-> GetBinCenter(Nstrips) < 0){		    		    
		     //std::cout << " PanelStripsStatus " << PanelStripsStatus <<std::endl;		    
		     std::reverse(PanelStripsStatus  .begin(), PanelStripsStatus  .end());				    
		     std::reverse(PanelStripsStatusOK.begin(), PanelStripsStatusOK.end());		
		    }
                       
        for(int ibin=1 ; ibin!=nbin+1 ; ibin++){
          if ( h_PanelId )PanelCode = (int)h_PanelId-> GetBinContent(ibin) ;
	        if(PanelCode !=PanelStripId)continue;
			    if(ibin%2!=0){
			      if (h_TrackProj) {n_tr_peta  =(int)h_TrackProj   -> GetBinContent(ibin) ;
			    }
			      //if(n_tr_peta >0){				
				  if ( h_PanelId )Binposition = (int)h_PanelId-> GetBinCenter(ibin) ;
				  int ibin_perp=0;
	        if(Binposition>0){ 
				    ibin_perp=ibin+1;
				    eta_effphi = getBinContentIfValid(h_Eff, ibin+1);
			    }else{
				    ibin_perp=ibin-1;
				    eta_effphi = getBinContentIfValid(h_Eff, ibin-1);
 			    }
 			    gapeff = getBinContentIfValid(h_GapEff, ibin);
 			    errgapeff = getBinErrorIfValid(h_GapEff, ibin);
 			    effeta = getBinContentIfValid(h_Eff, ibin);
 			    erreffeta = getBinErrorIfValid(h_Eff, ibin);
				
          gapeffeta   =               gapeff;
          errgapeffeta=            errgapeff;
          EffThreshold = (effeta<Minimum_efficiency)|| (eta_effphi<Minimum_efficiency);
          
          reseta_cs1 = getBinContentIfValid(h_Res_CS1, ibin);
          errreseta_cs1 = getBinErrorIfValid(h_Res_CS1, ibin);
          
          reseta_cs2 = getBinContentIfValid(h_Res_CS2,ibin) ;
          errreseta_cs2 = getBinErrorIfValid(h_Res_CS2,ibin) ;
          
          reseta_csother      = getBinContentIfValid( h_Res_CSmore2,ibin) ;
          errreseta_csother   = getBinErrorIfValid(h_Res_CSmore2,ibin) ;
          
          noiseeta	       = getBinContentIfValid( h_NoiseTot,ibin) ;
          errnoiseeta          = getBinErrorIfValid( h_NoiseTot,ibin) ;
          	     
				  noiseeta_cor	       = getBinContentIfValid( h_NoiseCorr,ibin) ;      
          errnoiseeta_cor     = getBinErrorIfValid(h_NoiseCorr,ibin) ; 
               
          cl_sizeeta	       = getBinContentIfValid( h_CS,ibin) ;
          errcl_sizeeta       = getBinErrorIfValid( h_CS,ibin) ;
          
          entriesCSeta        = getBinContentIfValid( h_CS_entries,ibin) ;
          entriesCS1eta       = getBinContentIfValid( h_CS1_entries,ibin) ;
          entriesCS2eta       = getBinContentIfValid(h_CS2_entries,ibin) ;
          
				  if (entriesCSeta>0){
            rateCS1eta     =entriesCS1eta/entriesCSeta;
            rateCS2eta     =entriesCS2eta/entriesCSeta;
            rateCSmore2eta =(entriesCSeta-(entriesCS1eta+entriesCS2eta))/entriesCSeta;
          }

				  if (applyEffThreshold){ 
				    if(effeta<Minimum_efficiency&&eta_effphi<Minimum_efficiency){
               effeta      =Minimum_efficiency;
               gapeffeta   =Minimum_efficiency;
               erreffeta   =                 0.1;
               errgapeffeta=                 0.1;
               PanelStripsStatus   = PanelStripsStatusOK;
               cl_sizeeta     =   1;
               errcl_sizeeta  = 0.1,
			         rateCS1eta     =   1;
               rateCS2eta     =   0;
               rateCSmore2eta =   0; 
				     } else if(effeta<Minimum_efficiency&&eta_effphi>Minimum_efficiency){
               effeta      = Minimum_efficiency;
               gapeffeta   =            eta_effphi;
               erreffeta   =                   0.1;
               errgapeffeta=                   0.1;
               PanelStripsStatus   = PanelStripsStatusOK;
               cl_sizeeta     =   1;
               errcl_sizeeta  = 0.1,
			         rateCS1eta     = 1;
               rateCS2eta     = 0;
               rateCSmore2eta = 0;
				     } else if(effeta>Minimum_efficiency&&eta_effphi<Minimum_efficiency){
				       gapeffeta   =              effeta;
				       errgapeffeta=                 0.1;
				     }
				  }	
          sprintf(arr_effeta	   ,	"%f ", effeta		 ) ;   arr_effeta 	  [5]	=0;
          sprintf(arr_erreffeta	   ,	"%f ", erreffeta	 ) ;   arr_erreffeta	  [5]	=0;
          sprintf(arr_gapeffeta	   ,	"%f ", gapeffeta	 ) ;   arr_gapeffeta	  [5]	=0;
          sprintf(arr_errgapeffeta     ,	"%f ", errgapeffeta	 ) ;   arr_errgapeffeta	  [5]	=0;
          sprintf(arr_reseta_cs1	   ,	"%f ", reseta_cs1	 ) ;   arr_reseta_cs1	  [5]	=0;
          sprintf(arr_errreseta_cs1    ,	"%f ", errreseta_cs1	 ) ;   arr_errreseta_cs1    [5]	=0;
          sprintf(arr_reseta_cs2	   ,	"%f ", reseta_cs2	 ) ;   arr_reseta_cs2	  [5]	=0;
          sprintf(arr_errreseta_cs2    ,	"%f ", errreseta_cs2	 ) ;   arr_errreseta_cs2    [5]	=0;
          sprintf(arr_reseta_csother   ,	"%f ", reseta_csother	 ) ;   arr_reseta_csother   [5]	=0;
          sprintf(arr_errreseta_csother,	"%f ", errreseta_csother ) ;   arr_errreseta_csother[5]	=0;
          sprintf(arr_noiseeta	   ,	"%f ", noiseeta 	 ) ;   arr_noiseeta	  [5]	=0;
          sprintf(arr_errnoiseeta	   ,	"%f ", errnoiseeta	 ) ;   arr_errnoiseeta	  [5]	=0;
          sprintf(arr_noiseeta_cor	   ,    "%f ", noiseeta_cor      ) ;   arr_noiseeta_cor	  [5]   =0;
          sprintf(arr_errnoiseeta_cor  ,    "%f ", errnoiseeta_cor   ) ;   arr_errnoiseeta_cor  [5]   =0;
          sprintf(arr_cl_sizeeta	   ,	"%f ", cl_sizeeta	 ) ;   arr_cl_sizeeta	  [5]	=0;
          sprintf(arr_errcl_sizeeta    ,	"%f ", errcl_sizeeta	 ) ;   arr_errcl_sizeeta    [5]	=0;
          sprintf(arr_rateCS1eta       ,	"%f ", rateCS1eta	 ) ;   arr_rateCS1eta       [5]	=0;
          sprintf(arr_rateCS2eta       ,	"%f ", rateCS2eta	 ) ;   arr_rateCS2eta       [5]	=0;
          sprintf(arr_rateCSmore2eta   ,	"%f ", rateCSmore2eta	 ) ;   arr_rateCSmore2eta   [5]	=0;
	
          char PanelRes   [255]; //eff_eta, res_cs1, res_cs2, res_csother, time, mean and rms
          char StripStatus   [4096]; //strips status 0 to 9 for dead noisy strips
				
          sprintf(PanelRes,  "%d %d %d %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", TableVersionCondDB,  n_tr_peta, StripsOnPanel,  arr_effeta, arr_erreffeta,  arr_gapeffeta, arr_errgapeffeta, arr_reseta_cs1, arr_errreseta_cs1, arr_reseta_cs2, arr_errreseta_cs2, arr_reseta_csother, arr_errreseta_csother, arr_noiseeta, arr_errnoiseeta, arr_noiseeta_cor, arr_errnoiseeta_cor, arr_cl_sizeeta, arr_errcl_sizeeta, arr_rateCS1eta, arr_rateCS2eta, arr_rateCSmore2eta) ;
          sprintf(StripStatus, "%s", PanelStripsStatus.c_str()) ;
				  std::string cool_tagCondDB="RecoCondDB";		        
          coolrpc.setSince(0U,0U);		
          coolrpc.setUntil(4294967295U,0U);	
          coolrpc.insertCondDB_withTag(run_number*0+429496729U,PanelCode,PanelRes, StripStatus,cool_tagCondDB);
          if(printout and EffThreshold and h_GapEff)std::cout << stripProfile_name << " under THR "<< EffThreshold <<" "<< PanelCode << " ibin " << ibin << " h_EffEta " << h_Eff->GetBinContent(ibin) <<" h_EffPhi " << h_Eff->GetBinContent(ibin_perp) << " h_GapEffEta " << h_GapEff->GetBinContent(ibin) <<" h_GapEffPhi " << h_GapEff->GetBinContent(ibin_perp) << " cool_EtaEff "<< effeta <<" cool_GapEffEta "<< gapeffeta <<" --- Eta Summary " << PanelRes<< " --- StripStatus " << StripStatus << std::endl;
          if(printout&&EffThreshold)std::cout<<"inCOOL_ETA_id_ntrk_panelEff_gapEff "<<PanelCode<<" "<<n_tr_peta<<" "<<(int)(n_tr_peta*effeta)<<" "<<(int)(n_tr_peta*gapeffeta)<<std::endl;	
          countpanelindb++;
          if(effeta==0.0)countpaneleff0++;
          if(n_tr_peta==0)countpaneltrack0++;
			  }else{
         if (h_TrackProj) n_tr_pphi  =(int)h_TrackProj   -> GetBinContent(ibin) ;
         //if(n_tr_pphi >0){

         if ( h_PanelId )Binposition = (int)h_PanelId-> GetBinCenter(ibin) ;
         int ibin_perp=0;
         if(Binposition>0){
           ibin_perp=ibin-1;
           gapeff = getBinContentIfValid(h_GapEff, ibin-1);
           errgapeff = getBinErrorIfValid(h_GapEff,ibin-1);
           phi_effeta = getBinContentIfValid(h_Eff, ibin-1);           
         }else{
           ibin_perp=ibin+1;
           gapeff = getBinContentIfValid(h_GapEff, ibin+1);
           errgapeff = getBinErrorIfValid(h_GapEff,ibin+1);
           phi_effeta = getBinContentIfValid(h_Eff, ibin+1);    
         }
         effphi 	     = getBinContentIfValid(h_Eff, ibin);
         erreffphi	     = getBinErrorIfValid(h_Eff,ibin);
         gapeffphi   =               gapeff;
         errgapeffphi=            errgapeff;
         EffThreshold = (effphi<Minimum_efficiency)|| (phi_effeta<Minimum_efficiency);
			     
			     
         resphi_cs1 = getBinContentIfValid ( h_Res_CS1, ibin	);
         errresphi_cs1 = getBinErrorIfValid(h_Res_CS1, ibin);
         
         resphi_cs2	     = getBinContentIfValid(h_Res_CS2,ibin) ;
         errresphi_cs2       = getBinErrorIfValid(h_Res_CS2,ibin) ;
         
         resphi_csother      = getBinContentIfValid(h_Res_CSmore2,ibin) ;
         errresphi_csother   = getBinErrorIfValid(h_Res_CSmore2,ibin) ;
         
         noisephi	     = getBinContentIfValid(h_NoiseTot,ibin) ;
         errnoisephi	     = getBinErrorIfValid(h_NoiseTot,ibin) ;
         
         noisephi_cor        = getBinContentIfValid(h_NoiseCorr,ibin) ;    
         errnoisephi_cor     = getBinErrorIfValid(h_NoiseCorr,ibin) ; 
            
         cl_sizephi	     = getBinContentIfValid(h_CS,ibin) ;
         errcl_sizephi       = getBinErrorIfValid(h_CS,ibin) ;
         
         entriesCSphi	     = getBinContentIfValid(h_CS_entries,ibin) ;
         entriesCS1phi	     = getBinContentIfValid(h_CS1_entries,ibin) ;
         entriesCS2phi	     = getBinContentIfValid(h_CS2_entries,ibin) ;
         if(entriesCSphi>0){
            rateCS1phi     =entriesCS1phi/entriesCSphi;
            rateCS2phi     =entriesCS2phi/entriesCSphi;
            rateCSmore2phi =(entriesCSphi-(entriesCS1phi+entriesCS2phi))/entriesCSphi;
         }	
			     
			     
			    if (applyEffThreshold){ 
			       if(effphi<Minimum_efficiency&&phi_effeta<Minimum_efficiency){
				  effphi      =Minimum_efficiency;
				  gapeffphi   =Minimum_efficiency;
				  erreffphi   =                 0.1;
				  errgapeffphi=                 0.1;
				  PanelStripsStatus   = PanelStripsStatusOK;
				  cl_sizephi     =   1;
				  errcl_sizephi  = 0.1,
			          rateCS1phi     = 1;
                                  rateCS2phi     = 0;
                                  rateCSmore2phi = 0;
			      }
			      else if(effphi<Minimum_efficiency&&phi_effeta>Minimum_efficiency){
				  effphi      =       Minimum_efficiency;
				  gapeffphi   =                 phi_effeta;
				  erreffphi   =                        0.1;
				  errgapeffphi=                        0.1;
				  PanelStripsStatus   = PanelStripsStatusOK;
				  cl_sizephi     =   1;
				  errcl_sizephi  = 0.1,
			          rateCS1phi     = 1;
                                  rateCS2phi     = 0;
                                  rateCSmore2phi = 0;
			      }
			      else if(effphi>Minimum_efficiency&&phi_effeta<Minimum_efficiency){   
				  gapeffphi   =              effphi; 
				  errgapeffphi=                 0.1;
				  PanelStripsStatus   = PanelStripsStatusOK;
			      }  
			     }

			     sprintf(arr_effphi		,    "%f ", effphi	      ) ;   arr_effphi	       [5]   =0;
			     sprintf(arr_erreffphi	,    "%f ", erreffphi	      ) ;   arr_erreffphi        [5]   =0;
 			     sprintf(arr_gapeffphi	,    "%f ", gapeffphi	      ) ;   arr_gapeffphi        [5]   =0;
			     sprintf(arr_errgapeffphi	,    "%f ", errgapeffphi      ) ;   arr_errgapeffphi     [5]   =0;
			     sprintf(arr_resphi_cs1	,    "%f ", resphi_cs1        ) ;   arr_resphi_cs1       [5]   =0;
			     sprintf(arr_errresphi_cs1	,    "%f ", errresphi_cs1     ) ;   arr_errresphi_cs1    [5]   =0;
			     sprintf(arr_resphi_cs2	,    "%f ", resphi_cs2        ) ;   arr_resphi_cs2       [5]   =0;
			     sprintf(arr_errresphi_cs2	,    "%f ", errresphi_cs2     ) ;   arr_errresphi_cs2    [5]   =0;
			     sprintf(arr_resphi_csother	,    "%f ", resphi_csother    ) ;   arr_resphi_csother   [5]   =0;
			     sprintf(arr_errresphi_csother,    "%f ", errresphi_csother ) ;   arr_errresphi_csother[5]   =0;
			     sprintf(arr_noisephi 	,    "%f ", noisephi	      ) ;   arr_noisephi         [5]   =0;
			     sprintf(arr_errnoisephi	,    "%f ", errnoisephi       ) ;   arr_errnoisephi      [5]   =0;
			     sprintf(arr_noisephi_cor     ,    "%f ", noisephi_cor      ) ;   arr_noisephi_cor     [5]   =0;
                             sprintf(arr_errnoisephi_cor  ,    "%f ", errnoisephi_cor   ) ;   arr_errnoisephi_cor  [5]   =0;
			     sprintf(arr_cl_sizephi	,    "%f ", cl_sizephi        ) ;   arr_cl_sizephi       [5]   =0;
			     sprintf(arr_errcl_sizephi	,    "%f ", errcl_sizephi     ) ;   arr_errcl_sizephi    [5]   =0;
			     sprintf(arr_rateCS1phi	,    "%f ", rateCS1phi        ) ;   arr_rateCS1phi       [5]   =0;
			     sprintf(arr_rateCS2phi	,    "%f ", rateCS2phi        ) ;   arr_rateCS2phi       [5]   =0;
			     sprintf(arr_rateCSmore2phi	,    "%f ", rateCSmore2phi    ) ;   arr_rateCSmore2phi   [5]   =0;
			    		
			     char PanelRes   [255]; //eff_eta, res_cs1, res_cs2, res_csother, time, mean and rms
			     char StripStatus   [4096]; //strips status 0 to 9 for dead noisy strips
 			     sprintf(PanelRes,  "%d %d %d %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", TableVersionCondDB,  n_tr_pphi, 
 			     StripsOnPanel,  arr_effphi, arr_erreffphi,  arr_gapeffphi, arr_errgapeffphi, arr_resphi_cs1, arr_errresphi_cs1, arr_resphi_cs2, 
 			       arr_errresphi_cs2, arr_resphi_csother, arr_errresphi_csother, arr_noisephi, arr_errnoisephi, arr_noisephi_cor, arr_errnoisephi_cor, 
 			       arr_cl_sizephi, arr_errcl_sizephi, arr_rateCS1phi, arr_rateCS2phi, arr_rateCSmore2phi) ;
 			     sprintf(StripStatus, "%s", PanelStripsStatus.c_str()) ;
 			     std::string cool_tag="RecoCondDB";		        
                             coolrpc.setSince(0U,0U);		
                             coolrpc.setUntil(4294967295U,0U);	
 			     coolrpc.insertCondDB_withTag(run_number*0+429496729U,PanelCode,PanelRes, StripStatus,cool_tag);
 			     
			     if(printout&&EffThreshold) std::cout << stripProfile_name << " under THR "<< EffThreshold <<" " << PanelCode << " ibin " << ibin << " h_EffPhi " << h_Eff->GetBinContent(ibin) 
			       <<" h_EffEta " << h_Eff->GetBinContent(ibin_perp) << " h_GapEffPhi " << h_GapEff->GetBinContent(ibin) <<" h_GapEffEta " << h_GapEff->GetBinContent(ibin_perp) << " cool_PhiEff "
			       << effphi <<" cool_GapEffPhi "<< gapeffphi <<" --- Phi Summary " << PanelRes<< " --- StripStatus " << StripStatus << std::endl;
			       
			     if(printout&&EffThreshold) std::cout<<"inCOOL_PHI_id_ntrk_panelEff_gapEff "<<PanelCode<<" "<<n_tr_pphi<<" "<<(int)(n_tr_pphi*effphi)<<" "<<(int)(n_tr_pphi*gapeffphi)<<std::endl;
			    countpanelindb++;
			    if(effphi==0.0)countpaneleff0++;
			    if(n_tr_pphi==0)countpaneltrack0++;
			  }		
	    	          StripsOnPanel=1;
	      	          PanelStripsStatus.clear();
	      	          PanelStripsStatusOK.clear();
 		        }
	    	        StripsOnPanel=1;
	      	        PanelStripsStatus.clear();
	      	        PanelStripsStatusOK.clear();			
		      }
                    }		  
		  }				
	        } // end loop on DoubletPhi	
	      } // end loop on layers
	    }// end Cool Folder	  
	    std::cout <<"Count RC panels in DB " << countpanelindb << " Count RPCpanels in DB with zero efficiency "<< countpaneleff0 <<" Count RPCpanels in DB with zero track "<<countpaneltrack0 <<std::endl;	    
  	  } // end for sectors
	  std::cout <<"Count RC panels in DB " << countpanelindb << " Count RPCpanels in DB with zero efficiency "<< countpaneleff0 <<" Count RPCpanels in DB with zero track "<<countpaneltrack0 <<std::endl;
	// write distribution plots all ATLAS
	TDirectory* dirA = f->GetDirectory( dir_sideA_track.c_str() ) ;
	
	if ( dirA != 0 ) {
	  dirA->cd();
	  writeIfValid( h_AverageEff_A            );
	  writeIfValid ( h_AverageGapEff_A         );
	  writeIfValid( h_AverageRes_CS1_A        );
	  writeIfValid ( h_AverageRes_CS1rms_A     );
	  writeIfValid ( h_AverageRes_CS2_A        );
	  writeIfValid ( h_AverageRes_CS2rms_A     );
	  writeIfValid ( h_AverageRes_CSmore2_A    );
	  writeIfValid ( h_AverageRes_CSmore2rms_A );
	  writeIfValid ( h_AverageOccupancy_A      ) ;
	  writeIfValid ( h_AverageCS_A             );
	  writeIfValid ( h_AverageTime_A           );
	  writeIfValid ( h_AverageNoiseCorr_A      );
	  writeIfValid( h_AverageNoiseTot_A       );
	}
	TDirectory* dirC = f->GetDirectory( dir_sideC_track.c_str() ) ;
	if ( dirC != 0 ) {
	  dirC->cd();
	  writeIfValid( h_AverageEff_C           );
	  writeIfValid ( h_AverageGapEff_C        );
	  writeIfValid( h_AverageRes_CS1_C       );
	  writeIfValid ( h_AverageRes_CS1rms_C    );
	  writeIfValid ( h_AverageRes_CS2_C       );
	  writeIfValid ( h_AverageRes_CS2rms_C    );
	  writeIfValid ( h_AverageRes_CSmore2_C   );
	  writeIfValid ( h_AverageRes_CSmore2rms_C);
	  writeIfValid ( h_AverageOccupancy_C     ) ;
	  writeIfValid ( h_AverageCS_C            );
	  writeIfValid ( h_AverageTime_C          );
	  writeIfValid ( h_AverageNoiseCorr_C     );
	  writeIfValid( h_AverageNoiseTot_C      );
	}
	
		 
      }
    }else {
      delete obj_run;
    } // end if tdir_run !=0    
    
  }
  f->Close();
  delete f;
}



 


bool
MonitoringFile::
RPCCheckHistogram(TFile* f,const char* HistoName)
 {
  if (!(f->Get(HistoName))) {
    return false;
  }
  else return true;
 }

}
