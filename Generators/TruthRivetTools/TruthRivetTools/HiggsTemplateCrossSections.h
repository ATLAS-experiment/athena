/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHRIVETTOOLS_HIGGSTEMPLATECROSSSECTIONS_H
#define TRUTHRIVETTOOLS_HIGGSTEMPLATECROSSSECTIONS_H

// -*- C++ -*-
#include "Rivet/Analysis.hh"
#include "Rivet/Particle.hh"
#include "Rivet/Projections/FastJets.hh"

// Definition of the StatusCode and Category enums
// Note: the Template XSec Defs *depends* on having included
//  the TLorentzVector header *before* it is included -- it
//  uses the include guard from TLorentzVector to decide
//  what is available
#include "TLorentzVector.h"
#include "TruthRivetTools/HiggsTemplateCrossSectionsDefs.h"
#include "AtlasHepMC/Relatives.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "CxxUtils/checker_macros.h"
#include <string>
#include <string_view>


namespace Rivet {

  /// @class HiggsTemplateCrossSections
  /// @brief  Rivet routine for classifying MC events according to the Higgs template cross section categories
  /// @author Jim Lacey (DESY) <james.lacey@cern.ch,jlacey@desy.de>
  /// @author Dag Gillberg (Carleton University) <dag.gillberg@cern.ch>
  class HiggsTemplateCrossSections : public Analysis {
  public:
    // Constructor
    HiggsTemplateCrossSections()
      : Analysis("HiggsTemplateCrossSections"),
        m_HiggsProdMode(HTXS::UNKNOWN), m_HiggsDecayMode(HTXS::UNKNOWNDecay) {}

  public:

    /// @name Utility methods
    /// Methods to identify the Higgs boson and
    /// associated vector boson and to build jets
    /// @{

    /// follow a "propagating" particle and return its last instance
    Particle getLastInstance(const Particle & ptcl) const {
      if ( ptcl.genParticle()->end_vertex() ) {
        if ( !hasChild(ptcl.genParticle(),ptcl.pid()) ) return ptcl;
        else return getLastInstance(ptcl.children()[0]);
      }
      return ptcl;
    }

    /// @brief Whether particle p originate from any of the ptcls
    bool originateFrom(const Particle& p, const Particles& ptcls ) const {
      auto prodVtx = p.genParticle()->production_vertex();
      if (prodVtx == nullptr) return false;
      // for each ancestor, check if it matches any of the input particles
      for (auto ancestor:Rivet::HepMCUtils::particles(std::move(prodVtx),Relatives::ANCESTORS)){
        for ( const auto & part:ptcls )
          if ( ancestor==part.genParticle() ) return true;
      }
      // if we get here, no ancestor matched any input particle
      return false;
    }

    /// @brief Whether particle p originates from p2
    bool originateFrom(const Particle& p, const Particle& p2 ) const {
      Particles ptcls = {p2}; return originateFrom(p,ptcls);
    }

    /// @brief Checks whether the input particle has a child with a given PDGID
    bool hasChild(HepMC::ConstGenParticlePtr ptcl, int pdgID) const {
      for (const Particle& child:Particle(*ptcl).children())
        if (child.pid()==pdgID) return true;
      return false;
    }

    /// @brief Checks whether the input particle has a parent with a given PDGID
    bool hasParent(HepMC::ConstGenParticlePtr ptcl, int pdgID) const {
      for (auto parent:Rivet::HepMCUtils::particles(ptcl->production_vertex(),Relatives::PARENTS))
        if (parent->pdg_id()==pdgID) return true;
      return false;
    }

    /// @brief Return true is particle decays to quarks
    bool quarkDecay(const Particle &p) const {
      for (const Particle& child:p.children())
        if (PID::isQuark(child.pid())) return true;
      return false;
    }

    /// @brief Return true if particle decays to charged leptons.
    bool ChLeptonDecay(const Particle &p) const {
      for (const Particle& child:p.children())
        if (
#if RIVET_VERSION_CODE >= 40000
          PID::isChargedLepton(child.pid())
#else
          PID::isChLepton(child.pid())
#endif // RIVET_VERSION_CODE
          ) return true;
      return false;
    }

    /// @brief Returns the classification object with the error code set.
    ///        Prints an warning message, and keeps track of number of errors
    HiggsClassification error(HiggsClassification &cat, HTXS::ErrorCode err,
                              std::string_view msg={}, int NmaxWarnings=20) const {
      // Set the error, and keep statistics
      cat.errorCode = err;
      const auto errIndex = static_cast<std::size_t>(err);
      if (errIndex < std::size(m_errorCount)) {
        ++m_errorCount[errIndex];
      } else {
        MSG_WARNING("Invalid HTXS error code: " << errIndex);
      }
      // Print warning message to the screen/log
      static std::atomic<int> Nwarnings = 0;
      if ( !msg.empty() && ++Nwarnings < NmaxWarnings )
          MSG_WARNING(msg);

      return cat;
    }
    /// @}

