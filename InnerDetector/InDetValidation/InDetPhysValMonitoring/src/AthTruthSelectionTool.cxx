/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthTruthSelectionTool.cxx
 * implementation file for truth selection in this package
 * @author shaun roe
 * @date 10 October 2016
 **/

#include "AthTruthSelectionTool.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODTruth/TruthVertex.h"

#include <vector>
#include <cmath>

#include "TrkParameters/TrackParameters.h"

namespace {
  constexpr int electronId(11);
  constexpr int gammaId(22);
  bool hasAncestor(const xAOD::TruthParticle* particle, const std::vector<int>& allowedAncestors) {
    bool pass = false;
    uint nPar = particle->nParents(); 
    for (uint i = 0; i < nPar; i++) {
      for (const int & ancestorID : allowedAncestors) {
        if (std::abs(particle->parent(i)->pdgId()) ==  ancestorID) {
          return true;
        }
      }
    }
    for (uint i = 0; i < nPar; i++) {
      const xAOD::TruthParticle* parent = particle->parent(i);
      if (hasAncestor(parent, allowedAncestors)) return true;
    }
    return pass;
  }
}


AthTruthSelectionTool::AthTruthSelectionTool(const std::string& type, const std::string& name,
                                             const IInterface* parent) :
  AthAlgTool(type, name, parent)
{
  // declare interface from base class
  declareInterface<IAthSelectionTool>(this);
}

