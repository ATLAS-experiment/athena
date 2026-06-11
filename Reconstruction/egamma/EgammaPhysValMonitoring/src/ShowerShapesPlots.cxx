/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "ShowerShapesPlots.h"

using CLHEP::GeV;

namespace Egamma{


  ShowerShapesPlots::ShowerShapesPlots(PlotBase* pParent, const std::string& sDir, std::string sParticleType):PlotBase(pParent, sDir), 
												       m_sParticleType(std::move(sParticleType)),
												       hadleak(nullptr),
												       weta1  (nullptr), 
												       weta2  (nullptr), 
												       de     (nullptr),
												       fracs1 (nullptr),
												       wtots1 (nullptr), 
												       f1     (nullptr),
												       Eratio(nullptr),
												       Rhad(nullptr),
												       Reta(nullptr),
												       Rphi(nullptr),
												       hadleakvset(nullptr),
												       weta1vset  (nullptr),
												       weta2vset  (nullptr), 
												       devset     (nullptr),
												       fracs1vset (nullptr),
												       wtots1vset (nullptr), 
												       f1vset     (nullptr),
												       Eratiovset(nullptr),
												       Rhadvset(nullptr),
												       Retavset(nullptr),
												       Rphivset(nullptr),
												       hadleakvseta(nullptr),
												       weta1vseta(nullptr),
												       weta2vseta(nullptr),
												       devseta(nullptr),
												       fracs1vseta(nullptr),
												       wtots1vseta(nullptr),
												       f1vseta(nullptr),
												       Eratiovseta(nullptr),
												       Rhadvseta(nullptr),
												       Retavseta(nullptr),
												       Rphivseta(nullptr)

  {}	

