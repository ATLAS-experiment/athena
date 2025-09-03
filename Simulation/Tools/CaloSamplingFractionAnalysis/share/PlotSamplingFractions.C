// root -b 'PlotSamplingFractions.C("G4_11.3","SF_LAr.root","G4_11.3 new EMEC","../25.0_G4_11.3_newEMEC/SF_LAr.root")' 
// root -b 'PlotSamplingFractions.C("G4_10.6","SF_LAr_G4_10_3_ref.root","G4_11.3","SF_LAr.root")' 


#include "TFile.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TProfile.h"
#include "TH1.h"
#include "TF1.h"
#include "TLegend.h"

void SetParams(TF1* f,double val,double err)
{
  f->SetParameter(0,val);
  f->SetParError(0,err);
}

void SetParams(TF1* f,double val,double err,double val2,double err2)
{
  f->SetParameter(0,val);
  f->SetParError(0,err);
  f->SetParameter(1,val2);
  f->SetParError(1,err2);
}

/*
G4 10.6 sampling fractions from https://its.cern.ch/jira/browse/ATLASSIM-4747

Sampling fraction : 0.10 < #eta < 0.75 = 0.18489 +- 0.00001
Sampling fraction : 0.85 < #eta < 1.35 = 0.21601 +- 0.00002

Sampling fraction : 1.45 < #eta < 1.49 = +0.09498 +- 0.00009 + (|#eta|-1.470)*( 0.03047 +- 0.00648 )
Sampling fraction : 1.53 < #eta < 1.57 = +0.09557 +- 0.00007 + (|#eta|-1.550)*( 0.04595 +- 0.00566 )
Sampling fraction : 1.64 < #eta < 1.76 = +0.09951 +- 0.00004 + (|#eta|-1.700)*( 0.04946 +- 0.00140 )
Sampling fraction : 1.84 < #eta < 1.96 = +0.10289 +- 0.00003 + (|#eta|-1.900)*( 0.05894 +- 0.00105 )
Sampling fraction : 2.03 < #eta < 2.08 = +0.10797 +- 0.00005 + (|#eta|-2.055)*( 0.06241 +- 0.00344 )
Sampling fraction : 2.14 < #eta < 2.26 = +0.11072 +- 0.00003 + (|#eta|-2.200)*( 0.06868 +- 0.00123 )
Sampling fraction : 2.34 < #eta < 2.45 = +0.11673 +- 0.00004 + (|#eta|-2.395)*( 0.07870 +- 0.00138 )
Sampling fraction : 2.56 < #eta < 2.76 = +0.08444 +- 0.00002 + (|#eta|-2.660)*( 0.02680 +- 0.00032 )
Sampling fraction : 2.85 < #eta < 3.14 = +0.09057 +- 0.00002 + (|#eta|-2.995)*( 0.04638 +- 0.00020 )

HEC front wheel : 0.0443018 +- 4.25744e-05  ->  0.0443
HEC  rear wheel : 0.0227768 +- 2.18439e-05  ->  0.02278

FCal: Run 2, G4 10.6, release 22.0.56	0.01758(4)	0.01289(8)	0.01589(11)

Energy fraction : 0.10 < #eta < 0.75 = 0.18283 +- 0.00001
Energy fraction : 0.85 < #eta < 1.35 = 0.21398 +- 0.00001

Energy fraction : 1.45 < #eta < 1.49 = +0.09028 +- 0.00007 + (|#eta|-1.470)*( 0.16663 +- 0.00465 )
Energy fraction : 1.53 < #eta < 1.57 = +0.09432 +- 0.00004 + (|#eta|-1.550)*( 0.04757 +- 0.00331 )
Energy fraction : 1.64 < #eta < 1.76 = +0.09858 +- 0.00002 + (|#eta|-1.700)*( 0.05136 +- 0.00085 )
Energy fraction : 1.84 < #eta < 1.96 = +0.10216 +- 0.00002 + (|#eta|-1.900)*( 0.05838 +- 0.00073 )
Energy fraction : 2.03 < #eta < 2.08 = +0.10729 +- 0.00003 + (|#eta|-2.055)*( 0.05819 +- 0.00238 )
Energy fraction : 2.14 < #eta < 2.26 = +0.11012 +- 0.00002 + (|#eta|-2.200)*( 0.06949 +- 0.00079 )
Energy fraction : 2.34 < #eta < 2.45 = +0.11610 +- 0.00002 + (|#eta|-2.395)*( 0.07782 +- 0.00089 )
Energy fraction : 2.56 < #eta < 2.76 = +0.08382 +- 0.00001 + (|#eta|-2.660)*( 0.02801 +- 0.00021 )
Energy fraction : 2.85 < #eta < 3.14 = +0.08900 +- 0.00001 + (|#eta|-2.995)*( 0.03456 +- 0.00013 )

*/

