/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBRANCHAUXDYNWRITER_H
#define TBRANCHAUXDYNWRITER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthContainers/AuxStoreInternal.h"
#include "RootAuxDynIO/IRootAuxDynIO.h"
#include "RootAuxDynIO.h"

// Forward declarations
class TFile;
class TTree;
class TBranch;
class TClass;

namespace RootAuxDynIO
{
   
   struct AuxInfo {
      std::string          name;
      std::string          type_name;
      const std::type_info*typeinfo        = nullptr;
      std::string          branch_name;
      TBranch*             branch          = nullptr;
      TClass*              tclass          = nullptr;
      bool                 is_basic_type   = false;
      void*                object          = nullptr;
      void*                buffer          = nullptr;
      bool                 written         = false;
      size_t               rows_written    = 0;

      // Dummy object instance; used when there was no request to write
      // this branch but we need to write it anyway (for example,
      // a dynamic variable that wasn't written on this event).
      using dummy_ptr_t = std::unique_ptr<void, std::function<void(void*)> >;
      dummy_ptr_t          dummyptr;
      void*                dummy           = 0;

      void*     setDummyAddr();

      // get the right pointer to use with  branch.setAddress() (different for objects and basic types)
      void*     objectAddr() { return is_basic_type? object : &object; }

      ~AuxInfo();
   };


   class TBranchAuxDynWriter : public AthMessaging, public IRootAuxDynWriter, public AuxDynAttrAccess
   {
   public:
      TBranchAuxDynWriter( TTree& tree, TClass& cls, int bufferSize, int splitLevel, int offsettab_len, bool branch_fill );
      virtual ~TBranchAuxDynWriter() { }
      
      /// set Filling mode (true/false) for branch containers
      virtual void        setBranchFillMode(bool mode) override final { m_branchFillMode = mode; }

      //  throws exceptions
      void createAuxBranch( AuxInfo& info );

      void setBranchOffsetTabLen(TBranch* b, int offsettab_len);

      /// handle writing of dynamic xAOD attributes of an object
      /// called from RootTreeContainer::writeObject()
      //  throws exceptions
      virtual int writeAuxAttributes( const std::string& base_branchname,
                                      void *store,
                                      size_t backfill_nrows )  override final;

      virtual bool needsCommit() override final  { return m_needsFill; }

      virtual int commit() override final;

   protected:
      TFile*               m_tfile            = nullptr;
      TTree*               m_ttree            = nullptr;
      int                  m_bufferSize       = 8192;
      int                  m_splitLevel       = 1;
      int                  m_branchOffsetTabLen = 0;
      bool                 m_branchFillMode   = false;
      bool                 m_needsFill        = false;

      /// cached aux branches data by auxid
      std::map<SG::auxid_t, AuxInfo>   m_auxInfoMap;
   };

} // namespace
#endif
