/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNWRITER_H
#define RNTUPLEAUXDYNWRITER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "RootAuxDynIO/RootAuxDynIO.h"

namespace SG {
class IAuxStoreIO;
}

namespace RootAuxDynIO {

class RNTupleAuxDynWriter : public AthMessaging, public IRNTupleAuxDynWriter {
 public:
  /// Default Constructor
  RNTupleAuxDynWriter();

  /// Default Destructor
  virtual ~RNTupleAuxDynWriter() = default;

  /// Collect Aux data information to be written out
  virtual std::vector<attrDataTuple> collectAuxAttributes(
      const std::string& base_branch, SG::IAuxStoreIO* store) override final;
};

}  // namespace RootAuxDynIO
#endif
