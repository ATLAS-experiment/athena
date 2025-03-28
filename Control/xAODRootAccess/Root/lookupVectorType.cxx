// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// ROOT include(s).
#include <TClass.h>
#include <TROOT.h>

// System include(s).
#include <format>

namespace xAOD::details {

TClass* lookupVectorType(TClass& cl) {

  TDataType* typ =
      gROOT->GetType(std::format("{}::vector_type", cl.GetName()).c_str());
  if (typ) {
    return TClass::GetClass(typ->GetFullTypeName());
  }
  return nullptr;
}

}  // namespace xAOD::details