  void ShowerShapesPlots::initializePlots(){

        hadleak = Book1D("hadleak", "Hadronic leakage of " + m_sParticleType+"; E_{hadleak} (GeV);Entries", 100, -0.07, 0.13); 
    weta1   = Book1D("weta1", "W_{#etas1} of "+ m_sParticleType+";W_{#etas1};Entries", 100, 0., 1.);
    weta2   = Book1D("weta2", "W_{#etas2} of "+ m_sParticleType+";W_{#etas2};Entries", 100, 0., 0.03);
    de      = Book1D("de", "#DeltaE of "+ m_sParticleType+";#DeltaE (GeV);Entries", 250, 0., 0.5);
    fracs1  = Book1D("fracs1", "Fracs1 of "+ m_sParticleType+";Fracs1;Entries", 350, 0., 3.5);
    wtots1  = Book1D("wtots1", "W_{tots1} of "+ m_sParticleType+";W_{tots1};Entries", 100, 0., 10.);
    f1      = Book1D("f1", "f1 of "+ m_sParticleType+";f1;Entries" , 120, -0.2, 1.0);
    Eratio  = Book1D("Eratio", "Eratio of "+ m_sParticleType+";Eratio;Entries" , 100, 0., 1.0);
    Rhad    = Book1D("Rhad", "Rhad of "+ m_sParticleType+";Rhad;Entries", 100, -0.5, 2.);
    Reta    = Book1D("Reta", "Reta of "+ m_sParticleType+";Reta;Entries", 100, -0., 3.);
    Rphi    = Book1D("Rphi", "Rphi of "+ m_sParticleType+";Rphi;Entries", 100, -1., 1.);

    hadleakvset = Book2D("hadleakvset", "Hadronic leakage vs E_{T} of " + m_sParticleType+"; E_{hadleak} (GeV) ; E_{T} (GeV) ", 100, -0.07, 0.13, 200, 0., 200); 
    weta1vset   = Book2D("weta1vset", "W_{#etas1} vs E_{T} of "+ m_sParticleType+";W_{#etas1}; E_{T} (GeV) ", 100, 0., 1., 200, 0., 200);
    weta2vset   = Book2D("weta2vset", "W_{#etas2} vs E_{T} of "+ m_sParticleType+";W_{#etas2}; E_{T} (GeV) ", 100, 0., 0.03, 200, 0., 200);
    devset      = Book2D("devset", "#DeltaE vs E_{T} of "+ m_sParticleType+";#DeltaE (GeV); E_{T} (GeV) ", 250, 0., 0.5, 200, 0., 200);
    fracs1vset  = Book2D("fracs1vset", "Fracs1 vs E_{T} of "+ m_sParticleType+";Fracs1; E_{T} (GeV) ", 350, 0., 3.5, 200, 0., 200);
    wtots1vset  = Book2D("wtots1vset", "W_{tots1} vsE_{T} of "+ m_sParticleType+";W_{tots1}; E_{T} (GeV) ", 100, 0., 10., 200, 0., 200);
    f1vset      = Book2D("f1vset", "f1 vs E_{T} of "+ m_sParticleType+";f1; E_{T} (GeV) " , 100, 0., 1.0, 200, 0., 200);
    Eratiovset  = Book2D("Eratiovset", "Eratio vs E_{T} of "+ m_sParticleType+";Eratio; E_{T} (GeV) " , 100, 0., 1.0, 200, 0., 200);
    Rhadvset    = Book2D("Rhadvset", "Rhad vs E_{T} of "+ m_sParticleType+";Rhad;E_{T} (GeV)", 100, -0.5, 2., 200, 0., 200);
    Retavset    = Book2D("Retavset", "Reta vs E_{T} of "+ m_sParticleType+";Reta;E_{T} (GeV)", 100, -0., 3., 200, 0., 200);
    Rphivset    = Book2D("Rphivset", "Rphi vs E_{T} of "+ m_sParticleType+";Rphi;E_{T} (GeV)", 100, -1., 1., 200, 0., 200);
    
    hadleakvseta = Book2D("hadleakvseta", "Hadronic leakage vs E_{T} of " + m_sParticleType+"; E_{hadleak} (GeV) ; #eta ", 100, -0.07, 0.13, 1000,-5.,5.); 
    weta1vseta   = Book2D("weta1vseta", "W_{#etas1} vs E_{T} of "+ m_sParticleType+";W_{#etas1}; #eta ", 100, 0., 1., 1000,-5.,5.);
    weta2vseta   = Book2D("weta2vseta", "W_{#etas2} vs E_{T} of "+ m_sParticleType+";W_{#etas2}; #eta ", 100, 0., 0.03, 1000,-5.,5.);
    devseta      = Book2D("devseta", "#DeltaE vs E_{T} of "+ m_sParticleType+";#DeltaE (GeV); #eta ", 250, 0., 0.5, 1000,-5.,5.);
    fracs1vseta  = Book2D("fracs1vseta", "Fracs1 vs E_{T} of "+ m_sParticleType+";Fracs1; #eta ", 350, 0., 3.5, 1000,-5.,5.);
    wtots1vseta  = Book2D("wtots1vseta", "W_{tots1} vsE_{T} of "+ m_sParticleType+";W_{tots1}; #eta ", 100, 0., 10., 1000,-5.,5.);
    f1vseta      = Book2D("f1vseta", "f1 vs E_{T} of "+ m_sParticleType+";f1; #eta " , 100, 0., 1.0, 1000,-5.,5.);
    Eratiovseta  = Book2D("Eratiovseta", "Eratio vs #eta of "+ m_sParticleType+";Eratio; #eta " , 100, 0., 1.0, 1000,-5.,5.);
    Rhadvseta    = Book2D("Rhadvseta", "Rhad vs #eta of "+ m_sParticleType+";Rhad;#eta", 100, -0.5, 2., 1000,-5.,5.);
    Retavseta    = Book2D("Retavseta", "Reta vs #eta of "+ m_sParticleType+";Reta;#eta", 100, -0., 3., 1000,-5.,5.);
    Rphivseta    = Book2D("Rphivseta", "Rphi vs #eta of "+ m_sParticleType+";Rphi;#eta", 100, -1., 1., 1000,-5.,5.);
    
  }

