/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"

#include "DataModelRoot/RootType.h"
#include "RootAuxDynIO.h"
#include "RNTupleAuxDynReader.h"
#include "RNTupleAuxDynWriter.h"
#include "TBranchAuxDynReader.h"
#include "TBranchAuxDynWriter.h"

#include "TBranch.h"
#include "TClass.h"
#include "TROOT.h"

#include <ROOT/RNTuple.hxx>

namespace RootAuxDynIO
{

   AuxDynAttrAccess::AuxDynAttrAccess(TClass& tc)
      : m_holderType( tc ),
        m_ioStoreOffset( auxStoreOffset(tc) )
   { }

   int AuxDynAttrAccess::auxStoreOffset(TClass &tc)
   {
      TClass *storeTClass = tc.GetBaseClass("SG::IAuxStoreIO");
      if( storeTClass ) {
         // This is a class implementing SG::IAuxStoreIO
         // Find IAuxStoreIO interface offset
         return tc.GetBaseClassOffset( storeTClass );
      }
      return -1;
   }

   bool AuxDynAttrAccess::hasAuxDynStore() const
   {
      return m_ioStoreOffset >= 0;
   }

   SG::IAuxStoreIO* AuxDynAttrAccess::castIOStore(void *object) {
      return ( hasAuxDynStore()?
               reinterpret_cast<SG::IAuxStoreIO*>( (char*)object + m_ioStoreOffset )
               : nullptr);
   }


   //  ---------------------  Dynamic Aux Attribute Writers

   bool
   FactoryTool::hasAuxStore(std::string_view fieldname, TClass *tc) const
   {
      // check the name first, and only if it does not match AUX_POSTFIX ask TClass
      return endsWithAuxPostfix(fieldname)
         or ( tc and ( tc->GetBaseClass("SG::IAuxStore")
                       // the IAuxStore property is used in DataModelTests
                       or RootType(tc).Properties().HasProperty("IAuxStore") ));
   }


   bool
   FactoryTool::hasAuxStoreIO(TClass *tc) const
   {
      return tc and tc->GetBaseClass("SG::IAuxStoreIO");
   }


   bool
   FactoryTool::isAuxDynBranch(TBranch *branch) const
   {
      const std::string bname = branch->GetName();
      TClass *tc = 0;
      EDataType type;
      if( branch->GetExpectedType(tc, type) ) {
         // error - not expecting this to happen ever, but better report
         errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "RootAuxDynIO::isAuxDynBranch");
         msg << "GetExpectedType() failed for branch: " << bname;
         return false;
      }
      if( hasAuxStore(bname, tc) ) {
          return tc->GetBaseClass("SG::IAuxStoreHolder") != nullptr;
      }
      return false;
   }

   //  ---------------------  Dynamic Aux Attribute Writers

   /// generate TBranchAuxDynWriter
   /// tree -> destination tree
   /// do_branch_fill -> flag telling to Fill each TBranch immediately
   std::unique_ptr<RootAuxDynIO::IRootAuxDynWriter>
   FactoryTool::getBranchAuxDynWriter(TTree& tree, TClass& cl, int bufferSize, int splitLevel,
                                      int offsettab_len,  bool do_branch_fill) const {
      return std::make_unique<TBranchAuxDynWriter>(tree, cl, bufferSize, splitLevel,
                                                   offsettab_len, do_branch_fill);
   }

   std::unique_ptr<RootAuxDynIO::IRNTupleAuxDynWriter>
   FactoryTool::getNTupleAuxDynWriter(TClass &tc) const {
      return std::make_unique<RNTupleAuxDynWriter>(tc);
   }


   //  ---------------------  Dynamic Aux Attribute Readers

   std::unique_ptr<IRootAuxDynReader>
   FactoryTool::getBranchAuxDynReader(TTree* tree, TBranch* branch) const {
      return std::make_unique<TBranchAuxDynReader>(tree, branch);
   }

   std::unique_ptr<IRootAuxDynReader>
   FactoryTool::getNTupleAuxDynReader(const std::string& field_name, const std::string& field_type,
                                      ROOT::RNTupleReader* reader) const {
      return std::make_unique<RNTupleAuxDynReader>(field_name, field_type, reader);
   }

}

// declare the component provided by this library (by type)
DECLARE_COMPONENT(RootAuxDynIO::FactoryTool)
