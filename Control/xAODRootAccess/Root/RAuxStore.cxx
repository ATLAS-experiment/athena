// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/RAuxStore.h"

#include "isRegisteredType.h"
#include "lookupVectorType.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/TAuxVectorFactory.h"
#include "xAODRootAccess/tools/Utils.h"

// Athena include(s).
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"
#include "AthContainers/tools/AuxVectorInterface.h"
#include "CxxUtils/as_const_ptr.h"
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

// ROOT include(s).
#include <TClass.h>
#include <TROOT.h>

#include <ROOT/RNTupleInspector.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>
#include <ROOT/RNTupleWriter.hxx>

// System include(s).
#include <cassert>
#include <functional>
#include <memory>
#include <string>

// Make the RNTuple types available in the ROOT namespace
// with all versions of ROOT.
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 36, 0)
namespace ROOT {
using Experimental::DescriptorId_t;
using Experimental::REntry;
using Experimental::RFieldDescriptor;
using Experimental::RNTupleInspector;
using Experimental::RNTupleModel;
using Experimental::RNTupleReader;
using Experimental::RNTupleView;
using Experimental::RNTupleWriter;
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 1)
using Experimental::RException;
using Experimental::RFieldBase;
#endif  // ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 1)
}  // namespace ROOT
#endif  // ROOT_VERSION_CODE < ROOT_VERSION(6, 36, 0)

namespace {

/// This function checks whether a given field (determined by the fieldName)
/// corresponds to a container or a single element based on its type
/// information
///
/// @param fieldDesc The description of the field to check.
/// @param auxid The auxiliary ID of the variable to check.
/// @return @c true if the field represents a container,
///         @c false otherwise.
///
bool isContainerField(const ROOT::RFieldDescriptor& fieldDesc,
                      SG::auxid_t auxid) {

  // For unknown types it doesn't matter if the field describes a
  // container or a single element.
  if (!xAOD::details::isRegisteredType(auxid)) {
    return true;
  }

  // If it's a primitive field, then it's not a container.
  if (xAOD::Utils::isPrimitiveType(fieldDesc.GetTypeName())) {
    return false;
  }

  // For non-primitive types, get the dictionary of the type.
  TClass* cl = TClass::GetClass(fieldDesc.GetTypeName().c_str());

  // If there is no class associated with the field then it should be
  // a field describing a standalone object. (As it should be a
  // "primitive" field in this case.)
  if (!cl) {
    ::Warning("::isPContainerField",
              XAOD_MESSAGE("Couldn't get a dictionary for type \"%s\""),
              fieldDesc.GetTypeName().c_str());
    return false;
  }

  // If there is a class, ask for the type_info of its type:
  const std::type_info* root_ti = cl->GetTypeInfo();
  if (!root_ti) {
    // This may be an emulated class. One known case is when the type name
    // is saved as "basic_string<char>" rather than "string" by Athena I/O.
    // (It's not fully understood why this happens for dynamic branches...)
    // So, let's see if we can get a functional TClass by massaging the
    // type name a bit.
    ::TString typeName(cl->GetName());
    typeName.ReplaceAll("basic_string<char>", "string");
    ::TClass* newCl = ::TClass::GetClass(typeName);
    if (newCl) {
      root_ti = newCl->GetTypeInfo();
    }
  }
  if (!root_ti) {
    ::Error("::isContainerField",
            XAOD_MESSAGE("Couldn't get an std::type_info object out of "
                         "type \"%s\""),
            cl->GetName());
    return false;
  }

  // Ask for the auxiliary type infos:
  const std::type_info* aux_obj_ti =
      SG::AuxTypeRegistry::instance().getType(auxid);
  if (!aux_obj_ti) {
    ::Error("::isContainerField",
            XAOD_MESSAGE("Couldn't get std::type_info object for "
                         "auxiliary id: %i"),
            static_cast<int>(auxid));
    return false;
  }
  const std::type_info* aux_vec_ti =
      SG::AuxTypeRegistry::instance().getVecType(auxid);
  if (!aux_vec_ti) {
    ::Error("::isContainerField",
            XAOD_MESSAGE("Couldn't get std::type_info object for "
                         "auxiliary id: %i"),
            static_cast<int>(auxid));
    return false;
  }

  // Check which one the ROOT type info agrees with:
  if (*root_ti == *aux_obj_ti) {
    // This branch describes a single object:
    return false;
  } else if (*root_ti == *aux_vec_ti) {
    // This branch describes a container of objects:
    return true;
  }

  // For enum and vector<enum> types (PFO...) the type given by
  // the aux type registry is vector<int>. We have to take it into account
  // here...
  if (cl->GetCollectionProxy() && (*aux_vec_ti == typeid(std::vector<int>))) {
    return true;
  }

  TClass* cl2 = xAOD::details::lookupVectorType(*cl);
  if (cl2) {
    if (*cl2->GetTypeInfo() == *aux_vec_ti) {
      return true;
    }
  }

  // If we got this far, the branch may have undergone schema evolution. If
  // it's one that ROOT can deal with itself, then we should still be able
  // to read the branch with this code.
  //
  // Note that even after looking at the ROOT source code, I'm still not
  // 100% sure whether we would need to delete the objects returned by
  // TClass::GetConversionStreamerInfo(...) in this code. :-( But based on
  // general experience with the ROOT code, I'm going to say no...
  TClass* aux_vec_cl =
      TClass::GetClass(xAOD::Utils::getTypeName(*aux_vec_ti).c_str());
  if (aux_vec_cl &&
      aux_vec_cl->GetConversionStreamerInfo(cl, cl->GetClassVersion())) {
    return true;
  }
  TClass* aux_obj_cl =
      TClass::GetClass(xAOD::Utils::getTypeName(*aux_obj_ti).c_str());
  if (aux_obj_cl &&
      aux_obj_cl->GetConversionStreamerInfo(cl, cl->GetClassVersion())) {
    return false;
  }

  // If neither, then something went wrong...
  ::Error("::isContainerField",
          XAOD_MESSAGE("Couldn't determine if field describes a single "
                       "object or a container"));
  ::Error("::isContainerField", XAOD_MESSAGE("ROOT type  : %s"),
          xAOD::Utils::getTypeName(*root_ti).c_str());
  ::Error("::isContainerField", XAOD_MESSAGE("Object type: %s"),
          xAOD::Utils::getTypeName(*aux_obj_ti).c_str());
  ::Error("::isContainerField", XAOD_MESSAGE("Vector type: %s"),
          xAOD::Utils::getTypeName(*aux_vec_ti).c_str());
  return kFALSE;
}

/// Class used internally to implement just-in-time reading
class RFieldHandle {