    /// @brief Main classificaion method.
    HiggsClassification classifyEvent(const Event& event, const HTXS::HiggsProdMode prodMode, const HTXS::HiggsDecayMode decayMode ) const {

      // the classification object
      HiggsClassification cat;
      cat.prodMode = prodMode;
      cat.errorCode = HTXS::UNDEFINED;
      cat.stage0_cat = HTXS::Stage0::UNKNOWN;
      cat.stage1_cat_pTjet25GeV = HTXS::Stage1::UNKNOWN;
      cat.stage1_cat_pTjet30GeV = HTXS::Stage1::UNKNOWN;
      cat.stage1_2_cat_pTjet25GeV = HTXS::Stage1_2::UNKNOWN;
      cat.stage1_2_cat_pTjet30GeV = HTXS::Stage1_2::UNKNOWN;
      cat.stage1_2_fine_cat_pTjet25GeV = HTXS::Stage1_2_Fine::UNKNOWN;
      cat.stage1_2_fine_cat_pTjet30GeV = HTXS::Stage1_2_Fine::UNKNOWN;
      cat.stage1_3_cat_pTjet25GeV = HTXS::Stage1_3::UNKNOWN;
      cat.stage1_3_cat_pTjet30GeV = HTXS::Stage1_3::UNKNOWN;
      cat.stage1_3_fine_cat_pTjet25GeV = HTXS::Stage1_3_Fine::UNKNOWN;
      cat.stage1_3_fine_cat_pTjet30GeV = HTXS::Stage1_3_Fine::UNKNOWN;
      cat.isTHW = false;
      cat.decaystage0_cat = HTXS::Stage0::UNKNOWNDecay;

      if (prodMode == HTXS::UNKNOWN)
        return error(cat,HTXS::PRODMODE_DEFINED,
                     "Unkown Higgs production mechanism. Cannot classify event."
                     " Classification for all events will most likely fail.");

      /*****
       * Step 1.
       *  Idenfify the Higgs boson and the hard scatter vertex
       *  There should be only one of each.
       */

      auto HSvtx = HepMC::signal_process_vertex(event.genEvent());
      int Nhiggs=0;
      for (auto ptcl : Rivet::HepMCUtils::particles(event.genEvent()) ) {

        // a) Reject all non-Higgs particles
        if ( !PID::isHiggs(ptcl->pdg_id()) ) continue;
        // b) select only the final Higgs boson copy, prior to decay
        if ( ptcl->end_vertex() && !hasChild(ptcl,PID::HIGGS) ) {
          cat.higgs = Particle(ptcl); ++Nhiggs;
        }
        // c) if HepMC::signal_proces_vertex is missing
        //    set hard-scatter vertex based on first Higgs boson
        if ( HSvtx==nullptr && ptcl->production_vertex() && !hasParent(ptcl,PID::HIGGS) )
          HSvtx = ptcl->production_vertex();
      }

      // Make sure things are in order so far
      if (Nhiggs!=1)
        return error(cat,HTXS::HIGGS_IDENTIFICATION,
                     "Current event has "+std::to_string(Nhiggs)+" Higgs bosons. There must be only one.");
      if (cat.higgs.children().size()<2)
        return error(cat,HTXS::HIGGS_DECAY_IDENTIFICATION,
                     "Could not identify Higgs boson decay products.");

      if (HSvtx == nullptr)
        return error(cat,HTXS::HS_VTX_IDENTIFICATION,"Cannot find hard-scatter vertex of current event.");

      /*****
       * Step 2.
       *   Identify associated vector bosons
       */

      // Find associated vector bosons
      bool is_uncatdV = false;
      Particles uncatV_decays;
      FourMomentum uncatV_p4(0,0,0,0);
      FourVector uncatV_v4(0,0,0,0);
      int nWs=0, nZs=0;
      if ( isVH(prodMode) ) {
        for (auto ptcl:Rivet::HepMCUtils::particles(HSvtx,Relatives::CHILDREN)) {
          if (PID::isW(ptcl->pdg_id())) { ++nWs; cat.V=Particle(ptcl); }
          if (PID::isZ(ptcl->pdg_id())) { ++nZs; cat.V=Particle(std::move(ptcl)); }
        }
        if(nWs+nZs>0) cat.V = getLastInstance(cat.V);
        else {
          for (auto ptcl:Rivet::HepMCUtils::particles(HSvtx,Relatives::CHILDREN)) {
            if (!PID::isHiggs(ptcl->pdg_id())) {
              uncatV_decays += Particle(ptcl);
              uncatV_p4 += Particle(ptcl).momentum();
              uncatV_v4 += Particle(std::move(ptcl)).origin();
            }
          }
          is_uncatdV = true; cat.V = Particle(24,uncatV_p4,uncatV_v4);
        }
      }

      if ( !is_uncatdV ){

        if ( isVH(prodMode) && !cat.V.genParticle()->end_vertex() )
          return error(cat,HTXS::VH_DECAY_IDENTIFICATION,"Vector boson does not decay!");

        if ( isVH(prodMode) && cat.V.children().size()<2 )
          return error(cat,HTXS::VH_DECAY_IDENTIFICATION,"Vector boson does not decay!");

        if ( ( prodMode==HTXS::WH && (nZs>0||nWs!=1) ) ||
             ( (prodMode==HTXS::QQ2ZH||prodMode==HTXS::GG2ZH) && (nZs!=1||nWs>0) ) )
          return error(cat,HTXS::VH_IDENTIFICATION,"Found "+std::to_string(nWs)+" W-bosons and "+
                       std::to_string(nZs)+" Z-bosons. Inconsitent with VH expectation.");
      }

      // Find and store the W-bosons from ttH->WbWbH
      Particles Ws;
      if ( prodMode==HTXS::TTH || prodMode==HTXS::TH ){
        // loop over particles produced in hard-scatter vertex
        for ( auto ptcl : Rivet::HepMCUtils::particles(HSvtx,Relatives::CHILDREN) ) {
          if ( !PID::isTop(ptcl->pdg_id()) ) continue;
          Particle top = getLastInstance(Particle(std::move(ptcl)));
          if ( top.genParticle()->end_vertex() )
            for (const auto &child:top.children())
              if ( PID::isW(child.pid()) ) Ws += getLastInstance(child);
        }
      }

      // Make sure result make sense
      if ( (prodMode==HTXS::TTH && Ws.size()<2) || (prodMode==HTXS::TH && Ws.size()<1 ) )
        return error(cat,HTXS::TOP_W_IDENTIFICATION,"Failed to identify W-boson(s) from t-decay!");

      // Differentiate tHq from tHW by presence of W in HSvtx children.
      if (prodMode == HTXS::TH) {
        for ( auto ptcl : Rivet::HepMCUtils::particles(std::move(HSvtx),Relatives::CHILDREN) ) {
          if (PID::isW(ptcl->pdg_id())) {
            cat.isTHW = true;
            break;
          }
        }
      }

      /*****
       * Step 3.
       *   Build jets
       *   Make sure all stable particles are present
       */

      // Create a list of the vector bosons that decay leptonically
      // Either the vector boson produced in association with the Higgs boson,
      // or the ones produced from decays of top quarks produced with the Higgs
      Particles leptonicVs;
      if ( !is_uncatdV ){
        if ( isVH(prodMode) && !quarkDecay(cat.V) ) leptonicVs += cat.V;
      }else leptonicVs = std::move(uncatV_decays);
      for ( const auto & W:Ws ) if ( W.genParticle()->end_vertex() && !quarkDecay(W) ) leptonicVs += W;

      // Obtain all stable, final-state particles
      const Particles FS = apply<FinalState>(event, "FS").particles();
      Particles hadrons;
      Particles decayparticles;

      FourMomentum sum(0,0,0,0), vSum(0,0,0,0), hSum(0,0,0,0);
      for ( const Particle &p : FS ) {
        // Add up the four momenta of all stable particles as a cross check
        sum += p.momentum();
        // ignore particles from the Higgs boson
        if ( originateFrom(p,cat.higgs) ) { hSum += p.momentum(); decayparticles += p; continue; }
        // Cross-check the V decay products for VH
        if ( isVH(prodMode) && !is_uncatdV && originateFrom(p,Ws) ) vSum += p.momentum();
        // ignore final state particles from leptonic V decays
        if ( leptonicVs.size() && originateFrom(p,leptonicVs) ) continue;
        // All particles reaching here are considered hadrons and will be used to build jets
        hadrons += p;
      }

      cat.p4decay_higgs = hSum;
      cat.p4decay_V = vSum;

      FinalState fps_temp;
      FastJets jets(fps_temp,
#if RIVET_VERSION_CODE >= 40000
                    JetAlg::ANTIKT,
#else
                    FastJets::ANTIKT,
#endif // RIVET_VERSION_CODE
                    0.4 );
      jets.calc(hadrons);

      cat.jets25 = jets.jetsByPt( Cuts::pT > 25.0 );
      cat.jets30 = jets.jetsByPt( Cuts::pT > 30.0 );

      // check that four mometum sum of all stable particles satisfies momentum consevation
      if ( sum.pt()>0.1 )
        return error(cat,HTXS::MOMENTUM_CONSERVATION,"Four vector sum does not amount to pT=0, m=E=sqrt(s), but pT="+
                     std::to_string(sum.pt())+" GeV and m = "+std::to_string(sum.mass())+" GeV");

      // check if V-boson was not included in the event record but decay particles were
      // EFT contact interaction: return UNKNOWN for category but set all event/particle kinematics
      if(is_uncatdV)
        return error(cat,HTXS::VH_IDENTIFICATION,"Failed to identify associated V-boson!");

      /*****
       * Step 4.
       *   Classify and save output
       */

      // Apply the categorization categorization
      cat.isZ2vvDecay = false;
      if( (prodMode==HTXS::GG2ZH || prodMode==HTXS::QQ2ZH) && !quarkDecay(cat.V) && !ChLeptonDecay(cat.V) ) cat.isZ2vvDecay = true;
      cat.stage0_cat = getStage0Category(prodMode,cat.higgs,cat.V);
      cat.stage1_cat_pTjet25GeV = getStage1Category(prodMode,cat.higgs,cat.jets25,cat.V);
      cat.stage1_cat_pTjet30GeV = getStage1Category(prodMode,cat.higgs,cat.jets30,cat.V);
      cat.stage1_2_cat_pTjet25GeV = getStage1_2_Category(prodMode,cat.higgs,cat.jets25,cat.V);
      cat.stage1_2_cat_pTjet30GeV = getStage1_2_Category(prodMode,cat.higgs,cat.jets30,cat.V);
      cat.stage1_2_fine_cat_pTjet25GeV = getStage1_2_Fine_Category(prodMode,cat.higgs,cat.jets25,cat.V);
      cat.stage1_2_fine_cat_pTjet30GeV = getStage1_2_Fine_Category(prodMode,cat.higgs,cat.jets30,cat.V);
      cat.stage1_3_cat_pTjet25GeV = getStage1_3_Category(prodMode,cat.higgs,cat.jets25,cat.V);
      cat.stage1_3_cat_pTjet30GeV = getStage1_3_Category(prodMode,cat.higgs,cat.jets30,cat.V);
      cat.stage1_3_fine_cat_pTjet25GeV = getStage1_3_Fine_Category(prodMode,cat.higgs,cat.jets25,cat.V,cat.isTHW);
      cat.stage1_3_fine_cat_pTjet30GeV = getStage1_3_Fine_Category(prodMode,cat.higgs,cat.jets30,cat.V,cat.isTHW);
      cat.errorCode = HTXS::SUCCESS; ++m_errorCount[HTXS::SUCCESS];

      // Apply the Higgs decay categorization
      if (decayMode != HTXS::HiggsDecayMode::UNKNOWNDecay) cat.decaystage0_cat = getStage0DecayCategory(cat.higgs, decayparticles, cat.decay_observables, cat.decay_cuts_passed);

      return cat;
    }

    /// @name Categorization methods
    /// Methods to assign the truth category based
    /// on the identified Higgs boson and associated
    /// vector bosons and/or reconstructed jets
    /// @{

    /// @brief Return bin index of x given the provided bin edges. 0=first bin, -1=underflow bin.
int getBin(double x, const std::vector<double>& bins) const {
    if (bins.empty() || x < bins.front()) {
        throw std::invalid_argument("Input value is out of bin range or bins vector is empty.");
    }

    for (size_t i = 1; i < bins.size(); ++i) {
        if (x < bins[i]) {
            return static_cast<int>(i - 1);
        }
    }

    return static_cast<int>(bins.size() - 1);
}

    /// @brief VBF topolog selection
    /// 0 = fail loose selction: m_jj > 400 GeV and Dy_jj > 2.8
    /// 1 pass loose, but fail additional cut pT(Hjj)<25. 2 pass tight selection
    int vbfTopology(const Jets &jets, const Particle &higgs) const {
      if (jets.size()<2) return 0;
      const FourMomentum &j1=jets[0].momentum(), &j2=jets[1].momentum();
      bool VBFtopo = (j1+j2).mass() > 400.0 && std::abs(j1.rapidity()-j2.rapidity()) > 2.8;
      return VBFtopo ? (j1+j2+higgs.momentum()).pt()<25 ? 2 : 1 : 0;
    }
    /// @brief VBF topology selection
    /// 0 = fail loose selection: m_jj > 350 GeV
    /// 1 pass loose, but fail additional cut pT(Hjj)<25. 2 pass pT(Hjj)>25 selection
    /// 3 pass tight (m_jj>700 GeV), but fail additional cut pT(Hjj)<25. 4 pass pT(Hjj)>25 selection
    int vbfTopology_Stage1_2(const Jets &jets, const Particle &higgs) const {
      if (jets.size()<2) return 0;
      const FourMomentum &j1=jets[0].momentum(), &j2=jets[1].momentum();
      double mjj = (j1+j2).mass();
      if(mjj>350 && mjj<=700) return (j1+j2+higgs.momentum()).pt()<25 ? 1 : 2;
      else if(mjj>700) return (j1+j2+higgs.momentum()).pt()<25 ? 3 : 4;
      else return 0;
    }
    /// @brief VBF topology selection for Stage1_2
    /// 0 = fail loose selection: m_jj > 350 GeV
    /// 1 pass loose, but fail additional cut pT(Hjj)<25. 2 pass pT(Hjj)>25 selection
    /// 3 pass 700<m_jj<1000 GeV, but fail additional cut pT(Hjj)<25. 4 pass pT(Hjj)>25 selection
    /// 5 pass 1000<m_jj<1500 GeV, but fail additional cut pT(Hjj)<25. 6 pass pT(Hjj)>25 selection
    /// 7 pass m_jj>1500 GeV, but fail additional cut pT(Hjj)<25. 8 pass pT(Hjj)>25 selection
    int vbfTopology_Stage1_2_Fine(const Jets &jets, const Particle &higgs) const {
      if (jets.size()<2) return 0;
      const FourMomentum &j1=jets[0].momentum(), &j2=jets[1].momentum();
      double mjj = (j1+j2).mass();
      if(mjj>350 && mjj<=700) return (j1+j2+higgs.momentum()).pt()<25 ? 1 : 2;
      else if(mjj>700 && mjj<=1000) return (j1+j2+higgs.momentum()).pt()<25 ? 3 : 4;
      else if(mjj>1000 && mjj<=1500) return (j1+j2+higgs.momentum()).pt()<25 ? 5 : 6;
      else if(mjj>1500) return (j1+j2+higgs.momentum()).pt()<25 ? 7 : 8;
      else return 0;
    }

