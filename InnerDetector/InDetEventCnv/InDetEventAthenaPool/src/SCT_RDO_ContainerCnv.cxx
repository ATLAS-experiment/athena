/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_RDO_ContainerCnv.h"

#include "InDetIdentifier/SCT_ID.h"

#include <memory>

#include <iostream>


//================================================================
namespace {
#ifdef SCT_DEBUG
  std::string shortPrint(const SCT_RDO_Container* main_input_SCT, unsigned maxprint=25) {
    std::ostringstream os;
    if (main_input_SCT) {
      for (unsigned i=0; i<maxprint; i++) {
        const auto* p = main_input_SCT->indexFindPtr(i);
        if (p != nullptr) {
          os<<" "<< p->size();
        }
        else {
          os<<" *";
        }
      }
    }
    else {
      os<<" [SCT_RDO_Container==nullptr]";
    }
    return os.str();
  }
#endif
}

SCT_RDO_ContainerCnv::SCT_RDO_ContainerCnv (ISvcLocator* svcloc)
  : SCT_RDO_ContainerCnvBase(svcloc, "SCT_RDO_ContainerCnv"),
    m_converter_p0()
{}

//================================================================
StatusCode SCT_RDO_ContainerCnv::initialize() {
  ATH_CHECK( SCT_RDO_ContainerCnvBase::initialize() );

  ATH_MSG_DEBUG("SCT_RDO_ContainerCnv::initialize()");

  // Get the sct helper from the detector store
  const SCT_ID* idhelper(nullptr);
  ATH_CHECK( detStore()->retrieve(idhelper, "SCT_ID") );

  m_converter_p0.initialize(idhelper);
  m_converter_TP1.initialize(idhelper);
  m_converter_SCT_TP1.initialize(idhelper);
  m_converter_SCT_TP2.initialize(idhelper);
  m_converter_SCT_TP3.initialize(idhelper);
  m_converter_SCT_TP4.initialize(idhelper);
  m_converter_PERS.initialize(idhelper);

  return StatusCode::SUCCESS;
}

//================================================================
SCT_RDO_Container_PERS* SCT_RDO_ContainerCnv::createPersistent(SCT_RDO_Container* transCont) {

#ifdef SCT_DEBUG
  ATH_MSG_DEBUG("createPersistent(): main converter. TRANS = "<<shortPrint(transCont));
#endif
  // converter_num  is a switch to determine which persistent type to use
  // 1: if concrete type private data is equivalent to InDetRawData_p1
  // 3: for cosmic/TB 
  //
  unsigned int converter_num(1);
  SCT_RDO_Container::const_iterator it_Coll     = transCont->begin();
  SCT_RDO_Container::const_iterator it_CollEnd  = transCont->end();
  // check one element of the container. The container can't be empty for this... 
  if (it_Coll != it_CollEnd) {
    while (it_Coll != it_CollEnd && (*it_Coll)->size() == 0) ++it_Coll;
    if (it_Coll != it_CollEnd) {
      const SCT_RDORawData* test((**it_Coll)[0]);
      if (dynamic_cast<const SCT1_RawData*>(test) != nullptr) {
        //ATH_MSG_DEBUG("Found container with SCT1_RawData concrete type objects");
        converter_num=1;
      } else if (dynamic_cast<const SCT3_RawData*>(test) != nullptr) {
        //ATH_MSG_DEBUG("Found container with SCT3_RawData concrete type objects");
        converter_num=3;
      } else {
        ATH_MSG_FATAL("Converter not implemented for this concrete type ");
        throw "Converter not implemented";
      }
    } else {
      ATH_MSG_WARNING("Container has only empty collections. Using TP1 converter");
    } 
  } else {
    ATH_MSG_WARNING("Empty container. Using TP1 converter");
  }
  // Switch facility depending on the concrete data type of the contained objects
  // Should do by getting the type_info of one of the contained objects
  SCT_RDO_Container_PERS* persObj(nullptr);
  if (converter_num == 1 || converter_num == 3) {
    m_converter_PERS.setType(converter_num);
    persObj = m_converter_PERS.createPersistent( transCont, msg() );
  } else {
    ATH_MSG_FATAL("This shouldn't happen!! ");
  } 
#ifdef SCT_DEBUG
  ATH_MSG_DEBUG("Success");
#endif
  return persObj; 
}
    
