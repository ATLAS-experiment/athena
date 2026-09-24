#ifndef ALPAKAEXAMPLEALG_H
#define ALPAKAEXAMPLEALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

class AlpakaExampleAlg : public AthReentrantAlgorithm {

  public:
    /// Inherit the base class's constructor
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// The function executing this algorithm
    virtual StatusCode execute( const EventContext& ctx ) const override;

}; // class AlpakaExampleAlg

#endif // ALPAKAEXAMPLEALG_H
