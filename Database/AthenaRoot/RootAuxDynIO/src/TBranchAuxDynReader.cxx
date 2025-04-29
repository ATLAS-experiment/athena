/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/exceptions.h"
#include "AthContainersRoot/getDynamicAuxID.h"
#include "RootUtils/Type.h"

#include "TBranchAuxDynReader.h"
#include "TBranchAuxDynStore.h"
#include "RootAuxDynIO.h"

#include "TTree.h"
#include "TBranch.h"
#include "TClass.h"
#include "TClassTable.h"
#include "TClassEdit.h"
#include "TVirtualCollectionProxy.h"
#include "TROOT.h"
#include "TDictAttributeMap.h"

using std::string;

namespace {

const std::type_info* dataTypeToTypeInfo (EDataType type, std::string& typeName)
{
   RootUtils::Type typ (type);
   typeName = typ.getTypeName();
   return typ.getTypeInfo();
}


/**
 * @brief Given TClass+EDataType (as returned by a branch), return the aux data type
 * @param expectedClass  TClass for the storage type if a class
 * @param expectedType   EDT type for the storage type if a primitive type
 * @param standalone     Is this for a standalone object?
 * @param[out] elementTypeName Name of the type for one aux data element.
 *                             Same as storageTypeName if @c standalone is true.
 * @param[out] storageTypeName Name of the type used for I/O
 *
 * If standalone is true, then we return the extracted type
 * directly.  Otherwise, the type should be an STL vector;
 * we return the type of the vector's payload.
 *
 * Returns 0 on failure.
 */
const std::type_info*
getAuxElementType( TClass *expectedClass, EDataType expectedType, bool standalone,
                   std::string& elementTypeName, std::string& storageTypeName)
{
   if (standalone) {
      if(expectedClass) {
         elementTypeName = expectedClass->GetName();
         storageTypeName = elementTypeName;
         return expectedClass->GetTypeInfo();
      }
      const std::type_info* ret = dataTypeToTypeInfo(expectedType, elementTypeName);
      storageTypeName = elementTypeName;
      return ret;
   }

   // Not standalone; branch should be a vector.
   if (!expectedClass) return 0;

   storageTypeName = expectedClass->GetName();
   if (strncmp (expectedClass->GetName(), "vector<", 7) == 0) {
      TVirtualCollectionProxy* prox = expectedClass->GetCollectionProxy();
      if (!prox) return 0;
      if (prox->GetValueClass()) {
         elementTypeName = prox->GetValueClass()->GetName();
         return prox->GetValueClass()->GetTypeInfo();
      }
      return dataTypeToTypeInfo (prox->GetType(), elementTypeName);
   }
   else if (strncmp (expectedClass->GetName(), "SG::PackedContainer<", 20) == 0){
      elementTypeName.clear();
      {
        // Protect against data race inside TClassEdit.
        // https://github.com/root-project/root/issues/10353
        // Should be fixed in root 6.26.02.
        R__WRITE_LOCKGUARD(ROOT::gCoreMutex);
        TClassEdit::TSplitType split (expectedClass->GetName());
        if (split.fElements.size() > 1) {
          elementTypeName = split.fElements[1];
        }
      }
      if (!elementTypeName.empty()) {
        RootUtils::Type typ (elementTypeName);
        return typ.getTypeInfo();
      }
   }
   return 0;
}


SG::auxid_t
getAuxIdForAttribute(const std::string& attr, TClass *tclass, EDataType edt, bool standalone,
                     SG::auxid_t linked_auxid)
{
   SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

   SG::auxid_t auxid = r.findAuxID(attr);
   if(auxid != SG::null_auxid)
      return auxid;
   
   string elemen_type_name;
   string branch_type_name;
   const std::type_info* ti = getAuxElementType(tclass, edt, standalone, elemen_type_name, branch_type_name);
   if( !ti )
      return auxid;

   return SG::getDynamicAuxID (*ti, attr, elemen_type_name, branch_type_name, standalone,
                               linked_auxid);
}


std::string
getKeyFromBranch(TBranch* branch)
{
   TClass *tc = nullptr;
   EDataType type;
   if( branch->GetExpectedType(tc, type) == 0  && tc ) {
      std::string key = branch->GetName();
      const std::string clname_pfx = std::string(tc->GetName()) + '_';
      if( key.starts_with( clname_pfx ) ) {
         key.erase(0, clname_pfx.size());
      }
      RootAuxDynIO::removeAuxPostfix(key);
      return key;
   }
   return "";
}

} // anonymous namespace


