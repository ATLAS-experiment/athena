/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_xAODJetFilter_H
#define GENERATORFILTERS_xAODJetFilter_H

#include "GeneratorModules/GenFilter.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <cmath>


/// @brief Applies the generator level filter for jet candidates
/// @author I Hinchliffe, May 2002
class xAODJetFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

  struct McObj {
    // Constructors and destructor
    McObj();
    McObj(CLHEP::HepLorentzVector& p)
      : m_p (p),
        m_Nobj (0)
    { }
    ~McObj() {}

    // Get functions
    CLHEP::HepLorentzVector& P() {return m_p;}
    double P(int i) {return m_p[i];}
    int Nobj() {return m_Nobj;}

    // Set functions
    void SetP(const CLHEP::HepLorentzVector& p) {m_p=p;}
    void SetP(const double&,const double&,const double&,const double&);
    void SetXYZM(const double&,const double&,const double&, const double&);
    void SetNobj(int& nn) {m_Nobj = nn;}

    // Overloaded operators for sorting on Pt
    bool operator<(const McObj& rhs) const {return m_p.perp()<rhs.m_p.perp();}
    bool operator>(const McObj& rhs) const {return m_p.perp()>rhs.m_p.perp();}

  protected:
    CLHEP::HepLorentzVector m_p;
    int m_Nobj;
  };


private:
   SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
   Gaudi::Property<double> m_userEta{this, "EtaRange", 2.7, " "}; //range over which triggers are searched for
   Gaudi::Property<double> m_userThresh{this, "JetThreshold", 17000., " "}; // lowest et jet
   Gaudi::Property<double> m_stop{this, "SeedThreshold", 1000., " "}; //seed tower threshold
   Gaudi::Property<double> m_cone{this, "ConeSize", 0.4, " "};  //cone sixe
   Gaudi::Property<int> m_gride{this, "GridSizeEta",2, " "}; //how many cells in eta
   Gaudi::Property<int> m_gridp{this, "GridSizePhi",2, " "}; //how many cells in phi
   Gaudi::Property<int> m_userNumber{this, "JetNumber",1, " "}; //how many are we looking for
   Gaudi::Property<bool> m_type{this, "JetType",true, " "}; // cone or grid to define jet

  static const int m_grphi = 105 ; // -CLHEP::pi to CLHEP::pi in units of 0.06 (approx)
  static const int m_greta = 200 ; // -6.0 to 6.0 in units 0.06 {approx}

  // Internal parameters
  double m_etaRange{}; //range over which search runs
  double m_emaxeta{}; // largest eta of bins
  double m_edeta{}; // size of eta bins
  double m_edphi{}; //size of phi bins
  int m_nphicell{}; // number of phi cells inside half cone
  int m_netacell{}; // number of eta cells inside half cone
  int m_nphicell2{}; // number of phi cells inside full cone
  int m_netacell2{}; // number of eta cells inside full cone
  std::vector<McObj> m_Jets; //store jets

};

#endif