//================================================================
SCT_RDO_Container* SCT_RDO_ContainerCnv::createTransient(const Token* token) {

  static const Guid   p0_guid("B82A1D11-3F86-4F07-B380-B61BA2DAF3A9"); // with SCT1_RawData
  static const Guid   TP1_guid("DA76970C-E019-43D2-B2F9-25660DCECD9D"); // for t/p separated version with InDetRawDataContainer_p1
  static const Guid   SCT_TP1_guid("8E13963E-13E5-4D10-AA8B-73F00AFF8FA8"); // for t/p separated version with SCT_RawDataContainer_p1
  static const Guid   SCT_TP2_guid("D1258125-2CBA-476E-8578-E09D54F477E1"); // for t/p separated version with SCT_RawDataContainer_p2
  static const Guid   SCT_TP3_guid("5FBC8D4D-7B4D-433A-8487-0EA0C870CBDB"); // for t/p separated version with SCT_RawDataContainer_p3
  static const Guid   SCT_TP4_guid("6C7540BE-E85C-4777-BC1C-A9FF11460F54"); // for t/p separated version with SCT_RawDataContainer_p4

#ifdef SCT_DEBUG
  ATH_MSG_DEBUG("createTransient(const Token* token): main converter");
#endif
  if ( compareClassGuid(token, SCT_TP4_guid) ) {
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token): New TP version - TP4 branch");
#endif

    std::unique_ptr< SCT_RawDataContainer_p4 >   col_vect( poolReadObject< SCT_RawDataContainer_p4 >(token) );
    SCT_RDO_Container* res = m_converter_SCT_TP4.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), TP4 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;

  }
  else if ( compareClassGuid(token, SCT_TP3_guid) ) {
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token): New TP version - TP3 branch");
#endif

    std::unique_ptr< SCT_RawDataContainer_p3 >   col_vect( poolReadObject< SCT_RawDataContainer_p3 >(token) );
    SCT_RDO_Container* res = m_converter_SCT_TP3.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), TP3 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;

  }
  else if ( compareClassGuid(token, SCT_TP2_guid) ) {
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token): New TP version - TP2 branch");
#endif

    std::unique_ptr< SCT_RawDataContainer_p2 >   col_vect( poolReadObject< SCT_RawDataContainer_p2 >(token) );
    SCT_RDO_Container* res = m_converter_SCT_TP2.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), TP2 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;

  }
  else if ( compareClassGuid(token, SCT_TP1_guid) ) {
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token): New TP version - TP1 branch");
#endif
    std::unique_ptr< SCT_RawDataContainer_p1 >   col_vect( poolReadObject< SCT_RawDataContainer_p1 >(token) );
    SCT_RDO_Container* res = m_converter_SCT_TP1.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), TP1 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;


  }
  else if ( compareClassGuid(token, TP1_guid) ) {
    ATH_MSG_DEBUG("createTransient(const Token* token): New TP version - TP1 branch");
                                                                                                                                                             
    std::unique_ptr< InDetRawDataContainer_p1 >   col_vect( poolReadObject< InDetRawDataContainer_p1 >(token) );
    SCT_RDO_Container* res = m_converter_TP1.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), TP1 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;


  }
  else if ( compareClassGuid(token, p0_guid) ) {
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token): Old input file - p0 branch");
#endif
    std::unique_ptr< SCT_RDO_Container_p0 >   col_vect( poolReadObject< SCT_RDO_Container_p0 >(token) );
    SCT_RDO_Container* res = m_converter_p0.createTransient( col_vect.get(), msg() );
#ifdef SCT_DEBUG
    ATH_MSG_DEBUG("createTransient(const Token* token), p0 branch: returns TRANS = "<<shortPrint(res));
#endif
    return res;
  }

  throw std::runtime_error("Unsupported persistent version of SCT_RDO_Container");
}

//================================================================
