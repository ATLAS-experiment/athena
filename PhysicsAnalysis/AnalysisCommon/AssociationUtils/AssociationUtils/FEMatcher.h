/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_FEMATCHER_H
#define ASSOCIATIONUTILS_FEMATCHER_H

#include "xAODBase/IParticle.h"
#include "xAODMissingET/MissingETAssociationMap.h"

// Columnar includes
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ParticleDef.h"

namespace ADD_CMODE(ORUtils) 
{

  /// @class FEMatcher
  /// @brief Implements the flow element matching strategy

  /// @author Laura Miller <laura.miller@cern.ch>

  class FEMatcher : public columnar::ColumnarTool<> {
    public:

      // Constructor and destructor
      FEMatcher();
      ~FEMatcher(){};
      
      bool objectsMatch (const xAOD::IParticle* tPar, const xAOD::IParticle* rPar, xAOD::MissingETAssociationMap feMap, bool swapArgs = false) const;

    private:

      xAOD::MissingETAssociationMap m_feMap;

  }; // class FEMatcher
} // namespace ADD_CMODE(ORUtils)


#endif 