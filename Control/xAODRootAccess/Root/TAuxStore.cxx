// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/TAuxStore.h"

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
#include "CxxUtils/checker_macros.h"
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

// ROOT include(s).
#include <TBranch.h>
#include <TBranchElement.h>
#include <TClass.h>
#include <TError.h>
#include <TROOT.h>
#include <TStreamerElement.h>
#include <TStreamerInfo.h>
#include <TString.h>
#include <TTree.h>
#include <TVirtualCollectionProxy.h>

// System include(s):
#include <cassert>
#include <format>
#include <sstream>
#include <stdexcept>

namespace {

/// Check if a branch holds a primitive variable or not
bool isPrimitiveBranch(TBranch& br) {

  // The variables needed for the check:
  ::TClass* cl = nullptr;
  ::EDataType dType = kOther_t;

  // Get the variable type from the branch:
  if (br.GetExpectedType(cl, dType)) {
    ::Error("::isPrimitiveBranch",
            XAOD_MESSAGE("Couldn't determine the type of branch "
                         "\"%s\""),
            br.GetName());
    return false;
  }

  // The check is made using the data type variable:
  return ((dType != kOther_t) && (dType != kNoType_t) && (dType != kVoid_t));
}

/// Check if a branch describes a container or a single object
///
/// The code needs to know if a given branch describes a single object, or
/// a container of objects. This function decides about this.
///
/// In case the function encounters a problem it returns
/// <code>kFALSE</code>, as that is more likely to cause a crash a little
/// later...
///
/// The function should probably be a bit optimised in the future, as it
/// makes a lot of security checks at the moment.
///
/// @param br    The branch whose type we're interested in
/// @param auxid The auxiliary ID of the variable described by the branch
/// @returns @c true if the branch describes a container,
///          @c false if it describes a single object
///
bool isContainerBranch(TBranch& br, SG::auxid_t auxid) {

  // For unknown types it doesn't matter if the branch describes a
  // container or a single element.
  if (!xAOD::details::isRegisteredType(auxid)) {
    return true;
  }

  // The variables needed for the check:
  ::TClass* cl = nullptr;
  ::EDataType dType = kOther_t;

  // Get the variable type from the branch:
  if (br.GetExpectedType(cl, dType)) {
    ::Error("::isContainerBranch",
            XAOD_MESSAGE("Couldn't determine the type of branch \"%s\""),
            br.GetName());
    return false;
  }

  // If there is no class associated with the branch then it should be
  // a branch describing a standalone object. (As it should be a
  // "primitive" branch in this case.)
  if (!cl) {
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
    ::Error("::isContainerBranch",
            XAOD_MESSAGE("Couldn't get an std::type_info object out of "
                         "branch \"%s\" of type \"%s\""),
            br.GetName(), cl->GetName());
    return false;
  }

  // Ask for the auxiliary type infos:
  const std::type_info* aux_obj_ti =
      SG::AuxTypeRegistry::instance().getType(auxid);
  if (!aux_obj_ti) {
    ::Error("::isContainerBranch",
            XAOD_MESSAGE("Couldn't get std::type_info object for "
                         "auxiliary id: %i"),
            static_cast<int>(auxid));
    return false;
  }
  const std::type_info* aux_vec_ti =
      SG::AuxTypeRegistry::instance().getVecType(auxid);
  if (!aux_vec_ti) {
    ::Error("::isContainerBranch",
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
  ::Error("::isContainerBranch",
          XAOD_MESSAGE("Couldn't determine if branch describes a single "
                       "object or a container"));
  ::Error("::isContainerBranch", XAOD_MESSAGE("ROOT type  : %s"),
          xAOD::Utils::getTypeName(*root_ti).c_str());
  ::Error(":isContainerBranch", XAOD_MESSAGE("Object type: %s"),
          xAOD::Utils::getTypeName(*aux_obj_ti).c_str());
  ::Error("::isContainerBranch", XAOD_MESSAGE("Vector type: %s"),
          xAOD::Utils::getTypeName(*aux_vec_ti).c_str());
  return false;
}

/// Class used internally to implement just-in-time reading
///
/// This is a much simplified version of the code that we used in the
/// D3PDReader code to read variables just in time for the users.
///
class TBranchHandle {

 public:
  /// Constructor
  TBranchHandle(bool staticBranch, bool primitiveBranch,
                const std::type_info* ti, void* obj, SG::auxid_t auxid,
                std::string_view prefix)
      : m_branch(0),
        m_entry(0),
        m_object(obj),
        m_static(staticBranch),
        m_primitive(primitiveBranch),
        m_typeInfo(ti),
        m_needsRead(true),
        m_auxid(auxid),
        m_prefix(prefix) {}

  /// Get entry from the branch that was loaded with @c TTree::LoadTree()
  ///
  /// This function takes care of implementing just-in-time reading of
  /// branches for us.
  ///
  /// @returns The number of bytes read. A negative number in case of error.
  ///
  ::Int_t getEntry() {

    // A little sanity check:
    if (!m_branch) {
      // This is no longer an error. We can have such objects for
      // decorations, which don't exist on the input.
      return 0;
    }

    // Update the I/O monitoring:
    xAOD::IOStats::instance().stats().readBranch(std::string{m_prefix},
                                                 m_auxid);

    // Make sure that the branch is associated to a tree
    // as the entry to be read is retrieved from the tree
    if (!m_branch->GetTree()) {
      Error(
          "xAOD::TAuxStore::TBranchHandle::getEntry",
          XAOD_MESSAGE("Branch=%s is not associated to any tree while reading "
                       "of branches within this class relies on that"),
          m_branch->GetName());
      return -1;
    }

    // Get the entry that should be read
    // The entry to be read is set with TTree::LoadTree()
    // NB: for a branch from a friend tree and if the friend tree has an index
    // built, then the entry to read is found when calling the TTree::LoadTree()
    // function that matches the major and minor values between the main tree
    // and the friend tree
    ::Long64_t entry = m_branch->GetTree()->GetReadEntry();

    if (entry < 0) {
      // Raise error as it implies
      // either that the TTree::LoadTree() function has not been called
      // or
      // the entry requested to be read by the user
      // is not corresponding to any entry for the friend tree
      Error("xAOD::TAuxStore::TBranchHandle::getEntry",
            XAOD_MESSAGE(
                "Entry to read is not set for branch=%s from tree=%s. "
                "It is either because TTree::LoadTree(entry) was not called "
                "beforehand in the TEvent class OR "
                "the entry requested to be read for the main tree is not "
                "corresponding to an event for the friend tree"),
            m_branch->GetName(), m_branch->GetTree()->GetName());
      return -1;
    }

    // Check if anything needs to be done:
    if ((entry == m_entry) && (!m_needsRead)) {
      return 0;
    }

    // Switch the branch in the right mode:
    if (!m_primitive) {
      if ((m_branch->GetMakeClass() != m_static) &&
          (!m_branch->SetMakeClass(m_static))) {
        ::Error("xAOD::TAuxStore::TBranchHandle::getEntry",
                XAOD_MESSAGE("Failed to call SetMakeClass(%i) on "
                             "branch \"%s\""),
                static_cast<int>(m_static), m_branch->GetName());
        return -1;
      }
    }

    // Load the entry.
    const ::Int_t nbytes = m_branch->GetEntry(entry);

    // If the load was successful, remember that we loaded this entry.
    if (nbytes >= 0) {
      m_entry = entry;
      // The reading will now be done:
      m_needsRead = false;
    }

    // Return the number of bytes read.
    return nbytes;
  }

  /// Get a pointer to the branch being held
  ///
  /// No magic here. <code>TTree::SetBranchAddress</code> needs a pointer
  /// to a <code>TBranch</code> pointer. This function just makes sure that
  /// we can give it such a pointer, which will stay valid during the job.
  ///
  /// @returns A pointer to the branch object handled by this object
  ///
  ::TBranch** branchPtr() { return &m_branch; }

  /// Get a pointer to the object
  ///
  /// This function is used in the implementation of the IAuxStoreIO interface.
  /// In there we just need a simple pointer to the object, we don't care
  /// about ROOT's idiosyncracies.
  ///
  /// @returns A pointer to the object in memory
  ///
  void* objectPtr() { return m_object; }

  /// Get a pointer to the object (or its pointer) in memory for reading
  ///
  /// This is a tricky one. When reading a single sub-branch of a branch
  /// that was written using an auxiliary container with direct ROOT I/O,
  /// TTree expects us to give it a simple pointer to the object that it
  /// should fill with content. But when reading a top level branch, like
  /// the dynamically created branches that we make, TTree needs to receive
  /// a pointer to a pointer.
  ///
  /// Also, when reading a primitive type, ROOT also expects a simple pointer
  /// to the primitive variable in memory.
  ///
  /// This function returns the correct kind of pointer for
  /// <code>TTree::SetBranchAddress</code> in all situations.
  ///
  /// @returns The pointer to be given to <code>TTree::SetBranchAddress</code>
  ///
  void* inputObjectPtr() {
    // Return the correct pointer:
    if (m_static || m_primitive) {
      return m_object;
    } else {
      return &m_object;
    }
  }
  /// Get a pointer to the object (or its pointer) in memory for writing
  ///
  /// This is a slightly simplified version of GetInputObjectPtr(), for the
  /// case when we need to write a branch, and have to pass a pointer to
  /// <code>TTree::Branch</code>. In this case we don't need to be mindful of
  /// whether the variable was read as a static or dynamic variable. The only
  /// interesting thing is if the variable is of a primitive or an object
  /// type.
  ///
  /// @returns The pointer to be given to <code>TTree::Branch</code>
  ///
  void* outputObjectPtr() {
    // Return the correct pointer:
    if (m_primitive) {
      return m_object;
    } else {
      return &m_object;
    }
  }
  /// Get the type of the variable in memory
  const std::type_info* typeInfo() const { return m_typeInfo; }
  /// Tell the object that the branch contents need to be re-read
  ///
  /// After @c TEvent::Fill is called, the @c TAuxStore objects used for data
  /// writing are cleared. Since @c TEvent doesn't know if the object is also
  /// used to read information or not. (If it's just used for writing, then
  /// this explicit clearing is mandatory.)
  ///
  /// In case the user wants to continue processing the same event after
  /// the @c fill() call (because for instance the code is writing multiple
  /// trees), then we need to be aware that the variable needs to be
  /// re-read from the input to be in a good shape for the next write
  /// operation.
  ///
  void reset() { m_needsRead = true; }

 private:
  /// The branch that this object is handling
  ::TBranch* m_branch;
  /// The last entry that was loaded for this branch
  ::Long64_t m_entry;
  /// The pointer to the object in memory
  void* m_object;
  /// Is this a static variable in question?
  bool m_static;
  /// Is this a primitive variable?
  bool m_primitive;
  /// Type info for the variable
  const std::type_info* m_typeInfo;
  /// Read status flag
  bool m_needsRead;
  /// The auxiliary ID of the branch
  SG::auxid_t m_auxid;
  /// View of the m_prefix string of the parent object
  std::string_view m_prefix;

};  // class TBranchHandle

}  // namespace

namespace xAOD {

struct TAuxStore::impl {

  /// Scan the input TTree for auxiliary branches
  ///
  /// When writing an output tree, while reading information from an input
  /// tree, it can often happen that we want to copy the contents of some
  /// variables that we don't actually need during the event processing of
  /// this particular job. Since the user doesn't ask for all the possible
  /// input variables, this function needs to look at the input TTree, and
  /// try to figure out which of the branches in the tree belong to this
  /// object.
  ///
  /// The function creates a "proper" or "virtual" auxiliary ID for each of
  /// the branches found, so they can be referenced in the "usual way" in
  /// the other parts of the code later on.
  ///
  /// @returns @c StatusCode::SUCCESS if the function was
  ///          successful, something else otherwise
  ///
  StatusCode scanInputTree() {

    // Check if an input tree is even available:
    if (!m_inTree) {
      // It's not an error if it isn't.
      return StatusCode::SUCCESS;
    }

    // Check if the input was already scanned:
    if (m_inputScanned) {
      return StatusCode::SUCCESS;
    }

    // Get a list of all branches in the tree:
    TObjArray* branches = m_inTree->GetListOfBranches();

    // Check each of them:
    for (Int_t i = 0; i < branches->GetEntriesFast(); ++i) {

      // The name of this top-level branch:
      const TString brName = branches->At(i)->GetName();

      // Access the branch pointer:
      TBranch* br = dynamic_cast<TBranch*>(branches->At(i));
      if (!br) {
        ::Fatal("xAOD::TAuxStore::impl::scanInputTree",
                XAOD_MESSAGE("Logic error detected"));
      }

      // For top-level stores let's scan the static branches as well:
      if (m_data.m_topStore && (brName == m_data.m_prefix)) {

        // Make sure the object has been instantiated so that aux data
        // registrations will have been done.
        br->SetAddress(0);

        // Get a list of its sub-branches:
        TObjArray* sbranches = br->GetListOfBranches();

        // ...and then loop over them:
        for (Int_t j = 0; j < sbranches->GetEntriesFast(); ++j) {

          // The name of the sub-branch:
          const TString brName = sbranches->At(j)->GetName();

          // Try to make a variable name out of the branch name:
          const TString auxName =
              brName(brName.Index(".") + 1, brName.Length());

          // Skip this entry if it refers to a base class:
          if (auxName.BeginsWith("xAOD::") || auxName.BeginsWith("SG::") ||
              (auxName == "ILockable")) {
            continue;
          }

          // The sub-branch:
          ::TBranch* sbr = dynamic_cast< ::TBranch*>(sbranches->At(j));
          if (!sbr) {
            ::Fatal("xAOD::TAuxStore::impl::scanInputTree",
                    XAOD_MESSAGE("Logic error detected"));
          }

          // Leave the rest up to the function that is shared with the
          // dynamic branches:
          RETURN_CHECK("xAOD::TAuxStore::impl::scanInputTree",
                       setupAuxBranch(*sbr, auxName, true));
        }

        // Don't check the rest of the loop's body:
        continue;
      }

      // Check if it has the right prefix to be a dynamic variable:
      if (!brName.BeginsWith(m_data.m_dynPrefix.data())) {
        continue;
      }
      // It's possible to create dynamic variables with an empty name
      // as well. Which is a bug. Such variables are just ignored
      // for now.
      if (brName == m_data.m_dynPrefix) {
        ::Warning("xAOD::TAuxStore::impl::scanInputTree",
                  "Dynamic branch with empty name found on container: %s",
                  m_data.m_prefix.data());
        continue;
      }

      // The auxiliary property name:
      const TString auxName = brName(brName.Index(".") + 1, brName.Length());

      // Leave the rest up to the function that is shared with the
      // dynamic branches:
      RETURN_CHECK("xAOD::TAuxStore::impl::scanInputTree",
                   setupAuxBranch(*br, auxName, false));
    }

    // Okay, the input was successfully scanned:
    m_inputScanned = true;

    // Return gracefully:
    return StatusCode::SUCCESS;
  }

  /// Find the type_info to use as the aux type for a given branch.
  const std::type_info* auxBranchType(
      ::TBranch& br, std::string_view auxName, bool staticBranch,
      std::string* expectedClassName = nullptr) {

    // Get the branch's type:
    ::TClass* expectedClass = nullptr;
    ::EDataType expectedType = kOther_t;
    if (br.GetExpectedType(expectedClass, expectedType) &&
        ((!staticBranch) || (!auxName.starts_with("m_")))) {
      ::Warning("xAOD::TAuxStore::impl::auxBranchType",
                "Couldn't get the type of branch \"%s\"", br.GetName());
    }

    // Check for schema evolution:
    // If a branch has automatic schema evolution from one class to another,
    // then what we get from GetExpectedType will be the on-disk class.
    // What we have in memory is given by GetCurrentClass.
    if (expectedClass) {
      if (TBranchElement* bre = dynamic_cast<TBranchElement*>(&br)) {
        TClass* newClass = bre->GetCurrentClass();
        if (newClass && newClass != expectedClass) {
          expectedClass = newClass;
        }
      }
      if (expectedClassName) {
        *expectedClassName = expectedClass->GetName();
      }
    }

    // If this is a primitive variable, and we're still not sure whether this
    // is a store for an object or a container, the answer is given...
    if ((!expectedClass) &&
        (m_data.m_structMode == EStructMode::kUndefinedStore)) {
      m_data.m_structMode = EStructMode::kObjectStore;
    }

    // Get the type_info of the branch.
    const std::type_info* ti = nullptr;
    if (m_data.m_structMode == EStructMode::kObjectStore) {
      if (expectedClass) {
        ti = expectedClass->GetTypeInfo();
      } else {
        ti = &(Utils::getTypeInfo(expectedType));
      }
    } else {
      if (!expectedClass) {
        if ((!staticBranch) || (!auxName.starts_with("m_"))) {
          ::Warning("xAOD::TAuxStore::impl::auxBranchType",
                    "Couldn't get the type of branch \"%s\"", br.GetName());
        }
      } else {
        ::TVirtualCollectionProxy* prox = expectedClass->GetCollectionProxy();

        if (!prox) {
          TClass* cl2 = details::lookupVectorType(*expectedClass);
          if (cl2) {
            prox = cl2->GetCollectionProxy();
          }
        }

        if (!prox) {
          if ((!staticBranch) || (!auxName.starts_with("m_"))) {
            ::Warning("xAOD::TAuxStore::impl::auxBranchType",
                      "Couldn't get the type of branch \"%s\"", br.GetName());
          }
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

  /// Register one input branch as an available auxiliary variable
  ///
  /// This function takes care of assigning an auxiliary ID to a given branch.
  /// It tries its best to find an auxiliary vector factory for the branch's
  /// type, but if it fails, it still falls back to using
  /// <code>SG::AuxTypePlaceholder</code> as the type. In which case of course
  /// only dumb copying will be possible for the given branch. (And no vector
  /// operations on the branch's payload until the variable of the branch is
  /// accessed explicitly.)
  ///
  /// @param br Pointer to the branch under investigation
  /// @param auxName The name of the auxiliary property, extracted from the
  ///                branch's name
  /// @param staticBranch <code>kTRUE</code> if this is a "static branch", and
  ///                     <code>kFALSE</code> if it's a dynamic one
  /// @returns @c StatusCode::SUCCESS if the function was
  ///          successful, something else otherwise
  ///
  StatusCode setupAuxBranch(::TBranch& br, std::string_view auxName,
                            bool staticBranch) {

    // Get the (on disk) type of the branch.
    std::string expectedClassName;
    const std::type_info* ti =
        auxBranchType(br, auxName, staticBranch, &expectedClassName);
    if (ti == nullptr) {
      // If we didn't find a type_info for the branch, give up now...
      return StatusCode::SUCCESS;
    }

    // Get the registry:
    SG::AuxTypeRegistry& registry = SG::AuxTypeRegistry::instance();

    // Check if the registry already knows this variable name. If yes, let's
    // use the type known by the registry. To be able to deal with simple
    // schema evolution in dynamic branches.
    if (const SG::auxid_t regAuxid = registry.findAuxID(std::string{auxName});
        regAuxid != SG::null_auxid) {
      m_data.m_auxIDs.insert(regAuxid);
      return StatusCode::SUCCESS;
    }

    SG::AuxVarFlags flags = SG::AuxVarFlags::SkipNameCheck;
    SG::auxid_t linked_auxid = SG::null_auxid;

    if (SG::AuxTypeRegistry::isLinkedName(std::string{auxName})) {
      flags |= SG::AuxVarFlags::Linked;
    } else if (SG::AuxTypeRegistry::classNameHasLink(expectedClassName)) {
      std::string linkedAttr =
          SG::AuxTypeRegistry::linkedName(std::string{auxName});
      std::string linkedBranch = SG::AuxTypeRegistry::linkedName(br.GetName());
      ::TBranch* lbr = m_inTree->GetBranch(linkedBranch.c_str());
      const std::type_info* lti = nullptr;
      if (lbr) {
        lti = auxBranchType(*lbr, linkedAttr, staticBranch);
      }
      if (lti) {
        linked_auxid = registry.getAuxID(
            *lti, linkedAttr, "",
            SG::AuxVarFlags::SkipNameCheck | SG::AuxVarFlags::Linked);
      }
      if (linked_auxid == SG::null_auxid) {
        ::Error("xAOD::TAuxStore::impl::setupAuxBranch",
                "Could not find linked variable for %s type %s", auxName.data(),
                expectedClassName.c_str());
      }
    }

    // Check for an auxiliary ID for this branch:
    SG::auxid_t auxid =
        registry.getAuxID(*ti, std::string{auxName}, "", flags, linked_auxid);

    // First try to find a compiled factory for the vector type:
    if (auxid == SG::null_auxid) {

      // Construct the name of the factory's class:
      // But be careful --- if we don't exactly match the name
      // in TClassTable, then we may trigger autoparsing.  Besides the
      // resource usage that implies, that can lead to crashes in dbg
      // builds due to cling bugs.
      std::string tn = Utils::getTypeName(*ti);
      if (tn.starts_with("std::vector<")) {
        tn.erase(0, 5);
      }
      std::string fac_class_name =
          "SG::AuxTypeVectorFactory<" + tn + ",allocator<" + tn;
      if (fac_class_name[fac_class_name.size() - 1] == '>') {
        fac_class_name += ' ';
      }
      fac_class_name += "> >";

      // Look for the dictionary of this type:
      ::TClass* fac_class = TClass::GetClass(fac_class_name.c_str());
      if (fac_class && fac_class->IsLoaded()) {
        ::TClass* base_class = ::TClass::GetClass("SG::IAuxTypeVectorFactory");
        if (base_class && base_class->IsLoaded()) {
          const Int_t offs = fac_class->GetBaseClassOffset(base_class);
          if (offs >= 0) {
            void* fac_vp = fac_class->New();
            if (fac_vp) {
              unsigned long tmp =
                  reinterpret_cast<unsigned long>(fac_vp) + offs;
              SG::IAuxTypeVectorFactory* fac =
                  reinterpret_cast<SG::IAuxTypeVectorFactory*>(tmp);
              registry.addFactory(
                  *ti, *fac->tiAlloc(),
                  std::unique_ptr<SG::IAuxTypeVectorFactory>(fac));
              auxid = registry.getAuxID(*ti, std::string{auxName}, "", flags,
                                        linked_auxid);
            }
          }
        }
      }
    }

    // If that didn't succeed, let's assign a generic factory to this type:
    if (auxid == SG::null_auxid && linked_auxid == SG::null_auxid) {

      // Construct the name of the vector type:
      std::string vec_class_name = "std::vector<" + Utils::getTypeName(*ti);
      if (vec_class_name[vec_class_name.size() - 1] == '>') {
        vec_class_name += ' ';
      }
      vec_class_name += '>';

      // Get the dictionary for the type:
      ::TClass* vec_class = ::TClass::GetClass(vec_class_name.c_str());
      if (vec_class && vec_class->IsLoaded()) {
        auto fac = std::make_unique<TAuxVectorFactory>(vec_class);
        if (fac->tiAlloc()) {
          const std::type_info* tiAlloc = fac->tiAlloc();
          registry.addFactory(*ti, *tiAlloc, std::move(fac));
        } else {
          std::string tiAllocName = fac->tiAllocName();
          registry.addFactory(*ti, tiAllocName, std::move(fac));
        }
        auxid = registry.getAuxID(*ti, std::string{auxName}, "",
                                  SG::AuxVarFlags::SkipNameCheck);
      } else {
        ::Warning("xAOD::TAuxStore::impl::setupAuxBranch",
                  "Couldn't find dictionary for type: %s",
                  vec_class_name.c_str());
      }
    }

    // Check if we succeeded:
    if (auxid == SG::null_auxid) {
      if (linked_auxid != SG::null_auxid) {
        ::Error("xAOD::TAuxStore::impl::setupAuxBranch",
                XAOD_MESSAGE("Dynamic ROOT vector factory not implemented for "
                             "linked types; branch "
                             "\"%s\""),
                br.GetName());
      } else {
        ::Error("xAOD::TAuxStore::impl::setupAuxBranch",
                XAOD_MESSAGE("Couldn't assign auxiliary ID to branch "
                             "\"%s\""),
                br.GetName());
      }
      return StatusCode::FAILURE;
    }

    // Remember the auxiliary ID:
    m_data.m_auxIDs.insert(auxid);
    return StatusCode::SUCCESS;
  }

  /// Variables coming from @c AuxStoreBase
  Members& m_data;

  /// The basket size for the output branches
  int m_basketSize = 2048;
  /// The split level for the output branches
  int m_splitLevel = 0;

  /// The TTree being read from
  ::TTree* m_inTree = nullptr;
  /// The TTree being written to
  ::TTree* m_outTree = nullptr;

  /// "Scan status" of the input TTree
  bool m_inputScanned = false;

  /// Branches reading the various auxiliary variables
  std::vector<std::unique_ptr<TBranchHandle> > m_branches;
  /// "Write status" of the different variables
  std::vector<bool> m_branchesWritten;
  /// Mark branches we've found to be missing. (Because TTree::GetBranch
  /// is very expensive.)
  std::vector<bool> m_missingBranches;

  /// Mutex object used for multithreaded synchronisation
  mutable mutex_t m_mutex;
};

TAuxStore::TAuxStore(std::string_view prefix, bool topStore, EStructMode mode,
                     int basketSize, int splitLevel)
    : details::AuxStoreBase(topStore, mode),
      m_impl{std::make_unique<impl>(m_data, basketSize, splitLevel)} {

  setPrefix(prefix);
}

TAuxStore::~TAuxStore() = default;

void TAuxStore::setPrefix(std::string_view prefix) {

  m_data.m_prefix = prefix;
  m_data.m_dynPrefix = Utils::dynBranchPrefix(m_data.m_prefix);
  reset();
}

int TAuxStore::basketSize() const {

  assert(m_impl);
  return m_impl->m_basketSize;
}

void TAuxStore::setBasketSize(int value) {

  assert(m_impl);
  m_impl->m_basketSize = value;
}

int TAuxStore::splitLevel() const {

  assert(m_impl);
  return m_impl->m_splitLevel;
}

void TAuxStore::setSplitLevel(int value) {

  assert(m_impl);
  m_impl->m_splitLevel = value;
}

/// This function is called by the infrastructure to connect the object
/// to an input TTree whenever a new input file is opened.
///
/// @param tree Pointer to the TTree that is being read from
///
StatusCode TAuxStore::readFrom(::TTree& tree, bool printWarnings) {

  assert(m_impl);

  // Make sure that everything will be re-read after this:
  reset();

  // We will need to check again which branches are available:
  m_impl->m_missingBranches.clear();

  // Remember the tree:
  m_impl->m_inTree = &tree;

  // Catalogue all the branches:
  RETURN_CHECK("xAOD::TAuxStore::readFrom", m_impl->scanInputTree());

  // Check if we'll be likely to be able to read the "static"
  // variables:
  assert(m_impl->m_inTree != nullptr);
  TBranch* br = m_impl->m_inTree->GetBranch(m_data.m_prefix.data());
  if (br == nullptr) {
    // We might not even have static branches, so this is not an error
    // by itself...
    return StatusCode::SUCCESS;
  }
  // In order to read complex objects, like smart pointers from an
  // auxiliary container variable-by-variable, the split level of the
  // branch must be exactly 1.
  if ((br->GetSplitLevel() != 1) && m_data.m_topStore && printWarnings) {
    ::Warning("xAOD::TAuxStore::readFrom",
              "Static branch (%s) with split level %i discovered",
              m_data.m_prefix.data(), br->GetSplitLevel());
    ::Warning("xAOD::TAuxStore::readFrom",
              "The reading of complex variables from it may/will fail!");
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// This function is called by the infrastructure to connect the object
/// to an output TTree.
///
/// @param tree Pointer to the TTree that is being written to
///
StatusCode TAuxStore::writeTo(::TTree& tree) {

  assert(m_impl);

  // Look for any auxiliary branches that have not been connected to yet:
  RETURN_CHECK("xAOD::TAuxStore::writeTo", m_impl->scanInputTree());

  // Store the TTree pointer:
  m_impl->m_outTree = &tree;

  // Create all the variables that we already know about. Notice that the
  // code makes a copy of the auxid set on purpose. Because the underlying
  // AuxSelection object gets modified while doing the for loop.
  const SG::auxid_set_t selAuxIDs = getSelectedAuxIDs();
  for (SG::auxid_t id : selAuxIDs) {
    RETURN_CHECK("xAOD::TAuxStore::writeTo", setupOutputData(id));
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

int TAuxStore::getEntry(int getall) {

  assert(m_impl);

  // Guard against multi-threaded execution:
  guard_t guard(m_impl->m_mutex);

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
    return 0;
  }

  // Get all the variables at once:
  int bytesRead = 0;
  for (auto& branchHandle : m_impl->m_branches) {
    if (branchHandle) {
      bytesRead += branchHandle->getEntry();
    }
  }
  return bytesRead;
}

void TAuxStore::reset() {

  assert(m_impl);

  for (auto& branchHandle : m_impl->m_branches) {
    if (branchHandle) {
      branchHandle->reset();
    }
  }
  m_impl->m_inputScanned = false;
}

bool TAuxStore::hasEntryFor(SG::auxid_t auxid) const {

  assert(m_impl);
  return ((m_impl->m_branches.size() > auxid) && m_impl->m_branches[auxid]);
}

StatusCode TAuxStore::getEntryFor(SG::auxid_t auxid) {

  assert(m_impl);
  assert(m_impl->m_branches.size() > auxid);
  assert(m_impl->m_branches[auxid]);
  const ::Int_t readBytes = m_impl->m_branches[auxid]->getEntry();
  if (readBytes < 0) {
    ::Error("xAOD::TAuxStore::getEntryFor",
            XAOD_MESSAGE("Couldn't read in variable %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

bool TAuxStore::hasOutput() const {

  assert(m_impl);
  return (m_impl->m_outTree != nullptr);
}

/// Connect a variable to the input tree
///
/// This internal function takes care of connecting to an individual
/// (sub-)branch in the input file for a given auxiliary variable. It
/// needs to handle a number of different use cases, so it's a bit long.
///
/// @param auxid The ID of the variable to connect to
/// @returns @c StatusCode::SUCCESS if the function was
///          successful, something else otherwise
///
StatusCode TAuxStore::setupInputData(SG::auxid_t auxid) {

  assert(m_impl);

  // Return right away if we already know that the branch is missing.
  if ((auxid < m_impl->m_missingBranches.size()) &&
      m_impl->m_missingBranches[auxid]) {
    return StatusCode::RECOVERABLE;
  }

  // Make sure the internal storage is large enough:
  if (m_data.m_vecs.size() <= auxid) {
    m_data.m_vecs.resize(auxid + 1);
  }
  if (m_impl->m_branches.size() <= auxid) {
    m_impl->m_branches.resize(auxid + 1);
  }

  // Check if we need to do anything:
  if (m_data.m_vecs[auxid] && m_impl->m_branches[auxid]) {
    return StatusCode::SUCCESS;
  }

  // A little sanity check.
  if (m_impl->m_inTree == nullptr) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("No input TTree set up!"));
    return StatusCode::FAILURE;
  }

  // Another sanity check.
  if (m_data.m_vecs[auxid] || m_impl->m_branches[auxid]) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Internal logic error!"));
    return StatusCode::FAILURE;
  }

  // Convenience access to the registry.
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  // Get the property name:
  const TString statBrName =
      std::format("{}{}", m_data.m_prefix, r.getName(auxid));
  const TString dynBrName =
      std::format("{}{}", m_data.m_dynPrefix, r.getName(auxid));

  // Check if the branch exists:
  Bool_t staticBranch = true;
  TString brName = statBrName;

  TBranch* br = m_impl->m_inTree->GetBranch(statBrName);
  if (!br) {
    br = m_impl->m_inTree->GetBranch(dynBrName);
    if (!br) {
      // Since TTree::GetBranch / TTObjArray::FindObject is expensive,
      // remember that we didn't find this branch in this file.
      if (m_impl->m_missingBranches.size() <= auxid) {
        m_impl->m_missingBranches.resize(auxid + 1);
      }
      m_impl->m_missingBranches[auxid] = true;
      // The branch doesn't exist, but this is not an error per se.
      // The user may just be calling isAvailable(...) on the variable.
      return StatusCode::RECOVERABLE;
    }
    // We have a dynamic branch:
    staticBranch = false;
    brName = dynBrName;
  }

  // Check if it's a "primitive branch":
  const Bool_t primitiveBranch = isPrimitiveBranch(*br);
  // Check if it's a "container branch":
  const Bool_t containerBranch =
      (primitiveBranch ? false : isContainerBranch(*br, auxid));

  // Set the structure mode if it has not been defined externally:
  if (m_data.m_structMode == EStructMode::kUndefinedStore) {
    m_data.m_structMode = (containerBranch ? EStructMode::kContainerStore
                                           : EStructMode::kObjectStore);
  }

  // Check that the branch type makes sense:
  if ((containerBranch &&
       (m_data.m_structMode != EStructMode::kContainerStore) &&
       !r.isLinked(auxid)) ||
      ((!containerBranch) &&
       (m_data.m_structMode != EStructMode::kObjectStore))) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Branch type and requested structure mode "
                         "differ for branch: %s"),
            brName.Data());
    return StatusCode::FAILURE;
  }

  // Check what variable it is:
  ::TClass* clDummy = 0;
  ::EDataType dType = kOther_t;
  if (br->GetExpectedType(clDummy, dType)) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Couldn't determine the type of branch \"%s\""),
            brName.Data());
    return StatusCode::FAILURE;
  }

  // Get the property type:
  const std::type_info* brType = 0;
  if (details::isRegisteredType(auxid)) {
    // Get the type from the auxiliary type registry:
    brType = (containerBranch ? r.getVecType(auxid) : r.getType(auxid));
  } else {
    // Get the type from the input branch itself:
    brType = (clDummy ? clDummy->GetTypeInfo() : &(Utils::getTypeInfo(dType)));
  }
  if (!brType) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Can't read/copy variable %s (%s)"), brName.Data(),
            clDummy->GetName());
    return StatusCode::RECOVERABLE;
  }
  const TString brTypeName = Utils::getTypeName(*brType).c_str();

  // Check if we have the needed dictionary for an object branch:
  ::TClass* brClass = 0;
  if (!primitiveBranch) {
    // Get the property's class:
    brClass = ::TClass::GetClass(*brType, true, true);
    if (!brClass) {
      brClass = ::TClass::GetClass(brTypeName);
    }
    if (!brClass) {
      ::Error("xAOD::TAuxStore::setupInputData",
              XAOD_MESSAGE("No dictionary available for class \"%s\""),
              brTypeName.Data());
      return StatusCode::FAILURE;
    }
  }

  // Create the smart object holding this vector:
  if (details::isRegisteredType(auxid)) {
    m_data.m_vecs[auxid] = r.makeVector(auxid, (size_t)0, (size_t)0);
    if (!containerBranch) {
      m_data.m_vecs[auxid]->resize(1);
    }
    if (clDummy &&
        strncmp(clDummy->GetName(), "SG::PackedContainer<", 20) == 0) {
      std::unique_ptr<SG::IAuxTypeVector> packed =
          m_data.m_vecs[auxid]->toPacked();
      std::swap(m_data.m_vecs[auxid], packed);
    }
  } else {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Couldn't create in-memory vector for "
                         "variable %s (%i)"),
            brName.Data(), static_cast<int>(auxid));
    return StatusCode::FAILURE;
  }

  // Create a new branch handle:
  const std::type_info* objType = brType;
  if (containerBranch) {
    objType = m_data.m_vecs[auxid]->objType();
    if (!objType)
      objType = r.getType(auxid);
  }
  m_impl->m_branches[auxid] = std::make_unique<TBranchHandle>(
      staticBranch, primitiveBranch, objType,
      (containerBranch ? m_data.m_vecs[auxid]->toVector()
                       : m_data.m_vecs[auxid]->toPtr()),
      auxid, m_data.m_prefix);

  // Set the tree/branch in the "right mode":
  if (staticBranch) {
    br->SetMakeClass();
  }

  // Connect to the branch:
  ::Int_t status = 0;
  if (clDummy && ::TString(clDummy->GetName()).Contains("basic_string<char>")) {
    // This is pretty much just a hack. As it happens, Athena I/O can
    // create dynamic branches that consider themselves to be of type
    // "vector<basic_string<char> >" and similar. (Instead of the
    // canonical "vector<string>" name.) When we encounter such a branch,
    // we just connect to it without performing any compatibility checks.
    // Since we don't need to apply any read rules in this case anyway.
    status = m_impl->m_inTree->SetBranchAddress(
        brName, m_impl->m_branches[auxid]->inputObjectPtr(),
        m_impl->m_branches[auxid]->branchPtr());
  } else {
    status = m_impl->m_inTree->SetBranchAddress(
        brName, m_impl->m_branches[auxid]->inputObjectPtr(),
        m_impl->m_branches[auxid]->branchPtr(), brClass, dType,
        ((!staticBranch) && (!primitiveBranch)));
  }
  if (status < 0) {
    ::Error("xAOD::TAuxStore::setupInputData",
            XAOD_MESSAGE("Coulnd't connect to branch \"%s\""), brName.Data());
    ::Error("xAOD::TAuxStore::setupInputData", XAOD_MESSAGE("Return code: %i"),
            status);
    m_data.m_vecs[auxid].reset();
    m_impl->m_branches[auxid].reset();
    return StatusCode::FAILURE;
  }

  // Get the current entry:
  m_impl->m_branches[auxid]->getEntry();

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
        ::Error("xAOD::TAuxStore::setupInputData",
                XAOD_MESSAGE("Internal logic error!"));
        continue;
      }
      // Okay, we do need to remove this object:
      m_data.m_vecs[i].reset();
      m_impl->m_branches[i].reset();
      m_data.m_auxIDs.erase(i);
    }
  }

  SG::auxid_t linked_auxid = r.linkedVariable(auxid);
  if (linked_auxid != SG::null_auxid) {
    return setupInputData(linked_auxid);
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

/// Connect a variable to the output tree
///
/// This function is used internally to create a "simple" output branch
/// with the contents of a single auxiliary variable.
///
/// @param auxid The ID of the variable to create an output branch for
/// @returns @c StatusCode::SUCCESS if the function was
///          successful, something else otherwise
///
StatusCode TAuxStore::setupOutputData(SG::auxid_t auxid) {

  assert(m_impl);

  // Check whether we need to do anything:
  if (!m_impl->m_outTree) {
    return StatusCode::SUCCESS;
  }

  // Check if the variable needs to be written out:
  if (!isAuxIDSelected(auxid)) {
    return StatusCode::SUCCESS;
  }

  // Make sure that containers are large enough:
  if (m_data.m_vecs.size() <= auxid) {
    m_data.m_vecs.resize(auxid + 1);
  }
  if (m_impl->m_branches.size() <= auxid) {
    m_impl->m_branches.resize(auxid + 1);
  }
  if (m_impl->m_branchesWritten.size() <= auxid) {
    m_impl->m_branchesWritten.resize(auxid + 1);
  }

  // Check if this auxiliary variable is already in the output:
  if (m_impl->m_branchesWritten[auxid]) {
    return StatusCode::SUCCESS;
  }

  // The registry:
  SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

  // Check if the variable was put into the transient store as a
  // decoration, and now needs to be put into the output file:
  if ((!m_data.m_vecs[auxid]) && m_data.m_transientStore &&
      (m_data.m_transientStore->getAuxIDs().test(auxid))) {

    // Get the variable from the transient store:
    const void* pptr = m_data.m_transientStore->getData(auxid);
    if (!pptr) {
      ::Fatal("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("Internal logic error detected"));
      return StatusCode::FAILURE;
    }

    // Create the new object:
    m_data.m_vecs[auxid] = reg.makeVector(auxid, m_data.m_size, m_data.m_size);
    void* ptr = m_data.m_vecs[auxid]->toPtr();
    if (!ptr) {
      ::Error("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("Couldn't create decoration in memory "
                           "for writing"));
      return StatusCode::FAILURE;
    }

    // Get the type of this variable:
    const std::type_info* type = reg.getType(auxid);
    if (!type) {
      ::Error("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("Couldn't get the type of transient "
                           "variable %i"),
              static_cast<int>(auxid));
      return StatusCode::FAILURE;
    }
    // Now get the factory for this variable:
    const SG::IAuxTypeVectorFactory* factory = reg.getFactory(auxid);
    if (!factory) {
      ::Error("xAOD::TAuxStore::setupOutputData",
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

    // Finally, do the copy:
    factory->copy(auxid, SG::AuxVectorInterface(*this), 0,
                  SG::AuxVectorInterface(*m_data.m_transientStore), 0,
                  m_data.m_size);
  }

  // Check if we know about this variable to be on the input,
  // but haven't connected to it yet:
  if ((m_data.m_auxIDs.test(auxid)) && (!m_data.m_vecs[auxid]) &&
      (!m_impl->m_branches[auxid])) {
    RETURN_CHECK("xAOD::TAuxStore::setupOutputData", setupInputData(auxid));
  }

  // Check that we know the store's type:
  if ((m_data.m_structMode != EStructMode::kContainerStore) &&
      (m_data.m_structMode != EStructMode::kObjectStore)) {
    ::Error("xAOD::TAuxStore::setupOutputData",
            XAOD_MESSAGE("Structure mode unknown for variable %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return StatusCode::FAILURE;
  }

  // Check if the variable exists already in memory:
  if (!m_data.m_vecs[auxid]) {
    m_data.m_vecs[auxid] =
        SG::AuxTypeRegistry::instance().makeVector(auxid, (size_t)0, (size_t)0);
    if (m_data.m_structMode == EStructMode::kObjectStore) {
      m_data.m_vecs[auxid]->resize(1);
    }
  }

  // Check if the branch handle exists already:
  if (!m_impl->m_branches[auxid]) {
    // Get the property type:
    const std::type_info* brType =
        (m_data.m_structMode == EStructMode::kContainerStore
             ? SG::AuxTypeRegistry::instance().getVecType(auxid)
             : SG::AuxTypeRegistry::instance().getType(auxid));
    // Create the handle object:
    bool primitiveBranch = (strlen(brType->name()) == 1);
    m_impl->m_branches[auxid] = std::make_unique<TBranchHandle>(
        false, (strlen(brType->name()) == 1),
        (primitiveBranch ? brType : m_data.m_vecs[auxid]->objType()),
        (m_data.m_structMode == EStructMode::kObjectStore
             ? m_data.m_vecs[auxid]->toPtr()
             : m_data.m_vecs[auxid]->toVector()),
        auxid, m_data.m_prefix);
  }

  // Construct a name for the branch:
  const TString brName =
      std::format("{}{}", m_data.m_dynPrefix,
                  SG::AuxTypeRegistry::instance().getName(auxid));

  // If the output branch exists already, assume that it was us making
  // it:
  ::TBranch* br = m_impl->m_outTree->GetBranch(brName);
  if (br) {
    // Apparently a branch that was already set up for copying as a basic
    // variable, now got accessed explicitly. So let's update the output
    // branch to point to this new location now.
    br->SetAddress(m_impl->m_branches[auxid]->outputObjectPtr());
    // Update the cache. Notice that the "write status" of the typeless
    // auxiliary ID is not turned off. But it shouldn't matter, as the
    // variable will not be accessed in a typeless way anymore.
    m_impl->m_branchesWritten[auxid] = true;
    // Return gracefully:
    return StatusCode::SUCCESS;
  }

  // Check that we know the type of the branch:
  const std::type_info* brType = m_impl->m_branches[auxid]->typeInfo();
  if (!brType) {
    ::Error("xAOD::TAuxStore::setupOutputData",
            XAOD_MESSAGE("There's an internal logic error in the "
                         "code"));
    return StatusCode::FAILURE;
  }
  const std::string brTypeName = Utils::getTypeName(*brType);

  // Decide if this is a primitive branch:
  const Bool_t primitiveBranch = (strlen(brType->name()) == 1);

  // Let's create the branch now:
  if (primitiveBranch) {

    // Get the "ROOT type" belonging to this primitive:
    const char rootType = Utils::rootType(brType->name()[0]);
    if (rootType == '\0') {
      ::Error("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("Type not known for variable \"%s\" "
                           "of type \"%s\""),
              brName.Data(), brTypeName.c_str());
      return StatusCode::FAILURE;
    }

    // Construct the type description:
    std::ostringstream typeDesc;
    typeDesc << brName << "/" << rootType;

    // Create the branch:
    br = m_impl->m_outTree->Branch(
        brName, m_impl->m_branches[auxid]->outputObjectPtr(),
        typeDesc.str().c_str(), m_impl->m_basketSize);

  } else {

    // Access the dictionary for the type:
    TClass* cl = TClass::GetClass(*brType);
    if (!cl) {
      cl = TClass::GetClass(brTypeName.c_str());
    }
    if (!cl) {
      ::Error("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("Couldn't find dictionary for type: %s"),
              brTypeName.c_str());
      return StatusCode::FAILURE;
    }
    if (!cl->GetStreamerInfo()) {
      ::Error("xAOD::TAuxStore::setupOutputData",
              XAOD_MESSAGE("No streamer info available for type %s"),
              cl->GetName());
      return StatusCode::FAILURE;
    }

    // Create the branch:
    br = m_impl->m_outTree->Branch(brName, cl->GetName(),
                                   m_impl->m_branches[auxid]->outputObjectPtr(),
                                   m_impl->m_basketSize, m_impl->m_splitLevel);
  }

  // Check if we succeeded:
  if (!br) {
    ::Error("xAOD::TAuxStore::setupOutputData",
            XAOD_MESSAGE("Failed creating branch \"%s\" of type "
                         "\"%s\""),
            brName.Data(), brTypeName.c_str());
    return StatusCode::FAILURE;
  }

  // If this is not the first event, fill up the branch with dummy
  // info:
  for (Long64_t i = 0; i < m_impl->m_outTree->GetEntries(); ++i) {
    br->Fill();
  }

  // Update the cache:
  m_impl->m_branchesWritten[auxid] = true;

  // Also, remember that we now handle this variable:
  m_data.m_auxIDs.insert(auxid);

  // We were successful:
  return StatusCode::SUCCESS;
}

const void* TAuxStore::getInputObject(SG::auxid_t auxid) const {

  assert(m_impl);
  assert(m_impl->m_branches.size() > auxid);
  assert(m_impl->m_branches[auxid]);
  return m_impl->m_branches[auxid]->objectPtr();
}

const std::type_info* TAuxStore::getInputType(SG::auxid_t auxid) const {

  assert(m_impl);
  assert(m_impl->m_branches.size() > auxid);
  assert(m_impl->m_branches[auxid]);
  return m_impl->m_branches[auxid]->typeInfo();
}

}  // namespace xAOD
