/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAMVACALIB_EGAMMAMVAFUNCTIONS
#define EGAMMAMVACALIB_EGAMMAMVAFUNCTIONS

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/PhotonxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "egammaMVALayerDepth.h"
#include "AthContainers/ConstAccessor.h"

#include "TLorentzVector.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <stdexcept>

// for the ConversionHelper (deprecated?)
#include <AsgMessaging/AsgMessaging.h>

/**
 * These functions are for calculating variables used by the
 * MVA calibration. The user can use the functions
 * - egammaMVAFunctions::initializeElectronFuncs
 * - egammaMVAFunctions::initializeUnconvertedPhotonFuncs
 * - egammaMVAFunctions::initializeConvertedPhotonFuncs
 * the will return an unordered map with key a string, corresponding
 * to the variable to be computed and as a value the function, with
 * signature float(const xAOD::Egamma*).
 **/

// Changing the definition of the functions means breaking backward
// compatibility with previous version of the MVA calibrations.

namespace egammaMVAFunctions
{
  // inline functions to avoid duplicates problem during linking (and performance)
  // cluster functions
  // REMEMBER to add the functions using corrected layer energies
  inline float compute_cl_eta(const xAOD::CaloCluster& cluster) { return cluster.eta(); }
  inline float compute_cl_phi(const xAOD::CaloCluster& cluster) { return cluster.phi(); }
  inline float compute_cl_e(const xAOD::CaloCluster& cluster) { return cluster.e(); }
  inline float compute_cl_etaCalo(const xAOD::CaloCluster& cluster) {
    double tmp = 0.;
    if(! (cluster.retrieveMoment(xAOD::CaloCluster::ETACALOFRAME, tmp))) {
      throw std::runtime_error("etaCalo not found in CaloCluster object");
    }
    return tmp;
  }
  inline float compute_cl_phiCalo(const xAOD::CaloCluster& cluster) {
    double tmp = 0.;
    if(! (cluster.retrieveMoment(xAOD::CaloCluster::PHICALOFRAME, tmp))) {
      throw std::runtime_error("phiCalo not found in CaloCluster object");
    }
    return tmp;
  }
  inline float compute_cl_etas1(const xAOD::CaloCluster& cluster) { return cluster.etaBE(1); }
  inline float compute_cl_etas2(const xAOD::CaloCluster& cluster) { return cluster.etaBE(2); }
  inline float compute_rawcl_Es0(const xAOD::CaloCluster& cl) { return cl.energyBE(0); }
  /*inline std::function<float(const xAOD::CaloCluster&)> compute_rawcl_Es0_auto(bool use_corrected)
  {
      if (use_corrected) return [](const xAOD::CaloCluster& cl) { return cl.energyBE(0); };
      else return [](const xAOD::CaloCluster& cl) { return cl.energyBE(0); };
  }*/
  inline float compute_rawcl_Es1(const xAOD::CaloCluster& cl) { return cl.energyBE(1); }
  inline float compute_rawcl_Es2(const xAOD::CaloCluster& cl) { return cl.energyBE(2); }
  inline float compute_rawcl_Es3(const xAOD::CaloCluster& cl) { return cl.energyBE(3); }

  inline float compute_correctedcl_Es0(const xAOD::CaloCluster& cl) {
    static const SG::ConstAccessor<double> acc ("correctedcl_Es0");
    return acc.isAvailable(cl) ? acc(cl) : cl.energyBE(0);
  }
  inline float compute_correctedcl_Es1(const xAOD::CaloCluster& cl) {
    static const SG::ConstAccessor<double> acc ("correctedcl_Es1");
    return acc.isAvailable(cl) ? acc(cl) : cl.energyBE(1);
  }
  inline float compute_correctedcl_Es2(const xAOD::CaloCluster& cl) {
    static const SG::ConstAccessor<double> acc ("correctedcl_Es2");
    return acc.isAvailable(cl) ? acc(cl) : cl.energyBE(2);
  }
  inline float compute_correctedcl_Es3(const xAOD::CaloCluster& cl) {
    static const SG::ConstAccessor<double> acc ("correctedcl_Es3");
    return acc.isAvailable(cl) ? acc(cl) : cl.energyBE(3);
  }

  inline float compute_rawcl_Eacc(const xAOD::CaloCluster& cl) { return cl.energyBE(1) + cl.energyBE(2) + cl.energyBE(3); }
  inline float compute_rawcl_f0(const xAOD::CaloCluster& cl) { return cl.energyBE(0) / (cl.energyBE(1) + cl.energyBE(2) + cl.energyBE(3)); }

  inline float compute_correctedcl_Eacc(const xAOD::CaloCluster& cl) { return compute_correctedcl_Es1(cl) + compute_correctedcl_Es2(cl) + compute_correctedcl_Es3(cl); }
  inline float compute_correctedcl_f0(const xAOD::CaloCluster& cl) { return compute_correctedcl_Es0(cl) / (compute_correctedcl_Eacc(cl)); }


