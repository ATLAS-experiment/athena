///////////////////////////////////////////////////////////////////////////////
// Draw_PhysVal_btagROC.c - Author: Arnaud Duperrin <duperrin@cppm.in2p3.fr> - September 2019
//
// Root macro class to produce ROC b-tagging physics validation plots.
// Compare ROC curves for severals samples and for different algorithms.
//
// Inputs: NTUP_PHYSVAL (merged) root samples (produced by mergePhysValFiles.py)
//
// Variables to edit:
//
// -List of taggers:
//  const vector<TString> taggers = {"IP3D","SV1","DL1dv01","DL1r","GN2v01","GN2Xv01"};
//
// -Output directories for plots
//  const TString HistoDir = "ROC/";
//
// -Input samples directory path
//  const TString sPath = "./";
//
// -Names of input samples:
//  vector<TString> InputFilesNames = {"task1_ref.root","task1_test1.root","task1_test2.root"}; 
//  
//
///////////////////////////////////////////////////////////////////
//Several possible usages:
// 1) root -l Draw_PhysVal_btagROC.c
//
// 2) root -l -q -b Draw_PhysVal_btagROC.c >&! output.log
//
// 3) root -l
//    .L Draw_PhysVal_btagROC.c+
//    Draw_PhysVal_btagROC()
///////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <cstdlib>
#include <set>
#include "TFile.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TApplication.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TSystem.h"
#include <algorithm>
#include "TStyle.h"
#include "TColor.h"
#include "TProfile.h"
#include "TMath.h"
#include "THStack.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include <TLegendEntry.h>
#include <TPaveText.h>
#include "TLatex.h"
#include "TGaxis.h"
#include "TROOT.h"
#include <algorithm>

using namespace std;
TString jetType="LR";

//Legend names
vector<TString> leg_entry = {"Reference","Test"};

float EffMin; //changes with Zprime/ttbar
const float EffMax=1.;


//Some global variables for plotting:
vector<TString> taggers = {"IP3D","DIPS","SV1","DL1dv01","GN2v01","GN2Xv01"};

const float CWidth=800;
const float CHeight=600;

//where to save histos
TString HistoDir = "ROC_"+jetType+"/";


// detail level (more WPs for high detail level)
const bool high_detail_level = false;

//samples directory path
const TString sPath = "./"; 

//MyGraphCleaner
const bool kDebugON = true;
const bool kDebugOFF = false;
const float kTruncateXatOne = 1.; 
const float kTruncateYatOne = 1.;
const bool kApplyTruncateXON = true;
const bool kApplyTruncateXOFF = false;
const bool kApplyTruncateYON = true;
const bool kApplyTruncateYOFF = false;
const bool kMonotonicON = true;
const bool kMonotonicOFF = false;
const bool kBinToBinMonotonicON = true;
const bool kBinToBinMonotonicOFF = false;
const bool kRejectNullErrorsON = true;
const bool kRejectNullErrorsOFF = false;
const bool kForceNullErrorsON = true;
const bool kForceNullErrorsOFF = false;


int lcol[]= { kBlack, kCyan, kRed, kGreen+2, kBlue };
int msty[]= { kFullCircle, kFullTriangleUp, kFullSquare, kFullTriangleDown, kOpenCircle };
//const int lstyle = 9;
const int lstyle = 2;

vector<TString> truth_labels = {"b", "c", "u"};
map<TString, vector<TString>> WP_values;