 public:
  /// Constructor
  RFieldHandle(ROOT::RNTupleView<void> field, SG::auxid_t auxid,
               std::string_view prefix, void* object, const std::type_info* ti)
      : m_field(std::move(field)),
        m_auxid(auxid),
        m_prefix(prefix),
        m_object(object),
        m_typeInfo(ti) {}

  /// Get entry from the field
  StatusCode getEntry(::Long64_t entry) {

    // Check if anything needs to be done:
    if ((m_entry == entry) && (!m_needsRead)) {
      return StatusCode::SUCCESS;
    }

    try {
      // Load the entry
      m_field(entry);
    } catch (const ROOT::RException& e) {
      ::Error("::RFieldInfo::getEntry",
              "Failed to load entry %lld for field %s.%s: %s", entry,
              m_prefix.data(),
              SG::AuxTypeRegistry::instance().getName(m_auxid).c_str(),
              e.what());
      return StatusCode::FAILURE;
    }

    // Remember what entry was loaded for this field.
    m_entry = entry;
    m_needsRead = false;
    return StatusCode::SUCCESS;
  }

  /// Get a pointer to the object
  ///
  /// This function is used in the implementation of the IAuxStoreIO interface.
  ///
  /// @returns A pointer to the object in memory
  ///
  void* objectPtr() { return m_object; }

  /// Get the type of the variable in memory
  const std::type_info* typeInfo() const { return m_typeInfo; }

  /// Tell the object that the branch contents need to be re-read
  void reset() { m_needsRead = true; }

 private:
  /// The typeless object taking care of reading the field
  ROOT::RNTupleView<void> m_field;
  /// The auxiliary ID of the branch
  SG::auxid_t m_auxid;
  /// View of the m_prefix string of the parent object
  std::string_view m_prefix;
  /// Pointer to the object in memory
  void* m_object = nullptr;
  /// Type of the object in memory
  const std::type_info* m_typeInfo = nullptr;
  /// The last entry that was loaded for this field
  ::Long64_t m_entry = -1;
  /// Read status flag
  bool m_needsRead = true;

};  // class RFieldInfo

bool fieldExists(std::string_view fieldName,
                 ROOT::RNTupleReader& ntupleReader) {
  // If it cannot find a field id it will give the maximum value of
  // unsigned long
  return (ntupleReader.GetDescriptor().FindFieldId(fieldName) !=
          std::numeric_limits<unsigned long>::max());
}

}  // namespace

namespace xAOD {

struct RAuxStore::impl {

  /// Scans the input ntuple for auxiliary data fields and sets up
  /// the necessary structures to access them. It ensures that the input ntuple
  /// is properly scanned and the auxiliary data fields are set up.
  ///
  /// @returns @c StatusCode::SUCCESS if the function was
  ///          successful, something else otherwise
  ///
  StatusCode scanInputTuple() {

    // Check if an input ntuple is even available.
    if (!m_inTuple) {
      // It's not an error if it isn't.
      return StatusCode::SUCCESS;
    }

    // Check if the input was already scanned.
    if (m_inputScanned) {
      return StatusCode::SUCCESS;
    }

    // Iterate over all fields of the input ntuple.
    for (const ROOT::RFieldBase* field :
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 35, 0)
         m_inTuple->GetModel().GetConstFieldZero().GetConstSubfields()
#else
         m_inTuple->GetModel().GetConstFieldZero().GetSubFields()
#endif  // ROOT_VERSION_CODE >= ROOT_VERSION(6, 35, 0)
    ) {

      // Get the name of the current field.
      const std::string fieldName = field->GetQualifiedFieldName();

      // Look for static fields.
      if (m_data.m_topStore && (fieldName == m_data.m_prefix)) {

        // Loop over the sub-fields of this field.
        for (const ROOT::RFieldBase* subField :
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 35, 0)
             field->GetConstSubFields()
#else
             field->GetSubFields()
#endif  // ROOT_VERSION_CODE >= ROOT_VERSION(6, 35, 0)
        ) {

          // Get the name of this sub-field.
          const std::string subFieldName = subField->GetQualifiedFieldName();
          const std::string subAuxName =
              subFieldName.substr(subFieldName.find(".") + 1);

          // Skip this entry if it refers to a base class.
          if (subAuxName.starts_with("xAOD::") ||
              subAuxName.starts_with("SG::") ||
              subAuxName.starts_with("ILockable")) {
            continue;
          }

          // Set up this field.
          RETURN_CHECK("xAOD::RAuxStore::impl::scanInputNtuple",
                       setupAuxField(*subField, subAuxName));
        }
        // Don't check the rest of the loop's body:
        continue;
      }