  void ShowerShapesPlots::fill(const xAOD::Egamma& egamma, const xAOD::EventInfo& eventInfo) {

    float weight = 1.;
    weight = eventInfo.beamSpotWeight();

    float eta2 = fabs (egamma.caloCluster()->etaBE (2));  
    float et37 = egamma.caloCluster()->e() / cosh (eta2);
    float ethad(0);
    float ethad1(0);
    float raphad(0);
    float raphad1(0);
    if(egamma.showerShapeValue(ethad, xAOD::EgammaParameters::ethad )){
      raphad = et37 > 0. ? ethad / et37 : 0.;
    }
    if(egamma.showerShapeValue(ethad1, xAOD::EgammaParameters::ethad1 )){
      raphad1 =  et37 > 0. ? ethad1 / et37 : 0.;
    }
    float hadrleak = (eta2 >= 0.8 && eta2 < 1.37) ? raphad : raphad1;
    hadleak->Fill(hadrleak, weight);
    hadleakvset->Fill(hadrleak, egamma.pt()/GeV, weight);
    hadleakvseta->Fill(hadrleak, egamma.eta(), weight);

    float shweta1(0);
    float shweta2(0);
    if(egamma.showerShapeValue(shweta1, xAOD::EgammaParameters::weta1 )&&
       egamma.showerShapeValue(shweta2, xAOD::EgammaParameters::weta2 )){
      weta1->Fill(shweta1, weight);
      weta2->Fill(shweta2, weight);
      weta1vset->Fill(shweta1, egamma.pt()/GeV, weight);
      weta2vset->Fill(shweta2, egamma.pt()/GeV, weight);
      weta1vseta->Fill(shweta1, egamma.eta(), weight);
      weta2vseta->Fill(shweta2, egamma.eta(), weight);
    }
    
    float emin(0);
    float emax2(0);
    if(egamma.showerShapeValue(emin, xAOD::EgammaParameters::emins1 )&&
       egamma.showerShapeValue(emax2, xAOD::EgammaParameters::e2tsts1 )){

      de->Fill( (emax2 - emin)/GeV, weight);
      devset->Fill( (emax2 - emin)/GeV, egamma.pt()/GeV, weight);
      devseta->Fill( (emax2 - emin)/GeV, egamma.eta(), weight); 
    }

    float shfracs1(0);
    float shwtots1(0);
    float fracf1(0);
    float eRatio(0);    
    float rhad(0);
    float reta(0);
    float rphi(0);

    if(egamma.showerShapeValue(shfracs1, xAOD::EgammaParameters::fracs1 )){
      fracs1->Fill(shfracs1, weight);
      fracs1vset->Fill(shfracs1, egamma.pt()/GeV, weight);
      fracs1vseta->Fill(shfracs1, egamma.eta(), weight);
    } 

    if(egamma.showerShapeValue(shwtots1, xAOD::EgammaParameters::wtots1 )){
      wtots1->Fill(shwtots1, weight);
      wtots1vset->Fill(shwtots1, egamma.pt()/GeV, weight);
      wtots1vseta->Fill(shwtots1, egamma.eta(), weight);
    }
    
    if(egamma.showerShapeValue(fracf1, xAOD::EgammaParameters::f1 )){
      f1->Fill(fracf1, weight);
      f1vset->Fill(fracf1, egamma.pt()/GeV, weight);
      f1vseta->Fill(fracf1, egamma.eta(), weight);
    }

    if(egamma.showerShapeValue(eRatio, xAOD::EgammaParameters::Eratio )){
      Eratio->Fill(eRatio, weight);
      Eratiovset->Fill(eRatio, egamma.pt()/GeV, weight);
      Eratiovseta->Fill(eRatio, egamma.eta(), weight);
    }
    
    if(egamma.showerShapeValue(rhad, xAOD::EgammaParameters::Rhad )){
      Rhad->Fill(rhad, weight);
      Rhadvset->Fill(rhad, egamma.pt()/GeV, weight);
      Rhadvseta->Fill(rhad, egamma.eta(), weight);
    }
    
    if(egamma.showerShapeValue(reta, xAOD::EgammaParameters::Reta )){
      Reta->Fill(reta, weight);
      Retavset->Fill(reta, egamma.pt()/GeV, weight);
      Retavseta->Fill(reta, egamma.eta(), weight);
    }
    
    if(egamma.showerShapeValue(rphi, xAOD::EgammaParameters::Rphi )){
      Rphi->Fill(rphi, weight);
      Rphivset->Fill(rphi, egamma.pt()/GeV, weight);
      Rphivseta->Fill(rphi, egamma.eta(), weight);
    }
   
  } // end of fill

} // end of namespace Egamma 
