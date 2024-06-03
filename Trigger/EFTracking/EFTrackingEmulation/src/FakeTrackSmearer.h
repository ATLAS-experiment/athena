// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// Stolen from A.Cerri

#ifndef EFTRACKINGEMULATION_FAKETRACKSMEARER_H
#define EFTRACKINGEMULATION_FAKETRACKSMEARER_H

#include "TRandom3.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TF2.h"
#include "TH1F.h"
#include "TROOT.h"


#include "FTS_Track.h"
#include <cmath>
namespace FitFunctions {
  #include "FitFunctions/URD/d0Fitparam_N.C"
  #include "FitFunctions/URD/z0Fitparam_N.C"  
  #include "FitFunctions/URD/effFitparam_N.C"
  #include "FitFunctions/URD/effFitparam_LRT.C"
  #include "FitFunctions/URD/ptqoptFitparam_N.C"
}
 

class FakeTrackSmearer
{
 public:
  FakeTrackSmearer(const std::string & InstanceName, long long randomseed=0, bool verbose=false)
    : m_baseName (InstanceName),
      m_verbose (verbose),
      m_myRandom (new TRandom3(randomseed))
    {    
      Prepare();
      Tracks.clear(); 
      
      // Set these in order to define the scenario:
      //SetSigmaScaleFactor(1.0);
      //SetResolutionPtCutOff(5.0);
      //UseResolutionPtCutOff(true);  
      
    }
    //
  FakeTrackSmearer(const FakeTrackSmearer & other) = delete;
  FakeTrackSmearer & operator=(const FakeTrackSmearer & other) = delete;

// prepare the functions to use
  void Prepare()
  {
    printf("Entering Prepare\n");    
    std::string name;

    name="d0res_eta"+m_baseName; //pt=10GeV
    d0res_eta=new TF1(name.c_str(),[&](double*x,double*p){return d0ResFunc(x[0],p[0]=10.,0); },0.0,4.0,1);
    name="z0res_eta"+m_baseName;
    z0res_eta=new TF1(name.c_str(),[&](double*x,double*p){return z0ResFunc(x[0],p[0]=10.,0); },0.0,4.0,1);
    name="curvres_eta"+m_baseName;
    curvres_eta=new TF1(name.c_str(),[&](double*x,double*p){return curvResFunc(x[0],p[0]=10.,0); },0.0,4.0,1);
    
    name="d0ref_eta"+m_baseName;
    d0ref_eta=new TF1(name.c_str(),[&](double*x,double*p){return d0RefFunc(x[0],p[0]=10.,0); },0.0,4.0,1);
    name="z0ref_eta"+m_baseName;
    z0ref_eta=new TF1(name.c_str(),[&](double*x,double*p){return z0RefFunc(x[0],p[0]=10.,0); },0.0,4.0,1);

    
    name="d0res_pt"+m_baseName; //eta=1
    d0res_pt=new TF1(name.c_str(),[&](double*x,double*p){return d0ResFunc(p[0]=1.,x[0],0); },1.0,200.0,1);
    name="z0res_pt"+m_baseName;
    z0res_pt=new TF1(name.c_str(),[&](double*x,double*p){return z0ResFunc(p[0]=1.,x[0],0); },1.0,200.0,1);
    name="curvres_pt"+m_baseName;
    curvres_pt=new TF1(name.c_str(),[&](double*x,double*p){return curvResFunc(p[0]=1.,x[0],0); },1.0,200.0,1);

    name="d0ref_pt"+m_baseName;
    d0ref_pt=new TF1(name.c_str(),[&](double*x,double*p){return d0RefFunc(p[0]=1.,x[0],0); },1.0,200.0,1);
    name="z0ref_pt"+m_baseName;
    z0ref_pt=new TF1(name.c_str(),[&](double*x,double*p){return z0RefFunc(p[0]=1.,x[0],0); },1.0,200.0,1);
   
    name="effLRT_d0"+m_baseName;
    m_parameterizedEfficiency_lowd0_LRT = 10.;
    m_parameterizedEfficiency_highd0_LRT = 400.;
    effLRT_d0=new TF1(name.c_str(),[&](double*x, double*p){p[0]=1.;return effFuncLRT(x[0],0); },0.,600.0,1);
 
  }


