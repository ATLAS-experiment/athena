/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"
#include "RootUtils/Type.h"

#include "RNTupleAuxDynReader.h"
#include "RNTupleAuxDynStore.h"
#include "AthContainersRoot/getDynamicAuxID.h"

#include "TClass.h"
#include "TClassEdit.h"
#include "TVirtualCollectionProxy.h"
#include "TROOT.h"

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "ROOT/RField.hxx"

using std::string;

namespace {

/**
 * @brief Given RNTuple field typename, return the aux data type
 * @param standalone     Is this for a standalone object?
 * @param[out] elementTypeName Name of the type for one aux data element.
 *                             Same as storageTypeName if @c standalone is true.
 * @param      storageTypeName Name of the type used for I/O
 *
 * If standalone is true, then we return the extracted type
 * directly.  Otherwise, the type should be an STL vector;
 * we return the type of the vector's payload.
 *
 * Returns 0 on failure.
 */
const std::type_info*
getAuxElementType( bool standalone, std::string& elementTypeName, const std::string& storageTypeName)
{
   if (standalone) {
      elementTypeName = storageTypeName;
      // Need to stay with 'long long' for [mc]eventNumber as defined in the EventInfo class
      if( elementTypeName == "std::uint64_t") return &typeid(unsigned long long);
      return RootUtils::Type(storageTypeName).getTypeInfo();
   }

   // Not standalone - type should be a vector.
   if( storageTypeName.rfind("vector<", 0) == 0
       || storageTypeName.rfind("std::vector<", 0) == 0 ) {
      const TClass *tclass = RootUtils::Type(storageTypeName).getClass();
      if( !tclass ) return nullptr;
      TVirtualCollectionProxy* proxy = tclass->GetCollectionProxy(); 
      if( !proxy ) return nullptr;
      if( proxy->GetValueClass() ) {
         elementTypeName = proxy->GetValueClass()->GetName();
         return proxy->GetValueClass()->GetTypeInfo();
      }
      RootUtils::Type elemtype( proxy->GetType() );
      elementTypeName = elemtype.getTypeName();
      return elemtype.getTypeInfo();
   }
   else
      if( storageTypeName.rfind("SG::PackedContainer<", 0) == 0) {
         elementTypeName.clear();
         {
            // Protect against data race inside TClassEdit.
            // https://github.com/root-project/root/issues/10353
            // Should be fixed in root 6.26.02.
            R__WRITE_LOCKGUARD(ROOT::gCoreMutex);
            TClassEdit::TSplitType split( storageTypeName.c_str() );
            if( split.fElements.size() > 1 ) {
               elementTypeName = split.fElements[1];
            }
         }
         if( !elementTypeName.empty() ) {
            return RootUtils::Type(elementTypeName).getTypeInfo();
         }
      }
   return 0;
}


SG::auxid_t
getAuxIdForAttribute(const SG::AuxTypeRegistry& r,
                     const ROOT::Experimental::RNTupleDescriptor& desc,
                     const std::string& field_prefix,
                     const std::string& attr_name,
                     const std::string& attr_type,
                     bool standalone);


SG::auxid_t
getLinkedAuxId (const SG::AuxTypeRegistry& r,
                const ROOT::Experimental::RNTupleDescriptor& desc,
                const std::string& field_prefix,
                const std::string& attr_name,
                const std::string& attr_type,
                bool standalone)
{
  SG::auxid_t linked_auxid = SG::null_auxid;
  if (SG::AuxTypeRegistry::classNameHasLink (attr_type)) {
    using ROOT::Experimental::DescriptorId_t;
    using ROOT::Experimental::RFieldDescriptor;
    using ROOT::Experimental::RNTupleDescriptor;
    using ROOT::Experimental::kInvalidDescriptorId;
    std::string linked_attr = SG::AuxTypeRegistry::linkedName (attr_name);
    DescriptorId_t did = desc.FindFieldId (field_prefix + linked_attr);
    if (did != kInvalidDescriptorId) {
      const RFieldDescriptor& linked_f = desc.GetFieldDescriptor (did);
      linked_auxid = getAuxIdForAttribute (r, desc, field_prefix,
                                           linked_attr, linked_f.GetTypeName(), standalone);
    }
    if (linked_auxid == SG::null_auxid) {
      errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "RNTupleAuxDynReader.cxx:getLinkedAuxId");
      msg << "Could not find linked variable  for " << attr_name
          << " type: " << attr_type;
    }
  }
  return linked_auxid;
}


