/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef PARTICLE_JET_LABEL_COMMON_H
#define PARTICLE_JET_LABEL_COMMON_H

#include "xAODTruth/TruthParticle.h"
#include "xAODJet/Jet.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthEventContainer.h"
#include "AsgDataHandles/ReadHandleKey.h"

// Eigen needed for Vector3D
#include "GeoPrimitives/GeoPrimitives.h"

#include <vector>

namespace ParticleJetTools {

  struct LabelNames {
    std::string singleint;
    std::string doubleint;
    std::string pt;
    std::string Lxy;
    std::string dr;
    std::string pdgId;
    std::string positionDPhi;
    std::string positionDEta;
    std::string uniqueID;
    std::string childLxy;
    std::string childPt;
    std::string childPdgId;
    std::string childPositionDPhi;
    std::string childPositionDEta;
    void check();
  };

  struct LabelDecorators {
    LabelDecorators(const LabelNames&);
    SG::AuxElement::Decorator<int> singleint;
    SG::AuxElement::Decorator<int> doubleint;
    SG::AuxElement::Decorator<float> pt;
    SG::AuxElement::Decorator<float> Lxy;
    SG::AuxElement::Decorator<float> dr;
    SG::AuxElement::Decorator<int> pdgId;
    SG::AuxElement::Decorator<float> positionDPhi;
    SG::AuxElement::Decorator<float> positionDEta;
    SG::AuxElement::Decorator<int> uniqueID;
    SG::AuxElement::Decorator<float> childLxy;
    SG::AuxElement::Decorator<float> childPt;
    SG::AuxElement::Decorator<int> childPdgId;
    SG::AuxElement::Decorator<float> childPositionDPhi;
    SG::AuxElement::Decorator<float> childPositionDEta;
  };

  class IParticleLinker {
  public:
    IParticleLinker(const SG::ReadHandleKey<xAOD::TruthParticleContainer>&,
                    const std::string& linkName);
    void decorate(const xAOD::Jet&,
                  const std::vector<const xAOD::TruthParticle*>&) const;
  private:
    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;
    SG::AuxElement::Decorator<IPLV> m_dec;
  };

  struct Particles {
    std::vector<const xAOD::TruthParticle*> b;
    std::vector<const xAOD::TruthParticle*> c;
    std::vector<const xAOD::TruthParticle*> tau;
    Amg::Vector3D origin;
  };

  Amg::Vector3D p3(const xAOD::TruthVertex* p);
  Amg::Vector3D p3(const xAOD::Jet& j);
  Amg::Vector3D signalProcessP3(const xAOD::TruthEventContainer&);

  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelNames& names);
  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelDecorators& decs);

  float partPt(const xAOD::TruthParticle* part);
  float partLxy(const xAOD::TruthParticle* part, const Amg::Vector3D& origin);
  float partDR(const xAOD::TruthParticle* part, const xAOD::Jet& jet);
  int partPdgId(const xAOD::TruthParticle* part);
  float positionDPhi(const xAOD::TruthParticle* part,
                     const xAOD::Jet& jet,
                     const Amg::Vector3D& origin);
  float positionDEta(const xAOD::TruthParticle* part,
                     const xAOD::Jet& jet,
                     const Amg::Vector3D& origin);


  void childrenRemoved
  ( const std::vector<const xAOD::TruthParticle*>& parents
    , std::vector<const xAOD::TruthParticle*>& children
    );

  template<typename T>
  void declareProperties(T& tool, LabelNames* n) {
    tool.declareProperty("LabelName", n->singleint="", "Jet label attribute to be added.");
    tool.declareProperty("DoubleLabelName", n->doubleint="", "Jet label attribute to be added (with the possibility of up to 2 matched hadrons).");
    tool.declareProperty("LabelPtName", n->pt="", "Attribute for labelling particle pt");
    tool.declareProperty("LabelLxyName", n->Lxy="", "Attribute for Lxy of labelling particle");
    tool.declareProperty("LabelDRName", n->dr="", "Attribute for dR(part, jet) for labelling particle");
    tool.declareProperty("LabelPdgIdName", n->pdgId="", "Attribute for pdgID of labelling particle");
    tool.declareProperty("LabelPositionDPhiName", n->positionDPhi="", "Attribute for the position dPhi of the labeling particle ");
    tool.declareProperty("LabelPositionDEtaName", n->positionDEta="", "Attribute for the position dEta of the labeling particle ");
    tool.declareProperty("LabelBarcodeName", n->uniqueID="", "Attribute for uniqueID of labeling particle");
    tool.declareProperty("ChildLxyName", n->childLxy="", "Attribute for the labeling particle child Lxy");
    tool.declareProperty("ChildPtName", n->childPt="", "Attribute for the labeling particle child Pt");
    tool.declareProperty("ChildPdgIdName", n->childPdgId="", "Attribute for the labeling particle child pdg ID");
    tool.declareProperty("ChildPositionDPhiName", n->childPositionDPhi="", "Attribute for the position dPhi of the labeling particle child");
    tool.declareProperty("ChildPositionDEtaName", n->childPositionDEta="", "Attribute for the position dEta of the labeling particle child");
  }

}

#endif
