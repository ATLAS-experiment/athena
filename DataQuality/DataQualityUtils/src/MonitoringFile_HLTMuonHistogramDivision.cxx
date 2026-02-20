/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
//Methods to Divide HLTMuon Histograms to calculate efficiency and occupancy after merging.
//Author: Akimasa Ishikawa (akimasa.ishikawa@cern.ch)
//Date:   Aug 2009
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DataQualityUtils/MonitoringFile.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <utility>
#include <map>
#include <string>

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TClass.h"
#include "TKey.h"
#include "TMath.h"
#include "TF1.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"


namespace dqutils {
  //   static const bool fdbg = false;
  static const bool fdbg = true;

  void MonitoringFile::HLTMuonHistogramDivision(const std::string& inFilename, std::string& run_dir) {
    bool HI_pp_key = false;//true::HI false::pp

    if (fdbg) std::cout << "  Start to Divide HLTMuon Histograms for Efficiency and Rate Ratio" << std::endl;
    PostProcessorFileWrapper mf(inFilename, "HLT Histogram Division");
    if (!mf.IsOpen()) {
      std::cerr << "HLTMuonPostProcess(): "
                << "Input file not opened \n";
      return;
    }
    if (mf.GetSize() < 1000.) {
      std::cerr << "HLTMuonPostProcess(): "
                << "Input file empty \n";
      return;
    }
    // get run directory name
    //Seemingly unnecessary lines are necessary
    TIter nextcd0(gDirectory->GetListOfKeys());
    TKey* key0 = (TKey*) nextcd0();
    if (key0 == 0) return;

    TDirectory* dir0 = dynamic_cast<TDirectory*> (key0->ReadObj());
    if (dir0 == 0) return;

    dir0->cd();

    auto getHPointer ATLAS_THREAD_SAFE = [&mf] ATLAS_NOT_THREAD_SAFE(const std::string &hname)->TH1F * {
      TH1F* pH {};
      mf.get(hname.c_str(), pH);
      if (!pH and fdbg) {
        std::cerr << "HLTMuon PostProcessing: no such histogram!! " << hname << std::endl;
      }
      return pH;
    };
    auto getHistogramPair  = [&getHPointer] (const std::string &numeratorName, const std::string &denominatorName)->std::pair<TH1F *, TH1F *>  {
      TH1F* pH2 {};
      TH1F* pH1 = getHPointer(numeratorName);
      if (pH1){
        pH2 = getHPointer(denominatorName);
      }
      return std::pair{pH1, pH2};
    };
    
    auto getH2Pointer ATLAS_THREAD_SAFE = [&mf] ATLAS_NOT_THREAD_SAFE(const std::string &hname)->TH2F * {
      TH2F* pH {};
      mf.get(hname.c_str(), pH);
      if (!pH and fdbg) {
        std::cerr << "HLTMuon PostProcessing: no such histogram!! " << hname << std::endl;
      }
      return pH;
    };
    
    
    ///

    // 110728: removing the iteration of searching run directory according to the new MDT code
    // 150621: reenable ...
    TIter next_run(mf.GetListOfKeys());
    TKey* key_run(0);
    while ((key_run = dynamic_cast<TKey*> (next_run())) != 0) { //== the while commented out at the end
      if (!key_run->IsFolder()) continue;
      run_dir = key_run->GetName();
      if (run_dir.find("run") == std::string::npos) {
        continue;
      }
      break;
    }
    if (run_dir.find("run") == std::string::npos) {
      std::cerr << "HLTMuon: unable to find run directory ..." << std::endl;
      return;
    }
    {
      if (fdbg) {
        std::cout << "HLTMuon: run directory is " << run_dir << std::endl;
      }
      std::string run_dir2 = run_dir;

      //===HLTMuon

      TH1F* hHI_PP_Flag(0);
      std::string hi_pp_flag = run_dir + "/HLT/MuonMon/Common/HI_PP_Flag";
      hHI_PP_Flag = getHPointer(hi_pp_flag);
      if (hHI_PP_Flag) {
        HI_pp_key = hHI_PP_Flag->GetBinContent(1) > 0;
      }

      std::string muon_dir = run_dir + "/HLT/MuonMon/";

      std::string cm_dir = muon_dir + "Common/";
      std::string mf_dir = muon_dir + "L2MuonSA/";
      std::string mc_dir = muon_dir + "muComb/";
      std::string mi_dir = muon_dir + "muIso/";
      std::string tm_dir = muon_dir + "TileMu/";
      std::string ef_dir = muon_dir + "MuonEF/";
      // YY added
      std::string ztp_dir = muon_dir + "MuZTP/";

      std::string eff_dir = muon_dir + "Efficiency/";
      std::string nd_dir = muon_dir + "Efficiency/NumDenom/";
      std::string rate_dir = muon_dir + "Rate/";
      std::string rr_dir = muon_dir + "Rate/Ratio/";

      std::string seff;
      std::string seffg; // YY added
      std::string snum;
      std::string sden;
      std::string stmp;
      std::string stmpg;

      TH1F* h1tmp(0);
      TH1F* h1eff(0);
      TH1F* h1sumeff(0); // new YY
      TH1F* h1effsum(nullptr);
      TGraphAsymmErrors* h1tmpg;

      //==Efficiency
      //  L2MuonSA efficiency
      TDirectory* dir = mf.GetDirectory(eff_dir.c_str());
      if (!dir) {
        std::cerr << "HLTMuonHistogramDivision: directory " << eff_dir << " not found" << std::endl;
        return;
      }

      static const std::string effnames_L2MuonSA[] = {
        "L2MuonSA_effi_toRecMuonCB_pt",
        "L2MuonSA_effi_toRecMuonCB_pt_barrel",
        "L2MuonSA_effi_toRecMuonCB_pt_endcap",
        "L2MuonSA_effi_toRecMuonCB_eta",
        "L2MuonSA_effi_toRecMuonCB_phi",
      };

      for (const std::string& effname : effnames_L2MuonSA) {
        seff = eff_dir + effname;
        snum = mf_dir + effname + "_numer";
        sden = mf_dir + effname + "_denom";
        const auto & [h1num, h1den] = getHistogramPair(snum, sden);
        if (not h1num) continue;
        //
        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(effname.c_str());
        std::string stcar = h1tmp->GetTitle();
        stcar.replace(stcar.end() - 5, stcar.end(), "");
        h1tmp->SetTitle(stcar.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
      }//effnames
      mf.Write();

      //  muComb efficiency
      static const std::string effnames_muComb[] = {
        "muComb_effi_toOffl_pt",
        "muComb_effi_toOffl_pt_barrel",
        "muComb_effi_toOffl_pt_endcap",
        "muComb_effi_toOffl_eta",
        "muComb_effi_toOffl_phi",
      };

      for (const std::string& effname : effnames_muComb) {
        seff = eff_dir + effname;
        snum = mc_dir + effname + "_numer";
        sden = mc_dir + effname + "_denom";
        const auto & [h1num, h1den] = getHistogramPair(snum, sden);
        if (!h1num) continue;
        //
        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(effname.c_str());
        std::string stcar = h1tmp->GetTitle();
        stcar.replace(stcar.end() - 5, stcar.end(), "");
        h1tmp->SetTitle(stcar.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
      }//effnames
      mf.Write();

      //  muIso efficiency
      static const std::string effnames_muIso[] = {
        "muIso_effi_toOffl_pt",
      };

      for (const std::string& effname : effnames_muIso) {
        seff = eff_dir + effname;
        snum = mi_dir + effname + "_numer";
        sden = mi_dir + effname + "_denom";
        const auto & [h1num, h1den] = getHistogramPair(snum, sden);
        if (!h1num) continue;
        //
        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(effname.c_str());
        std::string stcar = h1tmp->GetTitle();
        stcar.replace(stcar.end() - 5, stcar.end(), "");
        h1tmp->SetTitle(stcar.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
      }//effnames
      mf.Write();

      //  TileMu efficiency
      static const std::string effnames_TileMu[] = {
        "TileMu_RecCBMuon_EffEta",
        "TileMu_RecCBMuon_EffPhi",
        "TileTrackMu_RecCBMuon_EffEta",
        "TileTrackMu_RecCBMuon_EffPhi",
        "TileTrackMu_RecCBMuon_EffPt",
      };
      static const std::string numnames_TileMu[] = {
        "Rec_Eta_Num",
        "Rec_Phi_Num",
        "TileTrackMu_Eta",
        "TileTrackMu_Phi",
        "TileTrackMu_Pt",
      };
      static const std::string dennames_TileMu[] = {
        "Rec_Eta",
        "Rec_Phi",
        "Rec_Eta",
        "Rec_Phi",
        "Rec_Pt",
      };

      for (unsigned int i = 0; i < std::size(effnames_TileMu); i++) {
        seff = eff_dir + effnames_TileMu[i];
        snum = tm_dir + numnames_TileMu[i];
        sden = tm_dir + dennames_TileMu[i];
        stmp = effnames_TileMu[i];
        const auto & [h1num, h1den] = getHistogramPair(snum, sden);
        if (!h1num) continue;

        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(stmp.c_str());
        std::istringstream iss(stmp);
        std::string token;
        int ili = 0;
        while (std::getline(iss, token, '_')) {
          stmp = token + "Feature wrt Offline)";
          ili++;
          if (ili > 0) break;
        }

        std::string stcar = h1tmp->GetTitle();
        if (stcar.find("Pt") != std::string::npos) {
          stmp = "Efficiency on p_{T} (" + stmp;
          h1tmp->GetXaxis()->SetTitle("p_{T}");
        }
        if (stcar.find("Eta") != std::string::npos) stmp = "Efficiency on #eta (" + stmp;
        if (stcar.find("Phi") != std::string::npos) stmp = "Efficiency on #phi (" + stmp;
        h1tmp->SetTitle(stmp.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
      }//effnames
      mf.Write();

      //  MuonEF efficiency
      static const std::string effnames_MuonEF[] = {
        "EFMS_effi_toOffl_pt",
        "EFMS_effi_toOffl_eta",
        "EFMS_effi_toOffl_phi",
        "EFSA_effi_toOffl_pt",
        "EFCB_effi_toOffl_pt",
      };

      for (const std::string& effname : effnames_MuonEF) {
        seff = eff_dir + effname;
        snum = ef_dir + effname + "_numer";
        sden = ef_dir + effname + "_denom";
        const auto & [h1num, h1den] = getHistogramPair(snum, sden);
        if (!h1num) continue;

        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(effname.c_str());
        std::string stcar = h1tmp->GetTitle();
        stcar.replace(stcar.end() - 5, stcar.end(), "");
        h1tmp->SetTitle(stcar.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
      }//effnames
      mf.Write();

      //==Turn on
      std::vector<std::string> chainsMSonly;
      std::vector<std::string> chainsStandard;
      std::vector<std::string> chainsMG;
      std::vector<std::string> chainsMI;
      std::vector<std::string> chainsGeneric;
      std::vector<std::string> chainsEFiso;
      std::vector<std::string> chainsEFFS;

      // Generic (EFsuper etc.)

      chainsGeneric.push_back("muChain1");   // MAM
      chainsGeneric.push_back("muChain2");  // MAM

      // Generic (Isolated muons)
      chainsEFiso.push_back("muChainEFiso1");            // MAM
      chainsEFiso.push_back("muChainEFiso2");            // MAM

      // MSonly

      chainsMSonly.push_back("muChainMSonly1");    // MAM
      chainsMSonly.push_back("muChainMSonly2");    // MAM

      // EFFS triggers (L. Yuan)
      chainsEFFS.push_back("muChainEFFS");  // MAM

      enum indexINDEP {
        INDORTH, INDEGAMMA, INDMET, INDJET, INDTAU, INDMBIAS
      };
      std::string trigger[INDMBIAS + 1];

      trigger[INDORTH] = "Orthog";//EGamma + Tau + Jet + MET
      trigger[INDEGAMMA] = "EGamma";
      trigger[INDMET] = "MET";
      trigger[INDJET] = "Jet";

      //int maxindep = 0; // YY 20.01.2012

      // YY added:
      static const std::string ESchainName = "_ES";

      static const std::string bestr[2] = {
        "_Barrel", "_Endcap"
      };

      // made it as enum: 20.1.2012
      std::string triggerES[7] = {
        "_ESstd", "_EStag", "_ESid", "_ESindep", "_ESHIL1", "_ESHIid", "_ESHIindep"
      };
      enum indexES {
        ESSTD, ESTAG, ESID, ESINDEP, ESHIL1, ESHIID, ESHIINDEP
      };

      int maxESbr = ESINDEP;

      bool CB_mon_ESbr[ESHIINDEP + 1];
      bool MS_mon_ESbr[ESHIINDEP + 1];

      CB_mon_ESbr[ESSTD] = 0;
      CB_mon_ESbr[ESTAG] = 0;
      CB_mon_ESbr[ESID] = 1;
      CB_mon_ESbr[ESINDEP] = 0;
      CB_mon_ESbr[ESHIL1] = 0;
      CB_mon_ESbr[ESHIID] = 0;
      CB_mon_ESbr[ESHIINDEP] = 0;

      MS_mon_ESbr[ESSTD] = 0;
      MS_mon_ESbr[ESTAG] = 1;
      MS_mon_ESbr[ESID] = 0;
      MS_mon_ESbr[ESINDEP] = 0;
      MS_mon_ESbr[ESHIL1] = 0;
      MS_mon_ESbr[ESHIID] = 0;
      MS_mon_ESbr[ESHIINDEP] = 0;

      std::vector<std::string> vectkwd;

      vectkwd.push_back(triggerES[ESTAG]);
      vectkwd.push_back(triggerES[ESID]);
      vectkwd.push_back("_Jet");
      vectkwd.push_back("_all");

      std::string hptName = "_hpt";
      std::string MSchainName = "_MSb";

      // YY: pt range.
      int iSTDL;
      int iSTDH;
      if (HI_pp_key) {
        iSTDL = 45;  // 12 GeV
        //iSTDL = 54;  // 15 GeV
        iSTDH = 75; // 25 GeV
      } else {
        iSTDL = 105;  // 60 GeV
        //iSTDL = 91;  // 40 GeV
        iSTDH = 120; // 100 GeV
        //iSTDH = 120; // 100 GeV
      }
      int iMSL = 105;  // 60 GeV
      int iMSH = 120;  // 100 GeV

      if (HI_pp_key) {
        iMSL = 54;//15GeV
        iMSH = 75;//25GeV
      }
      // YY added:
      enum ieffAlgo {
        iL2MuonSA = 0,   // StdAlgo
        iMuComb = 1,   // StdAlgo
        iEFCB   = 2,   // StdAlgo
        iMuGirl = 3,   // StdAlgo

        iEFSA   = 1    // MSAlgo
      };

      //  Standard Chains
      //std::string alg[5] = {"_L2MuonSA", "_MuComb", "_MuonEFMS", "_MuonEFSA", "_MuonEFCB"};
      //std::string wrtalg[5] = {"_L1", "_L2MuonSA", "_MuComb", "_MuComb", "_MuComb"};

      // ******************************************************//
      // start the code add by Yuan //
      //std::string FS_pre_trigger = "mu18it_tight";
      std::string FS_pre_trigger = "EFFSpre";
      for (unsigned int i = 0; i < chainsEFFS.size(); i++) {
        std::string chainName = chainsEFFS.at(i);

        std::string hists_str[9] = {
          chainName + "_tagEFFSpre" + "_Turn_On_Curve_wrt_probe_MuidCB",
          chainName + "_tagEFFSpre" + "_Turn_On_Curve_wrt_probe_MuidCB_Barrel",
          chainName + "_tagEFFSpre" + "_Turn_On_Curve_wrt_probe_MuidCB_Endcap",
          chainName + "_tagEFFSpre_mu0_15" + "_Turn_On_Curve_wrt_probe_MuidCB",
          chainName + "_tagEFFSpre_mu15_20" + "_Turn_On_Curve_wrt_probe_MuidCB",
          chainName + "_tagEFFSpre_mu20" + "_Turn_On_Curve_wrt_probe_MuidCB",
          chainName + "_Turn_On_Curve_wrt_subleading_MuidCB",
          FS_pre_trigger + "_dimuonTP" + "_Turn_On_Curve_wrt_probe_MuidCB",
          FS_pre_trigger + "_dimuonTP" + "_Turn_On_Curve_wrt_L1_probe_MuidCB",
        };

        bool for_mydebug = false;
        auto getHPointerQuietly ATLAS_THREAD_SAFE =
        [&mf, for_mydebug] ATLAS_NOT_THREAD_SAFE(const std::string &hname)->TH1F * {
          TH1F* pH {};
          mf.get(hname.c_str(), pH);
          if (!pH and for_mydebug) {
            std::cerr << "HLTMuon PostProcessing: no such histogram!! " << hname << std::endl;
          }
          return pH;
        };
        auto getHistogramPairQuietly = [&getHPointerQuietly] (const std::string &numeratorName, const std::string &denominatorName)->std::pair<TH1F *, TH1F *>  {
          TH1F* pH2 {};
          TH1F* pH1 = getHPointerQuietly(numeratorName);
          if (pH1){
            pH2 = getHPointerQuietly(denominatorName);
          }
          return std::pair{pH1, pH2};
        };
        

        for (int iROI = 0; iROI < 9; iROI++) {
          sden = nd_dir + hists_str[iROI] + "_Denominator";
          snum = nd_dir + hists_str[iROI] + "_Numerator";
          seff = eff_dir + hists_str[iROI];
          seffg = seff + "_Fit";

          stmp = hists_str[iROI];
          stmpg = hists_str[iROI] + "_Fit";
          const auto & [h1num, h1den] = getHistogramPairQuietly(snum, sden);
          if (!h1num) continue;

          h1tmp = (TH1F*) h1den->Clone();
          h1tmp->SetName(stmp.c_str());
          h1tmp->SetTitle(stmp.c_str());
          h1tmp->GetYaxis()->SetTitle("Efficiency");
          h1tmp->Reset();
          h1tmp->Divide(h1num, h1den, 1., 1., "B");
          dir->cd();
          h1tmp->Write();
          h1tmpg = new TGraphAsymmErrors();
          h1tmpg->SetMarkerStyle(20);
          h1tmpg->SetMinimum(0.0);
          h1tmpg->SetMaximum(1.05);
          h1tmpg->BayesDivide(h1num, h1den);
          h1tmpg->GetYaxis()->SetTitle("Efficiency");
          h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
          h1tmpg->SetName(stmpg.c_str());
          h1tmpg->Write();
          delete h1tmpg;
        } // end the loop on individual turn-on curves

        std::string L1_TP_str = FS_pre_trigger + "_dimuonTP_L1" + "_Turn_On_Curve_wrt_probe_MuidCB";
        sden = nd_dir + FS_pre_trigger + "_dimuonTP" + "_Turn_On_Curve_wrt_probe_MuidCB" + "_Denominator";
        snum = nd_dir + FS_pre_trigger + "_dimuonTP" + "_Turn_On_Curve_wrt_L1_probe_MuidCB" + "_Denominator";
        seff = eff_dir + L1_TP_str;
        seffg = seff + "_Fit";
        stmp = L1_TP_str;
        stmpg = L1_TP_str + "_Fit";
        const auto & [h1num, h1den] = getHistogramPairQuietly(snum, sden);
        if (!h1num) continue;

        h1tmp = (TH1F*) h1den->Clone();
        h1tmp->SetName(stmp.c_str());
        h1tmp->SetTitle(stmp.c_str());
        h1tmp->GetYaxis()->SetTitle("Efficiency");
        h1tmp->Reset();
        h1tmp->Divide(h1num, h1den, 1., 1., "B");
        dir->cd();
        h1tmp->Write();
        h1tmpg = new TGraphAsymmErrors();
        h1tmpg->SetMarkerStyle(20);
        h1tmpg->SetMinimum(0.0);
        h1tmpg->SetMaximum(1.05);
        h1tmpg->BayesDivide(h1num, h1den);
        h1tmpg->GetYaxis()->SetTitle("Efficiency");
        h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
        dir->cd();
        h1tmpg->SetName(stmpg.c_str());
        h1tmpg->Write();
        delete h1tmpg;

        //**** summargy plot **********//

        std::string histNumB = nd_dir + chainName + "_tagEFFSpre_Turn_On_Curve_wrt_probe_MuidCB_Barrel_Numerator";
        std::string histDenB = nd_dir + chainName + "_tagEFFSpre_Turn_On_Curve_wrt_probe_MuidCB_Barrel_Denominator";
        std::string histNumE = nd_dir + chainName + "_tagEFFSpre_Turn_On_Curve_wrt_probe_MuidCB_Endcap_Numerator";
        std::string histDenE = nd_dir + chainName + "_tagEFFSpre_Turn_On_Curve_wrt_probe_MuidCB_Endcap_Denominator";
        std::string histL1sum = eff_dir + chainName + "_EFplateau_wrtOffline";
        const auto & [h1numb, h1denb] = getHistogramPairQuietly(histNumB, histDenB);
        if (!h1numb) continue;
        
        const auto & [h1nume, h1dene] = getHistogramPairQuietly(histNumE, histDenE);
        if (!h1nume) continue;

        TH1F* h1sumL = getHPointerQuietly(histL1sum);
        if (!h1sumL) continue;
        //
        int iSTDL = 75;//25GeV
        int iSTDH = 120;
        if (HI_pp_key) {//HI run 4-25GeV
          iSTDL = 17;
          iSTDH = 75;
        }
        double sumeff {}, sumerr {};
        double sumn = h1numb->Integral(iSTDL, iSTDH); // 60-100 GeV
        double sumd = h1denb->Integral(iSTDL, iSTDH);
        if (sumd != 0.) {
          sumeff = (double) sumn / (double) sumd;
          sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
        }

        h1sumL->SetBinContent(1, sumeff);
        h1sumL->SetBinError(1, sumerr);

        sumn = h1nume->Integral(iSTDL, iSTDH);
        sumd = h1dene->Integral(iSTDL, iSTDH);
        if (sumd == 0.) {
          sumeff = 0.;
          sumerr = 0.;
        } else {
          sumeff = (double) sumn / (double) sumd;
          sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
        }
        h1sumL->GetYaxis()->SetTitle("Efficiency");
        h1sumL->SetBinContent(2, sumeff);
        h1sumL->SetBinError(2, sumerr);
        h1sumL->SetMinimum(0.0);
        h1sumL->SetMaximum(1.05);

        dir->cd();
        h1sumL->Write("", TObject::kOverwrite);

        std::string histNum_mu0_15 = nd_dir + chainName + "_tagEFFSpre_mu0_15_Turn_On_Curve_wrt_probe_MuidCB_Numerator";
        std::string histDen_mu0_15 = nd_dir + chainName + "_tagEFFSpre_mu0_15_Turn_On_Curve_wrt_probe_MuidCB_Denominator";
        std::string histNum_mu15_20 = nd_dir + chainName + "_tagEFFSpre_mu15_20_Turn_On_Curve_wrt_probe_MuidCB_Numerator";
        std::string histDen_mu15_20 = nd_dir + chainName + "_tagEFFSpre_mu15_20_Turn_On_Curve_wrt_probe_MuidCB_Denominator";
        std::string histNum_mu20 = nd_dir + chainName + "_tagEFFSpre_mu20_Turn_On_Curve_wrt_probe_MuidCB_Numerator";
        std::string histDen_mu20 = nd_dir + chainName + "_tagEFFSpre_mu20_Turn_On_Curve_wrt_probe_MuidCB_Denominator";
        std::string histEFsum_mu = eff_dir + chainName + "_EFplateau_wrtOffline_mu_dependence";

        TH1F* h1num_mu0_15 = getHPointerQuietly(histNum_mu0_15);
        if (!h1num_mu0_15) continue;

        TH1F* h1num_mu15_20 = getHPointerQuietly(histNum_mu15_20);
        if (!h1num_mu15_20) continue;

        TH1F* h1num_mu20 = getHPointerQuietly(histNum_mu20);
        if (!h1num_mu20) continue;

        TH1F* h1den_mu0_15 = getHPointerQuietly(histDen_mu0_15);
        if (!h1den_mu0_15) continue;

        TH1F* h1den_mu15_20 = getHPointerQuietly(histDen_mu15_20);
        if (!h1den_mu15_20) continue;

        TH1F* h1den_mu20 = getHPointerQuietly(histDen_mu20);
        if (!h1den_mu20) continue;

        TH1F* h1sum_mu = getHPointerQuietly(histEFsum_mu);
        if (!h1sum_mu) continue;

        sumn = h1num_mu0_15->Integral(iSTDL, iSTDH); // 25-100 GeV
        sumd = h1den_mu0_15->Integral(iSTDL, iSTDH);
        if (sumd == 0.) {
          sumeff = 0.;
          sumerr = 0.;
        } else {
          sumeff = (double) sumn / (double) sumd;
          sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
        }

        h1sum_mu->SetBinContent(1, sumeff);
        h1sum_mu->SetBinError(1, sumerr);

        sumn = h1num_mu15_20->Integral(iSTDL, iSTDH);
        sumd = h1den_mu15_20->Integral(iSTDL, iSTDH);
        if (sumd == 0.) {
          sumeff = 0.;
          sumerr = 0.;
        } else {
          sumeff = (double) sumn / (double) sumd;
          sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
        }
        h1sum_mu->SetBinContent(2, sumeff);
        h1sum_mu->SetBinError(2, sumerr);

        sumn = h1num_mu20->Integral(iSTDL, iSTDH);
        sumd = h1den_mu20->Integral(iSTDL, iSTDH);
        if (sumd == 0.) {
          sumeff = 0.;
          sumerr = 0.;
        } else {
          sumeff = (double) sumn / (double) sumd;
          sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
        }
        h1sum_mu->GetYaxis()->SetTitle("Efficiency");
        h1sum_mu->SetBinContent(3, sumeff);
        h1sum_mu->SetBinError(3, sumerr);
        h1sum_mu->SetMaximum(1.05);
        h1sum_mu->SetMinimum(0.0);
        dir->cd();
        h1sum_mu->Write("", TObject::kOverwrite);
        mf.Write();
      }
      //  end of the code add by Yuan //
      // ******************************************************//
      static const std::string alg2[3] = {
        "_L2MuonSA", "_MuonEFMS", "_MuonEFSA"
      };
      static const std::string wrtalg2[3] = {
        "_L1", "_L2MuonSA", "_L2MuonSA"
      };

      // ******************************************************//
      // ******************  MSonly Chains ********************//
      // ******************************************************//
      for (unsigned int i = 0; i < chainsMSonly.size(); i++) {
        std::string chainName = chainsMSonly.at(i);

#if 0
        for (int trg = 0; trg < maxindep; trg++) {
          sden = nd_dir + chainName + "_Turn_On_Curve_wrt_MuidSA" + trigger[trg] + "_Triggered_Denominator";

          for (int alg = 0; alg < 3; alg++) {
            snum = nd_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + trigger[trg] + "_Triggered_Numerator";
            seff = eff_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + trigger[trg] + "_Triggered";
            seffg = seff + "_Fit"; // YY added 20.04.10
            stmp = chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + trigger[trg] + "_Triggered";
            stmpg = chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + trigger[trg] + "_Triggered" + "_Fit";
            const auto & [h1num, h1den] = getHistogramPair(snum, sden);
            if (!h1num) continue;

            h1tmp = (TH1F*) h1den->Clone();
            h1tmp->SetName(stmp);
            h1tmp->SetTitle(stmp);
            h1tmp->GetYaxis()->SetTitle("Efficiency");
            h1tmp->Reset();
            h1tmp->Divide(h1num, h1den, 1., 1., "B");
            dir->cd();
            h1tmp->Write();
            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg);
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num, h1den);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
            dir->cd();
            h1tmpg->Write();
            delete h1tmpg;

            // summary for jet trigger
            if (0 == alg || 2 == alg) {
              if (3 == trg) { // jet
                double sumeff {}, sumerr {};
                double sumn = h1num->Integral(iMSL, iMSH);
                double sumd = h1den->Integral(iMSL, iMSH);
                if (sumd != 0.) {
                  sumeff = (double) sumn / (double) sumd;
                  sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
                }
                int iholx = -1;
                if (0 == alg) {
                  iholx = static_cast<int>(iL2MuonSA);
                } else if (2 == alg) {
                  iholx = static_cast<int>(iEFSA);
                }

                if (iholx >= 0) {
                  std::string s = eff_dir + chainName + "_highpt_effsummary_by" + vectkwd.at(2);
                  h1sumeff = getHPointer(s);
                  if (!h1sumeff) continue;

                  h1sumeff->SetBinContent(iholx + 1, sumeff);
                  h1sumeff->SetBinError(iholx + 1, sumerr);
                  h1sumeff->GetYaxis()->SetTitleOffset(1.3);
                  h1sumeff->SetMinimum(0.0);
                  h1sumeff->SetMaximum(1.05);
                  // saving
                  dir->cd();
                  h1sumeff->Write("", TObject::kOverwrite);
                }
              }
            }
          }//alg
        }//trg
#endif
        mf.Write();

        for (int alg = 0; alg < 3; alg++) {
          //wrt MuidSA
          sden = nd_dir + chainName + "_Turn_On_Curve_wrt_MuidSA_Denominator";
          snum = nd_dir + chainName + alg2[alg] + "_Turn_On_Curve_Numerator";
          seff = eff_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA";
          seffg = seff + "_Fit"; // YY added 20.04.10
          stmp = chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA";
          stmpg = chainName + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + "_Fit";
          const auto & [pNum, pDen] = getHistogramPair(snum, sden);
          if (!pNum) continue;

          h1tmp = (TH1F*) pDen->Clone();
          h1tmp->SetName(stmp.c_str());
          h1tmp->SetTitle(stmp.c_str());
          h1tmp->GetYaxis()->SetTitle("Efficiency");
          h1tmp->Reset();
          h1tmp->Divide(pNum, pDen, 1., 1., "B");
          dir->cd();
          h1tmp->Write();
          h1tmpg = new TGraphAsymmErrors();
          h1tmpg->SetName(stmpg.c_str());
          h1tmpg->SetMarkerStyle(20);
          h1tmpg->SetMinimum(0.0);
          h1tmpg->SetMaximum(1.05);
          h1tmpg->BayesDivide(pNum, pDen);
          h1tmpg->GetYaxis()->SetTitle("Efficiency");
          h1tmpg->GetXaxis()->SetTitle(pDen->GetXaxis()->GetTitle());
          dir->cd();
          h1tmpg->Write();
          delete h1tmpg;

          if (0 == alg || 2 == alg) { // no condition on ES bits = all events
            double sumeff {}, sumerr {};
            double sumn = pNum->Integral(iMSL, iMSH);
            double sumd = pDen->Integral(iMSL, iMSH);
            if (sumd != 0.) {
              sumeff = (double) sumn / (double) sumd;
              sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
            }
            int iholx = -1;
            if (0 == alg) {
              iholx = static_cast<int>(iL2MuonSA);
            } else if (2 == alg) {
              iholx = static_cast<int>(iEFSA);
            }

            if (iholx >= 0) {
              std::string s = eff_dir + chainName + "_highpt_effsummary_by" + vectkwd.at(3);
              h1sumeff = getHPointer(s);
              if (!h1sumeff) continue;
              h1sumeff->GetYaxis()->SetTitleOffset(1.3);
              h1sumeff->SetBinContent(iholx + 1, sumeff);
              h1sumeff->SetBinError(iholx + 1, sumerr);
              h1sumeff->SetMinimum(0.0);
              h1sumeff->SetMaximum(1.05);
              // saving
              dir->cd();
              h1sumeff->Write("", TObject::kOverwrite);
            }
          }
          //wrt MuidSA

          // for ES ----------------------------------------------------------------
          for (int ies = 0; ies <= maxESbr; ies++) {
            if (!MS_mon_ESbr[ies]) continue;
            // for ES, L1 ------------------------------------------------------------
            if (0 == alg) {
              sden = nd_dir + chainName + triggerES[ies] + "_Turn_On_Curve_wrt_MuidSA_Denominator";
              snum = nd_dir + chainName + triggerES[ies] + "_L2MuonSA" + "_Turn_On_Curve_wrt" + "_L1" + "_Denominator";
              seff = eff_dir + chainName + triggerES[ies] + "_L1" + "_Turn_On_Curve_wrt_MuidSA";
              seffg = seff + "_Fit";
              stmp = chainName + triggerES[alg] + "_L1" + "_Turn_On_Curve_wrt_MuidSA";
              stmpg = chainName + triggerES[alg] + "_L1" + "_Turn_On_Curve_wrt_MuidSA" + "_Fit";
              const auto & [h1num, h1den] = getHistogramPair(snum, sden);
              if (!h1num) continue;

              h1tmp = (TH1F*) h1den->Clone();
              h1tmp->SetName(stmp.c_str());
              h1tmp->SetTitle(stmp.c_str());
              h1tmp->GetYaxis()->SetTitle("Efficiency");
              h1tmp->Reset();
              h1tmp->Divide(h1num, h1den, 1., 1., "B");
              dir->cd();
              h1tmp->Write();
              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num, h1den);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
              dir->cd();
              h1tmpg->Write();
              delete h1tmpg;


              for (int be = 0; be < 2; be++) {
                sden = nd_dir + chainName + triggerES[ies] + "_Turn_On_Curve_wrt_MuidSA" + bestr[be] + "_Denominator";
                snum = nd_dir + chainName + triggerES[ies] + "_L2MuonSA" + "_Turn_On_Curve_wrt" + "_L1" + bestr[be] +
                       "_Denominator";
                seff = eff_dir + chainName + triggerES[ies] + "_L1" + bestr[be] + "_Turn_On_Curve_wrt_MuidSA";
                seffg = seff + "_Fit";
                stmp = chainName + triggerES[ies] + "_L1" + bestr[be] + "_Turn_On_Curve_wrt_MuidSA";
                stmpg = chainName + triggerES[ies] + "_L1" + bestr[be] + "_Turn_On_Curve_wrt_MuidSA" + "_Fit";
                const auto & [h1num, h1den] = getHistogramPair(snum, sden);
                if (!h1num) continue;

                h1tmp = (TH1F*) h1den->Clone();
                h1tmp->SetName(stmp.c_str());
                h1tmp->SetTitle(stmp.c_str());
                h1tmp->GetYaxis()->SetTitle("Efficiency");
                h1tmp->Reset();
                h1tmp->Divide(h1num, h1den, 1., 1., "B");
                dir->cd();
                h1tmp->Write();
                h1tmpg = new TGraphAsymmErrors();
                h1tmpg->SetName(stmpg.c_str());
                h1tmpg->SetMarkerStyle(20);
                h1tmpg->SetMinimum(0.0);
                h1tmpg->SetMaximum(1.05);
                h1tmpg->BayesDivide(h1num, h1den);
                h1tmpg->GetYaxis()->SetTitle("Efficiency");
                h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
                dir->cd();
                h1tmpg->Write();
                delete h1tmpg;
              }
            }

            // for ES, L1 end ------------------------------------------------------------
            sden = nd_dir + chainName + triggerES[ies] + "_Turn_On_Curve_wrt_MuidSA_Denominator";
            snum = nd_dir + chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_Numerator";
            seff = eff_dir + chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA";
            seffg = seff + "_Fit"; // YY added 20.04.10
            stmp = chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA";
            stmpg = chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt_MuidSA" + "_Fit";
            const auto & [h1num, h1den] = getHistogramPair(snum, sden);
            if (!h1num) continue;

            h1tmp = (TH1F*) h1den->Clone();
            h1tmp->SetName(stmp.c_str());
            h1tmp->SetTitle(stmp.c_str());
            h1tmp->GetYaxis()->SetTitle("Efficiency");
            h1tmp->Reset();
            h1tmp->Divide(h1num, h1den, 1., 1., "B");
            dir->cd();
            h1tmp->Write();
            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg.c_str());
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num, h1den);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
            dir->cd();
            h1tmpg->Write();
            delete h1tmpg;

            if (0 == alg || 2 == alg) {
              if (ESTAG == ies || ESINDEP == ies) {
                double sumeff, sumerr;
                double sumn = h1num->Integral(iMSL, iMSH);
                double sumd = h1den->Integral(iMSL, iMSH);
                if (sumd == 0.) {
                  sumeff = 0.;
                  sumerr = 0.;
                } else {
                  sumeff = (double) sumn / (double) sumd;
                  sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
                }
                int iholx = -1;
                if (0 == alg) {
                  iholx = static_cast<int>(iL2MuonSA);
                } else if (2 == alg) {
                  iholx = static_cast<int>(iEFSA);
                }

                if (iholx >= 0) {
                  std::string s = eff_dir + chainName + "_highpt_effsummary_by" + triggerES[ies];
                  h1sumeff = getHPointer(s);
                  if (!h1sumeff) continue;
                  h1sumeff->GetYaxis()->SetTitleOffset(1.3);
                  h1sumeff->SetBinContent(iholx + 1, sumeff);
                  h1sumeff->SetBinError(iholx + 1, sumerr);
                  h1sumeff->SetMinimum(0.0);
                  h1sumeff->SetMaximum(1.05);
                  // saving
                  dir->cd();
                  h1sumeff->Write("", TObject::kOverwrite);
                }
              }
            }
          }
          // for ES: end ----------------------------------------------------------------

          if (0 == alg || 2 == alg) {
            for (int be = 0; be < 2; be++) {
              //wrt MuidSA
              sden = nd_dir + chainName + "_Turn_On_Curve_wrt_MuidSA" + bestr[be] + "_Denominator";
              snum = nd_dir + chainName + alg2[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              seff = eff_dir + chainName + alg2[alg] + bestr[be] + "_Turn_On_Curve_wrt_MuidSA";
              seffg = seff + "_Fit"; // YY added 20.04.10
              stmp = chainName + alg2[alg] + bestr[be] + "_Turn_On_Curve_wrt_MuidSA";
              stmpg = chainName + alg2[alg] + bestr[be] + "_Turn_On_Curve_wrt_MuidSA" + "_Fit";
              const auto & [h1num, h1den] = getHistogramPair(snum, sden);
              if (!h1num) continue;

              h1tmp = (TH1F*) h1den->Clone();
              h1tmp->SetName(stmp.c_str());
              h1tmp->SetTitle(stmp.c_str());
              h1tmp->GetYaxis()->SetTitle("Efficiency");
              h1tmp->Reset();
              h1tmp->Divide(h1num, h1den, 1., 1., "B");
              dir->cd();
              h1tmp->Write();
              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num, h1den);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
              dir->cd();
              h1tmpg->Write();
              delete h1tmpg;
            }
          }

          //wrt upstream
          sden = nd_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + "_Denominator";
          snum = nd_dir + chainName + alg2[alg] + "_Turn_On_Curve_Numerator";
          seff = eff_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg];
          seffg = seff + "_Fit"; // YY added 20.04.10
          stmp = chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg];
          stmpg = chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + "_Fit";
          const auto & [h1num, h1den] = getHistogramPair(snum, sden);
          if (!h1num) continue;

          h1tmp = (TH1F*) h1den->Clone();
          h1tmp->SetName(stmp.c_str());
          h1tmp->SetTitle(stmp.c_str());
          h1tmp->GetYaxis()->SetTitle("Efficiency");
          h1tmp->Reset();
          h1tmp->Divide(h1num, h1den, 1., 1., "B");
          dir->cd();
          h1tmp->Write();
          h1tmpg = new TGraphAsymmErrors();
          h1tmpg->SetName(stmpg.c_str());
          h1tmpg->SetMarkerStyle(20);
          h1tmpg->SetMinimum(0.0);
          h1tmpg->SetMaximum(1.05);
          h1tmpg->BayesDivide(h1num, h1den);
          h1tmpg->GetYaxis()->SetTitle("Efficiency");
          h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
          dir->cd();
          h1tmpg->Write();
          delete h1tmpg;


          //wrt upstream
          // for ES --------------------------------------------------------------------
          for (int ies = 0; ies <= maxESbr; ies++) {
            if (!MS_mon_ESbr[ies]) continue;
            sden = nd_dir + chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] +
                   "_Denominator";
            snum = nd_dir + chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_Numerator";
            seff = eff_dir + chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg];
            seffg = seff + "_Fit"; // YY added 20.04.10
            stmp = chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg];
            stmpg = chainName + triggerES[ies] + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + "_Fit";
            const auto & [h1num, h1den] = getHistogramPair(snum, sden);
            if (!h1num) continue;