      // if fieldname doesnt start with the value of m_dynPrefix, skip
      if (fieldName.starts_with(m_data.m_dynPrefix) == false) {
        continue;
      }
      if (fieldName == m_data.m_dynPrefix) {
        ::Error("xAOD::RAuxStore::impl::scanInputNtuple",
                "Dynamic field with empty name found on container: %s",
                m_data.m_prefix.c_str());
        continue;
      }
      // The auxiliary property name:
      const std::string auxName = fieldName.substr(fieldName.find(":") + 1);
      // Leave the rest up to the function that is shared with the
      // dynamic fields:
      RETURN_CHECK("xAOD::RAuxStore::scanInputNtuple",
                   setupAuxField(*field, auxName));
    }

    // the input was successfully scanned:
    m_inputScanned = true;
    return StatusCode::SUCCESS;
  }

  /// This function retrieves the type information for a given auxiliary field.
  /// It uses the field's inspector to determine the type and handles cases
  /// where the expected class or collection proxy is not available.
  ///
  /// @param field The field to get the type of
  /// @param expectedClassName The (optional) name of the expected (on disk)
  ///                          class
  /// @returns A pointer to the type information for the field
  ///
  const std::type_info* auxFieldType(const ROOT::RFieldBase& field,
                                     std::string* expectedClassName = nullptr) {

    // Get the type name of the field. Not worrying about automatic schema
    // evolution for now.
    const std::string typeName = field.GetTypeName();
    ::TClass* expectedClass = ::TClass::GetClass(typeName.c_str());
    if (expectedClassName) {
      if (expectedClass) {
        *expectedClassName = expectedClass->GetName();
      } else {
        *expectedClassName = typeName;
      }
    }

    // If this is a primitive variable, and we're still not sure whether this
    // is a store for an object or a container, the answer is given...
    if ((Utils::isPrimitiveType(typeName)) &&
        (m_data.m_structMode == EStructMode::kUndefinedStore)) {
      m_data.m_structMode = EStructMode::kObjectStore;
    }

    // Get the type_info of the branch.
    const std::type_info* ti = nullptr;
    if (m_data.m_structMode == EStructMode::kObjectStore) {
      if (expectedClass) {
        ti = expectedClass->GetTypeInfo();
      } else {
        ti = &(Utils::getTypeInfo(typeName));
      }
    } else {
      if (!expectedClass) {
        ::Warning("xAOD::RAuxStore::impl::auxFieldType",
                  "Couldn't get the type of field \"%s\"",
                  field.GetFieldName().c_str());
      } else {
        ::TVirtualCollectionProxy* prox = expectedClass->GetCollectionProxy();

        if (!prox) {
          TClass* cl2 = details::lookupVectorType(*expectedClass);
          if (cl2) {
            prox = cl2->GetCollectionProxy();
          }
        }

        if (!prox) {
          ::Warning("xAOD::RAuxStore::impl::auxFieldType",
                    "Couldn't get the type of field \"%s\"",
                    field.GetFieldName().c_str());
        } else {
          if (prox->GetValueClass()) {
            ti = prox->GetValueClass()->GetTypeInfo();
          } else {
            ti = &(Utils::getTypeInfo(prox->GetType()));
          }
        }
      }
    }

    return ti;
  }

  /// This function sets up an auxiliary field by determining its type and
  /// attempting to register it with the auxiliary type registry.
  /// If the field type is not known, it tries to create a factory for the
  /// field's type. The function handles both static and dynamic fields and
  /// updates the set of known auxiliary IDs upon success.
  ///
  /// @param field The field to set up
  /// @param auxName The name of the auxiliary property, extracted from the
  ///                field's name
  /// @returns @c StatusCode::SUCCESS if the function was
  ///          successful, something else otherwise
  ///
  StatusCode setupAuxField(const ROOT::RFieldBase& field,
                           std::string_view auxName) {

    // Get the (on disk) type of the field.
    std::string expectedClassName;
    const std::type_info* ti = auxFieldType(field, &expectedClassName);
    if (ti == nullptr) {
      // If we didn't find a type_info for the field, give up now...
      return StatusCode::SUCCESS;
    }

    // Get the registry:
    SG::AuxTypeRegistry& registry = SG::AuxTypeRegistry::instance();

    // Check if the registry already knows this variable name. If yes, let's
    // use the type known by the registry. To be able to deal with simple
    // schema evolution in dynamic fields.
    if (const SG::auxid_t regAuxid = registry.findAuxID(std::string{auxName});
        regAuxid != SG::null_auxid) {
      m_data.m_auxIDs.insert(regAuxid);
      return StatusCode::SUCCESS;
    }

    SG::AuxVarFlags flags = SG::AuxVarFlags::SkipNameCheck;
    SG::auxid_t linkedAuxId = SG::null_auxid;

    if (SG::AuxTypeRegistry::isLinkedName(std::string{auxName})) {
      flags |= SG::AuxVarFlags::Linked;
    } else if (SG::AuxTypeRegistry::classNameHasLink(expectedClassName)) {
      const std::string linkedAttr =
          SG::AuxTypeRegistry::linkedName(std::string{auxName});
      const std::string linkedFieldName =
          SG::AuxTypeRegistry::linkedName(field.GetFieldName());
      const std::type_info* linkedTi = nullptr;
      if (::fieldExists(linkedFieldName, *m_inTuple)) {
        linkedTi =
            auxFieldType(m_inTuple->GetModel().GetConstField(linkedFieldName));
      }
      if (linkedTi) {
        linkedAuxId = registry.getAuxID(
            *linkedTi, linkedAttr, "",
            SG::AuxVarFlags::SkipNameCheck | SG::AuxVarFlags::Linked);
      }
      if (linkedAuxId == SG::null_auxid) {
        ::Error("xAOD::RAuxStore::setupAuxField",
                "Could not find linked variable for %s type %s", auxName.data(),
                expectedClassName.c_str());
      }
    }

    // Check for an auxiliary ID for this field:
    SG::auxid_t auxid =
        registry.getAuxID(*ti, std::string{auxName}, "", flags, linkedAuxId);

    // First try to find a compiled factory for the vector type:
    if (auxid == SG::null_auxid) {

      // Construct the name of the factory's class:
      // But be careful --- if we don't exactly match the name
      // in TClassTable, then we may trigger autoparsing.  Besides the
      // resource usage that implies, that can lead to crashes in dbg
      // builds due to cling bugs.
      std::string typeName = Utils::getTypeName(*ti);
      if (typeName.starts_with("std::vector<"))
        typeName.erase(0, 5);
      std::string factoryClassName =
          "SG::AuxTypeVectorFactory<" + typeName + ",allocator<" + typeName;
      if (factoryClassName[factoryClassName.size() - 1] == '>') {
        factoryClassName += ' ';
      }
      factoryClassName += "> >";

      // Look for the dictionary of this type:
      ::TClass* factoryClass = TClass::GetClass(factoryClassName.c_str());
      if (factoryClass && factoryClass->IsLoaded()) {
        ::TClass* baseClass = ::TClass::GetClass("SG::IAuxTypeVectorFactory");
        if (baseClass && baseClass->IsLoaded()) {
          const Int_t offset = factoryClass->GetBaseClassOffset(baseClass);
          if (offset >= 0) {
            void* factoryVoidPointer = factoryClass->New();
            if (factoryVoidPointer) {
              unsigned long tmp =
                  reinterpret_cast<unsigned long>(factoryVoidPointer) + offset;
              SG::IAuxTypeVectorFactory* factory =
                  reinterpret_cast<SG::IAuxTypeVectorFactory*>(tmp);
              registry.addFactory(
                  *ti, *factory->tiAlloc(),
                  std::unique_ptr<SG::IAuxTypeVectorFactory>(factory));
              auxid = registry.getAuxID(*ti, std::string{auxName}, "", flags,
                                        linkedAuxId);
            }
          }
        }
      }
    }

    // If that didn't succeed, let's assign a generic factory to this type:
    if (auxid == SG::null_auxid && linkedAuxId == SG::null_auxid) {
      // Construct the name of the vector type:
      std::string vectorClassName = "std::vector<" + Utils::getTypeName(*ti);
      if (vectorClassName[vectorClassName.size() - 1] == '>') {
        vectorClassName += ' ';
      }
      vectorClassName += '>';

      // Get the dictionary for the type:
      ::TClass* vectorClass = ::TClass::GetClass(vectorClassName.c_str());
      if (vectorClass && vectorClass->IsLoaded()) {
        auto factory = std::make_unique<TAuxVectorFactory>(vectorClass);
        if (factory->tiAlloc()) {
          const std::type_info* tiAlloc = factory->tiAlloc();
          registry.addFactory(*ti, *tiAlloc, std::move(factory));
        } else {
          std::string tiAllocName = factory->tiAllocName();
          registry.addFactory(*ti, tiAllocName, std::move(factory));
        }
        auxid = registry.getAuxID(*ti, std::string{auxName}, "",
                                  SG::AuxVarFlags::SkipNameCheck);
      } else {
        ::Warning("xAOD::RAuxStore::setupAuxField",
                  "Couldn't find dictionary for type: %s",
                  vectorClassName.c_str());
      }
    }

    // Check if we succeeded:
    if (auxid == SG::null_auxid) {
      if (linkedAuxId != SG::null_auxid) {
        ::Error("xAOD::RAuxStore::setupAuxField",
                XAOD_MESSAGE("Dynamic ROOT vector factory not implemented for "
                             "linked types; field \"%s\""),
                field.GetFieldName().c_str());
      } else {
        ::Error("xAOD::RAuxStore::setupAuxField",
                XAOD_MESSAGE("Couldn't assign auxiliary ID to field \"%s\""),
                field.GetFieldName().c_str());
      }
      return StatusCode::FAILURE;
    }

    // Remember the auxiliary ID:
    m_data.m_auxIDs.insert(auxid);
    return StatusCode::SUCCESS;
  }

  /// Variables coming from @c AuxStoreBase
  Members& m_data;

  /// The ntuple being read from
  ROOT::RNTupleReader* m_inTuple = nullptr;
  /// The ntuple being written to
  ROOT::RNTupleWriter* m_outTuple = nullptr;

  /// "Scan status" of the input RNTuple
  bool m_inputScanned = false;

  /// The entry to load from the ntuple
  ::Long64_t m_entry;

  /// Fields containing the various auxiliary variables
  std::vector<std::unique_ptr<RFieldHandle> > m_fields;
  /// "Write status" of the different variables
  std::vector<bool> m_fieldsWritten;
  /// Mark fields we've found to be missing.
  std::vector<bool> m_missingFields;

  /// Mutex object used for multithreaded synchronisation
  mutable mutex_t m_mutex;

};  // struct RAuxStore::impl

