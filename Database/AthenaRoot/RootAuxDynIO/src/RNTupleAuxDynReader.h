/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEAUXDYNREADER_H
#define RNTUPLEAUXDYNREADER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynReader.h"

#include "ROOT/RNTupleView.hxx"

#include <map>
#include <optional>
#include <string>

class TClass;

namespace RootAuxDynIO
{
   class RNTupleAuxDynReader : public AthMessaging, public RootAuxDynReader
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
         std::string   fieldName;
         std::optional< ROOT::RNTupleView<void> > view;
      };


      /// create Reader for Aux attributes of an AuxContainer object stored in a given field
      RNTupleAuxDynReader(const std::string& field_name, const std::string& field_type, ROOT::RNTupleReader* reader);

      /// initialize once the mode of the Aux store is known
      void init(bool standalone);

      /// attach RNTupleAuxStore to the current Aux container @object
      virtual void addReaderToObject(void* object, size_t row, std::recursive_mutex* iomtx = nullptr ) override final;

      /// get field informatino for @c auxid
      FieldInfo& getFieldInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store);

      ROOT::RNTupleReader* getNativeReader();

      virtual ~RNTupleAuxDynReader() = default;

   protected:
      std::string                       m_storeFieldName;
      // offset of the AxuStoreHolder base class in the objects read by the Reader
      int                               m_storeHolderOffset = -1;
      bool                              m_initialized = false;
      std::string                       m_key;

      // map auxid -> fieldInfo.
      std::map<SG::auxid_t, FieldInfo>  m_fieldInfos;

      // not owned
      ROOT::RNTupleReader*                    m_ntupleReader;
   };


   inline ROOT::RNTupleReader* RNTupleAuxDynReader::getNativeReader() {
      return m_ntupleReader;
   }

} //namespace
#endif