void Create_Run3_G4_10_6_ref(std::string infile)
{
  std::map<std::string,TF1*> ref;
  TFile* fin=TFile::Open(infile.c_str());
  for(TObject* obj : *(fin->GetListOfKeys())) {
    if(obj->IsA()!=TKey::Class()) continue;
    TKey* key=(TKey*)obj;
    if(std::string("TF1")!=key->GetClassName()) continue;

    TF1* f=(TF1*)fin->Get(key->GetName());
    std::string name=f->GetName();

    cout<<"SetParams(ref[\""<<name<<"\"],,);"<<endl;
    
    ref[name]=f;
  }
  fin->Close();
  
  SetParams(ref["SF_LArEM_eta_0.10_0.75"],0.18489,0.00001);
  SetParams(ref["SF_LArEM_eta_0.85_1.35"],0.21601,0.00002);
  SetParams(ref["SF_LArEM_eta_1.45_1.49"],+0.09498,0.00009,0.03047,0.00648);
  SetParams(ref["SF_LArEM_eta_1.53_1.57"],+0.09557,0.00007,0.04595,0.00566);
  SetParams(ref["SF_LArEM_eta_1.64_1.76"],+0.09951,0.00004,0.04946,0.00140);
  SetParams(ref["SF_LArEM_eta_1.84_1.96"],+0.10289,0.00003,0.05894,0.00105);
  SetParams(ref["SF_LArEM_eta_2.03_2.08"],+0.10797,0.00005,0.06241,0.00344);
  SetParams(ref["SF_LArEM_eta_2.14_2.26"],+0.11072,0.00003,0.06868,0.00123);
  SetParams(ref["SF_LArEM_eta_2.34_2.45"],+0.11673,0.00004,0.07870,0.00138);
  SetParams(ref["SF_LArEM_eta_2.56_2.76"],+0.08444,0.00002,0.02680,0.00032);
  SetParams(ref["SF_LArEM_eta_2.85_3.14"],+0.09057,0.00002,0.04638,0.00020);
  SetParams(ref["SF_HEC_fwh_eta_1.50_3.30"],0.0443018,4.25744e-05);
  SetParams(ref["SF_HEC_rwh_eta_1.60_3.30"],0.0227768,2.18439e-05);
  SetParams(ref["SF_fcal1_eta_3.50_3.80"],0.01758,0.00004);
  SetParams(ref["SF_fcal2_eta_3.50_3.80"],0.01289,0.00008);
  SetParams(ref["SF_fcal3_eta_3.50_3.80"],0.01589,0.00011);
  
  SetParams(ref["EF_LArEM_eta_0.10_0.75"],0.18283,0.00001);
  SetParams(ref["EF_LArEM_eta_0.85_1.35"],0.21398,0.00001);
  SetParams(ref["EF_LArEM_eta_1.45_1.49"],+0.09028,0.00007,0.16663,0.00465);
  SetParams(ref["EF_LArEM_eta_1.53_1.57"],+0.09432,0.00004,0.04757,0.00331);
  SetParams(ref["EF_LArEM_eta_1.64_1.76"],+0.09858,0.00002,0.05136,0.00085);
  SetParams(ref["EF_LArEM_eta_1.84_1.96"],+0.10216,0.00002,0.05838,0.00073);
  SetParams(ref["EF_LArEM_eta_2.03_2.08"],+0.10729,0.00003,0.05819,0.00238);
  SetParams(ref["EF_LArEM_eta_2.14_2.26"],+0.11012,0.00002,0.06949,0.00079);
  SetParams(ref["EF_LArEM_eta_2.34_2.45"],+0.11610,0.00002,0.07782,0.00089);
  SetParams(ref["EF_LArEM_eta_2.56_2.76"],+0.08382,0.00001,0.02801,0.00021);
  SetParams(ref["EF_LArEM_eta_2.85_3.14"],+0.08900,0.00001,0.03456,0.00013);

  TFile* fout=TFile::Open("SF_LAr_G4_10_6_ref.test.root","RECREATE");
  for(auto f : ref) f.second->Write();
  fout->ls();
  fout->Close();
  delete fin;
  delete fout;
}

