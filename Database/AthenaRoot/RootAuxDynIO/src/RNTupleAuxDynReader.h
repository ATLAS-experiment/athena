/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNREADER_H
#define RNTUPLEAUXDYNREADER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynIO/RootAuxDynIO.h" 

#include <map>
#include <string>

namespace RootAuxDynIO { class IRNTupleWriter; }
namespace ROOT { namespace Experimental {
#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 31, 0 )
  namespace Detail {
    class RPageSource;
    class RFieldBase;
  }
#else
  namespace Internal {
    class RPageSource;
  }
  class RFieldBase;
#endif
} }
class TClass;

namespace RootAuxDynIO
{
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
   using ROOT::Experimental::RFieldBase;
   using ROOT::Experimental::Internal::RPageSource;
#else
   using ROOT::Experimental::Detail::RFieldBase;
   using ROOT::Experimental::Detail::RPageSource;
#endif

   class RNTupleAuxDynReader : public AthMessaging, public IRootAuxDynReader
   {
   public :

      struct FieldInfo
      {
         enum Status { NotInitialized, Initialized, TypeError, NotFound };

         TClass*       tclass = 0;
         TClass*       SE_tclass = 0;
    
         bool          isPackedContainer = false;
         bool          needsSE = false;
         enum Status   status = NotInitialized;

         SG::auxid_t   auxid;
         std::string   attribName;
         std::unique_ptr<RFieldBase>  field;
      };


      /// create reader for Aux attributes of the Aux container object from @c field
      RNTupleAuxDynReader(RFieldBase* field, RPageSource* page_source);

      /// initialize once the mode of the Aux store is known
      void init(bool standalone);

      /// attach RNTupleAuxStore to the current Aux container @object
      virtual void addReaderToObject(void* object, size_t row, std::recursive_mutex* iomtx = nullptr ) override final;

      /// Aux IDs of all the Aux attributes belonging to the Aux container being read
      virtual const SG::auxid_set_t& auxIDs() const override final;

      void addBytes(size_t bytes);

      virtual size_t getBytesRead() const override final;

      virtual void resetBytesRead() override final;

      /// get field informatino for @c auxid
      const FieldInfo& getFieldInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store);

      virtual ~RNTupleAuxDynReader() {}

   protected:
      // auxids that could be found in registry for attribute names from the file
      SG::auxid_set_t                   m_auxids;
  
      std::string                       m_storeFieldName;
      // counter for bytes read
      size_t                            m_bytesRead = 0;
      // offset of the AxuStoreHolder base class in the objects read by the Reader
      int                               m_storeHolderOffset = -1;
      bool                              m_initialized = false;
      std::string                       m_key;

      // map auxid -> fieldInfo.
      std::map<SG::auxid_t, FieldInfo>  m_fieldInfos;

      // not owned
      RPageSource*                      m_pageSource;
   };



   inline void  RNTupleAuxDynReader::addBytes(size_t bytes) {
      m_bytesRead += bytes;
   }

   inline size_t RNTupleAuxDynReader::getBytesRead() const {
      return m_bytesRead;
   }

   inline void RNTupleAuxDynReader::resetBytesRead() {
      m_bytesRead = 0;
   }

   inline const SG::auxid_set_t& RNTupleAuxDynReader::auxIDs() const {
      return m_auxids;
   }

} //namespace
#endif
