/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/src/getDynamicAuxID.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2016
 * @brief Find the auxid for a dynamic branch.
 */


#include "AthContainersRoot/getDynamicAuxID.h"
#include "AthContainersRoot/RootAuxVectorFactory.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "TClass.h"
#include "TROOT.h"


namespace SG {


/**
 * @brief Look up a @c TClass given a name.  Try to avoid autoparsing.
 * @param cname Name of the class to find.
 */
TClass* getClassIfDictionaryExists (const std::string& cname)
{
  if (TClass* cl = (TClass*)gROOT->GetListOfClasses()->FindObject(cname.c_str())) {
    if (cl->IsLoaded() && cl->HasDictionary()) return cl;
    return nullptr;
  }

  // The idea of calling GetClassSharedLibs was to test if a class was
  // listed in the rootmap file, so we could avoid autoparsing.
  // This worked with older versions of root 6.
  // But now root has started doing dynamic linking itself,
  // and GetClassSharedLibs will itself trigger autoparsing.
  // So currently, this test does more harm than good.
  // Still need to see if we can find a reliable way of failing
  // a TClass lookup rather than triggering autoparsing.
  //if (gInterpreter->GetClassSharedLibs (cname.c_str()))
  {
    TClass* cl = TClass::GetClass (cname.c_str());
    if (cl->HasDictionary())
      return cl;
  }
  return nullptr;
}


/**
 * @brief Find the auxid for a dynamic branch.
 * @param ti Type of the auxiliary variable.
 *           Usually the type of the vector payload, but if @c standalone
 *           is true, then this is the type of the stored object.
 * @param name Auxiliary variable name.
 * @param elementTypeName Name of the type for one aux data element.
 *                        Should be the same as @c branchTypeName
 *                        if @c standalone is true.
 * @param branchTypeName Name of the type for this branch.
 * @param standalone True if this is a standalone object.
 */
SG::auxid_t getDynamicAuxID (const std::type_info& ti,
                             const std::string& name,
                             const std::string& elementTypeName,
                             const std::string& branchTypeName,
                             bool standalone)
{
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t auxid = SG::null_auxid;

  auxid = r.getAuxID (ti, name, "", SG::AuxTypeRegistry::Flags::SkipNameCheck);
  if (auxid != SG::null_auxid) return auxid;

  // Be careful --- if we don't exactly match the name
  // in TClassTable, then we may trigger autoparsing.  Besides the
  // resource usage that implies, that can lead to crashes in dbg
  // builds due to cling bugs.
  std::string tn = elementTypeName;
  if (tn.starts_with("std::vector<"))
    tn.erase (0, 5);
  std::string fac_class_name = "SG::AuxTypeVectorFactory<" +
    tn + ",allocator<" + tn;
  if (fac_class_name[fac_class_name.size()-1] == '>')
    fac_class_name += ' ';
      fac_class_name += "> >";
  TClass* fac_class = getClassIfDictionaryExists (fac_class_name);
  if (fac_class)
  {
    TClass* base_class = getClassIfDictionaryExists ("SG::IAuxTypeVectorFactory");
    if (base_class) {
      int offs = fac_class->GetBaseClassOffset (base_class);
      if (offs >= 0) {
        void* fac_vp = fac_class->New();
        if (fac_vp) {
          SG::IAuxTypeVectorFactory* fac = reinterpret_cast<SG::IAuxTypeVectorFactory*> (reinterpret_cast<unsigned long>(fac_vp) + offs);
          const std::type_info* tiAlloc = fac->tiAlloc();
          r.addFactory (ti, *tiAlloc, std::unique_ptr<SG::IAuxTypeVectorFactory> (fac));
          auxid = r.getAuxID(*fac->tiAlloc(), ti, name);
        }
      }
    }
  }

  if (auxid == SG::null_auxid) {
    std::string vec_name = branchTypeName;
    if (standalone) {
      vec_name = "std::vector<" + branchTypeName;
      if (vec_name[vec_name.size()-1] == '>')
        vec_name += " ";
      vec_name += ">";
    }
    TClass* vec_class = TClass::GetClass (vec_name.c_str());

    if (vec_class) {
      auto facp = std::make_unique<SG::RootAuxVectorFactory> (vec_class);
      std::string tiAllocName = facp->tiAllocName();
      (void)r.addFactory (ti, tiAllocName, std::move (facp));
      auxid = r.getAuxID(tiAllocName, ti, name);
    }
  }

  return auxid;
}


} // namespace SG