void TBranchAuxDynReader::BranchInfo::setAddress(void* data)
{
   if( needsSE ) {
      if( (edtyp == kULong_t or edtyp == kULong64_t or edtyp == kLong_t or edtyp == kLong64_t) and
          (SE_edt == kULong_t or SE_edt == kULong64_t or SE_edt == kLong_t or SE_edt == kLong64_t) and
          sizeof(Long_t) == sizeof(Long64_t) ) {
         // There is no need to attempt ROOT schema evolution between these types (and it will not work anyhow)
         needsSE = false;
      }
   }
   if( needsSE ) {
      // reading through the TTree - allows for schema evolution
      int rc = branch->GetTree()->SetBranchAddress( branch->GetName(), data, SE_tclass, SE_edt, true);
      if( rc < 0 ) {
         std::ostringstream msg;
         msg << "SetBranchAddress() failed for " << branch->GetName() << "  error=" << rc;
         throw msg.str();
      }
   } else {
      branch->SetAddress(data);
   }
}


// -----------------    TBranchAuxDynReader   ---------------------------------------

// Fix Reader for a specific tree and branch base name.
// Find all dynamic attribute branches that share the base name
TBranchAuxDynReader::TBranchAuxDynReader(TTree *tree, TBranch *base_branch)
   : m_baseBranchName( base_branch->GetName() ),
     m_key( getKeyFromBranch(base_branch) ),
     m_tree( tree )
{
   // The Branch here is the object (AuxContainer) branch, not the attribute branch
   TClass *tc = nullptr, *storeTC = nullptr;
   EDataType type;
   base_branch->GetExpectedType(tc, type);    //MN: Errors would be coaught in isAuxDynBranch() earlier
   if( tc ) storeTC = tc->GetBaseClass("SG::IAuxStoreHolder");
   if( storeTC ) m_storeHolderOffset = tc->GetBaseClassOffset( storeTC );
   if( m_storeHolderOffset < 0 ) {
      const std::string name = (tc? tc->GetName() : m_baseBranchName);
      errorcheck::ReportMessage msg (MSG::INFO, ERRORCHECK_ARGS, "TBranchAuxDynReader");
      msg << "IAuxStoreHolder interface not found in " << name << " - will not read dynamic attributes";
   }
   
   const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
   string branch_prefix = RootAuxDynIO::auxBranchName("", m_baseBranchName);
   TObjArray *all_branches = m_tree->GetListOfBranches();
   for( int i=0; i<all_branches->GetEntriesFast(); i++ ) {
      const char *bname =  (*all_branches)[i]->GetName();
      if( strncmp(bname, branch_prefix.c_str(), branch_prefix.size()) == 0 ) {
         const string attr_inFile  = bname+branch_prefix.size();
         const string attr = r.inputRename (m_key, attr_inFile);
         m_branchMap[attr] = (TBranch*)(*all_branches)[i];
      }
   }
}


SG::auxid_t TBranchAuxDynReader::initBranch(bool standalone,
                                            const std::string& attr,
                                            TBranch* branch)
{
  TClass*       expectedClass = 0;
  EDataType     expectedType = kOther_t;
  if( branch->GetExpectedType(expectedClass, expectedType) != 0) {
    // raise hell
  }

  SG::auxid_t linked_auxid = SG::null_auxid;
  if (expectedClass) {
    std::string className = expectedClass->GetName();
    if (SG::AuxTypeRegistry::classNameHasLink (className)) {
      std::string linkedAttr = SG::AuxTypeRegistry::linkedName (attr);
      auto it = m_branchMap.find (linkedAttr);
      if (it != m_branchMap.end()) {
        linked_auxid = initBranch (standalone, linkedAttr, it->second);
      }
      if (linked_auxid == SG::null_auxid) {
        errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "TBranchAuxDynReader::initBranch");
        msg << "Could not find linked variable  for " << branch->GetName()
            << " type: " << expectedClass->GetName();
      }
    }
  }

  SG::auxid_t auxid = getAuxIdForAttribute(attr, expectedClass, expectedType, standalone,
                                           linked_auxid);

  // add AuxID to the list
  // May still be null if we don't have a dictionary for the branch.
  if (auxid != SG::null_auxid) {
     addAuxID(auxid);
  } else {
    errorcheck::ReportMessage msg (MSG::WARNING, ERRORCHECK_ARGS, "TBranchAuxDynReader::initBranch");
    msg << "Could not find auxid for " << branch->GetName()
        << " type: " << expectedClass->GetName();

  }
  return auxid;
}

// Has to be a separate method because 'standalone' status is not know at construction time
// Prepare all branch infos for dynamic attributes (auxids and types)
void TBranchAuxDynReader::init(bool standalone)
{
   if( m_initialized )  return;
   
   for( const auto& attr2branch: m_branchMap ) {
      initBranch (standalone, attr2branch.first, attr2branch.second);
   }
   m_initialized = true;
}
  
