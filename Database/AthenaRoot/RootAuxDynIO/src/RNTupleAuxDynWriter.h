/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNWRITER_H
#define RNTUPLEAUXDYNWRITER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "RootAuxDynIO/IRootAuxDynIO.h"
#include "RootAuxDynIO.h"


namespace SG {
class IAuxStoreIO;
}

namespace RootAuxDynIO {

   class  RNTupleAuxDynWriter : public AthMessaging, public IRNTupleAuxDynWriter, public AuxDynAttrAccess {
   
public:
   /// Default Constructor
   explicit RNTupleAuxDynWriter(TClass& tc);
   
   /// Default Destructor
   virtual ~RNTupleAuxDynWriter() = default;
   
   /// Collect Aux data information to be written out
   virtual std::vector<attrDataTuple> collectAuxAttributes(
      const std::string& base_branch, void* object) override final;
};

}  // namespace RootAuxDynIO
#endif