SG::auxid_t
getAuxIdForAttribute(const SG::AuxTypeRegistry& r,
                     const ROOT::Experimental::RNTupleDescriptor& desc,
                     const std::string& field_prefix,
                     const std::string& attr_name,
                     const std::string& attr_type,
                     bool standalone)
{
   SG::auxid_t auxid = r.findAuxID(attr_name);
   if(auxid != SG::null_auxid)
      return auxid;
   
   string element_type;
   const std::type_info* ti = getAuxElementType(standalone, element_type, attr_type);
   if( !ti )
      return auxid;

   SG::auxid_t linked_auxid = getLinkedAuxId (r, desc, field_prefix, attr_name, attr_type, standalone);

   return SG::getDynamicAuxID (*ti, attr_name, element_type, attr_type, standalone, linked_auxid);
}

} // anonymous namespace


namespace RootAuxDynIO
{
   // New RNTupleReader for dynamic attributes of an AuxContainer-type object
   // stored in the Field 'field_name'
   RNTupleAuxDynReader::RNTupleAuxDynReader(const std::string& field_name,
                                            const std::string& field_type,
                                            RNTupleReader* reader)
      : AthMessaging( std::string("RNTupleAuxDynReader[")+field_name+"]" ),
        m_storeFieldName( field_name ),
        m_ntupleReader( reader )
   {
      const std::string field_prefix = field_type + "_";
      if( m_storeFieldName.rfind( field_type, 0 ) != std::string::npos ) {
         m_key = m_storeFieldName.substr( field_type.size()+1 );
      }
      else if( m_storeFieldName.ends_with( "Aux:" ) ) {
        m_key = m_storeFieldName.substr( 0, m_storeFieldName.size() - 4 );
      }
      ATH_MSG_VERBOSE("field name=" << m_storeFieldName << "  field_prefix=" << field_prefix << "  key=" << m_key);
      TClass *tc = TClass::GetClass( field_type.c_str() );
      if( tc ) {
         TClass *storeTC = tc->GetBaseClass("SG::IAuxStoreHolder");
         if( storeTC ) {
            m_storeHolderOffset = tc->GetBaseClassOffset( storeTC );
         } else {
            throw std::runtime_error(string("Class ") + tc->GetName() +" does not implement SG::IAuxStoreHolder");
         }
      } else {
         throw std::runtime_error(string("Class ") + field_type +" could not be found");
      }
   }


// Has to be a separate method because 'standalone' status is not know at construction time
// Prepare all Field infos for dynamic attributes (auxids and types)
   void RNTupleAuxDynReader::init( bool standalone )
   {
      if( m_initialized )  return;
   
      const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
      const string field_prefix = RootAuxDynIO::auxFieldName("", m_storeFieldName);
      const auto& desc = m_ntupleReader->GetDescriptor();
      for( const auto &f : desc.GetTopLevelFields() ) {
         const string field_name = f.GetFieldName();
         if( field_name.rfind(field_prefix,0) == 0 ) {
            const string attr_infile = field_name.substr(field_prefix.size());
            const string attr_name = reg.inputRename(m_key, attr_infile);
            const string field_type = f.GetTypeName();

            SG::auxid_t auxid = getAuxIdForAttribute(reg, desc, field_prefix,
                                                     attr_name, field_type, standalone);
            // add AuxID to the list
            // May still be null if we don't have a dictionary for this field
            if( auxid != SG::null_auxid ) {
               m_auxids.insert(auxid);
               m_fieldInfos[auxid].fieldName = field_name;
               m_fieldInfos[auxid].view = m_ntupleReader->GetView<void>(field_name, nullptr);
            } else {
               errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "RNTupleAuxDynReader::init");
               msg << "Could not find auxid for " << attr_infile << " type: " << field_type
                   << "  standalone=" << standalone;
            }
         } 
      }
      m_initialized = true;
   }
  
