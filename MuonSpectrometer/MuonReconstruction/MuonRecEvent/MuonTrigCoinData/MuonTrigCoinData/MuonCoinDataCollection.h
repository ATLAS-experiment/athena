/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRIGCOINDATA_MUONCOINDATACOLLECTION_H
#define MUONTRIGCOINDATA_MUONCOINDATACOLLECTION_H

// Base classes
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "AthContainers/DataVector.h"

#include "AthenaKernel/CLASS_DEF.h"

namespace Muon{

template< class CoinDataT >
class MuonCoinDataCollection : public DataVector< CoinDataT > {
public:

  // Constructor with parameters:
  //   Hashed offline identifier of the DE
  MuonCoinDataCollection(const IdentifierHash idHash);

  MuonCoinDataCollection(const MuonCoinDataCollection&) = delete;
  MuonCoinDataCollection &operator=(const MuonCoinDataCollection&) = delete;


 /** Default Constructor (for persistency)*/
  MuonCoinDataCollection() = default;
                     
  /** Destructor:*/
  virtual ~MuonCoinDataCollection() = default;

  // typedef needed for IdentifiableContainer base class
  typedef Identifier ID;

  // identifier of this detector element:
  Identifier identify() const;

  IdentifierHash identifyHash() const;

  void setIdentifier(Identifier id);
  // plottable
  virtual std::string type() const;
 

private:
  const IdentifierHash m_idHash{}; 
  Identifier m_id{}; // identifier of the DE
  

};
// member functions that use Collection T
#include "MuonTrigCoinData/MuonCoinDataCollection.icc"

}

#endif // MUONTRIGCOINDATA_MUONCOINDATACOLLECTION_H

