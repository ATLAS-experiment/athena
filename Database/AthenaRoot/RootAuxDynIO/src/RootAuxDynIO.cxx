/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"

#include "DataModelRoot/RootType.h"
#include "RootAuxDynIO/RootAuxDynIO.h"
#include "RNTupleAuxDynReader.h"
#include "RNTupleAuxDynWriter.h"
#include "TBranchAuxDynReader.h"
#include "TBranchAuxDynWriter.h"

#include "TFile.h"
#include "TBranch.h"
#include "TClass.h"
#include "TROOT.h"
#include "TDictAttributeMap.h"


#include <ROOT/RNTuple.hxx>
using ROOT::Experimental::RNTupleModel;
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
using ROOT::Experimental::Internal::RPageSource;
#else
using ROOT::Experimental::Detail::RPageSource;
#endif

namespace RootAuxDynIO
{

   bool
   hasAuxStore(std::string_view fieldname, TClass *tc) {
      // check the name first, and only if it does not match AUX_POSTFIX ask TClass
      return endsWithAuxPostfix(fieldname)
         or ( tc and ( tc->GetBaseClass("SG::IAuxStore")
                       // the IAuxStore property is used in DataModelTests
                       or RootType(tc).Properties().HasProperty("IAuxStore") ));
   }


   std::string
   getKeyFromBranch(TBranch* branch)
   {
      TClass *tc = 0;
      EDataType type;
      if( branch->GetExpectedType(tc, type) == 0  && tc != nullptr) {
         const char* brname = branch->GetName();
         const char* clname = tc->GetName();
         size_t namelen = strlen (clname);
         std::string key = brname;
         if( strncmp(brname, clname, namelen) == 0 && brname[namelen] == '_' ) {
            key.erase (0, namelen+1);
         }
         if( endsWithAuxPostfix(key) )  key.erase( key.size()-AUX_POSTFIX_LEN );
         return key;
      }
      return "";
   }


   std::string
   auxBranchName(const std::string& attr_name, const std::string& baseBranchName)
   {
      std::string branch_name = baseBranchName;
      if( !branch_name.empty() and branch_name.back() == '.' )  branch_name.pop_back();
      branch_name += RootAuxDynIO::AUXDYN_POSTFIX + attr_name;
      return branch_name;
   }

   std::string
   auxFieldName(const std::string& attr_name, const std::string& baseName)
   {
      std::string field_name = baseName;
      if( field_name.back() == '.' )  field_name.pop_back();
      field_name += ":" + attr_name;    // MN TODO <- find a good delimiter
      return field_name;
   }

   bool
   isAuxDynBranch(TBranch *branch)
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

   //  ---------------------  Dynamic Aux Attribute Readers

   std::unique_ptr<RootAuxDynIO::IRootAuxDynReader>
   getBranchAuxDynReader(TTree* tree, TBranch* branch) {
      return std::make_unique<TBranchAuxDynReader>(tree, branch);
   }

   std::unique_ptr<RootAuxDynIO::IRootAuxDynReader>
   getNTupleAuxDynReader(RFieldBase* field, RPageSource* source) {
      return std::make_unique<RNTupleAuxDynReader>(field, source);
   }

   //  ---------------------  Dynamic Aux Attribute Writers

   /// generate TBranchAuxDynWriter
   /// tree -> destination tree
   /// do_branch_fill -> flag telling to Fill each TBranch immediately
   std::unique_ptr<RootAuxDynIO::IRootAuxDynWriter>
   getBranchAuxDynWriter(TTree* tree, int bufferSize, int splitLevel, int offsettab_len,  bool do_branch_fill) {
      return std::make_unique<TBranchAuxDynWriter>(tree, bufferSize, splitLevel, offsettab_len, do_branch_fill);
   }

   std::unique_ptr<RootAuxDynIO::IRNTupleWriter>
   getNTupleAuxDynWriter(TFile* file, const std::string& ntupleName, bool enableBufferedWrite, bool enableMetrics) {
      return std::make_unique<RNTupleAuxDynWriter>(file, ntupleName, enableBufferedWrite, enableMetrics);
   }

}