void fill_WP_values(){
  if(high_detail_level){
    WP_values.insert(make_pair<TString, vector<TString>>("IP3D", {"50", "70", "80"}));
    WP_values.insert(make_pair<TString, vector<TString>>("RNNIP", {"50", "70", "80"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DIPS", {"50", "70", "80"}));
    WP_values.insert(make_pair<TString, vector<TString>>("SV1", {"40", "50", "60"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DL1dv01", {"60", "70", "77", "85"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DL1r", {"60", "70", "77", "85"}));
    WP_values.insert(make_pair<TString, vector<TString>>("GN2v01", {"60", "70", "77", "85"}));
    WP_values.insert(make_pair<TString, vector<TString>>("GN2Xv01", {"50","55","60","65","75","80","85"}));
  }
  else{
    WP_values.insert(make_pair<TString, vector<TString>>("IP3D", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("RNNIP", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DIPS", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("SV1", {"60"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DL1dv01", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("DL1r", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("GN2v01", {"70"}));
    WP_values.insert(make_pair<TString, vector<TString>>("GN2Xv01", {"60"}));
  }
}

TString getRefHistoName(TString var, TString truth_label){

  TString jetCollection;
  if(jetType=="EMTopo") jetCollection = "AntiKt4EMTopoJets";
  else if(jetType=="PFlow") jetCollection = "AntiKt4EMPFlowJets";
  else if(jetType=="LR") jetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";
  else if(jetType=="VR") jetCollection = "AntiKtVR30Rmax4Rmin02PV0TrackJets";

  TString name;

  if(var == "pt_ttbar"){
    name = "BTag/" + jetCollection + "/jet/jet/BTag_" + jetCollection
      + "_jet_jet_pt_" + truth_label + "_ttbar";
  }
  else if(var == "pt_Zprime"){
    name = "BTag/" + jetCollection + "/jet/jet/BTag_" + jetCollection
      + "_jet_jet_pt_" + truth_label + "_Zprime";
  }
  else if(var == "Lxy"){
    if(truth_label == "l"){
      name = "BTag/" + jetCollection + "/SV/_" + truth_label + "/BTag_"
	+ jetCollection + "_SV_SV1_Lxy_" + truth_label;
    }
    else{
      name = "BTag/" + jetCollection + "/other_histograms/histos/BTag_"
	+ jetCollection + "_truth_Truth_Lxy_" + truth_label;
    }
  }
  else{
    throw std::invalid_argument("getRefHistoName called with unknown var name: " + var + ". Only pt_ttbar, pt_Zprime and Lxy allowed. Aborting." );
  }

  return name;
}


void VALID_LABEL(Double_t x,Double_t y,Color_t color, TString sample, bool drawRatio=true) {
  TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize);
  l.SetTextSize(.045);
  if(!drawRatio) l.SetTextSize(.04);    
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextColor(color);
  l.DrawLatex(x,y,"Physics Validation");

  TLatex l2; //l.SetTextAlign(12); l.SetTextSize(tsize);
  l2.SetTextSize(.04);
  if(!drawRatio) l2.SetTextSize(.035);    
  l2.SetNDC();
  l2.SetTextFont(42);
  l2.SetTextColor(color);
  TString txt = "b-tagging, "+sample;
  l2.DrawLatex(x,y-.05,txt.Data());
  
}

void myDescriptionLabel(TString txt, float x, float y, Color_t color, bool drawRatio=true) {
  TLatex l;
  l.SetTextSize(.04);
  if(!drawRatio) l.SetTextSize(.035);    
  l.SetNDC();
  l.SetTextFont(42);
  l.SetTextColor(color);
  l.DrawLatex(x,y-0.1,txt.Data());
}

void myText(TString txt, float x, float y, Color_t color, bool drawRatio=true, float tsize=0.04) {
  TLatex l;
  l.SetTextSize(tsize);
  if(!drawRatio) l.SetTextSize(.04);    
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextColor(color);
  l.DrawLatex(x,y,txt.Data());

}

TGraphErrors * h_RejvEff(TH1 *hsig, TH1 *hbkg, bool isSV1, bool drawCtag=false) {
  int nbins = hsig->GetNbinsX();
  int inf = 0;
  int sup = nbins+1;
  double Isig = hsig->Integral(inf,sup);
  double Ibkg = hbkg->Integral(inf,sup);
  int point = 0;

  set<double> repetitionControl;
  vector<double> x;
  vector<double> ex;
  vector<double> y;
  vector<double> ey;

  for(int cut=inf; cut<sup; cut++) {
    double sig = hsig->Integral(cut,sup);
    double bkg = hbkg->Integral(cut,sup);
    if (repetitionControl.count(sig)) continue;
    repetitionControl.insert(sig);

    double cutVal = hsig->GetBinLowEdge(cut);
    double eff = 0;
    double rej = 0;
    double efferr = 0;
    double rejerr = 0;
    eff = sig / Isig;
    efferr = sqrt( eff * ( 1 - eff ) / Isig );
    if(bkg != 0){
      rej = Ibkg / bkg;
      rejerr = sqrt( rej * ( rej - 1 ) / bkg );
    }
    if ((isSV1 && eff<0.8) || !isSV1) {
      x.push_back(eff);
      y.push_back(rej);
      ex.push_back(efferr);
      ey.push_back(rejerr);
    }
  }
  TGraphErrors * rejVsEff = new TGraphErrors(x.size());
  for (unsigned int i=0;i<x.size();++i) {
    rejVsEff->SetPoint(i, x.at(i), y.at(i));
    rejVsEff->SetPointError(i, ex.at(i), ey.at(i));
  }
  std::string histoName = std::string("rejVsEff_")+hsig->GetName();
  histoName = drawCtag ? histoName + "_ctag" : histoName;
  rejVsEff->SetName(histoName.c_str());
  return rejVsEff;
}


TGraphErrors* h_EffvsVar(TH1* histo, TH1* histo_WPcuts) {

  if(!histo) {
    cout << "histo does not exist or is broken.\n";
    return nullptr;
  }
  if(!histo_WPcuts) {
    cout << "histo_WPcuts does not exist or is broken.\n";
    return nullptr;
  } 

  // test if the histos are healty and stuff
  if (!histo || !histo_WPcuts || histo->GetNbinsX() != histo_WPcuts->GetNbinsX() || histo->Integral() == 0) return NULL;

  vector<double> x;
  vector<double> ex;
  vector<double> y;
  vector<double> ey;

  for (int bin_i = 1; bin_i <= histo->GetNbinsX(); ++bin_i){ 
    if(histo->GetBinContent(bin_i) == 0) {continue;}
    if(histo_WPcuts->GetBinContent(bin_i) == 0) {continue;}  // c'est important
    double eff = histo_WPcuts->GetBinContent(bin_i) / histo->GetBinContent(bin_i);
    double eff_err = sqrt(histo_WPcuts->GetBinContent(bin_i)*(1-eff))/histo->GetBinContent(bin_i);        
    double var = histo->GetBinCenter(bin_i);
    double var_err = histo->GetBinError(bin_i);

    x.push_back(var);
    ex.push_back(0.); // pas important but I like it
     y.push_back(eff);
    ey.push_back(eff_err);
  }

  TGraphErrors* effVsVar = new TGraphErrors(x.size()); // ca c'est important!!!

  for (unsigned int i=0; i < x.size(); ++i) {
    effVsVar->SetPoint(i, x.at(i), y.at(i));
    effVsVar->SetPointError(i, ex.at(i), ey.at(i));
  }
  std::string histoName = std::string("effVsVar_")+histo_WPcuts->GetName();
  effVsVar->SetName(histoName.c_str());
  return effVsVar;

}


TGraphErrors* h_EffvsVar_debug(TH1* histo, TH1* histo_WPcuts) {

  if(!histo) {
    cout << "histo does not exist or is broken.\n";
    return nullptr;
  }
  if(!histo_WPcuts) {
    cout << "histo_WPcuts does not exist or is broken.\n";
    return nullptr;
  }

  // test if the histos are healty and stuff
  if (!histo || !histo_WPcuts || histo->GetNbinsX() != histo_WPcuts->GetNbinsX() || histo->Integral() == 0) return NULL;

  vector<double> x;
  vector<double> ex;
  vector<double> y;
  vector<double> ey;

  for (int bin_i = 1; bin_i <= histo->GetNbinsX(); ++bin_i){ 
    if(histo->GetBinContent(bin_i) == 0) {continue;}
    if(histo_WPcuts->GetBinContent(bin_i) == 0) {continue;}
    double eff = histo_WPcuts->GetBinContent(bin_i) / histo->GetBinContent(bin_i);
    double eff_err = sqrt(histo_WPcuts->GetBinContent(bin_i)*(1-eff))/histo->GetBinContent(bin_i);        
    double var = histo->GetBinCenter(bin_i);
    double var_err = histo->GetBinError(bin_i);
    x.push_back(var);
    ex.push_back(0.);
    y.push_back(eff);
    ey.push_back(eff_err);
  }

  TGraphErrors* effVsVar = new TGraphErrors(x.size());

  for (unsigned int i=0; i < x.size(); ++i) {
    effVsVar->SetPoint(i, x.at(i), y.at(i));
    effVsVar->SetPointError(i, ex.at(i), ey.at(i));
  }
  return effVsVar;

}


// Perform interpolation of pVariation 
TGraphErrors * Magic(const TGraphErrors* pReference, const TGraphErrors* pVariation) {
  const auto nPoints=pReference->GetN();

  for (int i=0;i<nPoints;++i) {
    double x,y;
    pReference->GetPoint(i, x, y);
  }
  TGraphErrors* result= new TGraphErrors(nPoints);

  auto GetFirstPoint=[&](float pXValue)->int {
    const auto nThisPoints=pVariation->GetN();
    unsigned int firstX=0;
    for (int i=0;i<nThisPoints;++i) {
      double x,y;
      pVariation->GetPoint(i, x, y);
      if (x<pXValue) return i;
    }
    return nThisPoints;
  };

  auto GetInterpolation=[&](float pXValue, int error)->double{
    const auto firstPoint=GetFirstPoint(pXValue);
    const auto secondPoint=firstPoint+(firstPoint==0?1:-1);
    double x1, x2, y1, y2;
    pVariation->GetPoint(firstPoint, x1, y1);
    pVariation->GetPoint(secondPoint, x2, y2);
    y1+=pVariation->GetErrorY(firstPoint)*error;
    y2+=pVariation->GetErrorY(secondPoint)*error;

    const auto A=TMath::Log(y1);
    const auto k=(TMath::Log(y2)-A)/(x2-x1);

    const auto NewY= y1*TMath::Exp(k*(pXValue-x1));
   return NewY;
  };

  for (int iPoint=0;iPoint<nPoints;++iPoint) {
    double x,y;
    pReference->GetPoint(iPoint, x,y);
    const auto refX=x;
    const auto refEX=pReference->GetErrorX(iPoint);
    const auto interY=GetInterpolation(refX, 0);
    const auto interEY=GetInterpolation(refX, +1);
    result->SetPoint(iPoint, refX, interY);
    result->SetPointError(iPoint, refEX, interEY-interY);
  }

  return result;
}

void PlotLikelihoodTaggers(const vector<TString>& InputFileNames,
			   const vector<vector<TH1F*>>& h_b,
			   const vector<vector<TH1F*>>& h_u){

  TCanvas* c1 = new TCanvas("c1","c1",0,0,CWidth,CHeight);
  c1->Divide(2*taggers.size(),InputFileNames.size());

  int l = 1;
  for(unsigned int it=0;it<InputFileNames.size();++it) {
    std::cout<<"--Reading input histo from file: "<<InputFileNames[it]<<std::endl;
    for (unsigned int i=0;i<taggers.size();i++) {
      cout << "-taggers["<<i<<"]="<< taggers[i] << endl;
      c1->cd(l);
      h_b[it][i]->Draw();
      c1->cd(l+1);
      h_u[it][i]->Draw();
      l=l+2;
    }
  }
}


void MyGraphCleaner(TGraphErrors* gr, bool debug=false, bool ApplyTruncatexAt = false, float truncatexAt=1., bool ApplyTruncateyAt = false, float truncateyAt=1., bool forceMonotonic=false, bool forceBinToBinMonotonic=false, bool rejectNullErrors=false, bool forceNullErrors=false, float forceAroundMean=0.) {
  if(0==gr) return;
  
  debug=false;

  if(debug) {cout<<endl; cout << "====" << endl;}
  gr->Sort();

  int npr=0;
  int np = gr->GetN();
  if(debug) std::cout<<"graph "<<gr->GetName()<<" has "<<np<<" points"<<std::endl;
  double eff1,rej1;
  double eff2,rej2;
  int qq = gr->GetPoint(0,eff1,rej1);
  double rejMin = rej1;
  if(debug) std::cout<<"rejMin (Y at begining) = " << rejMin << endl;
  double Ymean= gr->GetMean(2);
  if(debug) std::cout<<"GetMean in Y = " << Ymean << endl;
  if(forceMonotonic && debug) std::cout<<"reference point for monotonic cleaning: "<<eff1<<" "<<rej1<<std::endl;

  int nop = 0;
  for(int i=0;i<np;i++) {
    nop++;
    if(nop>np) break;   
    qq = gr->GetPoint(i,eff2,rej2);
    double ex = gr->GetErrorX(i);
    double ey = gr->GetErrorY(i);
    if(debug) std::cout<<"->examining point "<<i<<" x="<<eff2<<" y=" <<rej2<<" ex="<<ex<<" ey="<<ey<<std::endl;
    if(eff2==0){
      if(debug) std::cout<<"removing point "<<i<<" of "<<gr->GetName() <<"b/c above truncation: "<<eff2<<std::endl;
      gr->RemovePoint(i);
      i--;
      npr++;
      continue;
    }
    if(eff2>truncatexAt && ApplyTruncatexAt) {
      if(debug) std::cout<<"removing point "<<i<<" of "<<gr->GetName() <<"x above truncation: "<<eff2<<std::endl;
      gr->RemovePoint(i);
      i--;
      npr++;
      continue;
    }
    if(rej2>truncateyAt && ApplyTruncateyAt) {
      if(debug) std::cout<<"removing point "<<i<<" of "<<gr->GetName() <<"y above truncation: "<<rej2<<std::endl;
      gr->RemovePoint(i);
      i--;
      npr++;
      continue;
    }

    //forceAroundMean = deviation around the mean value in Y 
    if(forceAroundMean!=0.) {
      if(((rej2+ey)/Ymean>forceAroundMean) || (Ymean/(rej2-ey)>forceAroundMean) ) {
    if(debug) std::cout<<"forceAAroundMean - removing point "<<i<<" of "<<gr->GetName()<<std::endl;
    if(debug) std::cout<<"pt "<<qq<<" "<<i<<" x="<<eff2<<" y="<<rej2 <<" (rej2+ey)/Ymean ="<<(rej2+ey)/Ymean<< " Ymean/(rej2-ey) = " << Ymean/(rej2-ey) << std::endl;
      gr->RemovePoint(i);
    i--;
    npr++;
      }
    }

    if(rejectNullErrors) {
      if(ex==0 && ey==0) {
    if(debug) std::cout<<"removing point "<<i<<" of "<<gr->GetName() <<"b/c null errors"<<std::endl;
      gr->RemovePoint(i);
        i--;
    npr++;
    continue;
      }
    }
    if(forceMonotonic) {
      if(rej2>rejMin) {
    if(debug) std::cout<<"forceMonotonic - removing point "<<i<<" of "<<gr->GetName()<<std::endl;
    if(debug) std::cout<<"pt "<<qq<<" "<<i<<" e="<<eff2<<" r="<<rej2 <<" previous min="<<rejMin<<std::endl;
      gr->RemovePoint(i);
    i--;
    npr++;
      }
    }
    if(forceBinToBinMonotonic) {
      if( ( (eff2>eff1)&&(rej2>rej1) ) ||
          ( (eff2<eff1)&&(rej2<rej1) ) ) {
    if(debug) std::cout<<"removing point "<<i<<" of "<<gr->GetName()<<std::endl;
    if(debug) std::cout<<"pt "<<qq<<" "<<i<<" e="<<eff2<<" r="<<rej2 <<" eprev="<<eff1<<" rejprev="<<rej1<<std::endl;
      gr->RemovePoint(i);
    i--;
    npr++;
      } else {
        eff1 = eff2;
        rej1 = rej2;
      }
    }

    if(forceNullErrors)
      gr->SetPointError(i,0.,0.);

  }

}

pair<double,double> GetMaxRatioTGraphROC(TGraphErrors* Gratio, double mineff, double maxeff) {

  double ratiomax=1.05, ratiomin=0.95;

  for (int i=0; i< Gratio->GetN(); i++){
    if (!((Gratio->GetPointX(i) < mineff) || (Gratio->GetPointX(i) > maxeff))){
      if (Gratio->GetPointY(i)> ratiomax){
	ratiomax=Gratio->GetPointY(i) *1.2;
	cout<<"ratiomax set to "<< Gratio->GetPointY(i)<<endl;
      }
      if (Gratio->GetPointY(i)< ratiomin){
	ratiomin=Gratio->GetPointY(i) *0.9;
	cout<<"ratiomin set to "<< Gratio->GetPointY(i)<<endl;
      }
    }
  }

  pair<double,double> ratio_ymax_ymin;
  ratio_ymax_ymin.first=ratiomax;
  ratio_ymax_ymin.second=ratiomin;
  cout<<"final set ratio max: "<<ratiomax<<endl;
  cout<<"final set ratio min: "<<ratiomin<<endl;

  return ratio_ymax_ymin;
}



void plotGraphs(const vector<TString>& InputFileNames,
		TString MC, TString sample,
		const vector<TString>& leg_entry,
		bool drawRatio=false, bool drawErrRatio=false,
		bool drawCtag=false,
		bool writeHistos=false, TString outputName="myHistos.root") {

  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
  gStyle->SetOptStat(kFALSE);

  vector<vector<TH1F*>> h_b,h_u;
  vector<vector<TGraphErrors*>> vGraph_bu;
  int ifirst = -1;
  cout << "=========================" << endl; 
  for(unsigned int it=0;it<InputFileNames.size();++it) {
    if(ifirst==-1) ifirst = it;
    TString InputFileName = sPath+InputFileNames[it];
    TFile *f = TFile::Open(InputFileName);
    cout<<"--Reading input histo from file: "<<InputFileName<<endl;
    vector<TH1F*> hb,hu;
    vector<TGraphErrors*> Graph_bu;

    for (unsigned int i=0;i<taggers.size();i++) {

      TString folder1,folder2;
	folder1 = "tagger_"+taggers[i]+"/other";
	folder2="tagger_"+taggers[i];

      TString jetCollection;
      if(jetType=="EMTopo") jetCollection = "AntiKt4EMTopoJets";
      else if(jetType=="PFlow") jetCollection = "AntiKt4EMPFlowJets";
      else if(jetType=="LR") jetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";
      else if(jetType=="VR") jetCollection = "AntiKtVR30Rmax4Rmin02PV0TrackJets";

      TString hname_b = "BTag/" + jetCollection + "/" + folder1 + "/BTag_"
	+ jetCollection + "_" + folder2 + "_b_matched_weight";
      TString hname_u = "BTag/" + jetCollection + "/" + folder1 + "/BTag_"
	+ jetCollection + "_" + folder2 + "_u_matched_weight";
      if(drawCtag){
	hname_u = "BTag/" + jetCollection + "/" + folder1 + "/BTag_"
	  + jetCollection + "_"+folder2+"_c_matched_weight";
      }

      TH1F *MVX_b = (TH1F*)f->Get(hname_b);
      TH1F *MVX_u = (TH1F*)f->Get(hname_u);
      
      hb.push_back(MVX_b);
      hu.push_back(MVX_u);
      bool isSV1 = false;
      if (taggers[i]=="SV1") isSV1 = true;
      TGraphErrors* Graphbu = h_RejvEff(MVX_b,MVX_u,isSV1, drawCtag);
      Graphbu->SetLineStyle(1);
      if(InputFileNames.size()>=2 && it>=1) Graphbu->SetLineStyle(lstyle);
      Graphbu->SetLineWidth(2);
      if(InputFileNames.size()>2) Graphbu->SetLineColor(lcol[it]);
      Graph_bu.push_back(Graphbu);

    } // taggers i

    h_b.push_back(hb);
    h_u.push_back(hu);
    vGraph_bu.push_back(Graph_bu);
  } //root files it

  cout << endl;

  //Compute ratio:
  vector<vector<TGraphErrors*>> gratio;
  vector <pair<double,double>> v_ratio_ymax_ymin;
  if(drawRatio){
    for (unsigned int i=0;i<taggers.size();i++) {
      double ymaxratio = 1.05; // to set range on Y of ratio pad
      double yminratio = 0.95; // to set range on Y of ratio pad
      vector<TGraphErrors*> vratio;
      TGraphErrors* gref=vGraph_bu[ifirst][i];

      for(unsigned int it=0;it<InputFileNames.size();++it) {
	if(ifirst==static_cast<int>(it)) continue;
	TGraphErrors* Corrected = Magic(vGraph_bu[ifirst][i],vGraph_bu[it][i]); // perform extrapolation of Test curve
	TGraphErrors* Gratio = Corrected;
	double x_ratio=0., y_ratio=0.;
	double eR=0., eT=0., eRatio=0.;
	for (int i=0;i<Gratio->GetN();i++) {
	  //central value
	  double x1,x2,y1,y2;
	  Gratio->GetPoint(i,x1,y1); // Test extrapolated
	  gref->GetPoint(i,x2,y2);

	  bool skiplooping= false;
	  if (y1==0 || y2==0 || isnan(y1) || isnan(y2)) skiplooping= true;
	  if (isinf(y1) || isinf(y2)) skiplooping= true;

	  if (!skiplooping) {
	    x_ratio = x1;
	    y_ratio = y1/y2;
	    //uncertainty
	    eT = Gratio->GetErrorY(i);
	    eR = gref->GetErrorY(i);
	    eRatio = (y_ratio)*sqrt(pow((eR/y2),2) + pow(eT/y1,2));

	  } // if (!skiplooping)

	  Gratio->SetPoint(i,x_ratio,y_ratio);
	  Gratio->SetPointError(i,Gratio->GetErrorX(i),eRatio);
        
	} // for (int i=0;i<Gratio->GetN();i++)

	vratio.push_back(Gratio);
	float forceAroundMean=0.;
	MyGraphCleaner(vratio.back(),kDebugOFF,kApplyTruncateXON,kTruncateXatOne,kApplyTruncateYOFF,kTruncateYatOne,kMonotonicOFF,kBinToBinMonotonicOFF,kRejectNullErrorsOFF,kForceNullErrorsOFF,forceAroundMean);
	vratio.back()->SetLineWidth(2);
	vratio.back()->SetLineStyle(vGraph_bu[it][i]->GetLineStyle());
	vratio.back()->SetLineColor(vGraph_bu[it][i]->GetLineColor());
      } // it

      pair<double,double> ratio_ymax_ymin = GetMaxRatioTGraphROC(vratio.back(),EffMin,EffMax);
      gratio.push_back(vratio);
      v_ratio_ymax_ymin.push_back(ratio_ymax_ymin);
      //pair<double,double> ratio_ymax_ymin;
      cout << "yminratio = " << yminratio << " ymaxratio=" << ymaxratio << endl;
    } // i tagger

  } // compute ratio

  //////////////////////////////////////////////////
  for (unsigned int i=0;i<taggers.size();i++) {

    //set Xaxis range
    double mineff = EffMin;
    double maxeff = EffMax;
    //if (taggers[i]=="SV1") maxeff = 0.75;
    vGraph_bu[ifirst][i]->GetXaxis()->SetLimits(mineff,maxeff);
    vGraph_bu[ifirst][i]->SetMinimum(1.);
    if(drawRatio) vGraph_bu[ifirst][i]->SetMinimum(0.8);
    
    //set Yaxis range
    double ymax = max(vGraph_bu[ifirst][i]->Eval(mineff), vGraph_bu[ifirst][i]->Eval(mineff+0.2)); // get Y value at X=mineff

    for (int j=0; j< vGraph_bu[ifirst][i]->GetN(); j++){
      if (!((vGraph_bu[ifirst][i]->GetPointX(j) < mineff) || (vGraph_bu[ifirst][i]->GetPointX(j) > maxeff))){
            if (vGraph_bu[ifirst][i]->GetPointY(j)> ymax){
                ymax=vGraph_bu[ifirst][i]->GetPointY(j) *1.2;
                cout<<"ymax set to "<< vGraph_bu[ifirst][i]->GetPointY(j)<<endl;
            }
        }
    }

    ymax += 5.*ymax; //add 5x to ymax
    cout << "taggers[i]=" << taggers[i] << " ymax= " << ymax << endl;
    vGraph_bu[ifirst][i]->SetMaximum(ymax);
  
    TCanvas* c1 = new TCanvas("c1"+taggers[i],"c1"+taggers[i],0,0,CWidth,CHeight);
    c1->SetLogy(1);
    c1->SetGrid();

    if(drawRatio){
      TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
      pad1->SetLeftMargin(0.15);
      pad1->SetBottomMargin(0); // Upper and lower plot are joined
      pad1->Draw();             // Draw the upper pad: pad1
      pad1->cd();               // pad1 becomes the current pad
      pad1->SetGrid();
      gPad->SetTicks();
      gPad->SetLogy();
      vGraph_bu[ifirst][i]->GetXaxis()->SetLabelSize(0);
    } // drawRatio

    cout << "taggers[i]=" << taggers[i] << " ymax= " << ymax << endl;
    vGraph_bu[ifirst][i]->SetMaximum(ymax);
    vGraph_bu[ifirst][i]->Draw("ALE");

    vGraph_bu[ifirst][i]->GetXaxis()->SetTitle("b-jet efficiency");
    if(!drawCtag){
      vGraph_bu[ifirst][i]->GetYaxis()->SetTitle("light-jet rejection");
    }
    else{
      vGraph_bu[ifirst][i]->GetYaxis()->SetTitle("c-jet rejection");
    }
    vGraph_bu[ifirst][i]->GetYaxis()->SetTitleOffset(1.3);
 
   if(!drawRatio){
      vGraph_bu[ifirst][i]->GetXaxis()->SetTitleSize(0.04);
      vGraph_bu[ifirst][i]->GetXaxis()->SetLabelSize(0.04);
      vGraph_bu[ifirst][i]->GetYaxis()->SetTitleSize(0.04);
      vGraph_bu[ifirst][i]->GetYaxis()->SetLabelSize(0.04);
    }

    for(unsigned int it=0;it<InputFileNames.size();++it) {
      if(ifirst==static_cast<int>(it)) continue;
      vGraph_bu[it][i]->Draw("LE");
    }//file name

    //Legend
    TLegend* lg = new TLegend(0.55,0.5,0.88,0.845);
    lg->SetBorderSize(0);
    lg->SetFillStyle(0);
    for(unsigned int it=0;it<InputFileNames.size();it++) {
      lg->AddEntry(vGraph_bu[it][i], leg_entry[it],"L");

      // Guess this is always the test file.......?????????????
      if(writeHistos && it == 1){
        if(gSystem->AccessPathName(outputName)) {
          //File doesn't exist already, make it
          TFile filerio(outputName,"RECREATE");
          TDirectory* d = filerio.mkdir("BTag/"+HistoDir);
          filerio.cd("BTag/"+HistoDir);
          vGraph_bu[it][i]->Write(vGraph_bu[it][i]->GetName());
          filerio.Close();
        } else {
          //File already exist, just open it
          TFile filerio(outputName,"UPDATE");
          filerio.cd("BTag/"+HistoDir);
          vGraph_bu[it][i]->Write(vGraph_bu[it][i]->GetName());
          filerio.Close();
        }
      }

    }
    lg->SetTextSize(0.03);
    if(drawRatio) lg->SetTextSize(0.04);
    lg->Draw();

    //Text
    VALID_LABEL(0.25,0.9,1,sample,drawRatio);
    myText(taggers[i],0.25,0.3,1,drawRatio,0.07); 

    if(drawRatio){
      // lower plot will be in pad
      c1->cd();          // Go back to the main canvas before defining pad2

      //range on Y axis for ratio plot //
      double ymin_ratio = 0.9;
      double ymax_ratio = 1.1;
 
      // 
      double scale = 0.3; // scale up and down by 20% to set the ratio bondaries

      double ymaxratio= v_ratio_ymax_ymin[i].first;
      double yminratio= v_ratio_ymax_ymin[i].second;
      
      if(yminratio<ymin_ratio) ymin_ratio = yminratio-scale*(1.-yminratio);
      if(ymaxratio>ymax_ratio) ymax_ratio = ymaxratio+scale*(ymaxratio-1.);



      TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
      pad2->Range(mineff,ymin_ratio,maxeff,ymax_ratio);
      pad2->SetLeftMargin(0.15);
      pad2->SetTopMargin(0);
      pad2->SetBottomMargin(0.35);
      pad2->Draw();
      pad2->cd();       // pad2 becomes the current pad
      gStyle->SetEndErrorSize(0);
      gPad->SetTicks();  
      gPad->SetLogy(0);

      gratio[i][0]->SetMinimum(ymin_ratio);
      gratio[i][0]->SetMaximum(ymax_ratio);

      gratio[i][0]->SetFillColor(18);

      //Draw error band on ratio?
      if (!drawErrRatio) {
        gratio[i][0]->Draw("alx0"); // A: axis L: simple line 
      } else {
        gratio[i][0]->Draw("AC3"); // 
        gratio[i][0]->Draw("lx"); //x: without error bar  
      }

      gratio[i][0]->GetYaxis()->SetLabelSize(0.);
      TGaxis *axis2 = new TGaxis( mineff, 1.001*ymin_ratio, mineff, 0.999*ymax_ratio, 1.001*ymin_ratio, 0.999*ymax_ratio, 505,"");
      axis2->SetLabelFont(43); // Absolute font size in pixel (precision 3)
      axis2->SetLabelSize(15);
      axis2->Draw();        

      // Y axis ratio plot settings
      gratio[i][0]->GetYaxis()->SetTitle("Ratio");
      gratio[i][0]->GetYaxis()->SetNdivisions(505);
      gratio[i][0]->GetYaxis()->SetTitleSize(20);
      gratio[i][0]->GetYaxis()->SetTitleFont(43);
      gratio[i][0]->GetYaxis()->SetTitleOffset(1.5);
      gratio[i][0]->GetYaxis()->CenterTitle();
      
      // X axis ratio plot settings
      gratio[i][0]->GetXaxis()->SetTitleSize(20);
      gratio[i][0]->GetXaxis()->SetTitleFont(43);
      gratio[i][0]->GetXaxis()->SetTitleOffset(4.);
      gratio[i][0]->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
      gratio[i][0]->GetXaxis()->SetLabelSize(15);
      gratio[i][0]->GetXaxis()->SetTitle("b-jet efficiency");
      gratio[i][0]->GetXaxis()->SetLimits(mineff,maxeff);
      
      TLine* line = new TLine(mineff, 1., maxeff,1.);
      line->Draw("same");
      line->SetLineStyle(lstyle);
      
      for(unsigned int it=1;it<gratio[i].size();it++){
    gratio[i][it]->Draw("Lxsame");
      }

    } // drawRatio

 
    //check if the directory where to save histo exits
    if(gSystem->AccessPathName(HistoDir)){
      std::cout << HistoDir << " directory does not exist. Will create one." << std::endl;
      gSystem->Exec("mkdir "+HistoDir);
    } 

    if(drawCtag) {Histo = HistoDir+taggers[i]+"-cVSb.pdf";}
    else {Histo = HistoDir+taggers[i]+".pdf";}
    //cout << "Saving Histo = " << Histo.Data() << endl;
    c1->SaveAs(Histo.Data(),"RECREATE");
   
  } // tagger i

} // plotGraphs


// a method for ploting the efficiency vs a variable
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void plotGraphsEffVsVar(TString var_name, const vector<TString>& InputFileNames,
			TString MC, TString sample,
			const vector<TString>& leg_entry,
			bool drawRatio = false, bool drawErrRatio=false,
			bool writeHistos=false, TString outputName="myHistos.root") {

  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
  gStyle->SetOptStat(kFALSE);

  /////////////////////////////////////////////
  // get the histograms and graphs

  vector<vector<vector<vector<TH1F*> > > > histos, histos_WPcuts;
  vector<vector<vector<vector<TGraphErrors*> > > > graphs;
  int ifirst = -1;
  cout << "=========================" << endl; 
  // loop over the input files
  for(unsigned int it = 0; it < InputFileNames.size(); ++it) {
    if(ifirst==-1) ifirst = it;
    // get the file
    TString InputFileName = sPath+InputFileNames[it];
    TFile *f = TFile::Open(InputFileName);
    cout<<"--Reading input histo from file: "<<InputFileName<<endl;
    // define vectors to hold histos
    vector<vector<vector<TH1F*> > > histos_forTaggers, histos_WPcuts_forTaggers;
    vector<vector<vector<TGraphErrors*> > > graphs_forTaggers;

    // loop over the taggers
    for (unsigned int i=0; i < taggers.size(); i++) {
      // get the right histos

      TString folder1,folder2;
      folder1 = "tagger_"+taggers[i]+ "/_" + var_name; folder2="tagger_"+taggers[i];
 
      // define vectors
      vector<vector<TH1F*> > histos_forLabels, histos_WPcuts_forLabels;
      vector<vector<TGraphErrors*> > graphs_forLabels;

      // loop over the truth labels
      for(unsigned int i_truthlabel = 0; i_truthlabel < truth_labels.size(); i_truthlabel++){
        TString histo_truth_label = truth_labels[i_truthlabel];
        if(histo_truth_label == "u") histo_truth_label = "l";
        TString hname = getRefHistoName(var_name, histo_truth_label);
        TH1F *histo = (TH1F*)f->Get(hname);

        // define vectors
        vector<TH1F*> histos_forWPs, histos_WPcuts_forWPs;
        vector<TGraphErrors*> graphs_forWPs;

        // loop over the WP cuts
        vector<TString> tagger_WPs = WP_values.at(taggers[i]);
        for(unsigned int i_WP = 0; i_WP < tagger_WPs.size(); i_WP++){

	  TString jetCollection;
	  if(jetType=="EMTopo") jetCollection = "AntiKt4EMTopoJets";
	  else if(jetType=="PFlow") jetCollection = "AntiKt4EMPFlowJets";
	  else if(jetType=="LR") jetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";
          else if(jetType=="VR") jetCollection = "AntiKtVR30Rmax4Rmin02PV0TrackJets";

	  TString hname_WPcuts = "BTag/" + jetCollection + "/" + folder1
	    + "/BTag_" + jetCollection + "_" + folder2 + "_"
	    + truth_labels[i_truthlabel] + "_" + tagger_WPs[i_WP] + "_matched_"
	    + var_name;

          TH1F *histo_WPcuts = (TH1F*)f->Get(hname_WPcuts);
          cout<<"hist_WPcuts name: " << hname_WPcuts << endl;
          cout<<"histo_WPcuts: "<<histo_WPcuts<<endl;

          // define the graph
          TGraphErrors* graph = h_EffvsVar(histo, histo_WPcuts);

          //Truncate y efficiency above 0.99 first...
          float TruncateAtY = 0.99;
          float forceAroundMean=0.;
          MyGraphCleaner(graph,kDebugON,kApplyTruncateXOFF,kTruncateXatOne,kApplyTruncateYON,kTruncateYatOne,kMonotonicOFF,kBinToBinMonotonicOFF,kRejectNullErrorsOFF,kForceNullErrorsOFF,forceAroundMean);

          forceAroundMean=3.;
          MyGraphCleaner(graph,kDebugON,kApplyTruncateXOFF,kTruncateXatOne,kApplyTruncateYON,kTruncateYatOne,kMonotonicOFF,kBinToBinMonotonicOFF,kRejectNullErrorsOFF,kForceNullErrorsOFF,forceAroundMean);

          MyGraphCleaner(graph,kDebugON,kApplyTruncateXOFF,kTruncateXatOne,kApplyTruncateYON,kTruncateYatOne,kMonotonicOFF,kBinToBinMonotonicOFF,kRejectNullErrorsOFF,kForceNullErrorsOFF,forceAroundMean);

          // set line styles
          graph->SetLineStyle(1);
          if(InputFileNames.size()>=2 && it>=1) {graph->SetLineStyle(lstyle);}
          graph->SetLineWidth(2);
          if(InputFileNames.size()>2) {graph->SetLineColor(lcol[it]); graph->SetMarkerStyle(msty[it]);}

          // add them to the vectors
          histos_forWPs.push_back(histo);
          histos_WPcuts_forWPs.push_back(histo_WPcuts);
          graphs_forWPs.push_back(graph);
        } // WPs
        // add them to the vectors
        histos_forLabels.push_back(histos_forWPs);
        histos_WPcuts_forLabels.push_back(histos_WPcuts_forWPs);
        graphs_forLabels.push_back(graphs_forWPs);
      } // truth labels
      histos_forTaggers.push_back(histos_forLabels);
      histos_WPcuts_forTaggers.push_back(histos_WPcuts_forLabels);
      graphs_forTaggers.push_back(graphs_forLabels);
    } // taggers
    histos.push_back(histos_forTaggers);
    histos_WPcuts.push_back(histos_WPcuts_forTaggers);
    graphs.push_back(graphs_forTaggers);
  } //root files it

  cout << endl;

  /////////////////////////////////////////////
  // compute ratio

  vector<vector<vector<vector<TGraphErrors*> > > > graph_ratios;
  vector<vector<vector<pair<double,double> > > >    v_ratio_ymax_ymin;
  if(drawRatio){

    for (unsigned int i=0;i<taggers.size();i++) {
      vector<vector<vector<TGraphErrors*> > > graph_ratios_forLabels;
      vector<vector<pair<double,double> > >  v_ratio_ymax_ymin_forLabels;

      for(unsigned int i_truthlabel = 0; i_truthlabel < truth_labels.size(); i_truthlabel++){
        vector<vector<TGraphErrors*> > graph_ratios_forWPs;
        vector<pair<double,double> > v_ratio_ymax_ymin_forWPs;
        vector<TString> tagger_WPs = WP_values.at(taggers[i]);

        for(unsigned int i_WP = 0; i_WP < tagger_WPs.size(); i_WP++){
          vector<TGraphErrors*> graph_ratios_forFiles;
          TGraphErrors* Gref = graphs[ifirst][i][i_truthlabel][i_WP];

	  double Xmin,Xmax,Ymin,Ymax;  // ??
	  Gref->GetPoint(0,Xmin,Ymin);  //
	  Gref->GetPoint(Gref->GetN()-1,Xmax,Ymax); //
	  double ymaxratio = 0.; // to set range on Y of ratio pad
	  double yminratio = 1.; // to set range on Y of ratio pad
      
          for(unsigned int it=0;it<InputFileNames.size();++it) {
	    if(ifirst==static_cast<int>(it)) continue;
	    TGraphErrors* Gratio=(TGraphErrors*)graphs[it][i][i_truthlabel][i_WP]->Clone();
        
	    double x_ratio=0., y_ratio=0.;
	    double eR=0., eT=0., eRatio=0.;
            // loop over bins

	    for (int i=0; i < Gratio->GetN(); i++) {
	      //central value
	      double x1,x2,y1,y2;
	      Gratio->GetPoint(i,x1,y1); // Test extrapolated
	      Gref->GetPoint(i,x2,y2);
          
	      bool skiplooping = false;
	      if (y1==0 || y2==0 || isnan(y1) || isnan(y2)) skiplooping = true;
	      if (isinf(y1) || isinf(y2)) skiplooping = true;
          
	      if (!skiplooping) {
		x_ratio = x1;
		y_ratio = y1/y2; // Test/Ref
		//uncertainty
		eT = Gratio->GetErrorY(i);
		eR = Gref->GetErrorY(i);
		eRatio = (y_ratio)*sqrt(pow((eR/y2),2) + pow(eT/y1,2));
              }
          
	      Gratio->SetPoint(i,x_ratio,y_ratio);
	      Gratio->SetPointError(i,Gratio->GetErrorX(i),eRatio);
            } // end loop over bins
        
	    Gratio->SetLineWidth(2);
	    Gratio->SetLineStyle(graphs[it][i][i_truthlabel][i_WP]->GetLineStyle());
	    Gratio->SetLineColor(graphs[it][i][i_truthlabel][i_WP]->GetLineColor());
	    graph_ratios_forFiles.push_back(Gratio);
        
	    pair<double,double> ratio_ymax_ymin_tmp = GetMaxRatioTGraphROC(graph_ratios_forFiles.back(),Xmin,Xmax);
	    double ymax_tmp = ratio_ymax_ymin_tmp.first;
	    double ymin_tmp = ratio_ymax_ymin_tmp.second;
	    if (ymax_tmp>ymaxratio) ymaxratio=ymax_tmp;
	    if (ymin_tmp<yminratio) yminratio=ymin_tmp;
          } // loop over files

          graph_ratios_forWPs.push_back(graph_ratios_forFiles);
	  pair<double,double> ratio_ymax_ymin;
	  cout << "yminratio = " << yminratio << " ymaxratio=" << ymaxratio << endl;
	  ratio_ymax_ymin.first=ymaxratio;
	  ratio_ymax_ymin.second=yminratio;
	  v_ratio_ymax_ymin_forWPs.push_back(ratio_ymax_ymin);
        } // loop over WPs

        graph_ratios_forLabels.push_back(graph_ratios_forWPs);
        v_ratio_ymax_ymin_forLabels.push_back(v_ratio_ymax_ymin_forWPs);
      } // loop over truth labels
      graph_ratios.push_back(graph_ratios_forLabels);
      v_ratio_ymax_ymin.push_back(v_ratio_ymax_ymin_forLabels);
    } // loop over taggers

  } // compute ratio

  /////////////////////////////////////////////
  // make the plots

  for (unsigned int i=0;i<taggers.size();i++) {

    vector<TString> tagger_WPs = WP_values.at(taggers[i]);
    for(unsigned int i_truthlabel = 0; i_truthlabel < truth_labels.size(); i_truthlabel++){
      for(unsigned int i_WP = 0; i_WP < tagger_WPs.size(); i_WP++){

        double scale = 0.3; // scale up and down by 30% to set the ratio bondaries
        double YmaxPlotEff = graphs[ifirst][i][i_truthlabel][i_WP]->GetHistogram()->GetMaximum();
        YmaxPlotEff = (1.+scale)*YmaxPlotEff;

        graphs[ifirst][i][i_truthlabel][i_WP]->SetMinimum(0.);
        graphs[ifirst][i][i_truthlabel][i_WP]->SetMaximum(YmaxPlotEff);

        TString myRandomNumber = to_string(rand());
        TCanvas* c2 = new TCanvas("c2"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber,"c2"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber,0,0,CWidth,CHeight);
        c2->SetGrid();

        if(drawRatio){
          TPad *pad21 = new TPad("pad21"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber, "pad21"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber, 0, 0.3, 1, 1.0);
          pad21->SetLeftMargin(0.15);
          pad21->SetBottomMargin(0); // Upper and lower plot are joined 
          pad21->Draw();             // Draw the upper pad: pad2
          pad21->cd();               // pad2 becomes the current pad
          pad21->SetGrid();
          gPad->SetTicks();
          graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetLabelSize(0); // to be put back
        } // drawRatio

        graphs[ifirst][i][i_truthlabel][i_WP]->SetFillColor(18);
        graphs[ifirst][i][i_truthlabel][i_WP]->SetFillStyle(3000);// https://root.cern.ch/doc/master/classTAttFill.html
        graphs[ifirst][i][i_truthlabel][i_WP]->Draw("AL3"); //"3" shows the errors as a band.
    TGraphErrors* tmp_gr = (TGraphErrors*)graphs[ifirst][i][i_truthlabel][i_WP]->Clone();
    tmp_gr->Draw("LX"); // "X" Do not draw error bars
    

    c2->Update();
    double Xmin,Xmax,Ymin,Ymax;  // ??
    Xmin=gPad->GetUxmin();
    Xmax=gPad->GetUxmax();
    Ymin=gPad->GetUymin();
    Ymax=gPad->GetUymax();

    if(var_name == "pt_ttbar") graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetTitle("jet pT (GeV) for ttbar");
    else if(var_name == "pt_Zprime") graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetTitle("jet pT (GeV) for Z'");
    else if(var_name == "Lxy") graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetTitle("Transverse SV vertex decay length Lxy (mm)"); 

        graphs[ifirst][i][i_truthlabel][i_WP]->GetYaxis()->SetTitle("\% jets passing WP cut (efficiency)");
        graphs[ifirst][i][i_truthlabel][i_WP]->GetYaxis()->SetTitleOffset(1.3);
 
        if(!drawRatio){
          graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetTitleSize(0.04);
          graphs[ifirst][i][i_truthlabel][i_WP]->GetXaxis()->SetLabelSize(0.04);
          graphs[ifirst][i][i_truthlabel][i_WP]->GetYaxis()->SetTitleSize(0.04);
          graphs[ifirst][i][i_truthlabel][i_WP]->GetYaxis()->SetLabelSize(0.04);
        }

        for(unsigned int it=0;it<InputFileNames.size();++it) {
          if(ifirst==static_cast<int>(it)) continue;
          graphs[it][i][i_truthlabel][i_WP]->Draw("L0");
        }//file name

        //Legend
        TLegend* lg2 = new TLegend(0.5,0.7,0.90,0.90);
        lg2->SetBorderSize(0);
        lg2->SetFillStyle(0);
        for(unsigned int it=0;it<InputFileNames.size();it++) {
          lg2->AddEntry(graphs[it][i][i_truthlabel][i_WP], leg_entry[it],"LF");

          if(writeHistos && it == 1){
            if(gSystem->AccessPathName(outputName)) {
	      //File doesn't exist already, make it
              TFile filerio(outputName,"RECREATE");
              filerio.cd();
              TDirectory* d = filerio.mkdir("BTag/"+HistoDir);
              filerio.cd("BTag/"+HistoDir);
              graphs[it][i][i_truthlabel][i_WP]->Write(graphs[it][i][i_truthlabel][i_WP]->GetName());
              filerio.Close();
            } else {
              //File already exist, just open it
              TFile filerio(outputName,"UPDATE");
              filerio.cd("BTag/"+HistoDir);
              graphs[it][i][i_truthlabel][i_WP]->Write(graphs[it][i][i_truthlabel][i_WP]->GetName());
              filerio.Close();
            }
          }

        }
        lg2->SetTextSize(0.03);
        if(drawRatio) lg2->SetTextSize(0.04);
        lg2->Draw();

        //Text
        VALID_LABEL(0.25,0.9,1,sample,drawRatio);
        myDescriptionLabel(taggers[i]+", "+tagger_WPs[i_WP]+"\% WP"+", "+truth_labels[i_truthlabel]+"-jets",0.25,0.9,1,drawRatio); 

        if(drawRatio){
          // lower plot will be in pad
          c2->cd();          // Go back to the main canvas before defining pad2

          //range on Y axis for ratio plot //
          double ymin_ratio = 0.9;
          double ymax_ratio = 1.1;
          double scale = 0.3; // scale up and down by 20% to set the ratio bondaries

          double ymaxratio = v_ratio_ymax_ymin[i][i_truthlabel][i_WP].first;
          double yminratio = v_ratio_ymax_ymin[i][i_truthlabel][i_WP].second;
      
          if(yminratio<ymin_ratio) ymin_ratio = yminratio-scale*(1.-yminratio);
          if(ymaxratio>ymax_ratio) ymax_ratio = ymaxratio+scale*(ymaxratio-1.);

          TPad *pad22 = new TPad("pad22"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber, "pad22"+taggers[i]+truth_labels[i_truthlabel]+tagger_WPs[i_WP]+myRandomNumber, 0, 0.05, 1, 0.3);
          pad22->Range(Xmin,ymin_ratio,Xmax,ymax_ratio); // ??
          pad22->SetLeftMargin(0.15);
          pad22->SetTopMargin(0);
          pad22->SetBottomMargin(0.35);
          pad22->Draw();
          pad22->cd();       // pad2 becomes the current pad
          gStyle->SetEndErrorSize(0);
          gPad->SetTicks();  
          gPad->SetLogy(0);

          graph_ratios[i][i_truthlabel][i_WP][0]->SetMinimum(ymin_ratio);
          graph_ratios[i][i_truthlabel][i_WP][0]->SetMaximum(ymax_ratio);

          graph_ratios[i][i_truthlabel][i_WP][0]->SetFillColor(18);

          //Draw error band on ratio?
          if (!drawErrRatio) {
            graph_ratios[i][i_truthlabel][i_WP][0]->Draw("alx0"); // A: axis L: simple line 
          } else {
            graph_ratios[i][i_truthlabel][i_WP][0]->Draw("AC3"); // 
            graph_ratios[i][i_truthlabel][i_WP][0]->Draw("lx"); //x: without error bar  
          }

          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetLabelSize(0.);
          TGaxis *axis22 = new TGaxis( Xmin, 1.001*ymin_ratio, Xmin, 0.999*ymax_ratio, 1.001*ymin_ratio, 0.999*ymax_ratio, 505,""); 
          axis22->SetLabelFont(43); // Absolute font size in pixel (precision 3)
          axis22->SetLabelSize(15);
          axis22->Draw();        

          // Y axis ratio plot settings
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetTitle("Ratio");
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetNdivisions(505);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetTitleSize(20);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetTitleFont(43);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->SetTitleOffset(1.5);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetYaxis()->CenterTitle();
      
          // X axis ratio plot settings
          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitleSize(20);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitleFont(43);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitleOffset(4.);
          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetLabelSize(15);
      if(var_name == "pt_ttbar") graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitle("jet pT (GeV) for ttbar");
      else if(var_name == "pt_Zprime") graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitle("jet pT (GeV) for Z'");
      else if(var_name == "Lxy") graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetTitle("Transverse SV vertex decay length Lxy (mm)"); 

          graph_ratios[i][i_truthlabel][i_WP][0]->GetXaxis()->SetLimits(Xmin,Xmax); // ??
      
          TLine* line = new TLine(Xmin, 1., Xmax, 1.); // ??
          line->Draw("same");
          line->SetLineStyle(lstyle);

          for(unsigned int it=1;it<graph_ratios[i][i_truthlabel][i_WP].size();it++){
	    graph_ratios[i][i_truthlabel][i_WP][it]->Draw("Lxsame");
          }

        } // drawRatio
    
        //check if the directory where to save histo exits
        if(gSystem->AccessPathName(HistoDir+"eff_vs_"+var_name)){
          std::cout << HistoDir<< "eff_vs_"+var_name+" directory does not exist. Will create one." << std::endl;
          gSystem->Exec("mkdir -p "+HistoDir);
          gSystem->Exec("mkdir "+HistoDir+"eff_vs_"+var_name);
        } 
    
        plot_name = HistoDir+"eff_vs_"+var_name+"/eff_vs_"+var_name+"_"+taggers[i]+"_"+truth_labels[i_truthlabel]+"-jets"+"_"+tagger_WPs[i_WP]+"_WP.pdf";
        c2->SaveAs(plot_name.Data(),"RECREATE");

      }
    } 
  } // tagger i

} // plotGraphs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Draw_PhysVal_btagROC(TString inputMC="ttbar",
			  TString jet_type="PFlow",
			  TString reffile="files_merged/merged_NTUP_PHYSVAL_ref.root",
			  TString testfile="files_merged/merged_NTUP_PHYSVAL_test.root",
			  TString outputName="MyHistos.root",
			  const vector<TString>& def_taggers=vector<TString>{"IP3D","DIPS","SV1","DL1dv01","GN2v01","GN2Xv01"},
			  bool writeHistos=true){

    jetType=jet_type;
    HistoDir = "ROC_"+jetType+"/";

    if (inputMC=="Zprime"){
        EffMin=0.1;
    }
    else if (inputMC=="ttbar") {
        EffMin=0.55;
    }

  //Confine all the code things into two knobs 
  TH1::SetDefaultSumw2(true);
  fill_WP_values();

  ////////////////////////////////// 
  //      validation              //
  ////////////////////////////////// 
  //ttbar
  TString sample = inputMC == "ttbar" ? "#sqrt{s}=13 TeV, t#bar{t}" : "#sqrt{s}=13 TeV, Z^{'}";
  TString MC=inputMC;
  cout<<"inputMC"<<inputMC<<endl;
  cout<<"sample: "<<sample<<endl;

  vector<TString> InputFilesNames = {reffile, testfile};
  if (gSystem->AccessPathName(reffile,kFileExists)) {
    cout << "--File not found, check that the inputs are there: " << reffile << endl;
    gSystem->Exit(1);
  }
  if (gSystem->AccessPathName(testfile,kFileExists)) { 
    cout << "--File not found, check that the inputs are there: " << testfile << endl;
    gSystem->Exit(1);
  }

  if (def_taggers.size() != 0){
    taggers = def_taggers;
  }

  ///////////////////
  //Plot ROC curves 
  bool drawRatio=true;
  bool drawErrRatio=false;
  bool drawCtag=true;
  plotGraphs(InputFilesNames,MC,sample,leg_entry,drawRatio,drawErrRatio, false, writeHistos, outputName);
  plotGraphs(InputFilesNames,MC,sample,leg_entry,drawRatio,drawErrRatio,drawCtag, writeHistos, outputName);
  plotGraphsEffVsVar("Lxy", InputFilesNames,MC,sample,leg_entry,drawRatio,drawErrRatio, writeHistos, outputName);

  drawRatio=true;
  if(MC == "ttbar"){
    //cout << "ttbar"<<endl;
    plotGraphsEffVsVar("pt_ttbar", InputFilesNames,MC,sample,leg_entry,drawRatio,drawErrRatio, writeHistos, outputName);
  }
  else if (MC == "Zprime"){
    plotGraphsEffVsVar("pt_Zprime", InputFilesNames,MC,sample,leg_entry,drawRatio,drawErrRatio, writeHistos, outputName);
  }
  ///////////////////

} // end of Draw_PhysVal_btagROC()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

