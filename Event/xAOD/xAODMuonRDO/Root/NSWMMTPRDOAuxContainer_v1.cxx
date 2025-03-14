/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODMuonRDO/versions/NSWMMTPRDOAuxContainer_v1.h"

namespace {
   static const std::string preFixStr {"NSWMMTP_"};
}
#define TPAUX_VARIABLE(VAR) \
   do { \
      static const std::string varName =preFixStr+#VAR; \
      static const auxid_t auxid = getAuxID(varName, VAR); \
      regAuxVar(auxid, varName, VAR); \
    } while (false);

namespace xAOD {
    NSWMMTPRDOAuxContainer_v1::NSWMMTPRDOAuxContainer_v1()
    : AuxContainerBase() { 
      //ROD info
      TPAUX_VARIABLE(sourceID);
      TPAUX_VARIABLE(moduleID);
      TPAUX_VARIABLE(ROD_L1ID);
      TPAUX_VARIABLE(ROD_BCID);
      
      //TP head
      TPAUX_VARIABLE(EC);
      TPAUX_VARIABLE(sectID);
      TPAUX_VARIABLE(L1ID);
      TPAUX_VARIABLE(BCID);
      
      //TP L1A head
      TPAUX_VARIABLE(l1a_request_BCID);
      TPAUX_VARIABLE(l1a_release_BCID);
      TPAUX_VARIABLE(l1a_window_open);
      TPAUX_VARIABLE(l1a_window_center);
      TPAUX_VARIABLE(l1a_window_close);
      TPAUX_VARIABLE(l1a_window_open_offset);
      TPAUX_VARIABLE(l1a_window_center_offset);
      TPAUX_VARIABLE(l1a_window_close_offset);

      //L1A data quality 
      TPAUX_VARIABLE(l1a_timeout);
      TPAUX_VARIABLE(l1a_engines);
      //ART data
      TPAUX_VARIABLE(art_BCID);
      TPAUX_VARIABLE(art_layer);
      TPAUX_VARIABLE(art_channel);
      
      //trigger data
      TPAUX_VARIABLE(trig_globalX);
      TPAUX_VARIABLE(trig_globalU);
      TPAUX_VARIABLE(trig_BCID);
      TPAUX_VARIABLE(trig_dTheta);
      TPAUX_VARIABLE(trig_ROI_rID);
      TPAUX_VARIABLE(trig_ROI_phiID);      
    }
}
#undef TPAUX_VARIABLE
