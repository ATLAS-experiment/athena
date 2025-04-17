/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBRANCHAUXDYNREADER_H
#define TBRANCHAUXDYNREADER_H

#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynReader.h"

#include <map>
#include <string>

#include "TDataType.h"
class TTree;
class TClass;
class TBranch;

class TBranchAuxDynReader : public RootAuxDynReader
{
public :

   struct BranchInfo
   {
      enum Status { NotInitialized, Initialized, TypeError, NotFound };

      TBranch*      branch = 0;
      TClass*       tclass = 0;
      EDataType     edtyp  = kOther_t;

      // to handle type differences
      bool          needsSE   = false;
      TClass*       SE_tclass = 0;
      EDataType     SE_edt    = kOther_t;
    
      bool          isPackedContainer = false;
      enum Status   status = NotInitialized;

      SG::auxid_t   auxid;
      std::string   attribName;

      void setAddress(void* data);
   };

   TBranchAuxDynReader(TTree *tree, TBranch *base_branch);
  
   void init(bool standalone);

   virtual void addReaderToObject(void* object, size_t ttree_row, std::recursive_mutex* iomtx = nullptr ) override final;

   BranchInfo& getBranchInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store);

   virtual ~TBranchAuxDynReader() = default;

protected:
   std::string                           m_baseBranchName;
   // offset of the AxuStoreHolder base class in the objects read by the Reader
   int                                   m_storeHolderOffset = -1;
   bool                                  m_initialized = false;
   std::string                           m_key;

   TTree*                                m_tree = nullptr;
   // map of attribute name to TBranch* as read from the file
   std::map<std::string, TBranch*>       m_branchMap;
   // map auxid -> branch info. not sure if it can be different from m_branchMap
   std::map<SG::auxid_t, BranchInfo>     m_branchInfos;

private:
   SG::auxid_t initBranch (bool standalone, const std::string& attr, TBranch* branch);
};


#endif