StatusCode
AthTruthSelectionTool::initialize() {
  // can set cut properties now
  using P_t = xAOD::TruthParticle;
  using Accept_t = Accept<P_t>;
  //
  const std::vector<Accept_t> filters = {
    // if p.pt=0, TVector3 generates an error when querying p.eta(); a limit of 1e-7 was not found to be enough to
    // prevent this
    // the following also vetoes the case where the p.pt()=NaN, as any inequality with NaN evaluates to false
    Accept_t([this](const P_t& p) -> bool {
      return((p.pt() > 0.1) ? (std::abs(p.eta()) < m_maxEta) : false);
    }, std::string("eta")),
    Accept_t([this](const P_t& p) -> bool {
      return(p.pt() > m_minPt);
    }, std::string("min_pt"))
  };
  //
  m_cutList = CutList<P_t>(filters);
  if (m_maxProdVertRadius>0) {
    m_cutList.add(Accept_t([&m_maxProdVertRadius = std::as_const(m_maxProdVertRadius)](const P_t& p) -> bool {
                       return((not (p.hasProdVtx()))or(p.prodVtx()->perp() < m_maxProdVertRadius));
                  },
                  "decay_before_" + std::to_string(m_maxProdVertRadius)));
  }
  if (m_maxPt > 0) {
    m_cutList.add(Accept_t([&m_maxPt = std::as_const(m_maxPt)](const P_t& p) {
      return(p.pt() < m_maxPt);
    }, "max_pt"));
  }
  if (m_requireSiHit > 0) {
    m_cutList.add(Accept_t([&m_requireSiHit = std::as_const(m_requireSiHit)](const P_t& p) {
      static const SG::AuxElement::ConstAccessor< float > nSilHitsAcc("nSilHits");
      if (nSilHitsAcc.isAvailable(p)) return (nSilHitsAcc(p) >= m_requireSiHit);
      else return false;
    }, "siHit"));
  }
  if (m_requireOnlyPrimary) {
    m_cutList.add(Accept_t([](const P_t& p) {
      return(!HepMC::is_simulation_particle(&p));
    }, "OnlyPrimary"));
  }
  if (m_requireCharged) {
    m_cutList.add(Accept_t([&m_selectedCharge = std::as_const(m_selectedCharge)](const P_t& p) {
      if(m_selectedCharge ==0) return(not (p.isNeutral()));
      else return(not(p.isNeutral()) and p.charge()==m_selectedCharge);
    }, "charged"));
  }
  if (m_requireStable) {
    m_cutList.add(Accept_t([](const P_t& p) {
      return(MC::isStable(&p));
    }, "stable"));
  }
  if (m_pdgId > 0) {
    m_cutList.add(Accept_t([&m_pdgId = std::as_const(m_pdgId)](const P_t& p) {
      return(std::abs(p.pdgId()) == m_pdgId);
    }, "pdgId"));
  }
  if (m_vetoPdgId > 0) {
    m_cutList.add(Accept_t([&m_vetoPdgId = std::as_const(m_vetoPdgId)](const P_t& p) {
      return(std::abs(p.pdgId()) != m_vetoPdgId);
    }, "vetoPdgId"));
  }
  if (m_grandparent) {
    m_cutList.add(Accept_t([](const P_t& p) {
      return((p.nParents() == 0) || ((p.nParents() == 1)and((p.parent(0))->nParents() == 0)));
    }, "hasNoGrandparent"));
  }
  //require the truth particles to come from certain ancesters
  if (!m_ancestors.empty()) {
    m_cutList.add(Accept_t([&m_ancestors = std::as_const(m_ancestors)](const P_t& p) -> bool {
      const xAOD::TruthParticle* pTruth = dynamic_cast<const xAOD::TruthParticle*>(&p);  
      if (not pTruth) return false;
      else return hasAncestor(pTruth, m_ancestors);   
    }, "ancestors"));
  }
  if (m_poselectronfromgamma) {
    m_cutList.add(Accept_t([](const P_t& p) {
      return((p.absPdgId() == electronId)and(p.nParents() >= 1) and(p.parent(0)) and(p.parent(0)->pdgId() == gammaId));
    }, "poselectronfromgamma"));
  }
  if (m_radiusCylinder > 0) {
    //    m_cutList.add(Accept_t(acceptExtrapolatedTPToSurface, "SelectCylinder"));
    if (not m_cylinder) {
      ATH_MSG_VERBOSE("Creating and caching cylinder surface");
      Amg::Transform3D trnsf;
      trnsf.setIdentity();
      m_cylinder = std::make_unique<Trk::CylinderSurface>( trnsf, m_radiusCylinder, 20000.);
    }
    m_cutList.add(Accept_t([this](const P_t& p) -> bool {
          ATH_MSG_VERBOSE("Checking particle for intersection with cylinder of radius " << m_radiusCylinder);
          //create surface we extrapolate to and cache it
          const xAOD::TruthVertex* ptruthVertex = p.prodVtx();
          if (ptruthVertex == nullptr) {
            //cannot derive production vertex, reject track
            ATH_MSG_VERBOSE("Rejecting particle without production vertex.");
            return false;
          }
          const auto xPos = ptruthVertex->x();
          const auto yPos = ptruthVertex->y();
          const auto z_truth = ptruthVertex->z();
          const Amg::Vector3D position(xPos, yPos, z_truth);
          const Amg::Vector3D momentum(p.px(), p.py(), p.pz());
          const Trk::CurvilinearParameters cParameters(position, momentum, p.charge());
          const Trk::TrackParameters *exParameters = m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),
                                                                                 cParameters, 
                                                                                 *m_cylinder, 
                                                                                 Trk::anyDirection, false, Trk::pion).release();
          if (!exParameters) {
            ATH_MSG_VERBOSE("Failed extrapolation. Rejecting track.");
            return false;
          }
          ATH_MSG_VERBOSE("Extrapolated parameters to cylinder: " << *exParameters);
          const float ex_abs_z = fabs(exParameters->position().z());
          if ( (ex_abs_z > m_minZCylinder) and (ex_abs_z < m_maxZCylinder) ) {
            ATH_MSG_VERBOSE("Particle accepted.");
            return true;
          }
          //else..
          ATH_MSG_VERBOSE("Particle rejected");
          return false;
        }, "SelectCylinder"));
  } else if (m_zDisc > 0) {
    //m_cutList.add(Accept_t(acceptExtrapolatedTPToSurface, "SelectDisc"));
    if (not m_disc1 || not m_disc2) { //m_disc2 == 0 implied
      ATH_MSG_VERBOSE("Creating and caching disc surface");
      Amg::Transform3D trnsf_shiftZ = Amg::Transform3D(Amg::Translation3D(0.,0.,m_zDisc));
      m_disc1 = std::make_unique<Trk::DiscSurface>( trnsf_shiftZ, m_minRadiusDisc, m_maxRadiusDisc);
      trnsf_shiftZ = Amg::Translation3D(0.,0.,-m_zDisc);
      m_disc2 = std::make_unique<Trk::DiscSurface>( trnsf_shiftZ, m_minRadiusDisc, m_maxRadiusDisc);
    }
    m_cutList.add(Accept_t([this](const P_t& p) -> bool {
          ATH_MSG_VERBOSE("Checking particle for intersection with discs of |z| " << m_zDisc);
          //create surface we extrapolate to and cache it
          const xAOD::TruthVertex* ptruthVertex = p.prodVtx();
          if (ptruthVertex == nullptr) {
            //cannot derive production vertex, reject track
            ATH_MSG_VERBOSE("Rejecting particle without production vertex.");
            return false;
          }
          const auto xPos = ptruthVertex->x();
          const auto yPos = ptruthVertex->y();
          const auto z_truth = ptruthVertex->z();
          const Amg::Vector3D position(xPos, yPos, z_truth);
          const Amg::Vector3D momentum(p.px(), p.py(), p.pz());
          const Trk::CurvilinearParameters cParameters(position, momentum, p.charge());
          const Trk::TrackParameters *exParameters = m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),
                                                                                 cParameters, 
                                                                                 *m_disc1, Trk::anyDirection, true, Trk::pion).release();
          if (exParameters) {
            //since boundary check is true, should be enough to say we've hit the disk..
            ATH_MSG_VERBOSE("Successfully extrapolated track to disk at +" << m_zDisc << ": " << *exParameters);
            float ex_radius = sqrt(pow(exParameters->position().x(),2)+pow(exParameters->position().y(),2));
            ATH_MSG_VERBOSE("radial position at surface: " << ex_radius);
            if ((ex_radius > m_minRadiusDisc) and (ex_radius < m_maxRadiusDisc)) {
              ATH_MSG_VERBOSE("Confirmed within the disk. Accepting particle");
              return true;
            }
            //else...
            ATH_MSG_VERBOSE("Strange, extrapolation succeeded but extrapolated position not within disc radius! Test next disc");
          }
          exParameters = m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),cParameters, *m_disc2, Trk::anyDirection, true, Trk::pion).release();
          if (exParameters) {
            //since boundary check is true, should be enough to say we've hit the disk..
            ATH_MSG_VERBOSE("Successfully extrapolated track to disk at -" << m_zDisc << ": " << *exParameters);
            float ex_radius = sqrt(pow(exParameters->position().x(),2)+pow(exParameters->position().y(),2));
            ATH_MSG_VERBOSE("radial position at surface: " << ex_radius);
            if ((ex_radius > m_minRadiusDisc) and (ex_radius < m_maxRadiusDisc)) {
              ATH_MSG_VERBOSE("Confirmed within the disk. Accepting particle");
              return true;
            }
            //else...
            ATH_MSG_VERBOSE("Strange, extrapolation succeeded but extrapolated position not within disc radius! Rejecting");
          }
          //else..
          ATH_MSG_VERBOSE("Particle rejected");
          return false;
        }, "SelectDisc"));
  } //m_zDisc > 0

  std::string msg = std::to_string(m_cutList.size()) + " truth acceptance cuts are used:\n";
  for (const auto& i:m_cutList.names()) {
    msg += i + "\n";
  }
  ATH_MSG_INFO(msg);

  ATH_CHECK(m_extrapolator.retrieve(EnableTool{ m_radiusCylinder > 0 || m_zDisc >0 }));

  return StatusCode::SUCCESS;
}

StatusCode
AthTruthSelectionTool::finalize() {
  // nop
  return StatusCode::SUCCESS;
}


std::vector<std::string>
AthTruthSelectionTool::names() const {
  return m_cutList.names();
}

IAthSelectionTool::CutResult
AthTruthSelectionTool::accept(const xAOD::IParticle* particle) const {
  const xAOD::TruthParticle* pTruth = dynamic_cast<const xAOD::TruthParticle*>(particle);

  //@TODO
  if (not pTruth) {
    return m_cutList.size()+1; // marker for invalid particles
  }
  return m_cutList.accept(*pTruth);
}

IAthSelectionTool::CutResult
AthTruthSelectionTool::testAllCuts(const xAOD::IParticle * particle, std::vector<unsigned int> &counter) const {
  const xAOD::TruthParticle* pTruth = dynamic_cast<const xAOD::TruthParticle*>(particle);

  //@TODO
  if (not pTruth) {
    return m_cutList.size()+1; // marker for invalid particles
  }
  return m_cutList.testAllCuts(*pTruth,counter);
}