  /// @brief VBF topology selection for Stage1_3
  /// Includes additional deltaphijj binning
  int vbfTopology_Stage1_3_Fine(const Jets &jets, const Particle &higgs) const {
    if (jets.size() < 2) return 0;
    const FourMomentum &j1 = jets[0].momentum(), &j2 = jets[1].momentum();
    double mjj = (j1 + j2).mass();
    double pthjj = (j1 + j2 + higgs.momentum()).pt();
    double deltaphijj =
      j1.eta() > j2.eta()
      ? deltaPhi(j1, j2)
      : -1*deltaPhi(j1, j2);
    // mjj-pthjj binning
    int mjj_pthjj_bin = 0;
    if (mjj > 350 && mjj <= 700)
      mjj_pthjj_bin = pthjj < 25 ? 1 : 2;
    else if (mjj > 700 && mjj <= 1000)
      mjj_pthjj_bin = pthjj < 25 ? 3 : 4;
    else if (mjj > 1000 && mjj <= 1500)
      mjj_pthjj_bin = pthjj < 25 ? 5 : 6;
    else if (mjj > 1500)
      mjj_pthjj_bin = pthjj < 25 ? 7 : 8;
    else
      mjj_pthjj_bin = 0;
    // deltaphijj binning
    constexpr double pi = 3.14159265358979323846;
    int deltaphijj_bin = mjj > 350 ? 8*getBin(deltaphijj, {-1*pi, -0.5*pi, 0, 0.5*pi, pi}) : 0;
    // total vbfTopo binning
    return deltaphijj_bin + mjj_pthjj_bin;
  }


    /// @brief Whether the Higgs is produced in association with a vector boson (VH)
    bool isVH(HTXS::HiggsProdMode p) const { return p==HTXS::WH || p==HTXS::QQ2ZH || p==HTXS::GG2ZH; }

    /// @brief Stage-0 HTXS categorization
    HTXS::Stage0::Category getStage0Category(const HTXS::HiggsProdMode prodMode,
                                             const Particle &higgs,
                                             const Particle &V) const {
      using namespace HTXS::Stage0;
      int ctrlHiggs = std::abs(higgs.rapidity())<2.5;
      // Special cases first, qq→Hqq
      if ( (prodMode==HTXS::WH||prodMode==HTXS::QQ2ZH) && quarkDecay(V) ) {
        return ctrlHiggs ? VH2HQQ : VH2HQQ_FWDH;
      } else if ( prodMode==HTXS::GG2ZH && quarkDecay(V) ) {
        return Category(HTXS::GGF*10 + ctrlHiggs);
      }
      // General case after
      return  Category(prodMode*10 + ctrlHiggs);
    }

    /// @brief Stage-1 categorization
    HTXS::Stage1::Category getStage1Category(const HTXS::HiggsProdMode prodMode,
                                             const Particle &higgs,
                                             const Jets &jets,
                                             const Particle &V) const {
      using namespace HTXS::Stage1;
      int Njets=jets.size(), ctrlHiggs = std::abs(higgs.rapidity())<2.5, fwdHiggs = !ctrlHiggs;
      double pTj1 = jets.size() ? jets[0].momentum().pt() : 0;
      int vbfTopo = vbfTopology(jets,higgs);

      // 1. GGF Stage 1 categories
      //    Following YR4 write-up: XXXXX
      if (prodMode==HTXS::GGF || (prodMode==HTXS::GG2ZH && quarkDecay(V)) ) {
        if (fwdHiggs)        return GG2H_FWDH;
        if (Njets==0)        return GG2H_0J;
        else if (Njets==1)   return Category(GG2H_1J_PTH_0_60+getBin(higgs.pt(),{0,60,120,200}));
        else if (Njets>=2) {
          // events with pT_H>200 get priority over VBF cuts
          if(higgs.pt()<=200){
            if      (vbfTopo==2) return GG2H_VBFTOPO_JET3VETO;
            else if (vbfTopo==1) return GG2H_VBFTOPO_JET3;
          }
          // Njets >= 2jets without VBF topology
          return Category(GG2H_GE2J_PTH_0_60+getBin(higgs.pt(),{0,60,120,200}));
        }
      }
      // 2. Electroweak qq->Hqq Stage 1 categories
      else if (prodMode==HTXS::VBF || ( isVH(prodMode) && quarkDecay(V)) ) {
        if (std::abs(higgs.rapidity())>2.5) return QQ2HQQ_FWDH;
              if (pTj1>200) return QQ2HQQ_PTJET1_GT200;
        if (vbfTopo==2) return QQ2HQQ_VBFTOPO_JET3VETO;
        if (vbfTopo==1) return QQ2HQQ_VBFTOPO_JET3;
        double mjj = jets.size()>1 ? (jets[0].mom()+jets[1].mom()).mass():0;
        if ( 60 < mjj && mjj < 120 ) return QQ2HQQ_VH2JET;
              return QQ2HQQ_REST;
      }
      // 3. WH->Hlv categories
      else if (prodMode==HTXS::WH) {
        if (fwdHiggs) return QQ2HLNU_FWDH;
        else if (V.pt()<150) return QQ2HLNU_PTV_0_150;
              else if (V.pt()>250) return QQ2HLNU_PTV_GT250;
              // 150 < pTV/GeV < 250
              return jets.size()==0 ? QQ2HLNU_PTV_150_250_0J : QQ2HLNU_PTV_150_250_GE1J;
      }
      // 4. qq->ZH->llH categories
      else if (prodMode==HTXS::QQ2ZH) {
        if (fwdHiggs) return QQ2HLL_FWDH;
              else if (V.pt()<150) return QQ2HLL_PTV_0_150;
              else if (V.pt()>250) return QQ2HLL_PTV_GT250;
              // 150 < pTV/GeV < 250
              return jets.size()==0 ? QQ2HLL_PTV_150_250_0J : QQ2HLL_PTV_150_250_GE1J;
      }
      // 5. gg->ZH->llH categories
      else if (prodMode==HTXS::GG2ZH ) {
        if (fwdHiggs) return GG2HLL_FWDH;
        if      (V.pt()<150) return GG2HLL_PTV_0_150;
        else if (jets.size()==0) return GG2HLL_PTV_GT150_0J;
        return GG2HLL_PTV_GT150_GE1J;
      }
      // 6.ttH,bbH,tH categories
      else if (prodMode==HTXS::TTH) return Category(TTH_FWDH+ctrlHiggs);
      else if (prodMode==HTXS::BBH) return Category(BBH_FWDH+ctrlHiggs);
      else if (prodMode==HTXS::TH ) return Category(TH_FWDH+ctrlHiggs);
      return UNKNOWN;
    }

    /// @brief Stage-1.2 categorization
    HTXS::Stage1_2::Category getStage1_2_Category(const HTXS::HiggsProdMode prodMode,
                         const Particle &higgs,
                         const Jets &jets,
                         const Particle &V) const {
      using namespace HTXS::Stage1_2;
      int Njets=jets.size(), ctrlHiggs = std::abs(higgs.rapidity())<2.5, fwdHiggs = !ctrlHiggs;
      int vbfTopo = vbfTopology_Stage1_2(jets,higgs);

      // 1. GGF Stage 1 categories
      //    Following YR4 write-up: XXXXX
      if (prodMode==HTXS::GGF || (prodMode==HTXS::GG2ZH && quarkDecay(V)) ) {
    if (fwdHiggs)        return GG2H_FWDH;
    if ( higgs.pt()>200 ) return Category(GG2H_PTH_200_300+getBin(higgs.pt(),{200,300,450,650}));
    if (Njets==0)  return higgs.pt()<10 ? GG2H_0J_PTH_0_10 : GG2H_0J_PTH_GT10;
    if (Njets==1)  return Category(GG2H_1J_PTH_0_60+getBin(higgs.pt(),{0,60,120,200}));
    if (Njets>1){
        //VBF topology
        if(vbfTopo) return Category(GG2H_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25+vbfTopo-1);
        //Njets >= 2jets without VBF topology (mjj<350)
        return Category(GG2H_GE2J_MJJ_0_350_PTH_0_60+getBin(higgs.pt(),{0,60,120,200}));
    }
      }

      // 2. Electroweak qq->Hqq Stage 1.2 categories
      else if (prodMode==HTXS::VBF || ( isVH(prodMode) && quarkDecay(V)) ) {
    if (std::abs(higgs.rapidity())>2.5) return QQ2HQQ_FWDH;
    int Njets=jets.size();
    if (Njets==0)        return QQ2HQQ_0J;
    else if (Njets==1)   return QQ2HQQ_1J;
    else if (Njets>=2) {
        double mjj = (jets[0].mom()+jets[1].mom()).mass();
        if ( mjj < 60 )      return QQ2HQQ_GE2J_MJJ_0_60;
        else if ( 60 < mjj && mjj < 120 ) return QQ2HQQ_GE2J_MJJ_60_120;
        else if ( 120 < mjj && mjj < 350 ) return QQ2HQQ_GE2J_MJJ_120_350;
        else if (  mjj > 350 ) {
            if (higgs.pt()>200) return QQ2HQQ_GE2J_MJJ_GT350_PTH_GT200;
            if(vbfTopo) return Category(QQ2HQQ_GE2J_MJJ_GT350_PTH_GT200+vbfTopo);
        }
    }
      }
      // 3. WH->Hlv categories
      else if (prodMode==HTXS::WH) {
        if (fwdHiggs) return QQ2HLNU_FWDH;
        else if (V.pt()<75) return QQ2HLNU_PTV_0_75;
        else if (V.pt()<150) return QQ2HLNU_PTV_75_150;
        else if (V.pt()>250) return QQ2HLNU_PTV_GT250;
        // 150 < pTV/GeV < 250
        return jets.size()==0 ? QQ2HLNU_PTV_150_250_0J : QQ2HLNU_PTV_150_250_GE1J;
      }
      // 4. qq->ZH->llH categories
      else if (prodMode==HTXS::QQ2ZH) {
        if (fwdHiggs) return QQ2HLL_FWDH;
        else if (V.pt()<75) return QQ2HLL_PTV_0_75;
        else if (V.pt()<150) return QQ2HLL_PTV_75_150;
        else if (V.pt()>250) return QQ2HLL_PTV_GT250;
        // 150 < pTV/GeV < 250
        return jets.size()==0 ? QQ2HLL_PTV_150_250_0J : QQ2HLL_PTV_150_250_GE1J;
      }
      // 5. gg->ZH->llH categories
      else if (prodMode==HTXS::GG2ZH ) {
        if (fwdHiggs) return GG2HLL_FWDH;
        else if (V.pt()<75) return GG2HLL_PTV_0_75;
        else if (V.pt()<150) return GG2HLL_PTV_75_150;
        else if (V.pt()>250) return GG2HLL_PTV_GT250;
        return jets.size()==0 ? GG2HLL_PTV_150_250_0J : GG2HLL_PTV_150_250_GE1J;
      }
      // 6.ttH,bbH,tH categories
      else if (prodMode==HTXS::TTH) {
        if (fwdHiggs) return TTH_FWDH;
        else return Category(TTH_PTH_0_60+getBin(higgs.pt(),{0,60,120,200,300}));
      }
      else if (prodMode==HTXS::BBH) return Category(BBH_FWDH+ctrlHiggs);
      else if (prodMode==HTXS::TH ) return Category(TH_FWDH+ctrlHiggs);
      return UNKNOWN;
    }