// Called by the AuxStore when it is reading new attribute data from the file
// All information is cached in a BranchInfo object for better performance
TBranchAuxDynReader::BranchInfo&
TBranchAuxDynReader::getBranchInfo(const SG::auxid_t& auxid, const SG::AuxStoreInternal& store)
{
   BranchInfo& brInfo = m_branchInfos[auxid];
   if( brInfo.status == BranchInfo::NotInitialized )
   {
      SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
      brInfo.auxid = auxid;
      brInfo.attribName = r.getName(auxid);
      // Don't match this attribute if it's been renamed.
      // For example, suppose we've renamed attribute `foo' to `foo_old',
      // and someone then comes and asks for `foo'.
      // `foo' will not be found in the m_branchMap test below
      // (`foo_old' will be in this map).  However, in the following
      // else clause, we'll recreate the branch name from `foo'.
      // This branch exists (renaming is only in the transient store),
      // so if we didn't have the condition here, then we'd then
      // make a `foo' attribute from that branch.
      if (r.inputRename (m_key, brInfo.attribName) != brInfo.attribName) {
        brInfo.status = BranchInfo::NotFound;
        return brInfo;
      }

      auto it = m_branchMap.find (brInfo.attribName);
      if (it != m_branchMap.end()) {
        brInfo.branch = it->second;
      }
      else {
        const string aux_branch_name = RootAuxDynIO::auxBranchName(brInfo.attribName, m_baseBranchName);
        brInfo.branch = m_tree->GetBranch( aux_branch_name.c_str() );
      }
      
      // mark initialized here so it remembers this branch was not found
      if( !brInfo.branch ) {
         brInfo.status = BranchInfo::NotFound;
         return brInfo;
      }
      if( brInfo.branch->GetExpectedType( brInfo.tclass, brInfo.edtyp) ) {
         brInfo.status = BranchInfo::TypeError;
         throw string("Error getting branch type for ") + brInfo.branch->GetName();
      }
   
      if( !store.standalone() )
         if( brInfo.tclass && strncmp( brInfo.tclass->GetName(), "SG::PackedContainer<", 20) == 0)
            brInfo.isPackedContainer = true;

      string elem_tname, branch_tname;
      // AuxElement TypeID
      const std::type_info* ti = getAuxElementType( brInfo.tclass, brInfo.edtyp, store.standalone(),
                                                    elem_tname, branch_tname );
      const std::type_info* reg_ti = r.getType(auxid);
      // I/O / Storage TypeID
      const std::type_info *io_tinf =  store.getIOType(auxid);
      const std::type_info *tcls_tinf = brInfo.tclass ? brInfo.tclass->GetTypeInfo() : ti;
      
      if (not io_tinf){
        brInfo.status = BranchInfo::TypeError;
        throw string("Error getting IO type for AUX branch ") + brInfo.branch->GetName();
      }
      // if there is a TClass compare the whole storage types (usually vectors), because the Element type
      // returned by CollProxy loses the pointer component and element type comparison for vector<T*> fails
      brInfo.needsSE = brInfo.tclass ?
          io_tinf != tcls_tinf && (!tcls_tinf || strcmp(io_tinf->name(), tcls_tinf->name()) != 0)
         : ti && ti != reg_ti && strcmp(ti->name(), reg_ti->name()) != 0;
      if( brInfo.needsSE ) {
         // type in registry is different than type in the file.
         // will need to use ROOT auto schema evolution 
         errorcheck::ReportMessage msg (MSG::INFO, ERRORCHECK_ARGS, "TBranchAuxDynReader");
         msg << "attribute '" << brInfo.attribName << "' (id=" << auxid
             << " typename=" << SG::normalizedTypeinfoName(*reg_ti)
             << ") has different type than the branch: " << branch_tname;
         msg << "  Marking for schema evolution.";
         
         brInfo.SE_tclass  = TClass::GetClass(*io_tinf);
         brInfo.SE_edt = kOther_t;
         if( !brInfo.SE_tclass  ) {
            brInfo.SE_edt = TDataType::GetType(*io_tinf);
            if( brInfo.SE_edt <=0 ) {
               brInfo.status = BranchInfo::TypeError;
               throw string("Error getting ROOT type for AUX branch ") + brInfo.branch->GetName()
                  + " typeinfo=" + io_tinf->name();
            }
         }
      }       
      brInfo.status = BranchInfo::Initialized;
   }
   return brInfo;
}


void TBranchAuxDynReader::addReaderToObject(void* object, size_t ttree_row, std::recursive_mutex* iomtx)
{
   if( m_storeHolderOffset >= 0 ) {
      auto store_holder = reinterpret_cast<SG::IAuxStoreHolder*>((char*)object + m_storeHolderOffset);
      bool standalone { store_holder->getStoreType()==SG::IAuxStoreHolder::AST_ObjectStore };
      if( !m_initialized )
         init(standalone);
      store_holder->setStore( new TBranchAuxDynStore(*this, ttree_row, standalone, iomtx) );
   }
}
