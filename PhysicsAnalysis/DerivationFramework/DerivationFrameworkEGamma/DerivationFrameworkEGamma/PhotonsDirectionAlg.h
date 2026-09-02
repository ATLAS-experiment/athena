/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_PHOTONSDIRECTIONALG_H
#define DERIVATIONFRAMEWORK_PHOTONSDIRECTIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEgamma/PhotonContainer.h"
//
#include <string>
#include <vector>
namespace DerivationFramework {

  class PhotonsDirectionAlg : public AthReentrantAlgorithm
  {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::PhotonContainer> m_collName{ this,
        "PhotonContainer",
        "Photons",
        "Input Photons" };

    SG::WriteHandleKey<std::vector<float>> m_sgEta{ this,
      "EtaSGEntry",
      "",
      "output Eta vector" };

    SG::WriteHandleKey<std::vector<float>> m_sgPhi{ this,
      "PhiSGEntry",
      "",
      "output Phi vector" };

    SG::WriteHandleKey<std::vector<float>> m_sgEt{ this,
      "EtSGEntry",
      "",
      "output E vector" };

    SG::WriteHandleKey<std::vector<float>> m_sgE{ this,
      "ESGEntry",
      "",
      "output E vector" };

    bool m_doEta = false;
    bool m_doPhi = false;
    bool m_doEt = false;
    bool m_doE = false;
  };
}

#endif // DERIVATIONFRAMEWORK_PHOTONSDIRECTIONALG_H