  inline float compute_calibHitsShowerDepth(const std::array<float, 4>& cl, float eta)
  {
    const float denominator = cl[0] + cl[1] + cl[2] + cl[3];
    if (denominator == 0) return 0.;

    const std::array<float, 4> radius(get_MVAradius(eta));

    // loop unrolling
    return (radius[0] * cl[0]
          + radius[1] * cl[1]
          + radius[2] * cl[2]
          + radius[3] * cl[3]) / denominator;
  }

  inline float compute_rawcl_calibHitsShowerDepth(const xAOD::CaloCluster& cl)
  {
      const std::array<float, 4> cluster_array {{ compute_rawcl_Es0(cl),
                                                  compute_rawcl_Es1(cl),
                                                  compute_rawcl_Es2(cl),
                                                  compute_rawcl_Es3(cl) }};
      return compute_calibHitsShowerDepth(cluster_array, compute_cl_eta(cl));
  }

  inline float compute_correctedcl_calibHitsShowerDepth(const xAOD::CaloCluster& cl) {
    const std::array<float, 4> cluster_array {{ compute_correctedcl_Es0(cl),
                                                compute_correctedcl_Es1(cl),
                                                compute_correctedcl_Es2(cl),
                                                compute_correctedcl_Es3(cl) }};
    return compute_calibHitsShowerDepth(cluster_array, compute_cl_eta(cl));
  }

  // ------------------------------------------------------------------
  // Forward-electron getters
  // ------------------------------------------------------------------

  inline float compute_et(const xAOD::CaloCluster& cl) {
    const float e   = cl.e();
    const float eta = cl.eta();
    const float c   = std::cosh(eta);
    return (c != 0.f) ? (e / c) : 0.f;
  }
  // be 100% sure what variables the FE BDT should use. For the run 2 
  inline float cl_getMoment(const xAOD::CaloCluster& cl,
                            xAOD::CaloCluster::MomentType m,
                            const char* name) {
    double tmp = 0.;
    if (!cl.retrieveMoment(m, tmp)) {
      throw std::runtime_error(std::string("Forward-electron missing moment: ") + name);
    }
    return static_cast<float>(tmp);
  }

