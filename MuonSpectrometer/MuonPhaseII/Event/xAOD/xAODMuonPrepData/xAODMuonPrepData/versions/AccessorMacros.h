/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_ACCESSOR_MACROS_H
#define XAODMUONPREPDATA_ACCESSOR_MACROS_H
#include "GeoModelHelpers/throwExcept.h"
/**
 *  Macros to implement the scalar variables of the xAOD::MuonPrepData objects
*/
#define IMPLEMENT_SETTER_GETTER( CLASS_NAME, DTYPE, GETTER, SETTER)            \
    DTYPE CLASS_NAME::GETTER() const {                                         \
       static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
       return acc(*this);                                                      \
    }                                                                          \
                                                                               \
    void CLASS_NAME::SETTER(DTYPE value) {                                     \
       static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
       acc(*this) = value;                                                     \
    }
/**
 *  Macro to implement the scalar variable of the xAOD::MuonPrepData object which
 *  is then casted to an enum type
*/
#define IMPLEMENT_SETTER_GETTER_WITH_CAST( CLASS_NAME, STORE_DTYPE, CAST_DTYPE, GETTER, SETTER) \
    CAST_DTYPE CLASS_NAME::GETTER() const {                                                     \
       static const SG::AuxElement::Accessor<STORE_DTYPE> acc{preFixStr + #GETTER};             \
       return static_cast<CAST_DTYPE>(acc(*this));                                              \
    }                                                                                           \
                                                                                                \
    void CLASS_NAME::SETTER(CAST_DTYPE value) {                                                 \
       static const SG::AuxElement::Accessor<STORE_DTYPE> acc{preFixStr + #GETTER};             \
       acc(*this) = static_cast<STORE_DTYPE>(value);                                            \
    }  
/**
 *  Macro to implement vector like variables of the xAOD::MuonPrepData objects 
*/
#define IMPLEMENT_VECTOR_SETTER_GETTER(CLASS_NAME, DTYPE, GETTER, SETTER)                    \
      const std::vector<DTYPE>& CLASS_NAME::GETTER() const {                                 \
         static const SG::AuxElement::Accessor<std::vector<DTYPE>> acc{preFixStr + #GETTER}; \
         return acc(*this);                                                                  \
      }                                                                                      \
                                                                                             \
      void CLASS_NAME::SETTER(const std::vector<DTYPE>& value) {                             \
         static const SG::AuxElement::Accessor<std::vector<DTYPE>> acc{preFixStr + #GETTER}; \
         acc(*this) = value;                                                                 \
      }
/**
 *  Macro to handle the readoutElement. If the object is created within the RDO -> Prd conversion
 *  the method simply returns the pointer to the given readoutElement. In contrast, if the object is 
 *  created from disk, the readoutElement link is not automatically restored. At the first time, when
 *  method is called, the MuonDetectorManager is retrieved from the detector store and the associated
 *  readout element is fetched. In Phase II, alignment constants are parsed via the GeometryContext, hence
 *  there only exists one instance of the MuonDetectorManager and there's no problem with the scheduler
*/
#define IMPLEMENT_READOUTELEMENT(CLASS_NAME, CACHED_VALUE, READOUT_ELEMENT_TYPE)                \
    void CLASS_NAME::setReadoutElement(const MuonGMR4::READOUT_ELEMENT_TYPE* readoutEle) {      \
        CACHED_VALUE.set(readoutEle);                                                           \
    }                                                                                           \
                                                                                                \
    const MuonGMR4::READOUT_ELEMENT_TYPE* CLASS_NAME::readoutElement() const {                  \
        if (!CACHED_VALUE.isValid()) {                                                          \
            ServiceHandle<StoreGateSvc> service{"DetectorStore", #CLASS_NAME};                  \
            const MuonGMR4::MuonDetectorManager* detMgr{};                                      \
            if (!service.retrieve().isSuccess() ||                                              \
                !service->retrieve(detMgr).isSuccess()){                                        \
                THROW_EXCEPTION("Failed to retrieve the Run4 muon detector manager. "<<         \
                         "Please schedule the MuonGeometry in your job");                       \
            }                                                                                   \
            const IdentifierHash hash{identifierHash()};                                        \
            const MuonGMR4::READOUT_ELEMENT_TYPE* re = detMgr->get##READOUT_ELEMENT_TYPE(hash); \
            if (!re) {                                                                          \
                const Identifier id{static_cast<Identifier::value_type>(identifier())};         \
                THROW_EXCEPTION(detMgr->idHelperSvc()->toString(id)                             \
                            <<" does not have a readout element.");                             \
            }                                                                                   \
            CACHED_VALUE.set(re);                                                               \
        }                                                                                       \
        return (*CACHED_VALUE.ptr());                                                           \
    }

/**
 *  Macro to declare the variables belonging to the AuxElementContainer
*/
#define PRD_AUXVARIABLE(VAR)                                \
   do {                                                     \
      static const std::string varName =preFixStr+#VAR;     \
      static const auxid_t auxid = getAuxID(varName, VAR);  \
      regAuxVar(auxid, varName, VAR);                       \
    } while (false);                                        

#endif