            h1tmp = (TH1F*) h1den->Clone();
            h1tmp->SetName(stmp.c_str());
            h1tmp->SetTitle(stmp.c_str());
            h1tmp->GetYaxis()->SetTitle("Efficiency");
            h1tmp->Reset();
            h1tmp->Divide(h1num, h1den, 1., 1., "B");
            dir->cd();
            h1tmp->Write();
            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg.c_str());
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num, h1den);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
            dir->cd();
            h1tmpg->Write();
            delete h1tmpg;
          }
          // for ES: end --------------------------------------------------------------------

          if (0 == alg || 2 == alg) {
            for (int be = 0; be < 2; be++) {
              //wrt upstream
              sden = nd_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + bestr[be] + "_Denominator";
              snum = nd_dir + chainName + alg2[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              seff = eff_dir + chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + bestr[be];
              seffg = seff + "_Fit"; // YY added 20.04.10
              stmp = chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + bestr[be];
              stmpg = chainName + alg2[alg] + "_Turn_On_Curve_wrt" + wrtalg2[alg] + bestr[be] + "_Fit";
              const auto & [h1num, h1den] = getHistogramPair(snum, sden);
              if (!h1num) continue;

              h1tmp = (TH1F*) h1den->Clone();
              h1tmp->SetName(stmp.c_str());
              h1tmp->SetTitle(stmp.c_str());
              h1tmp->GetYaxis()->SetTitle("Efficiency");
              h1tmp->Reset();
              h1tmp->Divide(h1num, h1den, 1., 1., "B");
              dir->cd();
              h1tmp->Write();
              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num, h1den);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
              dir->cd();
              h1tmpg->Write();
              delete h1tmpg;
            }
          }
        }//alg
        mf.Write();
      }//i

      //==Rate
      dir = mf.GetDirectory(rr_dir.c_str());
      if (!dir) {
        if (fdbg) {
          std::cerr << "HLTMuonHistogramDivision: directory " << rr_dir << " not found" << std::endl;
        }
        return;
      }

      //  EF rate / offline rate
      static const std::string type[3] = {
        "MS_", "SA_", "CB_"
      };
      static const std::string off[3] = {
        "Moore_MS_", "Moore_SA_", "Muid_"
      };
      static const std::string cut[2] = {
        "4", "10"
      };

      for (int itype = 0; itype < 3; itype++) {
        for (int icut = 0; icut < 2; icut++) {
          seff = rr_dir + "EF_" + type[itype] + "Over_" + off[itype] + cut[icut] + "GeV_Cut";
          snum = rate_dir + "Number_Of_EF_" + type[itype] + "Muons_" + cut[icut] + "GeV_Cut";
          sden = rate_dir + "Number_Of_" + off[itype] + "Muons_" + cut[icut] + "GeV_Cut";
          stmp = "EF_" + type[itype] + "Over_" + off[itype] + cut[icut] + "GeV_Cut";
          stmpg = "EF_" + type[itype] + "Over_" + off[itype] + cut[icut] + "GeV_Cut" + "_Fit";
          const auto & [h1num, h1den] = getHistogramPair(snum, sden);
          if (!h1num) continue;

          h1tmp = (TH1F*) h1den->Clone();
          h1tmp->SetName(stmp.c_str());
          h1tmp->SetTitle(stmp.c_str());
          h1tmp->GetYaxis()->SetTitle("Efficiency");
          h1tmp->Reset();
          h1tmp->Divide(h1num, h1den, 1., 1., "B");
          dir->cd();
          h1tmp->Write();
        }//icut
      }//itype
      mf.Write();

      // Triggers / Event
      std::vector<std::string> chains;
      std::vector<std::string> chains2;

      {
        std::map<std::string, std::string> ztpmap;

        ztpmap["muChain1"] = "L1_MU15";
        ztpmap["muChain2"] = "L1_MU15";
        ztpmap["muChainEFiso1"] = "L1_MU15";
        ztpmap["muChainEFiso2"] = "L1_MU15";
        ztpmap["muChainMSonly1"] = "L1_MU15";
        ztpmap["muChainMSonly2"] = "L1_MU15";

        std::map<std::string, int> ztp_isomap;
        ztp_isomap["muChain1"] = 0;
        ztp_isomap["muChain2"] = 0;
        ztp_isomap["muChainEFiso1"] = 1;
        ztp_isomap["muChainEFiso2"] = 1;
        ztp_isomap["muChainMSonly1"] = 0;
        ztp_isomap["muChainMSonly2"] = 0;

        for (std::map<std::string, std::string>::iterator itmap = ztpmap.begin(); itmap != ztpmap.end(); ++itmap) {
          std::string histdirmuztp = run_dir + "/HLT/MuonMon/MuZTP/" + itmap->first;
          TDirectory* ztpdir = mf.GetDirectory(histdirmuztp.c_str());
          bool isefisochain = ztp_isomap[itmap->first] > 0;

          //efficiency histograms
          std::vector<std::string> var;
          var.push_back("_Pt_");
          var.push_back("_Pt_EC_");
          var.push_back("_Pt_B_");
          var.push_back("_Pt_4bins_");
          var.push_back("_Pt_B_4bins_");
          var.push_back("_Eta_");
          var.push_back("_Phi_");
          var.push_back("_Phi_EC_");
          var.push_back("_Phi_B_");
          for (unsigned int k = 0; k < var.size(); k++) {
            std::vector<std::string> level;
            level.push_back("L1");
            level.push_back("L2");
            level.push_back("EF");
            if (isefisochain) level.push_back("EFIso");
            for (unsigned int j = 0; j < level.size(); j++) {
              //ABSOLUTE
              seffg = histdirmuztp + "/muZTP_eff_" + level[j] + var[k] + itmap->first;
              snum = histdirmuztp + "/muZTP" + var[k] + level[j] + "fired_" + itmap->first;
              sden = histdirmuztp + "/muZTP" + var[k] + itmap->first;
              stmp = "muZTP_eff_" + level[j] + var[k] + itmap->first;
              stmpg = "muZTP_eff_" + level[j] + var[k] + itmap->first;
              const auto & [h1num, h1den] = getHistogramPair(snum, sden);
              if (!h1num) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num, h1den);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();

              seffg = histdirmuztp + "/muZTP_eff_" + level[j] + "_Eta_1bin_" + itmap->first;
              snum = histdirmuztp + "/muZTP_Eta_1bin_" + level[j] + "fired_" + itmap->first;
              sden = histdirmuztp + "/muZTP_Eta_1bin_" + itmap->first;
              stmpg = "muZTP_eff_" + level[j] + "_Eta_1bin_" + itmap->first;
              const auto & [h1num2, h1den2] = getHistogramPair(snum, sden);
              if (!h1num2) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num2, h1den2);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den2->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();

              seffg = histdirmuztp + "/muZTP_eff_" + level[j] + "_Eta_2bins_" + itmap->first;
              snum = histdirmuztp + "/muZTP_Eta_2bins_" + level[j] + "fired_" + itmap->first;
              sden = histdirmuztp + "/muZTP_Eta_2bins_" + itmap->first;
              stmpg = "muZTP_eff_" + level[j] + "_Eta_2bins_" + itmap->first;
              const auto & [h1num3, h1den3] = getHistogramPair(snum, sden);

              //h1num = getHPointer(snum);
              if (!h1num3) continue;
              //h1den = getHPointer(sden);
              //if (!h1den) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmp.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num3, h1den3);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den3->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();

              seffg = histdirmuztp + "/muZTP_eff_" + level[j] + "_Eta_1bin_cut_" + itmap->first;
              snum = histdirmuztp + "/muZTP_Eta_1bin_cut_" + level[j] + "fired_" + itmap->first;
              sden = histdirmuztp + "/muZTP_Eta_1bin_cut_" + itmap->first;
              stmpg = "muZTP_eff_" + level[j] + "_Eta_1bin_cut_" + itmap->first;
              const auto & [h1num4, h1den4] = getHistogramPair(snum, sden);
              if (!h1num4) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num4, h1den4);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den4->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();

              seffg = histdirmuztp + "/muZTP_eff_" + level[j] + "_Eta_2bins_cut_" + itmap->first;
              snum = histdirmuztp + "/muZTP_Eta_2bins_cut_" + level[j] + "fired_" + itmap->first;
              sden = histdirmuztp + "/muZTP_Eta_2bins_cut_" + itmap->first;
              stmpg = "muZTP_eff_" + level[j] + "_Eta_2bins_cut_" + itmap->first;
              const auto & [h1num5, h1den5] = getHistogramPair(snum, sden);
              if (!h1num5) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num5, h1den5);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den5->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();

              //2D ETA_PHI
              seff = histdirmuztp + "/muZTP_eff_EtaPhi_" + level[j] + "_" + itmap->first;
              snum = histdirmuztp + "/muZTP_EtaPhi_" + level[j] + "_" + itmap->first;
              sden = histdirmuztp + "/muZTP_EtaPhi_all_" + itmap->first;
              stmp = "muZTP_eff_EtaPhi_" + level[j] + "_" + itmap->first;

              TH2F* h2num(0);
              TH2F* h2den(0);
              TH2F* h2tmp(0);

              h2num = getH2Pointer(snum);
              if (!h2num) continue;
              h2den = getH2Pointer(sden);
              if (!h2den) continue;

              h2tmp = (TH2F*) h1den->Clone();
              h2tmp->SetName(stmp.c_str());
              h2tmp->SetTitle(stmp.c_str());
              h2tmp->GetYaxis()->SetTitle("Efficiency");
              h2tmp->Reset();
              h2tmp->Divide(h1num, h1den, 1., 1., "B");
              dir->cd();
              h2tmp->Write();
              mf.Write();
            }//level

            //RELATIVE
            seffg = histdirmuztp + "/muZTP_eff_EFwrtL2" + var[k] + itmap->first;
            snum = histdirmuztp + "/muZTP" + var[k] + "EFL2fired_" + itmap->first;
            sden = histdirmuztp + "/muZTP" + var[k] + "L2fired_" + itmap->first;
            stmpg = "muZTP_eff_EFwrtL2" + var[k] + itmap->first;
            const auto & [h1num, h1den] = getHistogramPair(snum, sden);
            if (!h1num) continue;

            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg.c_str());
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num, h1den);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
            ztpdir->cd();
            h1tmpg->Write();
            delete h1tmpg;
            mf.Write();

            seffg = histdirmuztp + "/muZTP_eff_EFwrtL1" + var[k] + itmap->first;
            snum = histdirmuztp + "/muZTP" + var[k] + "EFfired_" + itmap->first;
            sden = histdirmuztp + "/muZTP" + var[k] + "L1fired_" + itmap->first;
            stmpg = "muZTP_eff_EFwrtL1" + var[k] + itmap->first;
            const auto & [h1num2, h1den2] = getHistogramPair(snum, sden);
            if (!h1num2) continue;

            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg.c_str());
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num2, h1den2);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den2->GetXaxis()->GetTitle());
            ztpdir->cd();
            h1tmpg->Write();
            delete h1tmpg;
            mf.Write();

            seffg = histdirmuztp + "/muZTP_eff_L2wrtL1" + var[k] + itmap->first;
            snum = histdirmuztp + "/muZTP" + var[k] + "L2fired_" + itmap->first;
            sden = histdirmuztp + "/muZTP" + var[k] + "L1fired_" + itmap->first;
            stmpg = "muZTP_eff_L2wrtL1" + var[k] + itmap->first;
            const auto & [h1num3, h1den3] = getHistogramPair(snum, sden);
            if (!h1num3) continue;

            h1tmpg = new TGraphAsymmErrors();
            h1tmpg->SetName(stmpg.c_str());
            h1tmpg->SetMarkerStyle(20);
            h1tmpg->SetMinimum(0.0);
            h1tmpg->SetMaximum(1.05);
            h1tmpg->BayesDivide(h1num3, h1den3);
            h1tmpg->GetYaxis()->SetTitle("Efficiency");
            h1tmpg->GetXaxis()->SetTitle(h1den3->GetXaxis()->GetTitle());
            ztpdir->cd();
            h1tmpg->Write();
            delete h1tmpg;
            mf.Write();

            if (isefisochain) {
              seffg = histdirmuztp + "/muZTP_eff_EFIsowrtEF" + var[k] + itmap->first;
              snum = histdirmuztp + "/muZTP" + var[k] + "EFIsofired_" + itmap->first;
              sden = histdirmuztp + "/muZTP" + var[k] + "EFfired_" + itmap->first;
              stmpg = "muZTP_eff_EFIsowrtEF" + var[k] + itmap->first;
              const auto & [h1num, h1den] = getHistogramPair(snum, sden);
              if (!h1num) continue;

              h1tmpg = new TGraphAsymmErrors();
              h1tmpg->SetName(stmpg.c_str());
              h1tmpg->SetMarkerStyle(20);
              h1tmpg->SetMinimum(0.0);
              h1tmpg->SetMaximum(1.05);
              h1tmpg->BayesDivide(h1num, h1den);
              h1tmpg->GetYaxis()->SetTitle("Efficiency");
              h1tmpg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
              ztpdir->cd();
              h1tmpg->Write();
              delete h1tmpg;
              mf.Write();
            }
          }//var
        }
        // mf.Write();
      } // procMuZTP


      {
        TDirectory* efdir = mf.GetDirectory(eff_dir.c_str());
        if (!efdir) {
          if (fdbg) {
            std::cerr << "HLTMuonHistogramDivision: directory " << eff_dir << " not found" << std::endl;
          }
          return;
        }

        // MAM
        const int MAXARR = 6;
        static const std::string charr[MAXARR] = {
          "muChain1", "muChain2", "muChainEFiso1", "muChainEFiso2", "muChainMSonly1", "muChainMSonly2"
        };
        static const std::string monarr[MAXARR] = {
          "_EFmuon", "_EFmuon", "_EFmuon", "_EFmuon", "_MuonEFSA", "_MuonEFSA"
        };
        static const std::string monL2arr[MAXARR] = {
          "_L2MuonSA", "_L2MuonSA", "_L2MuonSA", "_L2MuonSA", "_L2MuonSA", "_L2MuonSA"
        };
        bool isBarrelMon[MAXARR] = {
          false, false, false, false, true, true
        }; // enable MSonly
        bool isMSbMon[MAXARR] = {
          true, true, false, false, false, false
        }; // Skip isol and MSonly
        bool monL1[MAXARR] = {
          true, true, true, true, false, false
        }; // Skip MSonly
        bool isefIsolation[MAXARR] = {
          false, false, true, true, false, false
        }; // EF isolation  add by Yuan

        // I need to prepare maps ... mu20_MG -> MuGirlEF etc.
        for (int ialg = 0; ialg < MAXARR; ialg++) {
          std::string chainName = charr[ialg];
          std::string MoniAlg = monarr[ialg];
          std::string MoniL2Alg = monL2arr[ialg];

          /* 1. Picking up ztp graph */
          std::string hdirztp = muon_dir + "MuZTP/" + chainName + "/";
          std::string histZtpNum, histZtpDen;
          if (!isBarrelMon[ialg]) {
            histZtpNum = hdirztp + "muZTP_Pt_4bins_EFfired_" + chainName;
            histZtpDen = hdirztp + "muZTP_Pt_4bins_L1fired_" + chainName;
          } else {
            histZtpNum = hdirztp + "muZTP_Pt_B_4bins_EFfired_" + chainName;
            histZtpDen = hdirztp + "muZTP_Pt_B_4bins_L1fired_" + chainName;
          }
          if (isefIsolation[ialg]) histZtpNum = hdirztp + "muZTP_Pt_4bins_EFIsofired_" + chainName; // add by Yuan
          std::string histZtpEff = eff_dir + chainName + "_highpt3bins_effwrtL1";
          const auto & [h1num, h1den] = getHistogramPair(histZtpNum, histZtpDen);
          if (!h1num) continue;
          h1eff = getHPointer(histZtpEff);
          if (!h1eff) continue;

          /* 2. Filling summary histogram from ZTP values */
          for (int ibin = 2; ibin <= 3; ibin++) {
            // at the moment it is not correct if we run the algorithm # 4: mu40_MSonly_barrel .
            double sumeff {}, sumerr {};
            double sumn = h1num->Integral(ibin, ibin); ////
            if (isBarrelMon[ialg] || isMSbMon[ialg]) sumn = h1num->Integral(ibin + 1, ibin + 1);
            double sumd = h1den->Integral(ibin, ibin); ////
            if (isBarrelMon[ialg] || isMSbMon[ialg]) sumd = h1den->Integral(ibin + 1, ibin + 1);
            if (sumd != 0.) {
              sumeff = (double) sumn / (double) sumd;
              sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
            }
            h1eff->GetYaxis()->SetTitle("Efficiency");
            h1eff->SetBinContent(ibin - 1, sumeff);  ////
            h1eff->SetBinError(ibin - 1, sumerr);    ////
            h1eff->SetMinimum(0.0);
            h1eff->SetMaximum(1.05);
          }



          efdir->cd();
          h1eff->Write("", TObject::kOverwrite);

          if (monL1[ialg]) { // MUST skip muGirl, muIso and MSonly as histograms are not defined!!!
            std::string hdirztp = muon_dir + "MuZTP/" + chainName + "/";
            std::string histZtpNumB = hdirztp + "muZTP_Pt_B_L1fired_" + chainName;
            std::string histZtpDenB = hdirztp + "muZTP_Pt_B_" + chainName;
            std::string histZtpNumE = hdirztp + "muZTP_Pt_EC_L1fired_" + chainName;
            std::string histZtpDenE = hdirztp + "muZTP_Pt_EC_" + chainName;
            std::string histZtpL1sum = eff_dir + chainName + "_highptL1plateau_wrtOffline";

            TH1F* h1numb = getHPointer(histZtpNumB);
            if (!h1numb) continue;
            TH1F* h1nume = getHPointer(histZtpNumE);
            if (!h1nume) continue;
            TH1F* h1denb = getHPointer(histZtpDenB);
            if (!h1denb) continue;
            TH1F* h1dene = getHPointer(histZtpDenE);
            if (!h1dene) continue;
            TH1F* h1sumL = getHPointer(histZtpL1sum);
            if (!h1sumL) continue;

            double sumeff, sumerr;
            double sumn = h1numb->Integral(13, 25); // 12-25 GeV
            if (HI_pp_key) sumn = h1numb->Integral(13, 20); // 60-100 GeV
            double sumd = h1denb->Integral(13, 25);
            if (HI_pp_key) sumd = h1denb->Integral(13, 20);
            if (sumd == 0.) {
              sumeff = 0.;
              sumerr = 0.;
            } else {
              sumeff = (double) sumn / (double) sumd;
              sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
            }
            h1sumL->SetBinContent(1, sumeff);
            h1sumL->SetBinError(1, sumerr);

            sumn = h1nume->Integral(13, 25);
            if (HI_pp_key) sumn = h1numb->Integral(13, 20); // 60-100 GeV
            sumd = h1dene->Integral(13, 25);
            if (HI_pp_key) sumd = h1denb->Integral(13, 20);
            if (sumd == 0.) {
              sumeff = 0.;
              sumerr = 0.;
            } else {
              sumeff = (double) sumn / (double) sumd;
              sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
            }
            h1sumL->GetYaxis()->SetTitle("Efficiency");
            h1sumL->SetBinContent(2, sumeff);
            h1sumL->SetBinError(2, sumerr);
            h1sumL->SetMinimum(0.0);
            h1sumL->SetMaximum(1.05);
            efdir->cd();
            h1sumL->Write("", TObject::kOverwrite);
          }
        }
        mf.Write();
      } //procChainDQA_HighPt
        //===End HLTMuonw

      // ******************************************************//
      // *********************  generic ***********************//
      // ******************************************************//
      std::string monalg[3] = {
        "_L2MuonSA", "_MuComb", "_EFmuon"
      };
      std::string wrtalg[3] = {
        "_L1", "_L2MuonSA", "_MuComb"
      };
      std::string numer, denom, effi;
      std::string histdireff = eff_dir;

      TDirectory* efdir = mf.GetDirectory(eff_dir.c_str());
      if (!efdir) {
        if (fdbg) {
          std::cerr << "HLTMuonHistogramDivision: directory " << eff_dir << " not found" << std::endl;
        }
        return;
      }

      // processing Iso histograms in the same manner
      chainsGeneric.insert(chainsGeneric.end(), chainsEFiso.begin(), chainsEFiso.end());

      for (unsigned int i = 0; i < chainsGeneric.size(); i++) {
        std::string chainName = chainsGeneric.at(i);
        if (fdbg) {
          std::cout << "proc generic " << chainName << std::endl;
        }

        //wrt CB muon && Upstream trigger
        for (int alg = 0; alg < 3; alg++) {
          denom = chainName + "_Turn_On_Curve_wrt_MuidCB_Denominator";
          numer = chainName + monalg[alg] + "_Turn_On_Curve_Numerator";
          effi = chainName + monalg[alg] + "_Turn_On_Curve_wrt_MuidCB";
          HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");

          denom = chainName + MSchainName + "_Turn_On_Curve_wrt_MuidCB_Denominator";
          numer = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_Numerator";
          effi = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_wrt_MuidCB";
          HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");

          // Summary all - removed

          denom = chainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + "_Denominator";
          numer = chainName + monalg[alg] + "_Turn_On_Curve_Numerator";
          effi = chainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg];
          HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");


          denom = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + "_Denominator";
          numer = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_Numerator";
          effi = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg];
          HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");

          //  for ES ------------------------------------------------------------------------------------
          for (int i = 0; i <= maxESbr; i++) {
            if (!CB_mon_ESbr[i]) continue;
            if (0 == alg) {
              // L1 efficiency: new for 2011 HI runs and afterward
              // only division once since it is "the zero-th" algorithm
              denom = chainName + triggerES[i] + "_Turn_On_Curve_wrt_MuidCB_Denominator";
              numer = chainName + triggerES[i] + "_L2MuonSA" + "_Turn_On_Curve_wrt" + "_L1" + "_Denominator";
              effi = chainName + triggerES[i] + "_L1" + "_Turn_On_Curve_wrt_MuidCB";
              HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");

              // Need to implement barrel and endcap ...
              for (int be = 0; be < 2; be++) {
                denom = chainName + triggerES[i] + "_Turn_On_Curve_wrt_MuidCB" + bestr[be] + "_Denominator";
                numer = chainName + triggerES[i] + "_L2MuonSA" + "_Turn_On_Curve_wrt" + "_L1" + bestr[be] +
                        "_Denominator";
                effi = chainName + triggerES[i] + "_L1" + bestr[be] + "_Turn_On_Curve_wrt_MuidCB";
                HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
                const auto & [h1num, h1den] = getHistogramPair(nd_dir + numer, nd_dir + denom);
                if (!h1num) continue;

                if (ESINDEP == i) {
                  // integrating over and fill in a summary histogram
                  double sumeff {}, sumerr {};
                  double sumn = h1num->Integral(iSTDL, iSTDH); // 60-100 GeV
                  double sumd = h1den->Integral(iSTDL, iSTDH);

                  if (sumd != 0.) {
                    sumeff = (double) sumn / (double) sumd;
                    sumerr = sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
                  }

                  std::string s = histdireff + chainName + "_L1plateau_wrtOffline_by_ESindep";
                  TH1F* h1effL1 = getHPointer(s);
                  if (!h1effL1) continue;
                  h1effL1->SetBinContent(be + 1, sumeff);
                  h1effL1->SetBinError(be + 1, sumerr);
                  h1eff->SetMinimum(0.0);
                  h1eff->SetMaximum(1.05);
                  efdir->cd();
                  h1effL1->Write("", TObject::kOverwrite);
                }
              }
            }

            denom = chainName + triggerES[i] + "_Turn_On_Curve_wrt_MuidCB_Denominator";
            numer = chainName + triggerES[i] + monalg[alg] + "_Turn_On_Curve_Numerator";
            effi = chainName + triggerES[i] + monalg[alg] + "_Turn_On_Curve_wrt_MuidCB";
            HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
            const auto & [h1num, h1den] = getHistogramPair(nd_dir + numer, nd_dir + denom);

            // Summary ESid and ESindep

            if (!h1num) continue;
            if (0 == alg || 1 == alg || 2 == alg) {
              double sumd {}, sumn {}, sumeff {}, sumerr {};
              sumn = h1num->Integral(iSTDL, iSTDH);
              sumd = h1den->Integral(iSTDL, iSTDH);

              if (sumd != 0.) {
                sumeff = (double) sumn / (double) sumd;
                sumerr = std::sqrt((double) sumn * (1. - sumeff)) / (double) sumd;
              }
              int iholx = -1;
              if (0 == alg) {
                iholx = static_cast<int>(iL2MuonSA);
              } else if (1 == alg) {
                iholx = static_cast<int>(iMuComb);
              } else if (2 == alg) {
                iholx = static_cast<int>(iEFCB);
              }

              std::string s = histdireff + chainName + "_highpt_effsummary_by" + triggerES[i];
              h1effsum = getHPointer(s);
              if (!h1effsum) continue;
              //
              h1effsum->GetYaxis()->SetTitleOffset(1.3);
              h1effsum->SetBinContent(iholx + 1, sumeff);
              h1effsum->SetBinError(iholx + 1, sumerr);
              h1effsum->SetMinimum(0.0);
              h1effsum->SetMaximum(1.05);
              efdir->cd();
              h1effsum->Write("", TObject::kOverwrite);
            }

            denom = chainName + triggerES[i] + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + "_Denominator";
            numer = chainName + triggerES[i] + monalg[alg] + "_Turn_On_Curve_Numerator";
            effi = chainName + triggerES[i] + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg];
            HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
          }
          //  for ES : END ------------------------------------------------------------------------------

          // Barrel/Endcap
          if (0 == alg || 1 == alg || 2 == alg) {
            for (int be = 0; be < 2; be++) {
              denom = chainName + "_Turn_On_Curve_wrt_MuidCB" + bestr[be] + "_Denominator";
              numer = chainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              effi = chainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_wrt_MuidCB";
              HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
            }

            for (int be = 0; be < 2; be++) {
              denom = chainName + MSchainName + "_Turn_On_Curve_wrt_MuidCB" + bestr[be] + "_Denominator";
              numer = chainName + MSchainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              effi = chainName + MSchainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_wrt_MuidCB";
              HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
            }

            for (int be = 0; be < 2; be++) {
              denom = chainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + bestr[be] + "_Denominator";
              numer = chainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              effi = chainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + bestr[be];
              HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
            }


            for (int be = 0; be < 2; be++) {
              denom = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + bestr[be] +
                      "_Denominator";
              numer = chainName + MSchainName + monalg[alg] + bestr[be] + "_Turn_On_Curve_Numerator";
              effi = chainName + MSchainName + monalg[alg] + "_Turn_On_Curve_wrt" + wrtalg[alg] + bestr[be];
              HLTMuonHDiv(mf, histdireff, numer, denom, effi, "_Fit");
            }
          }
        }
      }
    }//while (now just open bracket: 110728
  }//MonitoringFile::HLTMuonHistogramDivision

  void MonitoringFile::HLTMuonHDiv(PostProcessorFileWrapper& mf,
                                   const std::string& sdir, const std::string& snum, const std::string& sden, const std::string& seff, const std::string& seffg) {
    auto getHPointer ATLAS_THREAD_SAFE = [&mf] ATLAS_NOT_THREAD_SAFE(const std::string &hname)->TH1F * {
      TH1F* pH {};
      mf.get(hname.c_str(), pH);
      if (!pH and fdbg) {
        std::cerr << "HLTMuon PostProcessing: no such histogram!! " << hname << std::endl;
      }
      return pH;
    };
    TH1F* h1tmpf(0);
    TH1F* h1num(0);
    TH1F* h1den(0);
    TGraphAsymmErrors* h1tmpfg = new TGraphAsymmErrors();

    std::string stmp = seff + seffg;

    h1num = getHPointer(sdir + "NumDenom/" + snum);
    if (!h1num) return;

    h1den = getHPointer(sdir + "NumDenom/" + sden);
    if (!h1den) return;

    TDirectory* dir = mf.GetDirectory(sdir.c_str());

    h1tmpf = (TH1F*) h1den->Clone();
    h1tmpf->SetName(stmp.c_str());
    h1tmpf->SetTitle(stmp.c_str());
    h1tmpf->GetYaxis()->SetTitle("Efficiency");
    h1tmpf->Reset();
    h1tmpf->Divide(h1num, h1den, 1., 1., "B");
    h1tmpf->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
    h1tmpf->SetMinimum(0.0);
    h1tmpfg->SetMaximum(1.05);
    h1tmpf->SetName(stmp.c_str());
    dir->cd();
    h1tmpf->Write();
    h1tmpfg->SetMarkerStyle(20);
    h1tmpfg->SetMinimum(0.0);
    h1tmpfg->SetMaximum(1.05);
    h1tmpfg->BayesDivide(h1num, h1den);
    h1tmpfg->GetYaxis()->SetTitle("Efficiency");
    h1tmpfg->GetXaxis()->SetTitle(h1den->GetXaxis()->GetTitle());
    dir->cd();
    h1tmpfg->SetName(stmp.c_str());
    h1tmpfg->Write();
    delete h1tmpfg;
  }
}//namespace
