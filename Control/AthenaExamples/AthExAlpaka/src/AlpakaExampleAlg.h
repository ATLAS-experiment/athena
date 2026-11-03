//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

#ifndef ATHEXALPAKA_ALPAKAEXAMPLEALG_H
#define ATHEXALPAKA_ALPAKAEXAMPLEALG_H

// Framework include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

class AlpakaExampleAlg : public AthReentrantAlgorithm {

  public:
    /// Inherit the base class's constructor
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// The function setting up the algorithm
    virtual StatusCode initialize() override;

    /// The function executing this algorithm
    virtual StatusCode execute( const EventContext& ctx ) const override;

  private:
    /// The number of synthetic elements to process per event.
    Gaudi::Property< unsigned int > m_nElements{
        this, "NElements", 8,
        "Number of synthetic elements to process per event" };

};  // class AlpakaExampleAlg

#endif  // ATHEXALPAKA_ALPAKAEXAMPLEALG_H

