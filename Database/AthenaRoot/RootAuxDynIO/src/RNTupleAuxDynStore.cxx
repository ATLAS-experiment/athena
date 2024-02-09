/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"
#include "AthContainersInterfaces/AuxDataOption.h"

#include "RNTupleAuxDynStore.h"
#include "RNTupleAuxDynReader.h"

#include <ROOT/RNTuple.hxx>
#include <ROOT/RField.hxx>

using namespace RootAuxDynIO;

RNTupleAuxDynStore::RNTupleAuxDynStore( RNTupleAuxDynReader& aux_reader,
                                        long long entry, bool standalone,
                                        std::recursive_mutex* iomtx )
   : RootAuxDynStore( aux_reader, entry, standalone, iomtx ),
     m_reader( aux_reader )
{
}


bool RNTupleAuxDynStore::readData(SG::auxid_t auxid)
{
   try {
      auto& fieldInfo = m_reader.getFieldInfo(auxid, *this);
      if( fieldInfo.status == RNTupleAuxDynReader::FieldInfo::NotFound ) return false; 

      // Make a 1-element vector in the underlying AuxStore for the given auxid
      SG::AuxStoreInternal::getDataInternal(auxid, 1, 1, true);
      if( fieldInfo.isPackedContainer ) {
         setOption (auxid, SG::AuxDataOption ("nbits", 32));
      }
  
      // get memory location where to write data from the branch entry
      // const_cast because Field::CaptureValue() requires void*
      [[maybe_unused]] void* data ATLAS_THREAD_SAFE = const_cast<void*>(SG::AuxStoreInternal::getIOData(auxid));
      
      // if have mutex, lock to prevent potential concurrent I/O from elsewhere
      auto io_lock = m_iomutex? std::unique_lock<std::recursive_mutex>(*m_iomutex)
         : std::unique_lock<std::recursive_mutex>();

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
      auto rfv = fieldInfo.field->BindValue(std::shared_ptr<void>(data, [](void *) {}));
      rfv.Read(m_entry);
#endif

      int  nbytes = 1;   // MN: TODO how to get this?
      if( nbytes <= 0 ) {
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
         throw std::string("Error reading field ") + fieldInfo.field->GetFieldName();
#else
         throw std::string("Error reading field ") + fieldInfo.field->GetName();
#endif
      }
      // read OK
      m_reader.addBytes(nbytes);
   }
   catch(const std::string& e_str) {
      ATHCONTAINERS_ERROR("RNTupleAuxDynStore::getData", e_str);
      return false;
   }
   return true;
}