    /// @brief Stage-1.2_Fine categorization
    HTXS::Stage1_2_Fine::Category getStage1_2_Fine_Category(const HTXS::HiggsProdMode prodMode,
                         const Particle &higgs,
                         const Jets &jets,
                         const Particle &V) const {
      using namespace HTXS::Stage1_2_Fine;
      int Njets=jets.size(), ctrlHiggs = std::abs(higgs.rapidity())<2.5, fwdHiggs = !ctrlHiggs;
      int vbfTopo = vbfTopology_Stage1_2_Fine(jets,higgs);

      // 1. GGF Stage 1.2 categories
      //    Following YR4 write-up: XXXXX
      if (prodMode==HTXS::GGF || (prodMode==HTXS::GG2ZH && quarkDecay(V)) ) {
    if (fwdHiggs)        return GG2H_FWDH;
    if ( higgs.pt()>200 ){
      if (Njets>0){
        double pTHj = (jets[0].momentum()+higgs.momentum()).pt();
        if( pTHj/higgs.pt()>0.15 ) return Category(GG2H_PTH_200_300_PTHJoverPTH_GT15+getBin(higgs.pt(),{200,300,450,650}));
        else return Category(GG2H_PTH_200_300_PTHJoverPTH_0_15+getBin(higgs.pt(),{200,300,450,650}));
      }
      else return Category(GG2H_PTH_200_300_PTHJoverPTH_0_15+getBin(higgs.pt(),{200,300,450,650}));
    }
    if (Njets==0)  return higgs.pt()<10 ? GG2H_0J_PTH_0_10 : GG2H_0J_PTH_GT10;
    if (Njets==1)  return Category(GG2H_1J_PTH_0_60+getBin(higgs.pt(),{0,60,120,200}));
    if (Njets>1){
        //double mjj = (jets[0].mom()+jets[1].mom()).mass();
        double pTHjj = (jets[0].momentum()+jets[1].momentum()+higgs.momentum()).pt();
        //VBF topology
        if(vbfTopo) return Category(GG2H_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25+vbfTopo-1);
        //Njets >= 2jets without VBF topology (mjj<350)
        if (pTHjj<25) return Category(GG2H_GE2J_MJJ_0_350_PTH_0_60_PTHJJ_0_25+getBin(higgs.pt(),{0,60,120,200}));
        else return Category(GG2H_GE2J_MJJ_0_350_PTH_0_60_PTHJJ_GT25+getBin(higgs.pt(),{0,60,120,200}));
    }
      }

      // 2. Electroweak qq->Hqq Stage 1.2 categories
      else if (prodMode==HTXS::VBF || ( isVH(prodMode) && quarkDecay(V)) ) {
    if (std::abs(higgs.rapidity())>2.5) return QQ2HQQ_FWDH;
    int Njets=jets.size();
    if (Njets==0)        return QQ2HQQ_0J;
    else if (Njets==1)   return QQ2HQQ_1J;
    else if (Njets>=2) {
        double mjj = (jets[0].mom()+jets[1].mom()).mass();
        double pTHjj = (jets[0].momentum()+jets[1].momentum()+higgs.momentum()).pt();
        if (mjj<350){
            if (pTHjj<25) return Category(QQ2HQQ_GE2J_MJJ_0_60_PTHJJ_0_25+getBin(mjj,{0,60,120,350}));
            else return Category(QQ2HQQ_GE2J_MJJ_0_60_PTHJJ_GT25+getBin(mjj,{0,60,120,350}));
        } else { //mjj>350 GeV
            if (higgs.pt()<200){
                return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25+vbfTopo-1);
            } else {
                return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_GT200_PTHJJ_0_25+vbfTopo-1);
            }
        }
    }
      }
      // 3. WH->Hlv categories
      else if (prodMode==HTXS::WH) {
        if (fwdHiggs) return QQ2HLNU_FWDH;
        int Njets=jets.size();
        if (Njets==0) return Category(QQ2HLNU_PTV_0_75_0J+getBin(V.pt(),{0,75,150,250,400}));
        if (Njets==1) return Category(QQ2HLNU_PTV_0_75_1J+getBin(V.pt(),{0,75,150,250,400}));
        return Category(QQ2HLNU_PTV_0_75_GE2J+getBin(V.pt(),{0,75,150,250,400}));
      }
      // 4. qq->ZH->llH categories
      else if (prodMode==HTXS::QQ2ZH) {
        if (fwdHiggs) return QQ2HLL_FWDH;
        int Njets=jets.size();
        if (Njets==0) return Category(QQ2HLL_PTV_0_75_0J+getBin(V.pt(),{0,75,150,250,400}));
        if (Njets==1) return Category(QQ2HLL_PTV_0_75_1J+getBin(V.pt(),{0,75,150,250,400}));
        return Category(QQ2HLL_PTV_0_75_GE2J+getBin(V.pt(),{0,75,150,250,400}));
      }
      // 5. gg->ZH->llH categories
      else if (prodMode==HTXS::GG2ZH ) {
        if (fwdHiggs) return GG2HLL_FWDH;
        int Njets=jets.size();
        if (Njets==0) return Category(GG2HLL_PTV_0_75_0J+getBin(V.pt(),{0,75,150,250,400}));
        if (Njets==1) return Category(GG2HLL_PTV_0_75_1J+getBin(V.pt(),{0,75,150,250,400}));
        return Category(GG2HLL_PTV_0_75_GE2J+getBin(V.pt(),{0,75,150,250,400}));
      }
      // 6.ttH,bbH,tH categories
      else if (prodMode==HTXS::TTH) {
        if (fwdHiggs) return TTH_FWDH;
        else return Category(TTH_PTH_0_60+getBin(higgs.pt(),{0,60,120,200,300,450}));
      }
      else if (prodMode==HTXS::BBH) return Category(BBH_FWDH+ctrlHiggs);
      else if (prodMode==HTXS::TH ) return Category(TH_FWDH+ctrlHiggs);
      return UNKNOWN;
    }
  /// @brief Stage-1.3 categorization
  HTXS::Stage1_3::Category getStage1_3_Category(const HTXS::HiggsProdMode prodMode, const Particle &higgs,
                                                const Jets &jets, const Particle &V) const {
    using namespace HTXS::Stage1_3;
    if (prodMode == HTXS::BBH) {
      const Category ggFCategory = getStage1_3_Category(HTXS::GGF, higgs, jets, V);
      if (ggFCategory == UNKNOWN) return UNKNOWN;
      return Category(BBH_FWDH + static_cast<int>(ggFCategory) - GG2H_FWDH);
    }

    int Njets = jets.size(), ctrlHiggs = std::abs(higgs.rapidity()) < 2.5, fwdHiggs = !ctrlHiggs;
    int vbfTopo = vbfTopology(jets, higgs);

    // 1. GGF Stage 1.3 categories
    if (prodMode == HTXS::GGF || (prodMode == HTXS::GG2ZH && quarkDecay(V))) {
      if (fwdHiggs) return GG2H_FWDH;
      if (higgs.pt() > 200) return Category(GG2H_PTH_200_300 + getBin(higgs.pt(), {200, 300, 450, 650, 1000}));
      if (Njets == 0) return Category(GG2H_0J_PTH_0_5 + getBin(higgs.pt(), {0, 5, 10, 15, 20, 25, 30, 200}));
      if (Njets == 1) return Category(GG2H_1J_PTH_0_30 + getBin(higgs.pt(), {0, 30, 60, 120, 200}));
      if (Njets > 1) {
        // VBF topology
        if (vbfTopo) return Category(GG2H_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25 + vbfTopo - 1);
        // Njets >= 2jets without VBF topology (mjj<350)
        return Category(GG2H_GE2J_MJJ_0_350_PTH_0_30 + getBin(higgs.pt(), {0, 30, 60, 120, 200}));
      }
    }
    // 2. Electroweak qq->Hqq Stage 1.3 categories
    else if (prodMode == HTXS::VBF || (isVH(prodMode) && quarkDecay(V))) {
      if (std::abs(higgs.rapidity()) > 2.5) return QQ2HQQ_FWDH;
      int Njets = jets.size();
      if (Njets == 0)
        return QQ2HQQ_0J;
      else if (Njets == 1)
        return QQ2HQQ_1J;
      else if (Njets >= 2) {
        double mjj = (jets[0].mom() + jets[1].mom()).mass();
        if (mjj < 60)
          return higgs.pt() < 200 ? QQ2HQQ_GE2J_MJJ_0_60_PTH_0_200 : QQ2HQQ_GE2J_MJJ_0_60_PTH_GT200;
        else if (60 < mjj && mjj < 120)
          return higgs.pt() < 200 ? QQ2HQQ_GE2J_MJJ_60_120_PTH_0_200 : QQ2HQQ_GE2J_MJJ_60_120_PTH_GT200;
        else if (120 < mjj && mjj < 350)
          return higgs.pt() < 200 ? QQ2HQQ_GE2J_MJJ_120_350_PTH_0_200 : QQ2HQQ_GE2J_MJJ_120_350_PTH_GT200;
        else if (mjj > 350) {
          if (higgs.pt() > 200)
            return higgs.pt() < 450 ? QQ2HQQ_GE2J_MJJ_GT350_PTH_200_450 : QQ2HQQ_GE2J_MJJ_GT350_PTH_GT450;
          if (vbfTopo) return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25 + vbfTopo - 1);
        }
      }
    }
    // 3. WH->Hlv Stage 1.3 categories
    else if (prodMode == HTXS::WH) {
      if (fwdHiggs)
        return QQ2HLNU_FWDH;
      else if (V.pt() < 75)
        return QQ2HLNU_PTV_0_75;
      else if (V.pt() < 150)
        return QQ2HLNU_PTV_75_150;
      else if (V.pt() < 250)
        return jets.size() == 0 ? QQ2HLNU_PTV_150_250_0J : QQ2HLNU_PTV_150_250_GE1J;
      else if (V.pt() < 400)
        return jets.size() == 0 ? QQ2HLNU_PTV_250_400_0J : QQ2HLNU_PTV_250_400_GE1J;
      else if (V.pt() < 600)
        return QQ2HLNU_PTV_400_600;
      return QQ2HLNU_PTV_GT600;
    }
    // 4. qq->ZH->llH Stage 1.3 categories
    else if (prodMode == HTXS::QQ2ZH) {
      if (fwdHiggs)
        return QQ2HLL_FWDH;
      else if (V.pt() < 75)
        return QQ2HLL_PTV_0_75;
      else if (V.pt() < 150)
        return QQ2HLL_PTV_75_150;
      else if (V.pt() < 250)
        return jets.size() == 0 ? QQ2HLL_PTV_150_250_0J : QQ2HLL_PTV_150_250_GE1J;
      else if (V.pt() < 400)
        return jets.size() == 0 ? QQ2HLL_PTV_250_400_0J : QQ2HLL_PTV_250_400_GE1J;
      else if (V.pt() < 600)
        return QQ2HLL_PTV_400_600;
      return QQ2HLL_PTV_GT600;
    }
    // 5. gg->ZH->llH Stage 1.3 categories
    else if (prodMode == HTXS::GG2ZH) {
      if (fwdHiggs)
        return GG2HLL_FWDH;
      else if (V.pt() < 75)
        return GG2HLL_PTV_0_75;
      else if (V.pt() < 150)
        return GG2HLL_PTV_75_150;
      else if (V.pt() < 250)
        return jets.size() == 0 ? GG2HLL_PTV_150_250_0J : GG2HLL_PTV_150_250_GE1J;
      else if (V.pt() < 400)
        return jets.size() == 0 ? GG2HLL_PTV_250_400_0J : GG2HLL_PTV_250_400_GE1J;
      else if (V.pt() < 600)
        return GG2HLL_PTV_400_600;
      return GG2HLL_PTV_GT600;
    }
    // 6.ttH,bbH,tH Stage 1.3 categories
    else if (prodMode == HTXS::TTH) {
      if (fwdHiggs)
        return TTH_FWDH;
      else
        return Category(TTH_PTH_0_60 + getBin(higgs.pt(), {0, 60, 120, 200, 300, 450, 650}));
    } else if (prodMode == HTXS::TH)
      return Category(TH_FWDH + ctrlHiggs);
    return UNKNOWN;
  }

  /// @brief Stage-1.3 Fine categorization
  HTXS::Stage1_3_Fine::Category getStage1_3_Fine_Category(const HTXS::HiggsProdMode prodMode, const Particle &higgs,
                                                          const Jets &jets, const Particle &V, const bool isTHW) const {
    using namespace HTXS::Stage1_3_Fine;
    if (prodMode == HTXS::BBH) {
      const Category ggFCategory = getStage1_3_Fine_Category(HTXS::GGF, higgs, jets, V, isTHW);
      if (ggFCategory == UNKNOWN) return UNKNOWN;
      return Category(BBH_FWDH + static_cast<int>(ggFCategory) - GG2H_FWDH);
    }

    int Njets = jets.size(), ctrlHiggs = std::abs(higgs.rapidity()) < 2.5, fwdHiggs = !ctrlHiggs;
    int vbfTopo = vbfTopology_Stage1_3_Fine(jets, higgs);

    // 1. GGF Stage 1.3 categories (fine)
    if (prodMode == HTXS::GGF || (prodMode == HTXS::GG2ZH && quarkDecay(V))) {
      if (fwdHiggs) return GG2H_FWDH;
      if (higgs.pt() > 200) {
        if (Njets > 0) {
          double pthj = (jets[0].momentum() + higgs.momentum()).pt();
          if (pthj / higgs.pt() > 0.15)
            return Category(GG2H_PTH_200_300_PTHJoverPTH_GT15 + getBin(higgs.pt(), {200, 300, 450, 650, 1000}));
          else
            return Category(GG2H_PTH_200_300_PTHJoverPTH_0_15 + getBin(higgs.pt(), {200, 300, 450, 650, 1000}));
        } else
          return Category(GG2H_PTH_200_300_PTHJoverPTH_0_15 + getBin(higgs.pt(), {200, 300, 450, 650, 1000}));
      }
      if (Njets == 0) return Category(GG2H_0J_PTH_0_5 + getBin(higgs.pt(), {0, 5, 10, 15, 20, 25, 30, 200}));
      if (Njets == 1) return Category(GG2H_1J_PTH_0_30 + getBin(higgs.pt(), {0, 30, 60, 120, 200}));
      if (Njets > 1) {
        double mjj = (jets[0].mom()+jets[1].mom()).mass();
        double pthjj = (jets[0].momentum() + jets[1].momentum() + higgs.momentum()).pt();
        // VBF topology
        if (mjj < 350){
            if (pthjj < 25)
              return Category(GG2H_GE2J_MJJ_0_350_PTH_0_30_PTHJJ_0_25 + getBin(higgs.pt(), {0, 30, 60, 120, 200}));
            else
              return Category(GG2H_GE2J_MJJ_0_350_PTH_0_30_PTHJJ_GT25 + getBin(higgs.pt(), {0, 30, 60, 120, 200}));
        } else
            return Category(GG2H_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25_DPHIJJ_MPI_MPIO2 + vbfTopo - 1);
      }
    }

    // 2. Electroweak qq->Hqq Stage 1.3 categories (fine)
    else if (prodMode == HTXS::VBF || (isVH(prodMode) && quarkDecay(V))) {
      if (std::abs(higgs.rapidity()) > 2.5) return QQ2HQQ_FWDH;
      int Njets = jets.size();
      if (Njets == 0)
        return QQ2HQQ_0J;
      else if (Njets == 1)
        return Category(QQ2HQQ_1J_PTH_0_200 + getBin(higgs.pt(), {0, 200, 450, 650}));
      else if (Njets >= 2) {
        double mjj = (jets[0].mom() + jets[1].mom()).mass();
        double pthjj = (jets[0].momentum() + jets[1].momentum() + higgs.momentum()).pt();
        if (mjj < 350) {
          if (higgs.pt() < 200){
            if (pthjj < 25)
              return Category(QQ2HQQ_GE2J_MJJ_0_60_PTH_0_200_PTHJJ_0_25 + getBin(mjj, {0, 60, 120, 350}));
            else
              return Category(QQ2HQQ_GE2J_MJJ_0_60_PTH_0_200_PTHJJ_GT25 + getBin(mjj, {0, 60, 120, 350}));              
          } else {
            if (pthjj < 25)
              return Category(QQ2HQQ_GE2J_MJJ_0_60_PTH_GT200_PTHJJ_0_25 + getBin(mjj, {0, 60, 120, 350}));
            else
              return Category(QQ2HQQ_GE2J_MJJ_0_60_PTH_GT200_PTHJJ_GT25 + getBin(mjj, {0, 60, 120, 350}));
          }
        } else {  // mjj>350 GeV
          if (higgs.pt() < 200)
            return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_0_200_PTHJJ_0_25_DPHIJJ_MPI_MPIO2 + vbfTopo - 1);
          else if (higgs.pt() < 450)
            return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_200_450_PTHJJ_0_25_DPHIJJ_MPI_MPIO2 + vbfTopo - 1);
          else
            return Category(QQ2HQQ_GE2J_MJJ_350_700_PTH_GT450 + getBin(mjj, {350, 700, 1000, 1500}));
        }
      }
    }

    // 3. WH->Hlv Stage 1.3 categories (fine)
    else if (prodMode == HTXS::WH) {
      if (fwdHiggs) return QQ2HLNU_FWDH;
      int Njets = jets.size();
      if (Njets == 0) return Category(QQ2HLNU_PTV_0_75_0J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      if (Njets == 1) return Category(QQ2HLNU_PTV_0_75_1J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      return Category(QQ2HLNU_PTV_0_75_GE2J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
    }

    // 4. qq->ZH->llH Stage 1.3 categories (fine)
    else if (prodMode == HTXS::QQ2ZH) {
      if (fwdHiggs) return QQ2HLL_FWDH;
      int Njets = jets.size();
      if (Njets == 0) return Category(QQ2HLL_PTV_0_75_0J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      if (Njets == 1) return Category(QQ2HLL_PTV_0_75_1J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      return Category(QQ2HLL_PTV_0_75_GE2J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
    }

    // 5. gg->ZH->llH Stage 1.3 categories (fine)
    else if (prodMode == HTXS::GG2ZH) {
      if (fwdHiggs) return GG2HLL_FWDH;
      int Njets = jets.size();
      if (Njets == 0) return Category(GG2HLL_PTV_0_75_0J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      if (Njets == 1) return Category(GG2HLL_PTV_0_75_1J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
      return Category(GG2HLL_PTV_0_75_GE2J + getBin(V.pt(), {0, 75, 150, 250, 400, 600}));
    }

    // 6.ttH,bbH,tH Stage 1.3 categories (fine)
    else if (prodMode == HTXS::TTH) {
      if (fwdHiggs)
        return TTH_FWDH;
      else
        return Category(TTH_PTH_0_60 + getBin(higgs.pt(), {0, 60, 120, 200, 300, 450, 650}));
    } else if (prodMode == HTXS::TH)
      return Category(THQ_FWDH + 2*isTHW + ctrlHiggs);
    return UNKNOWN;
  }

    /// @name Higgs decay categorization methods
    /// Methods to classify the Higgs boson decay mode according to the
    /// Stage-0 decay categorization defined in HTXS::Stage0::DecayCategory
    /// @{

    /// @brief Stage-0 Higgs decay categorization.
    ///        Identifies the stable final-state decay products originating from
    ///        the Higgs boson and classifies the decay mode.
    HTXS::Stage0::DecayCategory getStage0DecayCategory(const Particle &higgs,
                                                        const Particles &decayparticles,
                                                        std::vector<float> &decay_observables,
                                                        int &cuts_passed) const {
      using namespace HTXS::Stage0;

      decay_observables = std::vector<float>(11, -999);
      // 0: Z1m
      // 1: Z2m
      // 2: cthstr
      // 3: phi
      // 4: phi1
      // 5: cth1
      // 6: cth2
      // 7-10: m14, m23, m13, m24

      // Set up kinematics in the Higgs rest frame
      auto &higgsmom = higgs.momentum();

      double higgsm2 = higgsmom.invariant();
      double higgsm  = (higgsm2 > 0 ? sqrt(higgsm2) : 0);

      const int N = static_cast<int>(decayparticles.size());
      std::vector<double> Ep(N), pp(N);
      std::vector<std::vector<double>> cosangle(N, std::vector<double>(N, 1.0));

      LorentzTransform toHiggs = LorentzTransform::mkFrameTransform(higgsmom);
      for (int i = 0; i < N; ++i) {
        auto &mom  = decayparticles[i].momentum();
        double m2  = mom.invariant();
        double hp  = higgsmom * mom;
        Ep[i]      = hp / higgsm;
        double p2v = Ep[i] * Ep[i] - m2;
        pp[i]      = (p2v > 0 ? sqrt(p2v) : 0);
        for (int j = 0; j < i; ++j) {
          auto &mom2    = decayparticles[j].momentum();
          cosangle[i][j] = (Ep[i]*Ep[j] - mom*mom2) / (pp[i]*pp[j]);
          cosangle[j][i] = cosangle[i][j];
        }
      }

      // Work on a mutable copy boosted into the Higgs rest frame
      Particles dp_rest = decayparticles;
      for (int i = 0; i < N; ++i)
        dp_rest[i] = dp_rest[i].transformBy(toHiggs);

      // Accumulate identified objects into these vectors
      std::vector<FourMomentum> v_p4{};
      std::vector<int>          v_pid{};

      doAngleDressing(dp_rest, cosangle, v_p4, v_pid);
      
      // findZZ4ldecay outputs a byte, last 6 bits indicate cuts passed
      cuts_passed = findZZ4ldecay(std::move(v_p4), std::move(v_pid), decay_observables);
      bool isZZ4l = (cuts_passed & 0b111111) == 0b111111;

      // H -> ZZ* -> 4l
      if (isZZ4l) {
        switch ((cuts_passed >> 6) & 0b11) { // 0b xx 111111 (read @returns of findZZ4ldecay)
          case 0b01: return HZZ4e;
          case 0b10: return HZZ4mu;
          case 0b11: return HZZ2e2mu;
          case 0b00: MSG_WARNING("Found HZZ4l event but unable to categorise. This shouldn't happen!");
        }
      }
      
      //Here other final states will be added in future

      return DecayCategory(UNKNOWNDecay);
    }

    /// @brief dress leptons with photons within a 0.1 radian angle.
    ///        Also puts isolated photons (photons with no leptons within 0.1 rad cone) into v_p4.
    int doAngleDressing(const Particles &dp,
                     const std::vector<std::vector<double>> &cosangle,
                     std::vector<FourMomentum> &v_p4,
                     std::vector<int>          &v_pid) const {
      const double cos_cut = cos(0.1);
      int num_photons = 0;
      // dressers[i] contains the index of photons that get dressed to dp[i]
      std::vector<std::vector<int>> dressers(dp.size());

      // Loop over all photons to find their closest lepton
      for (size_t i = 0; i < dp.size(); i++) { 
        const Particle &p = dp[i];
        if (p.pid() != 22) continue;
        int max_cos_index = -1;
        for (size_t j = 0; j < dp.size(); j++) {
          if (!(PID::isElectron(dp[j].pid()) || PID::isMuon(dp[j].pid()))) continue;
          if (cosangle[i][j] < cos_cut) continue;
          if (max_cos_index != -1 && cosangle[i][j] < cosangle[i][max_cos_index]) continue;
          max_cos_index = j;
        }
        if(max_cos_index != -1) 
          dressers[max_cos_index].push_back(i);
        else {
          v_p4.push_back(p.momentum());
          v_pid.push_back(p.pid());
        }
      }

      // Then loop over leptons to dress photons into it
      for (size_t i = 0; i < dp.size(); i++) {
        const Particle &p = dp[i];
        if (!(PID::isElectron(p.pid()) || PID::isMuon(p.pid()))) continue;
        v_p4.push_back(p.momentum());
        v_pid.push_back(p.pid());
        for (auto photon : dressers[i]) {
          v_p4.back() += dp[photon].momentum();
          num_photons++;
        }
      }
      return num_photons;
    }

    /// @brief Select H->ZZ->4l candidate: 4 charged leptons, net charge=0,
    ///        combined invariant mass in [105, 130] GeV, passing angular overlap cut.
    /// @return a 8 bit number, first 6 bits for 6 cuts (lepton_num, jpsi, m12, m34, delta <, h_mass), 
    /// @return last 2 bits (i.e. most significant): 0b10 for contains mu, 0b01 for contains e (so 0b11 for 2e2mu)
    unsigned char findZZ4ldecay(std::vector<FourMomentum>        v_p4,
                       std::vector<int>                 v_pid,
                       std::vector<float>               &decay_observables) const {

      int lepton_num = 0;
      for (int pid : v_pid) lepton_num += (std::abs(pid) == 11 || std::abs(pid) == 13);

      std::vector<int> lepton_index{};

      unsigned char cut_passed = 0;
      // bit 0: lepton_num, bit 1: > 4 leptons after removing mij < 5, bit 2: m12, bit 3: m34, bit 4: angle ij > 0.1, bit 5: higgs mass
      cut_passed |= (lepton_num >= 4);
      cut_passed |= ZZ4lJpsi_cut(v_p4, v_pid) << 1;
      cut_passed |= ZZ4lm12m34_cut(v_p4, v_pid, lepton_index) << 2; // <- this outputs 2 bits of data
      cut_passed |= ZZ4langle_cut(v_p4, v_pid) << 4; // that's why it's << 4 here

      FourMomentum hsum{};
      for (uint i = 0; i < v_pid.size(); ++i) {
        if (std::abs(v_pid[i]) == 11 || std::abs(v_pid[i]) == 13) {
          hsum += v_p4[i];
        }
      }

      cut_passed |= (hsum.mass() > 105 && hsum.mass() < 130) << 5;

      // We put additional 2 bits of info to cut_passed to determine if is 4e, 2e2mu, or 4mu!
      // side note: [0] and [2] takes 1 particle from each lepton pair
      if(lepton_index.size() == 4) {
        if (PID::isElectron(v_pid[lepton_index[0]]) || PID::isElectron(v_pid[lepton_index[2]]))
          cut_passed |= 0b01000000; // 0b01111111 => 4e
        if (PID::isMuon(v_pid[lepton_index[0]]) || PID::isMuon(v_pid[lepton_index[2]]))
          cut_passed |= 0b10000000; // 0b10111111 => 4mu
                                    // 0b11111111 => 2e2mu
      }

      calculate_decay_observables(v_p4, lepton_index, decay_observables);
      return cut_passed;
    }

    /// @brief Reject lepton pairs whose opening angle is too small (cos > cos(0.1))
    bool ZZ4langle_cut(const std::vector<FourMomentum> &v_p4,
                        const std::vector<int>           &v_pid) const {
      
      std::vector<FourMomentum> v_p4_lepton = v_p4;

      const double cos_cut = cos(0.1);
      double cos_max = 0;
      for (uint i = 0; i < v_p4_lepton.size(); ++i) {
        if ( !(PID::isMuon(v_pid[i]) || PID::isElectron(v_pid[i])) ) continue;
        for (uint j = 0; j < i; ++j) {
          if ( !(PID::isMuon(v_pid[j]) || PID::isElectron(v_pid[j])) ) continue;
          double ca = (v_p4_lepton[i].E()*v_p4_lepton[j].E() - v_p4_lepton[i]*v_p4_lepton[j])
                      / (v_p4_lepton[i].p()*v_p4_lepton[j].p());
          cos_max = std::max(ca, cos_max);
        }
      }
      return cos_cut > cos_max;
    }

    /// @brief Apply m12 and m34 mass window cuts for ZZ->4l selection
    /// Also insert the leading lepton pair and subleading lepton pair index into lepton_index
    /// We always have lepton_index[0] = e/mu, lepton_index[1] = anti-(e/mu), etc
    /// @returns 0b00 for not pass anything, 0b01 for passm12, 0b10 for passm34, 0b11 for passm12 && passm34
    unsigned char ZZ4lm12m34_cut(const std::vector<FourMomentum> &v_p4,
                        const std::vector<int>           &v_pid,
                        std::vector<int>                 &lepton_index) const {
      std::vector<int> lepIdx;
      for (size_t i = 0; i < v_pid.size(); ++i)
        if (std::abs(v_pid[i]) == 11 || std::abs(v_pid[i]) == 13) lepIdx.push_back(i);
      if (lepIdx.size() < 2) return 0b00;

      const double mZ = 91.1876;

      double bestDM = 1e30;
      double bestM12 = -1, bestM34 = -1;
      size_t bestpair[4] = {999, 999, 999, 999};
      for (size_t i = 0; i < lepIdx.size(); i++) { // find pair for m12 first
        for (size_t j = i+1; j < lepIdx.size(); j++) {
          int a=lepIdx[i], b=lepIdx[j];
          if(v_pid[a] != -v_pid[b]) continue; // not SFOS
          double m12cur = (v_p4[a] + v_p4[b]).mass();
          if (std::fabs(m12cur - mZ) >= bestDM) continue;
          bestDM = std::fabs(m12cur - mZ);
          bestpair[0] = i; bestpair[1] = j;
          bestM12 = m12cur;
        }
      }

      // Find pair for m34
      for (size_t i = 0; i < lepIdx.size(); i++) {
        if (i == bestpair[0] || i == bestpair[1]) continue;
        for (size_t j = i+1; j < lepIdx.size(); j++) {
          if (j == bestpair[0] || j == bestpair[1]) continue;

          int a=lepIdx[i], b=lepIdx[j];
          if(v_pid[a] != -v_pid[b]) continue; // not SFOS
          double m34cur = (v_p4[a] + v_p4[b]).mass();
          if (m34cur <= bestM34) continue;
          bestpair[2] = i; bestpair[3] = j;
          bestM34 = m34cur;
        }
      }

      if (bestpair[0] == 999) {
        MSG_WARNING("Unable to find lepton pairings, returning null");
        return 0b00;
      }

      for (int i = 0; i < 4; i++) {
        if (bestpair[i] == 999) break;
        if(v_pid[lepIdx[bestpair[i]]] < 0 && ((i % 2) == 0)) { // we are at [0] or [2]... supposed to put matter
          std::swap(bestpair[i], bestpair[i+1]); // so we swap
        }
        lepton_index.push_back(lepIdx[bestpair[i]]);
      }

      return passm12(bestM12) | (passm34(bestM34) << 1);
    }

    /// @brief J/psi veto: reject events with a same-flavour opposite-sign lepton
    ///        pair with invariant mass below 5 GeV, or if there are more than 4 leptons,
    ///        remove all lepton pairs with invariant mass below 5 Gev
    bool ZZ4lJpsi_cut(std::vector<FourMomentum> &v_p4,
                       std::vector<int>           &v_pid) const {

      while (remove_least_mij(v_p4, v_pid));
      int lepton_num = 0;
      for (int pid : v_pid) lepton_num += (std::abs(pid) == 11 || std::abs(pid) == 13);
      return lepton_num >= 4;
    }

    /// @brief removes SFOS lepton pairs with invariant mass < 5GeV
    ///        removes only the pair with the least invariant mass
    bool remove_least_mij(std::vector<FourMomentum> &v_p4,
                      std::vector<int>           &v_pid) const {
      float mij_min = 5.0;
      size_t min_pair[2] = {999, 999};
      for (size_t i = 0; i < v_p4.size(); i++) {
        for (size_t j = i+1; j < v_p4.size(); j++) {
          if (v_pid[i] + v_pid[j] != 0) continue;
          if (!(PID::isMuon(v_pid[i]) || PID::isElectron(v_pid[i]))) continue;
          float mij = (v_p4[i] + v_p4[j]).mass();
          if (mij >= mij_min) continue;
          mij_min = mij;
          min_pair[0] = i;
          min_pair[1] = j;
        }
      }
      if (min_pair[0] == 999) return false;
      v_p4.erase(v_p4.begin() + min_pair[1]); // min_pair[1] > min_pair[0] always, so this is safe
      v_p4.erase(v_p4.begin() + min_pair[0]);

      v_pid.erase(v_pid.begin() + min_pair[1]);
      v_pid.erase(v_pid.begin() + min_pair[0]);

      return true;
    }

    /// @brief calculate the relevant 4l decay observables
    bool calculate_decay_observables(const std::vector<FourMomentum> &v_p4,
                                     const std::vector<int>          &v_4l_index,
                                     std::vector<float> &decay_observable) const {
      
      if (v_4l_index.size() != 4) {
        if (v_4l_index.size() >= 2) decay_observable[0] = (v_p4[v_4l_index[0]] + v_p4[v_4l_index[1]]).mass();
        return false;
      }

      FourMomentum v1 = (v_p4[v_4l_index[0]]);
      FourMomentum v2 = (v_p4[v_4l_index[1]]);
      FourMomentum v3 = (v_p4[v_4l_index[2]]);
      FourMomentum v4 = (v_p4[v_4l_index[3]]);

      float Z1m = (v1 + v2).mass();
      float Z2m = (v3 + v4).mass();
      
      float m14 = (v1 + v4).mass();
      float m23 = (v2 + v3).mass();
      float m13 = (v1 + v3).mass();
      float m24 = (v2 + v4).mass();

      FourMomentum Z1 = ( v1 + v2 );
      FourMomentum Z2 = ( v3 + v4 );

      Vector3 z1 = Z1.vector3().unit();
      Vector3 z2 = Z2.vector3().unit();

      // Costh*
      float cthstr = z1.z();

      Vector3 v1p = v1.vector3();
      Vector3 v2p = v2.vector3();
      Vector3 v3p = v3.vector3();
      Vector3 v4p = v4.vector3();
      Vector3 nz(0, 0, 1.);

      // Phi, Phi1
      Vector3 n1p = v1p.cross(v2p).unit();
      Vector3 n2p = v3p.cross(v4p).unit();
      Vector3 nscp = nz.cross(z1).unit();
      float phi = (z1.dot(n1p.cross(n2p)) / std::fabs(z1.dot(n1p.cross(n2p))) *
             std::acos(-n1p.dot(n2p)));
      float phi1 = (z1.dot(n1p.cross(nscp)) / std::fabs(z1.dot(n1p.cross(nscp))) *
              std::acos(n1p.dot(nscp)));

      // Costh1,2
      LorentzTransform toZ1 = LorentzTransform::mkFrameTransform(Z1);
      LorentzTransform toZ2 = LorentzTransform::mkFrameTransform(Z2);

      FourMomentum Z2_rfr_Z1 = toZ1.transform(Z2);  // now it's in Z1 RFR (both Z1 and Z2 are in H RFR)
      Vector3 z2_rfr_Z1 = Z2_rfr_Z1.vector3();

      FourMomentum Z1_rfr_Z2 =  toZ2.transform(Z1); // now it's in Z2 RFR (both Z1 and Z2 are still in H RFR)
      Vector3 z1_rfr_Z2 = Z1_rfr_Z2.vector3();

      FourMomentum v1_rfr_Z1 = toZ1.transform(v1); // Z1 and Z2 still in H RFR: put leptons
                                                       // in their Z's reference frame
      FourMomentum v3_rfr_Z2 = toZ2.transform(v3);

      float cth1 = -(z2_rfr_Z1.dot(v1_rfr_Z1.vector3()) /
               std::fabs(z2_rfr_Z1.mod() * v1_rfr_Z1.vector3().mod()));
      float cth2 = -(z1_rfr_Z2.dot(v3_rfr_Z2.vector3()) /
               std::fabs(z1_rfr_Z2.mod() * v3_rfr_Z2.vector3().mod()));
      
      decay_observable[0]  = Z1m;
      decay_observable[1]  = Z2m;
      decay_observable[2]  = cthstr;
      decay_observable[3]  = phi;
      decay_observable[4]  = phi1;
      decay_observable[5]  = cth1;
      decay_observable[6]  = cth2;
      decay_observable[7]  = m14;
      decay_observable[8]  = m23;
      decay_observable[9]  = m13;
      decay_observable[10] = m24;
      return true;
    }

    /// @brief Pass leading lepton-pair mass window [50, 106] GeV
    bool passm12(double m) const { return (m > 50.0 && m < 106.0); }

    /// @brief Pass sub-leading lepton-pair mass window [12, 115] GeV
    bool passm34(double m) const { return (m > 12.0 && m < 115.0); }
    
    /// @}


    /// @name Default Rivet analysis methods and steering methods
    /// @{

    /// @brief Sets the Higgs production mode
    void setHiggsProdMode( HTXS::HiggsProdMode prodMode ){ m_HiggsProdMode = prodMode; }

    /// @brief Sets the Higgs production mode
    void setHiggsDecayMode( HTXS::HiggsDecayMode decayMode ){ m_HiggsDecayMode = decayMode; }

    /// @brief default Rivet Analysis::init method
    /// Booking of histograms, initializing Rivet projection
    /// Extracts Higgs production mode from shell variable if not set manually using setHiggsProdMode
    void init() {
      printf("==============================================================\n");
      printf("========     HiggsTemplateCrossSections Initialization     =========\n");
      printf("==============================================================\n");
      // check that the production mode has been set
      // if running in standalone Rivet the production mode is set through an env variable
      if (m_HiggsProdMode==HTXS::UNKNOWN) {
        char *pm_env = getenv("HIGGSPRODMODE");
        string pm(pm_env==nullptr?"":pm_env);
        if      ( pm == "GGF"   ) m_HiggsProdMode = HTXS::GGF;
        else if ( pm == "VBF"   ) m_HiggsProdMode = HTXS::VBF;
        else if ( pm == "WH"    ) m_HiggsProdMode = HTXS::WH;
        else if ( pm == "ZH"    ) m_HiggsProdMode = HTXS::QQ2ZH;
        else if ( pm == "QQ2ZH" ) m_HiggsProdMode = HTXS::QQ2ZH;
        else if ( pm == "GG2ZH" ) m_HiggsProdMode = HTXS::GG2ZH;
        else if ( pm == "TTH"   ) m_HiggsProdMode = HTXS::TTH;
        else if ( pm == "BBH"   ) m_HiggsProdMode = HTXS::BBH;
        else if ( pm == "TH"    ) m_HiggsProdMode = HTXS::TH;
        else {
          MSG_WARNING("No HIGGSPRODMODE shell variable found. Needed when running Rivet stand-alone.");
        }
      }

      // Projections for final state particles
      const FinalState FS;
      declare(FS,"FS");

      // initialize the histograms with for each of the stages
      initializeHistos();
      m_sumw = 0.0;
      printf("==============================================================\n");
      printf("========             Higgs prod mode %d              =========\n",m_HiggsProdMode);
      printf("========          Sucessful Initialization           =========\n");
      printf("==============================================================\n");
    }

    // Perform the per-event analysis
    void analyze(const Event& event) {

      // get the classification
      HiggsClassification cat = classifyEvent(event,m_HiggsProdMode,m_HiggsDecayMode);

      // Fill histograms: categorization --> linerize the categories
      const double weight = 1.; // Event weights are now all 1 in Rivet
      m_sumw += weight;

      int F=cat.stage0_cat%10, P=cat.stage1_cat_pTjet30GeV/100;
      m_hist_stage0->fill( cat.stage0_cat/10*2+F, weight );

      // Stage 1 enum offsets for each production mode: GGF=12, VBF=6, WH= 5, QQ2ZH=5, GG2ZH=4, TTH=2, BBH=2, TH=2
      static const vector<int> offset({0,1,13,19,24,29,33,35,37,39});
      int off = offset[P];
      // Stage 1.2 enum offsets for each production mode: GGF=17, VBF=11, WH= 6, QQ2ZH=6, GG2ZH=6, TTH=6, BBH=2, TH=2
      static const vector<int> offset1_2({0,1,18,29,35,41,47,53,55,57});
      int off1_2 = offset1_2[P];
      // Stage 1.2-Fine enum offsets for each production mode: GGF=28, VBF=25, WH= 16, QQ2ZH=16, GG2ZH=16, TTH=7, BBH=2, TH=2
      static const vector<int> offset1_2_Fine({0,1,29,54,70,86,102,109,111,113});
      int off1_2_Fine = offset1_2_Fine[P];
      // Stage 1_3 enum offsets for each production mode: GGF=25, VBF=15, WH=9, QQ2ZH=9, GG2ZH=9, TTH=8, BBH=25, TH=2
      static const vector<int> offset1_3({0,1,26,41,50,59,68,76,101,103});
      int off1_3 = offset1_3[P];
      // Stage 1_3 Fine enum offsets for each production mode: GGF=62, VBF=86, WH=19, QQ2ZH=19, GG2ZH=19, TTH=8, BBH=62, TH=4
      static const vector<int> offset1_3_fine({0,1,63,149,168,187,206,214,276,280});
      int off1_3_fine = offset1_3_fine[P];


      m_hist_stage1_pTjet25->fill(cat.stage1_cat_pTjet25GeV%100 + off, weight);
      m_hist_stage1_pTjet30->fill(cat.stage1_cat_pTjet30GeV%100 + off, weight);
      m_hist_stage1_2_pTjet25->fill(cat.stage1_2_cat_pTjet25GeV%100 + off1_2, weight);
      m_hist_stage1_2_pTjet30->fill(cat.stage1_2_cat_pTjet30GeV%100 + off1_2, weight);
      m_hist_stage1_2_fine_pTjet25->fill(cat.stage1_2_fine_cat_pTjet25GeV%100 + off1_2_Fine, weight);
      m_hist_stage1_2_fine_pTjet30->fill(cat.stage1_2_fine_cat_pTjet30GeV%100 + off1_2_Fine, weight);
      m_hist_stage1_3_pTjet25->fill(cat.stage1_3_cat_pTjet25GeV%100 + off1_3, weight);
      m_hist_stage1_3_pTjet30->fill(cat.stage1_3_cat_pTjet30GeV%100 + off1_3, weight);
      m_hist_stage1_3_fine_pTjet25->fill(cat.stage1_3_fine_cat_pTjet25GeV%100 + off1_3_fine, weight);
      m_hist_stage1_3_fine_pTjet30->fill(cat.stage1_3_fine_cat_pTjet30GeV%100 + off1_3_fine, weight);

      // Fill histograms: variables used in the categorization
      m_hist_pT_Higgs->fill(cat.higgs.pT(),weight);
      m_hist_y_Higgs->fill(cat.higgs.rapidity(),weight);
      m_hist_pT_V->fill(cat.V.pT(),weight);

      m_hist_Njets25->fill(cat.jets25.size(),weight);
      m_hist_Njets30->fill(cat.jets30.size(),weight);

      m_hist_isZ2vv->fill(cat.isZ2vvDecay, weight);

      // Jet variables. Use jet collection with pT threshold at 30 GeV
      if (cat.jets30.size()) m_hist_pT_jet1->fill(cat.jets30[0].pt(),weight);
      if (cat.jets30.size()>=2) {
        const FourMomentum &j1 = cat.jets30[0].momentum(), &j2 = cat.jets30[1].momentum();
        m_hist_deltay_jj->fill(std::abs(j1.rapidity()-j2.rapidity()),weight);
        m_hist_dijet_mass->fill((j1+j2).mass(),weight);
        m_hist_pT_Hjj->fill((j1+j2+cat.higgs.momentum()).pt(),weight);
      }
    }

    void printClassificationSummary(){
      MSG_INFO (" ====================================================== ");
      MSG_INFO ("      Higgs Template X-Sec Categorization Tool          ");
      MSG_INFO ("                Status Code Summary                     ");
      MSG_INFO (" ====================================================== ");
      bool allSuccess = (numEvents()==m_errorCount[HTXS::SUCCESS]);
      if ( allSuccess ) MSG_INFO ("     >>>> All "<< m_errorCount[HTXS::SUCCESS] <<" events successfully categorized!");
      else{
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::SUCCESS] <<" events successfully categorized");
        MSG_INFO ("     >>>> --> the following errors occured:");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::PRODMODE_DEFINED] <<" had an undefined Higgs production mode.");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::MOMENTUM_CONSERVATION] <<" failed momentum conservation.");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::HIGGS_IDENTIFICATION] <<" failed to identify a valid Higgs boson.");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::HS_VTX_IDENTIFICATION] <<" failed to identify the hard scatter vertex.");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::VH_IDENTIFICATION] <<" VH: to identify a valid V-boson.");
        MSG_INFO ("     >>>> "<< m_errorCount[HTXS::TOP_W_IDENTIFICATION] <<" failed to identify valid Ws from top decay.");
      }
      MSG_INFO (" ====================================================== ");
      MSG_INFO (" ====================================================== ");
    }


    void finalize() {
      printClassificationSummary();
      double sf = m_sumw>0?1.0/m_sumw:1.0;
      for (auto hist:{m_hist_stage0,m_hist_stage1_pTjet25,m_hist_stage1_pTjet30,m_hist_stage1_2_pTjet25,m_hist_stage1_2_pTjet30,m_hist_stage1_2_fine_pTjet25,m_hist_stage1_2_fine_pTjet30,m_hist_stage1_3_pTjet25,m_hist_stage1_3_pTjet30,m_hist_stage1_3_fine_pTjet25,m_hist_stage1_3_fine_pTjet30,
        m_hist_Njets25,m_hist_Njets30,m_hist_pT_Higgs,m_hist_y_Higgs,m_hist_pT_V,m_hist_pT_jet1,m_hist_deltay_jj,m_hist_dijet_mass,m_hist_pT_Hjj,m_hist_isZ2vv})
        scale(hist, sf);
    }

    /*
     *  initialize histograms
     */

    void initializeHistos(){
      book(m_hist_stage0,"HTXS_stage0",20,0,20);
      book(m_hist_stage1_pTjet25,"HTXS_stage1_pTjet25",40,0,40);
      book(m_hist_stage1_pTjet30,"HTXS_stage1_pTjet30",40,0,40);
      book(m_hist_stage1_2_pTjet25,"HTXS_stage1_2_pTjet25",57,0,57);
      book(m_hist_stage1_2_pTjet30,"HTXS_stage1_2_pTjet30",57,0,57);
      book(m_hist_stage1_2_fine_pTjet25,"HTXS_stage1_2_fine_pTjet25",113,0,113);
      book(m_hist_stage1_2_fine_pTjet30,"HTXS_stage1_2_fine_pTjet30",113,0,113);
      book(m_hist_stage1_3_pTjet25, "STXS_stage1_3_pTjet25", 103, 0, 103);
      book(m_hist_stage1_3_pTjet30, "STXS_stage1_3_pTjet30", 103, 0, 103);
      book(m_hist_stage1_3_fine_pTjet25, "STXS_stage1_3_fine_pTjet25", 280, 0, 280);
      book(m_hist_stage1_3_fine_pTjet30, "STXS_stage1_3_fine_pTjet30", 280, 0, 280);
      book(m_hist_pT_Higgs,"pT_Higgs",80,0,400);
      book(m_hist_y_Higgs,"y_Higgs",80,-4,4);
      book(m_hist_pT_V,"pT_V",80,0,400);
      book(m_hist_pT_jet1,"pT_jet1",80,0,400);
      book(m_hist_deltay_jj ,"deltay_jj",50,0,10);
      book(m_hist_dijet_mass,"m_jj",50,0,2000);
      book(m_hist_pT_Hjj,"pT_Hjj",50,0,250);
      book(m_hist_Njets25,"Njets25",10,0,10);
      book(m_hist_Njets30,"Njets30",10,0,10);
      book(m_hist_isZ2vv,"isZ2vv",2,0,2);
    }
    /// @}

    /*
     *    initialize private members used in the classification procedure
     */

  private:
    double m_sumw=0.0;
    HTXS::HiggsProdMode m_HiggsProdMode;
    HTXS::HiggsDecayMode m_HiggsDecayMode;
    mutable std::array<std::atomic<size_t>, HTXS::NUM_ERRORCODES> m_errorCount ATLAS_THREAD_SAFE {};
    Histo1DPtr m_hist_stage0;
    Histo1DPtr m_hist_stage1_pTjet25, m_hist_stage1_pTjet30;
    Histo1DPtr m_hist_stage1_2_pTjet25, m_hist_stage1_2_pTjet30;
    Histo1DPtr m_hist_stage1_2_fine_pTjet25, m_hist_stage1_2_fine_pTjet30;
    Histo1DPtr m_hist_stage1_3_pTjet25, m_hist_stage1_3_pTjet30;
    Histo1DPtr m_hist_stage1_3_fine_pTjet25, m_hist_stage1_3_fine_pTjet30;
    Histo1DPtr m_hist_pT_Higgs, m_hist_y_Higgs;
    Histo1DPtr m_hist_pT_V, m_hist_pT_jet1;
    Histo1DPtr m_hist_deltay_jj, m_hist_dijet_mass, m_hist_pT_Hjj;
    Histo1DPtr m_hist_Njets25, m_hist_Njets30;
    Histo1DPtr m_hist_isZ2vv;
  };

  // the PLUGIN only needs to be decleared when running standalone Rivet
  // and causes compilation / linking issues if included in Athena / RootCore
  //check for Rivet environment variable RIVET_ANALYSIS_PATH
#ifdef RIVET_ANALYSIS_PATH
  // The hook for the plugin system
  DECLARE_RIVET_PLUGIN(HiggsTemplateCrossSections);
#endif

}

#endif
