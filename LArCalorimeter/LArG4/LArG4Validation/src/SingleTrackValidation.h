/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4VALIDATION_SINGLETRACKVALIDATION_H
#define LARG4VALIDATION_SINGLETRACKVALIDATION_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/ITHistSvc.h"
// For MC Truth information:
#include "GeneratorObjects/McEventCollection.h"

#include "TH1F.h"

class SingleTrackValidation : public AthAlgorithm {

public:

  SingleTrackValidation(const std::string & name, ISvcLocator *pSvcLocator);
  ~SingleTrackValidation();
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:

  SG::ReadHandleKey<McEventCollection> m_truthKey { this, "TruthKey", "TruthEvent" };

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
    , "CaloDetDescrManager"
    , "CaloDetDescrManager"
    , "SG Key for CaloDetDescrManager in the Condition Store" };

  // Read handle for conditions object to get the field cache
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this
    , "AtlasFieldCacheCondObj"
    , "fieldCondObj"
    , "Name of the Magnetic Field conditions object key"};

 ServiceHandle<IPartPropSvc> m_ppSvc{this, "PartPropSvc", "PartPropSvc"};
  ServiceHandle<ITHistSvc> m_histSvc { this, "THistSvc", "THistSvc", "Histogramming svc" };
  class Clockwork;
  Clockwork *m_c;

  TH1F* m_histos[162]{};

  SingleTrackValidation (const SingleTrackValidation&);
  SingleTrackValidation& operator= (const SingleTrackValidation&);
};

#endif
