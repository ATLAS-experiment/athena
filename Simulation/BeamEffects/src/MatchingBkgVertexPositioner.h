/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BEAMEFFECTS_MATCHINGBKGVERTEXPOSITIONER_H
#define BEAMEFFECTS_MATCHINGBKGVERTEXPOSITIONER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HepMC_Interfaces/ILorentzVectorGenerator.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/VertexContainer.h"

// STL includes
#include <string>

namespace Simulation {

/**
 *  @class MatchingBkgVertexPositioner
 *
 *  Returns a VertexShift based on what is given in the background input file.
 */

class MatchingBkgVertexPositioner
    : public extends<AthAlgTool, ILorentzVectorGenerator> {

 public:
  MatchingBkgVertexPositioner(const std::string &t, const std::string &n,
                            const IInterface *p);
  virtual ~MatchingBkgVertexPositioner() = default;

  StatusCode initialize() override final;

  CLHEP::HepLorentzVector *generate(
      const EventContext &ctx) const override final;

 private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{
      this, "PrimaryVertexContainerName", "PrimaryVertices"};
};

}  // namespace Simulation

#endif