  inline float compute_cl_significance (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::SIGNIFICANCE  , "SIGNIFICANCE"); }
  inline float compute_cl_secondLambda (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::SECOND_LAMBDA , "SECOND_LAMBDA"); }
  inline float compute_cl_lateral      (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::LATERAL       , "LATERAL"); }
  inline float compute_cl_longitudinal (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::LONGITUDINAL  , "LONGITUDINAL"); }
  inline float compute_cl_fracMax      (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::ENG_FRAC_MAX  , "ENG_FRAC_MAX"); }
  
  inline float compute_cl_secondR      (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::SECOND_R      , "SECOND_R"); }
  inline float compute_cl_centerLambda (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::CENTER_LAMBDA , "CENTER_LAMBDA"); }
  inline float compute_cl_secondDensity(const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::SECOND_ENG_DENS,"SECOND_ENG_DENS"); }
  inline float compute_cl_x            (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::CENTER_X      , "CENTER_X"); }
  inline float compute_cl_y            (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::CENTER_Y      , "CENTER_Y"); }
  inline float compute_cl_z            (const xAOD::CaloCluster& cl){ return cl_getMoment(cl, xAOD::CaloCluster::CENTER_Z      , "CENTER_Z"); }

  inline float compute_cl_secondR_fudge(const xAOD::Egamma& eg) {
    static const SG::AuxElement::Accessor<float> accR2("SECOND_R");
    if (accR2.isAvailable(eg)) { return accR2(eg); }
    return -1.;
  }

  inline float compute_eta_FCAL(const xAOD::CaloCluster& cl){
    float x = compute_cl_x(cl);
    float y = compute_cl_y(cl);
    float z = compute_cl_z(cl);
    float theta = std::acos(z/std::sqrt(x*x+y*y+z*z));
    return -std::log(std::tan(theta/2.));
  }
  inline float compute_etaMod_FCAL(const xAOD::CaloCluster& cl){
    return std::fmod(std::abs(compute_eta_FCAL(cl)),0.15);
  }
  inline float compute_cellIndex_FCAL(const xAOD::CaloCluster& cl){
    return std::floor(std::abs(compute_eta_FCAL(cl))/0.15);
  }
  inline float compute_etaMod_EMEC(const xAOD::CaloCluster& cl){
    return std::fmod(std::abs(cl.eta()),0.1);
  }
  inline float compute_cellIndex_EMEC(const xAOD::CaloCluster& cl){
    return std::floor(std::abs(cl.eta())/0.1);
  }
  inline float compute_phiMod_EMEC(const xAOD::CaloCluster& cl){
    static const float cz = std::numbers::pi/16.;
    float phi_mod = std::fmod(cl.phi(), cz);
    if (phi_mod < 0) phi_mod += cz;
    return phi_mod;
  }
  inline float compute_R12_EMEC(const xAOD::CaloCluster& cl){
    return float(cl.energy_max(CaloSampling::EME1)/cl.energy_max(CaloSampling::EME2));
  }

  // electron functions
  inline float compute_el_charge(const xAOD::Electron& el) { return el.charge(); }
  inline float compute_el_tracketa(const xAOD::Electron& el) { return el.trackParticle()->eta(); }
  inline float compute_el_trackpt(const xAOD::Electron& el) { return el.trackParticle()->pt(); }
  inline float compute_el_trackz0(const xAOD::Electron& el) { return el.trackParticle()->z0(); }
  inline float compute_el_refittedTrack_qoverp(const xAOD::Electron& el) { return el.trackParticle()->qOverP(); }
  inline int compute_el_author(const xAOD::Electron& el) {
    static const SG::ConstAccessor<unsigned short int> acc ("author");
    return acc (el);
  }

  // photon functions
  inline int compute_ph_convFlag(const xAOD::Photon& ph) {
    const auto original = xAOD::EgammaHelpers::conversionType(&ph);
    if (original == 3) return 2;
    else if (original != 0) return 1;
    else return original;
  }

  // a utility function
  inline float getPtAtFirstMeasurement(const xAOD::TrackParticle* tp)
  {
    if (!tp) return 0;
    for (unsigned int i = 0; i < tp->numberOfParameters(); ++i) {
      if (tp->parameterPosition(i) == xAOD::FirstMeasurement) {
        return hypot(tp->parameterPX(i), tp->parameterPY(i));
      }
    }
    return tp->pt();
  }

  // define a few without using conversion helper

  /// This ptconv function uses the vertex decorations
  inline float compute_ptconv_decor(const xAOD::Photon* ph)
  {
    static const SG::AuxElement::Accessor<float> accPx("px");
    static const SG::AuxElement::Accessor<float> accPy("py");
    
    auto vx = ph->vertex();
    return vx ? std::hypot(accPx(*vx), accPy(*vx)) : 0.0;
  }

  /// This ptconv is the old one used by MVACalib
  inline float compute_ptconv(const xAOD::Photon* ph)
  {
    auto vx = ph->vertex();
    if (!vx) return 0.0;
 
    TLorentzVector sum;
    if (vx->trackParticle(0)) sum += vx->trackParticle(0)->p4();
    if (vx->trackParticle(1)) sum += vx->trackParticle(1)->p4();
    return sum.Perp();
  }

  inline float compute_pt1conv(const xAOD::Photon* ph)
  {
    static const SG::AuxElement::Accessor<float> accPt1("pt1");
    
    const xAOD::Vertex* vx = ph->vertex();
    if (!vx) return 0.0;
    if (accPt1.isAvailable(*vx)) {
      return accPt1(*vx);
    } else {
      return getPtAtFirstMeasurement(vx->trackParticle(0));
    }
  }

  inline float compute_pt2conv(const xAOD::Photon* ph)
  {
    static const SG::AuxElement::Accessor<float> accPt2("pt2");
    
    const xAOD::Vertex* vx = ph->vertex();
    if (!vx) return 0.0;
    if (accPt2.isAvailable(*vx)) {
      return accPt2(*vx);
    } else {
      return getPtAtFirstMeasurement(vx->trackParticle(1));
    }
  }

  // using template to avoid rewriting code for 1st, 2nd track and
  // for all the summary types
  template<int itrack, xAOD::SummaryType summary>
  inline int compute_convtrkXhits(const xAOD::Photon* ph) {
      const auto vx = ph->vertex();
      if (!vx) return 0.;

      if (vx->trackParticle(0)) {
        uint8_t hits;
        if (vx->trackParticle(itrack)->summaryValue(hits, summary)) {
          return hits;
        }
      }
      return 0.;
  }

  inline int compute_convtrk1nPixHits(const xAOD::Photon* ph) { return compute_convtrkXhits<0, xAOD::numberOfPixelHits>(ph); }
  inline int compute_convtrk2nPixHits(const xAOD::Photon* ph) { return compute_convtrkXhits<1, xAOD::numberOfPixelHits>(ph); }
  inline int compute_convtrk1nSCTHits(const xAOD::Photon* ph) { return compute_convtrkXhits<0, xAOD::numberOfSCTHits>(ph); }
  inline int compute_convtrk2nSCTHits(const xAOD::Photon* ph) { return compute_convtrkXhits<1, xAOD::numberOfSCTHits>(ph); }

  // The functions to return the dictionaries of functions,
  // i.e., the variable name to function

  /// Define the map type since it's long
  using funcMap_t = std::unordered_map<std::string,
                              	       std::function<float(const xAOD::Egamma*, const xAOD::CaloCluster*)> >;

  /// A function to build the map for electrons
  std::unique_ptr<funcMap_t> initializeElectronFuncs(bool useLayerCorrected);

  /// A function to build the map for uncoverted photons
  std::unique_ptr<funcMap_t> initializeUnconvertedPhotonFuncs(bool useLayerCorrected);

  /// A function to build the map for converted photons
  std::unique_ptr<funcMap_t> initializeConvertedPhotonFuncs(bool useLayerCorrected);

  /// NEW: A function to build the map for forward electrons
  std::unique_ptr<funcMap_t> initializeForwardElectronFuncs(bool useLayerCorrected);

  /// The ConversionHelper struct is stll used by egammaMVATree in PhysicsAnalysis
  /// but not the functions in the dictionaries above. We could deprecate them
  struct ConversionHelper : asg::AsgMessaging
  {
    ConversionHelper(const xAOD::Photon* ph)
      : asg::AsgMessaging("ConversionHelper"),
        m_vertex(ph ? ph->vertex() : nullptr),
        m_tp0(m_vertex ? m_vertex->trackParticle(0) : nullptr),
        m_tp1(m_vertex ? m_vertex->trackParticle(1) : nullptr),
        m_pt1conv(0.), m_pt2conv(0.)
    {
     
      ATH_MSG_DEBUG("init conversion helper");
      if (!m_vertex) return;

      static const SG::AuxElement::Accessor<float> accPt1("pt1");
      static const SG::AuxElement::Accessor<float> accPt2("pt2");
      if (accPt1.isAvailable(*m_vertex) && accPt2.isAvailable(*m_vertex))
      {
        m_pt1conv = accPt1(*m_vertex);
        m_pt2conv = accPt2(*m_vertex);
      }
      else
      {
        ATH_MSG_WARNING("pt1/pt2 not available, will approximate from first measurements");
        m_pt1conv = getPtAtFirstMeasurement(m_tp0);
        m_pt2conv = getPtAtFirstMeasurement(m_tp1);
      }
    }

    ConversionHelper(const xAOD::Photon& ph)
      : ConversionHelper(&ph) { }  // delegating constr

    inline float ph_Rconv() const { return m_vertex ? hypot(m_vertex->position().x(), m_vertex->position().y()) : 0; }
    inline float ph_zconv() const { return m_vertex ? m_vertex->position().z() : 0.; }
    inline int ph_convtrk1nPixHits() const {
      if (!m_tp0) { return 0; }
      uint8_t hits = 0;
      if (m_tp0->summaryValue(hits, xAOD::numberOfPixelHits)) { return hits; }
      else {
        ATH_MSG_WARNING("cannot read xAOD::numberOfPixelHits");
        return 0;
      }
    }
    inline int ph_convtrk2nPixHits() const {
      if (!m_tp1) return 0;
      uint8_t hits;
      if (m_tp1->summaryValue(hits, xAOD::numberOfPixelHits)) { return hits; }
      else {
        ATH_MSG_WARNING("cannot read xAOD::numberOfPixelHits");
        return 0;
      }
    }
    inline int ph_convtrk1nSCTHits() const {
      if (!m_tp0) { return 0; }
      uint8_t hits;
      if (m_tp0->summaryValue(hits, xAOD::numberOfSCTHits)) { return hits; }
      else {
        ATH_MSG_WARNING("cannot read xAOD::numberOfSCTHits");
        return 0;
      }
    }
    inline int ph_convtrk2nSCTHits() const {
      if (!m_tp1) { return 0; }
      uint8_t hits;
      if (m_tp1->summaryValue(hits, xAOD::numberOfSCTHits)) { return hits; }
      else {
        ATH_MSG_WARNING("cannot read xAOD::numberOfSCTHits");
        return 0;
      }
    }
    inline float ph_pt1conv() const { return m_pt1conv; }
    inline float ph_pt2conv() const { return m_pt2conv; }
    inline float ph_ptconv() const {
      // TODO: evaluate if move to this new definition, by now keep the previous one
      // to be consistent with the training
      // return m_vertex ? xAOD::EgammaHelpers::momentumAtVertex(*m_vertex).perp() : 0.;
      TLorentzVector sum;
      if (m_tp0) sum += m_tp0->p4();
      if (m_tp1) sum += m_tp1->p4();
      return sum.Perp();
    }
  private:
    const xAOD::Vertex* m_vertex;
    const xAOD::TrackParticle* m_tp0;
    const xAOD::TrackParticle* m_tp1;
    float m_pt1conv, m_pt2conv;
  };


} // end namespace

#endif