void PlotOneFile(std::vector<TF1*>& ret, std::string infile, TCanvas* cLArEM, TCanvas* cLArOther,TCanvas* cEFLArEM, int col,int style)
{
  TFile* fin=TFile::Open(infile.c_str());
  for(TObject* obj : *(fin->GetListOfKeys())) {
    if(obj->IsA()!=TKey::Class()) continue;
    TKey* key=(TKey*)obj;
    if(std::string("TF1")!=key->GetClassName()) continue;

    TF1* f=(TF1*)fin->Get(key->GetName());
    std::string name=f->GetName();
    //cout<<"Object "<<name<<" is TF1"<<endl;
    //f->Print();
    f->SetLineColor(col);
    f->SetLineStyle(style);

    if(name.find("EF_LArEM_")!=std::string::npos) {
      cEFLArEM->cd();
    } else {  
      if(name.find("SF_LArEM_")!=std::string::npos) {
        cLArEM->cd();
      } else {
        cLArOther->cd();
      }
    }  
    f->Draw("SAME");
    ret.push_back(f);
  }
  fin->Close();
}

void PlotSamplingFractions(std::string intitle1, std::string infile1, std::string intitle2="", std::string infile2="")
{
  //if(intitle2!="") Create_Run3_G4_10_6_ref(infile2);
  
  TCanvas* cLArEM=new TCanvas("summary_SF_LArEM","LAr EM sampling fractions");
  TH2* hLArEM=new TH2F("SF_LArEM","LAr EM sampling fractions",100,0.0,3.2,100,0.07,0.23);
  hLArEM->SetStats(0);
  hLArEM->GetXaxis()->SetTitle("|#eta|");
  hLArEM->GetYaxis()->SetTitle("E_{G4hit}/E_{total}");
  hLArEM->Draw();
  
  TLegend *ptLArEM = new TLegend(0.7,0.7,0.9,0.9,"");
  ptLArEM->SetFillStyle(1001);
  ptLArEM->SetFillColor(10);
  ptLArEM->SetBorderSize(1);

  TCanvas* cLArOther=new TCanvas("summary_SF_LArOther","Other LAr sampling fractions");
  TH2* hLArOther=new TH2F("SF_LArOther","Other LAr sampling fractions",100,1.4,3.9,100,0.01,0.05);
  hLArOther->SetStats(0);
  hLArOther->GetXaxis()->SetTitle("|#eta|");
  hLArOther->GetYaxis()->SetTitle("E_{G4hit}/E_{total}");
  hLArOther->Draw();

  TLegend *ptLArOther = new TLegend(0.7,0.7,0.9,0.9,"");
  ptLArOther->SetFillStyle(1001);
  ptLArOther->SetFillColor(10);
  ptLArOther->SetBorderSize(1);


  TCanvas* cEFLArEM=new TCanvas("summary_EF_LArEM","LAr EM energy fractions");
  TH2* hEFLArEM=new TH2F("EF_LArEM","LAr EM energy fractions",100,0.0,3.2,100,0.07,0.23);
  hEFLArEM->SetStats(0);
  hEFLArEM->GetXaxis()->SetTitle("|#eta|");
  hEFLArEM->GetYaxis()->SetTitle("E_{G4hit}/E_{truth}");
  hEFLArEM->Draw();
  
  TLegend *ptEFLArEM = new TLegend(0.7,0.7,0.9,0.9,"");
  ptEFLArEM->SetFillStyle(1001);
  ptEFLArEM->SetFillColor(10);
  ptEFLArEM->SetBorderSize(1);

  std::vector<TF1*> vec1;
  PlotOneFile(vec1,infile1,cLArEM,cLArOther,cEFLArEM,1,kSolid);
  ptLArEM->AddEntry(vec1[0],intitle1.c_str(),"l");
  ptLArOther->AddEntry(vec1[0],intitle1.c_str(),"l");
  ptEFLArEM->AddEntry(vec1[0],intitle1.c_str(),"l");

  std::vector<TF1*> vec2;
  if(intitle2!="") {
    PlotOneFile(vec2,infile2,cLArEM,cLArOther,cEFLArEM,2,kDashed);
    ptLArEM->AddEntry(vec2[0],intitle2.c_str(),"l");
    ptLArOther->AddEntry(vec2[0],intitle2.c_str(),"l");
    ptEFLArEM->AddEntry(vec2[0],intitle2.c_str(),"l");
  }  
  
  std::vector< std::string > calos={"SF_LArEM","SF_HEC_","SF_fcal","EF_LArEM"};
  std::vector< std::string > calo_print_name={"Sampling Fractions LAr EM","Sampling Fractions HEC","Sampling Fractions FCal","Energy Fractions LAr EM"};
  ofstream outtxt;
  outtxt.open("comparison.txt");
  for(unsigned int icalo=0;icalo<calos.size();++icalo) {
    cout<<calo_print_name[icalo]<<endl;
    outtxt<<calo_print_name[icalo]<<endl;
    for(auto func : vec1) {
      std::string name=func->GetName();
      std::string subname=name;
      auto ipos=name.find(calos[icalo]);
      if(ipos==std::string::npos) continue;
      subname.erase(ipos,calos[icalo].size());
      
      ipos=subname.find("_eta_");
      subname.erase(ipos,subname.size()-ipos);
      
      double relerr=func->GetParError(0)/func->GetParameter(0);
      
      if(subname!="") {
        cout<<subname<<": ";
        outtxt<<subname<<": ";
      }  
      cout<<func->GetXmin()<<" < |eta| < "<<func->GetXmax()<<" : "<<intitle1<<"="<<func->GetParameter(0)<<" +- "<<func->GetParError(0);
      outtxt<<func->GetXmin()<<" < |eta| < "<<func->GetXmax()<<" : "<<intitle1<<"="<<func->GetParameter(0)<<" +- "<<func->GetParError(0);
      for(auto func2 : vec2) if(name==func2->GetName()) {
        cout<<" ; "<<intitle2<<"="<<func2->GetParameter(0)<<" +- "<<func2->GetParError(0);
        outtxt<<" ; "<<intitle2<<"="<<func2->GetParameter(0)<<" +- "<<func2->GetParError(0);
        double relerr2=func2->GetParError(0)/func2->GetParameter(0);

        double ratio=func2->GetParameter(0)/func->GetParameter(0);
        double err_ratio=0;
        if(fabs(ratio)>0.000001) {
          double relerr_ratio=TMath::Sqrt(relerr*relerr+relerr2*relerr2);
          err_ratio=ratio*relerr_ratio;
        }
        cout<<" ; ratio="<<ratio<<" +- "<<err_ratio<<" ("<<(ratio-1)/err_ratio<<" sigma)";
        outtxt<<" ; ratio="<<ratio<<" +- "<<err_ratio<<" ("<<(ratio-1)/err_ratio<<" sigma)";
        break;
      }
      cout<<endl;
      outtxt<<endl;
    }
    cout<<endl;
    outtxt<<endl;
  }  
  outtxt.close();
  
  cLArEM->cd();
  ptLArEM->Draw();

  cLArOther->cd();
  ptLArOther->Draw();
  
  cEFLArEM->cd();
  ptEFLArEM->Draw();
  
  cLArEM->SaveAs(".pdf");
  cLArOther->SaveAs(".pdf");
  cEFLArEM->SaveAs(".pdf");
}