RAuxStore::RAuxStore(std::string_view prefix, bool topStore, EStructMode mode)
    : details::AuxStoreBase(topStore, mode),
      m_impl{std::make_unique<impl>(m_data)} {

  setPrefix(prefix);
}

RAuxStore::~RAuxStore() = default;

void RAuxStore::setPrefix(std::string_view prefix) {

  m_data.m_prefix = prefix;
  m_data.m_dynPrefix = Utils::dynFieldPrefix(m_data.m_prefix);
  reset();
}

/// This function is called by the infrastructure to connect the object
/// to an input RNTuple whenever a new input file is opened.
///
/// @param reader The reader object set up by a parent object
/// @returns @c StatusCode::SUCCESS if the function was
///          successful, something else otherwise
///
StatusCode RAuxStore::readFrom(RNTupleReader& reader) {

  assert(m_impl);

  // Make sure that everything will be re-read after this:
  reset();

  // We will need to check again which branches are available:
  m_impl->m_missingFields.clear();

  // Remember the tree:
  m_impl->m_inTuple = &reader;

  // Catalogue all the branches:
  RETURN_CHECK("xAOD::RAuxStore::readFrom", m_impl->scanInputTuple());

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// This function is called by the infrastructure to connect the object
/// to an output RNTuple.
///
/// @param writer The writer object used for creating an output ntuple
/// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
///
StatusCode RAuxStore::writeTo(ROOT::RNTupleWriter& writer) {

  assert(m_impl);

  // Look for any auxiliary fields that have not been connected to yet.
  RETURN_CHECK("xAOD::RAuxStore::writeTo", m_impl->scanInputTuple());

  // Put the object into "output writing" mode.
  m_impl->m_outTuple = &writer;

  // Create all the variables that we already know about. Notice that the
  // code makes a copy of the auxid set on purpose. Because the underlying
  // AuxSelection object gets modified while doing the for loop.
  const SG::auxid_set_t selAuxIDs = getSelectedAuxIDs();
  for (SG::auxid_t id : selAuxIDs) {
    RETURN_CHECK("xAOD::RAuxStore::writeTo", setupOutputData(id));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode RAuxStore::getEntry(std::int64_t entry, int getall) {

  assert(m_impl);

  // Guard against multi-threaded execution:
  guard_t guard(m_impl->m_mutex);

  m_impl->m_entry = entry;

  // Reset the transient store. TEvent::fill() calls this function with
  // getall==99. When that is happening, we need to keep the transient
  // store still around. Since the user may want to interact with the
  // object after it was written out. (And since TEvent::fill() asks for
  // the transient decorations after calling getEntry(...).)
  if (m_data.m_transientStore && (getall != 99)) {
    // Remove the transient auxiliary IDs from the internal list:
    m_data.m_auxIDs -= m_data.m_transientStore->getAuxIDs();
    m_data.m_decorIDs -= m_data.m_transientStore->getDecorIDs();
    // Delete the object:
    m_data.m_transientStore.reset();
  }

  // Now remove the IDs of the decorations that are getting persistified:
  if (getall != 99) {
    for (SG::auxid_t auxid = 0; auxid < m_data.m_isDecoration.size(); ++auxid) {
      if (!m_data.m_isDecoration[auxid]) {
        continue;
      }
      m_data.m_auxIDs.erase(auxid);
      m_data.m_decorIDs.erase(auxid);
    }
  }

  // If we don't need everything loaded, return now:
  if (!getall) {
    return StatusCode::SUCCESS;
  }

  // Get all the variables at once:
  for (auto& field : m_impl->m_fields) {
    if (field) {
      RETURN_CHECK("xAOD::RAuxStore::getEntry", field->getEntry(entry));
    }
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode RAuxStore::commitTo(ROOT::REntry& entry) {

  assert(m_impl);

  // Loop through all of the output variables.
  for (SG::auxid_t id : getSelectedAuxIDs()) {
    // Now connect the output entry to the variable.
    const std::string fieldName =
        std::format("{}{}", m_data.m_dynPrefix,
                    SG::AuxTypeRegistry::instance().getName(id));
    void* fieldPtr = const_cast<void*>(getIOData(id));
    if (fieldPtr) {
      entry.BindRawPtr(fieldName, fieldPtr);
    } else {
      entry.EmplaceNewValue(fieldName);
    }
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

void RAuxStore::reset() {

  assert(m_impl);

  for (auto& field : m_impl->m_fields) {
    if (field) {
      field->reset();
    }
  }
  m_impl->m_inputScanned = false;
}

bool RAuxStore::hasEntryFor(SG::auxid_t auxid) const {

  assert(m_impl);
  return ((m_impl->m_fields.size() > auxid) && m_impl->m_fields[auxid]);
}

StatusCode RAuxStore::getEntryFor(SG::auxid_t auxid) {

  assert(m_impl);
  assert(m_impl->m_fields.size() > auxid);
  assert(m_impl->m_fields[auxid]);
  RETURN_CHECK("xAOD::RAuxStore::getEntryFor",
               m_impl->m_fields[auxid]->getEntry(m_impl->m_entry));
  return StatusCode::SUCCESS;
}

bool RAuxStore::hasOutput() const {

  assert(m_impl);
  return (m_impl->m_outTuple != nullptr);
}

/// This internal function takes care of connecting to an individual
/// (sub-)field in the input file for a given auxiliary variable.
///
/// @param auxid The ID of the variable to connect to
/// @returns @c StatusCode::SUCCESS if the function was
///          successful, something else otherwise
///
StatusCode RAuxStore::setupInputData(SG::auxid_t auxid) {

  // Return right away if we already know that the field is missing.
  if ((auxid < m_impl->m_missingFields.size()) &&
      m_impl->m_missingFields[auxid]) {
    return StatusCode::RECOVERABLE;
  }

  // We may call this function without an input being used. That's not an
  // error either.
  if (m_impl->m_inTuple == nullptr) {
    return StatusCode::RECOVERABLE;
  }

  // Make sure the internal storage is large enough:
  if (m_data.m_vecs.size() <= auxid) {
    m_data.m_vecs.resize(auxid + 1);
  }
  if (m_impl->m_fields.size() <= auxid) {
    m_impl->m_fields.resize(auxid + 1);
  }

  // Check if we need to do anything. Remember, output-only variables don't
  // have an associated RFieldHandle. To tell the caller that no input is
  // actually available for the variable (only an output), use a different
  // return value.
  if (m_data.m_vecs[auxid]) {
    return (m_impl->m_fields[auxid] ? StatusCode::SUCCESS
                                    : StatusCode::RECOVERABLE);
  }

  // Convenience access to the registry.
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  // Get the property name:
  const std::string statFieldName =
      std::format("{}{}", m_data.m_prefix, r.getName(auxid));
  const std::string dynFieldName =
      std::format("{}{}", m_data.m_dynPrefix, r.getName(auxid));

  // Check if the field exists:
  std::string fieldName = statFieldName;
  ROOT::DescriptorId_t fieldId;
  if ((fieldId = m_impl->m_inTuple->GetDescriptor().FindFieldId(
           statFieldName)) == std::numeric_limits<unsigned long>::max()) {
    if ((fieldId = m_impl->m_inTuple->GetDescriptor().FindFieldId(
             dynFieldName)) == std::numeric_limits<unsigned long>::max()) {
      // Remember that the field is missing.
      if (m_impl->m_missingFields.size() <= auxid) {
        m_impl->m_missingFields.resize(auxid + 1);
      }
      m_impl->m_missingFields[auxid] = true;
      // The field doesn't exist, but this is not an error per se.
      // The user may just be calling isAvailable(...) on the variable.
      return StatusCode::RECOVERABLE;
    }
    // We have a dynamic field:
    fieldName = dynFieldName;
  }

  // Get the object describing this field.
  const ROOT::RFieldDescriptor& fieldDesc =
      m_impl->m_inTuple->GetDescriptor().GetFieldDescriptor(fieldId);

  // Check if it's a "primitive field":
  const bool primitiveField = Utils::isPrimitiveType(fieldDesc.GetTypeName());
  // Check if it's a "container field":
  const bool containerField =
      (primitiveField ? false : isContainerField(fieldDesc, auxid));

  // Set the structure mode if it has not been defined externally:
  if (m_data.m_structMode == EStructMode::kUndefinedStore) {
    m_data.m_structMode = (containerField ? EStructMode::kContainerStore
                                          : EStructMode::kObjectStore);
  }

  // Check that the branch type makes sense:
  if ((containerField &&
       (m_data.m_structMode != EStructMode::kContainerStore) &&
       !r.isLinked(auxid)) ||
      ((!containerField) &&
       (m_data.m_structMode != EStructMode::kObjectStore))) {
    ::Error("xAOD::RAuxStore::setupInputData",
            XAOD_MESSAGE("Field type and requested structure mode "
                         "differ for field: %s"),
            fieldName.c_str());
    return StatusCode::FAILURE;
  }

  // Get the property type:
  const std::type_info* fieldType = nullptr;
  if (details::isRegisteredType(auxid)) {
    // Get the type from the auxiliary type registry:
    fieldType = (containerField ? r.getVecType(auxid) : r.getType(auxid));
  } else {
    // Get the type from the input field itself:
    TClass* clDummy = ::TClass::GetClass(fieldDesc.GetTypeName().c_str());
    fieldType = (clDummy ? clDummy->GetTypeInfo()
                         : &(Utils::getTypeInfo(fieldDesc.GetTypeName())));
  }
  if (!fieldType) {
    ::Error("xAOD::RAuxStore::setupInputData",
            XAOD_MESSAGE("Can't read/copy variable %s (%s)"), fieldName.c_str(),
            fieldDesc.GetTypeName().c_str());
    return StatusCode::RECOVERABLE;
  }
  const TString fieldTypeName = Utils::getTypeName(*fieldType).c_str();

  // Check if we have the needed dictionary for an object field:
  ::TClass* fieldClass = nullptr;
  if (!primitiveField) {
    // Get the property's class:
    fieldClass = ::TClass::GetClass(*fieldType, true, true);
    if (!fieldClass) {
      fieldClass = ::TClass::GetClass(fieldTypeName);
    }
    if (!fieldClass) {
      ::Error("xAOD::RAuxStore::setupInputData",
              XAOD_MESSAGE("No dictionary available for class \"%s\""),
              fieldTypeName.Data());
      return StatusCode::FAILURE;
    }
  }

  // Create the smart object holding this vector:
  if (details::isRegisteredType(auxid)) {
    m_data.m_vecs[auxid] = r.makeVector(auxid, (size_t)0, (size_t)0);
    if (!containerField) {
      m_data.m_vecs[auxid]->resize(1);
    }
    if (fieldClass &&
        strncmp(fieldClass->GetName(), "SG::PackedContainer<", 20) == 0) {
      std::unique_ptr<SG::IAuxTypeVector> packed =
          m_data.m_vecs[auxid]->toPacked();
      std::swap(m_data.m_vecs[auxid], packed);
    }
  } else {
    ::Error("xAOD::RAuxStore::setupInputData",
            XAOD_MESSAGE("Couldn't create in-memory vector for "
                         "variable %s (%i)"),
            fieldName.c_str(), static_cast<int>(auxid));
    return StatusCode::FAILURE;
  }

  // Access/create the field, and create a field handle object.
  void* objectPtr = (containerField ? m_data.m_vecs[auxid]->toVector()
                                    : m_data.m_vecs[auxid]->toPtr());
  m_impl->m_fields[auxid] = std::make_unique<RFieldHandle>(
      m_impl->m_inTuple->GetView<void>(fieldName.c_str(), objectPtr), auxid,
      m_data.m_prefix, objectPtr, fieldType);

  // Get the current entry.
  RETURN_CHECK("xAOD::RAuxStore::setupInputData",
               m_impl->m_fields[auxid]->getEntry(m_impl->m_entry));

  // Remember which variable got created:
  m_data.m_auxIDs.insert(auxid);

  // Check if we just replaced a generic object:
  if (details::isRegisteredType(auxid)) {
    // The name of the variable we just created:
    const std::string auxname = r.getName(auxid);
    // Check if there's another variable with this name already:
    for (SG::auxid_t i = 0; i < m_data.m_vecs.size(); ++i) {
      // Check if we have this aux ID:
      if (!m_data.m_vecs[i]) {
        continue;
      }
      // Ingore the object that we *just* created:
      if (i == auxid) {
        continue;
      }
      // The name of the variable:
      const std::string testname = r.getName(i);
      // Check if it has the same name:
      if (testname != auxname) {
        continue;
      }
      // Check that the other one is a non-registered type:
      if (details::isRegisteredType(i)) {
        ::Error("xAOD::RAuxStore::setupInputData",
                XAOD_MESSAGE("Internal logic error!"));
        continue;
      }
      // Okay, we do need to remove this object:
      m_data.m_vecs[i].reset();
      m_impl->m_fields[i].reset();
      m_data.m_auxIDs.erase(i);
    }
  }

  SG::auxid_t linked_auxid = r.linkedVariable(auxid);
  if (linked_auxid != SG::null_auxid) {
    return setupInputData(linked_auxid);
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function is used internally to create an output field
/// with the contents of a single auxiliary variable.
///
/// @param auxid The ID of the variable to create an output field for
/// @returns @c StatusCode::SUCCESS if the function was
///          successful, something else otherwise
///
StatusCode RAuxStore::setupOutputData(SG::auxid_t auxid) {

  assert(m_impl);

  // Check whether we need to do anything.
  if (!m_impl->m_outTuple) {
    return StatusCode::SUCCESS;
  }

  // Check if the variable needs to be written out.
  if (!isAuxIDSelected(auxid)) {
    return StatusCode::SUCCESS;
  }

  // Make sure that containers are large enough:
  if (m_data.m_vecs.size() <= auxid) {
    m_data.m_vecs.resize(auxid + 1);
  }
  if (m_impl->m_fieldsWritten.size() <= auxid) {
    m_impl->m_fieldsWritten.resize(auxid + 1);
  }

  // Check if this auxiliary variable is already in the output:
  if (m_impl->m_fieldsWritten[auxid]) {
    return StatusCode::SUCCESS;
  }

  // After this point, we either succeed with setting up the writing of this
  // variable, or the code fails completely. So let's set this flag already,
  // as unfortunately we can recursively end up here using the code below.
  m_impl->m_fieldsWritten[auxid] = true;

  // The registry:
  SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

  // Check if the variable was put into the transient store as a
  // decoration, and now needs to be put into the output file:
  if ((!m_data.m_vecs[auxid]) && m_data.m_transientStore &&
      (m_data.m_transientStore->getAuxIDs().test(auxid))) {

    // Get the variable from the transient store:
    const void* pptr = m_data.m_transientStore->getData(auxid);
    if (!pptr) {
      ::Fatal("xAOD::RAuxStore::setupOutputData",
              XAOD_MESSAGE("Internal logic error detected"));
      return StatusCode::FAILURE;
    }

    // Create the new object:
    m_data.m_vecs[auxid] = reg.makeVector(auxid, m_data.m_size, m_data.m_size);
    void* ptr = m_data.m_vecs[auxid]->toPtr();
    if (!ptr) {
      ::Error("xAOD::RAuxStore::setupOutputData",
              XAOD_MESSAGE("Couldn't create decoration in memory "
                           "for writing"));
      return StatusCode::FAILURE;
    }

    // Get the type of this variable:
    const std::type_info* type = reg.getType(auxid);
    if (!type) {
      ::Error("xAOD::RAuxStore::setupOutputData",
              XAOD_MESSAGE("Couldn't get the type of transient "
                           "variable %i"),
              static_cast<int>(auxid));
      return StatusCode::FAILURE;
    }
    // Now get the factory for this variable:
    const SG::IAuxTypeVectorFactory* factory = reg.getFactory(auxid);
    if (!factory) {
      ::Error("xAOD::RAuxStore::setupOutputData",
              XAOD_MESSAGE("No factory found for transient variable "
                           "%i"),
              static_cast<int>(auxid));
      return StatusCode::FAILURE;
    }

    // Mark it as a decoration already, otherwise the copy may fail.
    if (m_data.m_isDecoration.size() <= auxid) {
      m_data.m_isDecoration.resize(auxid + 1);
    }
    m_data.m_isDecoration[auxid] = true;

    // Finally, do the copy, and remove the variable from the transient store.
    factory->copy(auxid, SG::AuxVectorInterface(*this), 0,
                  SG::AuxVectorInterface(*m_data.m_transientStore), 0,
                  m_data.m_size);
  }

  // Check if we know about this variable to be on the input,
  // but haven't connected to it yet:
  if ((m_data.m_auxIDs.test(auxid)) && (!m_data.m_vecs[auxid]) &&
      (!m_impl->m_fields[auxid])) {
    RETURN_CHECK("xAOD::RAuxStore::setupOutputData", setupInputData(auxid));
  }

  // Check that we know the store's type:
  if ((m_data.m_structMode != EStructMode::kContainerStore) &&
      (m_data.m_structMode != EStructMode::kObjectStore)) {
    ::Error("xAOD::RAuxStore::setupOutputData",
            XAOD_MESSAGE("Structure mode unknown for variable %s"),
            reg.getName(auxid).c_str());
    return StatusCode::FAILURE;
  }

  // Check if the variable exists already in memory:
  if (!m_data.m_vecs[auxid]) {
    m_data.m_vecs[auxid] = reg.makeVector(auxid, (size_t)0, (size_t)0);
    if (m_data.m_structMode == EStructMode::kObjectStore) {
      m_data.m_vecs[auxid]->resize(1);
    }
  }

  // Figure out the type and name of the output field.
  const std::string fieldName =
      std::format("{}{}", m_data.m_dynPrefix, reg.getName(auxid));
  const std::string typeName = SG::normalizedTypeinfoName(
      *(m_data.m_structMode == EStructMode::kContainerStore
            ? reg.getVecType(auxid)
            : reg.getType(auxid)));

  // Update the output ntuple's model.
  {
    auto field = ROOT::RFieldBase::Create(fieldName, typeName).Unwrap();
    auto updater = m_impl->m_outTuple->CreateModelUpdater();
    updater->BeginUpdate();
    updater->AddField(std::move(field));
    updater->CommitUpdate();
  }

  // Remember that we now handle this variable.
  m_data.m_auxIDs.insert(auxid);

  // We were successful:
  return StatusCode::SUCCESS;
}

const void* RAuxStore::getInputObject(SG::auxid_t auxid) const {

  assert(m_impl);
  assert(m_impl->m_fields.size() > auxid);
  assert(m_impl->m_fields[auxid]);
  return m_impl->m_fields[auxid]->objectPtr();
}

const std::type_info* RAuxStore::getInputType(SG::auxid_t auxid) const {

  assert(m_impl);
  assert(m_impl->m_fields.size() > auxid);
  assert(m_impl->m_fields[auxid]);
  return m_impl->m_fields[auxid]->typeInfo();
}

}  // namespace xAOD