// Called by the AuxStore when it is reading new attribute data from the file
// All information is cached in a FieldInfo object for better performance
   RNTupleAuxDynReader::FieldInfo&
   RNTupleAuxDynReader::getFieldInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store)
   {
      FieldInfo& fieldInfo = m_fieldInfos[auxid];
      if( fieldInfo.status == FieldInfo::NotInitialized )
      {
         SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
         fieldInfo.auxid = auxid;
         fieldInfo.attribName = reg.getName(auxid);
         // Don't match this attribute if it's been renamed.
         // For example, suppose we've renamed attribute `foo' to `foo_old',
         // and someone then comes and asks for `foo'.
         // `foo' will not be found in the m_fieldMap test below
         // (`foo_old' will be in this map).  However, in the following
         // else clause, we'll recreate the field name from `foo'.
         // This field exists (renaming is only in the transient store),
         // so if we didn't have the condition here, then we'd then
         // make a `foo' attribute from that field.
         if( reg.inputRename(m_key, fieldInfo.attribName) != fieldInfo.attribName ) {
            fieldInfo.status = FieldInfo::NotFound;
            return fieldInfo;
         }

         if( fieldInfo.fieldName.empty() ) {
            // mark initialized here so it remembers this field was not found
            fieldInfo.status = FieldInfo::NotFound;
            return fieldInfo;
         }

         if( !store.standalone() and fieldInfo.fieldName.rfind("SG::PackedContainer<", 0) == 0 )
            fieldInfo.isPackedContainer = true;

         /*
           string elem_tname, branch_tname;
           const type_info* ti = getAuxElementType( fieldInfo.tclass, typ, store.standalone(),
           elem_tname, branch_tname );
           const type_info* reg_ti = reg.getType(auxid);
           if( ti && ti != reg_ti && strcmp(ti->name(), reg_ti->name()) != 0 )
           {
           // type in registry is different than type in the file.
           // will need to use ROOT auto schema evolution
           fieldInfo.needsSE = true;
           errorcheck::ReportMessage msg (MSG::INFO, ERRORCHECK_ARGS, "RNTupleAuxDynReader");
           msg << "attribute " << fieldInfo.attribName << " (id=" << auxid <<
           " typename=" << SG::AuxTypeRegistry::instance().getType(auxid)->name()
           << ") has different type than the branch " << branch_tname;

           const std::type_info *tinf =  store.getIOType(auxid);
           fieldInfo.SE_tclass  = TClass::GetClass(*tinf);
           fieldInfo.SE_edt = kOther_t;
           if( !fieldInfo.SE_tclass  ) {
           fieldInfo.SE_edt = TDataType::GetType(*tinf);
           if( fieldInfo.SE_edt <=0 ) {
           fieldInfo.status = FieldInfo::TypeError;
           throw string("Error getting ROOT type for AUX branch ") + fieldInfo.branch->GetName()
           + " typeinfo=" + tinf->name();
           }
           }
           }
         */
         fieldInfo.status = FieldInfo::Initialized;
      }
      return fieldInfo;
   }


   void RNTupleAuxDynReader::addReaderToObject(void* object, size_t row, std::recursive_mutex* iomtx)
   {
      auto store_holder = reinterpret_cast<SG::IAuxStoreHolder*>((char*)object + m_storeHolderOffset);
      bool standalone { store_holder->getStoreType()==SG::IAuxStoreHolder::AST_ObjectStore };
      if( !m_initialized )
         init(standalone);
      store_holder->setStore( new RNTupleAuxDynStore(*this, row, standalone, iomtx) );
   }

}// namespace
