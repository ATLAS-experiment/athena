/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @file StlAthenaPoolCnv.cxx
/// @brief Template-based AthenaPool converters for STL containers and built-in types
///
/// This file provides explicit instantiations of T_AthenaPoolCnv for:
/// - Built-in types (int, unsigned int, float, double, string)
/// - STL vectors of built-in types
/// - STL maps with built-in key/value types

#include "AthenaPoolCnvSvc/T_AthenaPoolCnv.h"

template <class T>
class StlAthenaPoolCnv : public T_AthenaPoolCnv<T>
{
  friend class CnvFactory< StlAthenaPoolCnv<T> >;
  using BaseCnv_t = T_AthenaPoolCnv<T>;
  using Self_t = StlAthenaPoolCnv<T>;
public:
  /// Standard constructor
  /// @param svcloc Pointer to the service locator
  explicit StlAthenaPoolCnv(ISvcLocator* svcloc) : BaseCnv_t(svcloc) {}

  /// Gaudi Service Interface method implementations:
  virtual StatusCode initialize() override
  {
    ATH_CHECK( this->BaseCnv_t::initialize() );

    RootType rflx_type{typeid(T)};
    if (!rflx_type) {
      ATH_MSG_ERROR("Could not get RootType from type_info ["
                    << typeid(T).name()
                    << "] for class ["
                    << ClassName<T>::name() << "]!");
      return StatusCode::FAILURE;
    }
    this->BaseCnv_t::m_classDesc = std::move(rflx_type);
    return StatusCode::SUCCESS;
  }
};


#define DECL_CNV(NAME, TDEF) \
  typedef StlAthenaPoolCnv< NAME > TDEF; \
  template class StlAthenaPoolCnv< NAME >; \
  DECLARE_CONVERTER(TDEF)

#define DECL2_CNV(N1, N2, TDEF) \
  typedef StlAthenaPoolCnv< N1, N2 > TDEF; \
  template class StlAthenaPoolCnv< N1, N2 >; \
  DECLARE_CONVERTER(TDEF)

#include "SGTools/BuiltinsClids.h"
// cppcheck-suppress unknownMacro
DECL_CNV(int, AthenaPoolIntCnv)
DECL_CNV(unsigned int, AthenaPoolUIntCnv)
DECL_CNV(float, AthenaPoolFloatCnv)
DECL_CNV(double, AthenaPoolDoubleCnv)
DECL_CNV(std::string, AthenaPoolStdStringCnv)

#include "SGTools/StlVectorClids.h"
DECL_CNV(std::vector<int>, AthenaPoolStdVectorIntCnv)
DECL_CNV(std::vector<unsigned int>, AthenaPoolStdVectorUIntCnv)
DECL_CNV(std::vector<float>, AthenaPoolStdVectorFloatCnv)
DECL_CNV(std::vector<double>, AthenaPoolStdVectorDoubleCnv)

#include "SGTools/StlMapClids.h"
DECL2_CNV(std::map<int, int>, AthenaPoolStdMapIntIntCnv)
DECL2_CNV(std::map<int, float>, AthenaPoolStdMapIntFloatCnv)
DECL2_CNV(std::map<int, double>, AthenaPoolStdMapIntDoubleCnv)

DECL2_CNV(std::map<std::string, int>, AthenaPoolStdMapStringIntCnv)
DECL2_CNV(std::map<std::string, unsigned int>, AthenaPoolStdMapStringUIntCnv)
DECL2_CNV(std::map<std::string, float>, AthenaPoolStdMapStringFloatCnv)
DECL2_CNV(std::map<std::string, double>, AthenaPoolStdMapStringDoubleCnv)

