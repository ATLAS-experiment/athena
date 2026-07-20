/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"
#include "AthContainersInterfaces/AuxDataOption.h"

#include "TBranchAuxDynStore.h"
#include "TBranchAuxDynReader.h"

#include "TTree.h"


TBranchAuxDynStore::TBranchAuxDynStore(TBranchAuxDynReader& reader, long long entry, bool standalone,
                                 std::recursive_mutex* iomtx)
   : RootAuxDynStore( reader, entry, standalone, iomtx ),
     m_reader(reader)
{
}


bool TBranchAuxDynStore::readData(SG::auxid_t auxid)
{
   try {
      auto& brInfo = m_reader.getBranchInfo(auxid, *this);
      if( !brInfo.branch ) return false; 

      // Make a 1-element vector.
      SG::AuxStoreInternal::getDataInternal(auxid, 1, 1, true);
      if( brInfo.isPackedContainer ) {
         setOption (auxid, SG::AuxDataOption ("nbits", 32));
      }

      // get memory location where to write data from the branch entry
      // const_cast because TTree::SetBranchAddress requires void*
      void *       vector ATLAS_THREAD_SAFE = const_cast<void*>(SG::AuxStoreInternal::getIOData (auxid));
      void *       data = &vector;
      if( standalone() && !brInfo.tclass ) {
         // reading fundamental type - ROOT expects a direct pointer
         data = vector;
      }
   
      // if have mutex, lock to prevent potential concurrent I/O from elsewhere
      auto io_lock = m_iomutex? std::unique_lock<std::recursive_mutex>(*m_iomutex)
         : std::unique_lock<std::recursive_mutex>();
      // read branch
      brInfo.setAddress(data);
      int  nbytes = brInfo.branch->GetEntry(m_entry);
      if( nbytes <= 0 )
         throw std::string("Error reading branch ") + brInfo.branch->GetName();
      // read OK
      m_reader.addBytes(nbytes);
      TTree::TClusterIterator clusterIterator = brInfo.branch->GetTree()->GetClusterIterator(m_entry);
      clusterIterator.Next();
      if (m_entry == clusterIterator.GetStartEntry() && brInfo.branch->GetTree()->GetMaxVirtualSize() != 0) {
         for (int i = brInfo.branch->GetReadBasket(); i < brInfo.branch->GetMaxBaskets()
	         && brInfo.branch->GetBasketEntry()[i] < clusterIterator.GetNextEntry(); i++) {
            brInfo.branch->GetBasket(i);
         }
      }
   }
   catch(const std::string& e_str) {
      ATHCONTAINERS_ERROR("TBranchAuxDynStore::getData", e_str);
      return false;
   }

   SG::AuxStoreInternal::toTransient (m_ctx, auxid);
   SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
   SG::auxid_t linked_auxid = r.linkedVariable (auxid);
   if (linked_auxid != SG::null_auxid) {
     return readData (linked_auxid);
   }
   return true;
}