  double d0ResFunc(double eta,double pt,int verbose)
  {
    // add here other d0 res functions
    return FitFunctions::getd0ResParam_N(eta,pt,verbose)/1000.0;// convert to mm
  }

   double z0ResFunc(double eta,double pt,int verbose)
  {
    // add here other z0 res functions
    return FitFunctions::getz0ResParam_N(eta,pt,verbose)/1000.0; // convert to mm
  }
   
  double etaResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused))) 
  {
    // add here other eta res functions
    return 0.;
  }

  double phiResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    // add here other phi res functions
    return 0.;
  }

  double curvResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    // add here other curv res functions
    if (pt!=0.) return FitFunctions::getptqoptResParam_N(eta,pt,verbose)/pt;
    return 0.;
  }

// Reference functions, in case needed ////

 double d0RefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.; // no reference to scale
  }

  double z0RefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.; // no reference to scale
  }

double etaRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }

double phiRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }

double curvRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }

double effFunc(double eta,double pt,int verbose)
  {
    // add here other efficiency functions    
    return FitFunctions::getEffParam_N(eta,pt,verbose);
  }
  
double effFuncLRT(double d0, int verbose)
  {
    // add here other efficiency functions
    return FitFunctions::getEffParam_LRT(d0, m_parameterizedEfficiency_lowd0_LRT, m_parameterizedEfficiency_highd0_LRT, verbose);
  }

  void InitArray(double *a,int n,const double in[])
  {
    for (int i=0;i<n;i++) a[i]=in[i];
  }



  void SetInputTracksPtCut(double ptcut) {m_inPtCut=ptcut;};
  void SetOutputTracksPtCut(double ptcut) {m_outPtCut=ptcut;};
  void SetMatchRatio(double m) { m_FMatches=m;}; // NMatch/NOffline  i.e. k
  void SetFakeFraction(double f) {m_FakeFraction=f; }; // i.e. f
  void SetSigmaScaleFactor(double s) {m_SigmaScaleFactor=s; };
  void SetResolutionPtCutOff(double cutoff) {m_resolutionPtCutOff=cutoff; };
  void UseResolutionPtCutOff(bool use ) {m_useResolutionPtCutOff=use ; };

  void Clear() {m_ntracks=0; m_nfakes=0; Tracks.clear(); };

  int GetNTrueTracks() {return m_ntracks;        };
  int GetNFakes()      {return m_nfakes;         };
  int GetNTracks()     {return m_ntracks+m_nfakes; };

  void AddTrackAndTruth(double d0,double z0,double curv,double eta,double phi)
  {
    m_useTrackingTruth=true; m_useInputSigmas=false;
    AddTrack(d0,z0,curv,eta,phi);
    m_useTrackingTruth=true; m_useInputSigmas=false;
  }


  void AddTrack(double d0,double z0,double curv,double eta,double phi)
  {
    // Adding one or more tracks to this input
    // input curv is in GeV    
    bool verbose=m_verbose;
      
    double abseta = std::abs(eta);
    double abspt = std::abs(1.0/curv); //GeV
    double absd0 = std::abs(d0);
    if (verbose) printf("Smearer::AddTrack:  Initial track: curv = %f, phi=%f, eta=%f, d0=%f, z0=%f (pt=%f)\n", curv, phi, eta, d0, z0, abspt);
    
 
    bool condition = (abspt>m_inPtCut) &&
      (d0RefFunc(abseta,abspt,verbose)>0.0) &&
      (d0ResFunc(abseta,abspt,verbose)>0.0) &&
      (z0RefFunc(abseta,abspt,verbose)>0.0) &&
      (z0ResFunc(abseta,abspt,verbose)>0.0) &&
      (curvResFunc(abseta,abspt,verbose)>0.0);

    if (!condition)
      return;
    
    
    //Call here SetFakeFraction() and SetMatchRatio();
    
    double curvres = m_SigmaScaleFactor * curvResFunc(abseta,abspt,0);
    double phires  = m_SigmaScaleFactor * phiResFunc(abseta,abspt,0);
    double etares  = m_SigmaScaleFactor * etaResFunc(abseta,abspt,0);
    double d0res   = m_SigmaScaleFactor * d0ResFunc(abseta,abspt,0);
    double z0res   = m_SigmaScaleFactor * z0ResFunc(abseta,abspt,0);
    if (verbose) printf("Smearer::AddTrack:  Smearing parameters: curv = %f, phi=%f, eta=%f, d0=%f, z0=%f\n", curvres, phires, etares, d0res, z0res);

   
  #ifdef STANDALONE_FAKETRACKSMEARER
    d0Narrow->Fill(d0res);
    z0Narrow->Fill(z0res);
  #endif
    
    
  #ifdef STANDALONE_FAKETRACKSMEARER
    d0Sim->Fill(s_narrow_d0_model->Eval(abseta));
    z0Sim->Fill(s_narrow_z0_model->Eval(abseta));
  #endif



    double avgntracks=(m_nominalEfficiency> m_FMatches)?m_nominalEfficiency: m_FMatches;
    double eff = m_nominalEfficiency;
    if (m_parameterizedEfficiency) {
      eff = eff * effFunc(abseta,abspt,verbose);
    } 
    else if (m_parameterizedEfficiency_LRT) {
      eff = eff * effFuncLRT( absd0, verbose);
    } 
    
    int ntracks=( m_myRandom->Rndm()<eff)?1:0;
    
    if (m_produceFakes)
      {
        if (m_useCoinToss) {
          if (avgntracks>m_nominalEfficiency)     ntracks+= m_myRandom->Poisson(avgntracks-m_nominalEfficiency);
          else if (avgntracks<m_nominalEfficiency) ntracks=( m_myRandom->Rndm()<avgntracks)?1:0;
        }
        else
          ntracks= m_myRandom->Poisson(avgntracks);
      }    

    if (verbose) printf("Smearer::AddTrack:  Now producing %d tracks\n", ntracks);
    for (int i=0;i<ntracks;i++)
      {

        // Creating close track
        double gend0,genz0, geneta, genphi, gencurv;
        gend0 = m_myRandom->Gaus(d0,d0res);
        genz0 = m_myRandom->Gaus(z0,z0res);
        geneta= m_myRandom->Gaus(eta,etares);
        genphi= m_myRandom->Gaus(phi,phires);
        gencurv= m_myRandom->Gaus(curv,curvres);

        EFTrackingSmearing::FTS_Track Track(
              1./gencurv, //pT
              geneta,
              genphi,
              gend0,
              genz0,
              curvres/gencurv/gencurv, // res pT
              etares,
              phires,
              d0res,
              z0res,
              false
        );
        if (verbose) printf("Smearer::AddTrack:  Producing this track: curv = %f, phi=%f, eta=%f, d0=%f, z0=%f pt=%f (ptres=%f)\n",gencurv, genphi, geneta, gend0, genz0, Track.pt(), Track.sigma_pt());
        if (std::abs(Track.pt())>=m_outPtCut)
          {
            Tracks.push_back(Track);
            m_ntracks++;
            if (verbose) printf("Smearer::AddTrack:  Adding track with pt=%f ==> track #%d\n",Track.pt(), m_ntracks);
          }
          else if (verbose) printf("Smearer::AddTrack: No track added because of the output pt cut\n");

      }

        
    // AddTrack Done
    return;      
  };

  void PlotD0Res(double eta)
  {
    TCanvas *d0rescanvas=(TCanvas *)gROOT->FindObjectAny("d0rescanvas");
    if (d0rescanvas==NULL) d0rescanvas=new TCanvas("d0rescanvas");
    d0res_pt->SetParameter(0,eta);
    d0ref_pt->SetParameter(0,eta);
    d0res_pt->Draw();
    d0ref_pt->SetLineColor(4);
    d0ref_pt->Draw("same");
  }


  void EnableFakes(bool enable=true)
  {
    m_produceFakes=enable;
    if (not Tracks.empty())
      printf("Smearer::EnableFakes: Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer\n");
  }

  void FakeKillerEnable(bool enable=true)
  {
    m_fakeKillerEnable=enable;
    if (not Tracks.empty())
      printf("Smearer::FakeKillerEnable:  Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (FakeKillerEnable)\n");
  }

  void IncludeFakesInResolutionCalculation(bool enable=true)
  {
    m_includeFakesInResolutionCalculation=enable;
    if (not Tracks.empty())
      printf("Smearer::IncludeFakesInResolutionCalculation:  Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (IncluldeFakesInResolutionCalculation)\n");
  }

  void UseCoinToss(bool enable=true)
  {
    m_useCoinToss=enable;
    if (not Tracks.empty())
      printf("Smearer::UseCoinToss:  Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (UseCoinToss)\n");
  }

  void SetTrackingEfficiency(double epsilon=0.95)
  {
    m_nominalEfficiency=epsilon;
  }

  void SetParameterizedEfficiency(bool param = false)
  {
    m_parameterizedEfficiency=param;
  }

  void SetParameterizedEfficiency_LRT(bool param = false)
  {
    m_parameterizedEfficiency_LRT=param;
  }

  void SetParameterizedEfficiency_highd0_LRT(double d0)
  {
    m_parameterizedEfficiency_highd0_LRT = d0;
  }
  
  void SetParameterizedEfficiency_lowd0_LRT(double d0)
  {
    m_parameterizedEfficiency_lowd0_LRT = d0;
  }

  std::vector<EFTrackingSmearing::FTS_Track> Tracks;
  double z0(int idx)    {return Tracks[idx].z0();};
  double d0(int idx)    {return Tracks[idx].d0();};
  double phi(int idx)   {return Tracks[idx].phi();};
  double pt(int idx)    {return Tracks[idx].pt();};
  double curv(int idx)  {return Tracks[idx].pt();};
  double eta(int idx)   {return Tracks[idx].eta();};

  
  TH1F *d0Narrow = nullptr;
  TH1F *z0Narrow = nullptr;
  TH1F *d0Sim = nullptr;
  TH1F *z0Sim = nullptr;

  TF1 *d0res_eta = nullptr;
  TF1 *d0ref_eta = nullptr;
  TF1 *z0res_eta = nullptr;
  TF1 *z0ref_eta = nullptr;
  TF1 *d0res_pt = nullptr;
  TF1 *d0ref_pt = nullptr;
  TF1 *z0res_pt = nullptr;
  TF1 *z0ref_pt = nullptr;
  TF1 *curvres_eta = nullptr;
  TF1 *curvres_pt = nullptr;
  TF1 *effLRT_d0 = nullptr;


 private:
  std::string m_baseName;
  int m_ntracks = 0;
  int m_nfakes = 0;
  bool m_verbose = false;
  TRandom3 *m_myRandom = nullptr;

  double m_FMatches = 1.; // NMatch/NOffline  i.e. k
  double m_FakeFraction = 0.;
  double m_SigmaScaleFactor = 1.0;
  double m_nominalEfficiency = 0.95;

  bool m_parameterizedEfficiency = false;
  bool m_parameterizedEfficiency_LRT = false;
  bool m_includeFakesInResolutionCalculation =  false;
  bool m_fakeKillerEnable = false;
  bool m_useCoinToss = false;
  bool m_useInputSigmas = false;
  bool m_produceFakes = true;
  bool m_useResolutionPtCutOff = false;
  bool m_useTrackingTruth = true;
  double m_resolutionPtCutOff=0.0;
  double m_inPtCut = 0.0;
  double m_outPtCut = 1.0;
  double m_parameterizedEfficiency_highd0_LRT = 0.;
  double m_parameterizedEfficiency_lowd0_LRT = 0.;
  

};

#endif
