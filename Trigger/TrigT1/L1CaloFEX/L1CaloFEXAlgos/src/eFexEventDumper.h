/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexEventDumper  -  description:
//     An algorithm for debugging eFex simulation/hw mismatches by dumping out events to ROOT files.
//     Should be used for debugging individual events - this algorithm should not be used in standard transforms.
//     Each event is dumped to an individual interactive canvas in the output file.
//     Thread-safety has not been considered in its design - should be used in single-thread applications.
//                              -------------------
//     begin                : 24 04 2025
//     email                : will@cern.ch
//***************************************************************************/

#ifndef eFexEventDumper_H
#define eFexEventDumper_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "TFile.h"


namespace LVL1 {

    class eFexEventDumper : public AthReentrantAlgorithm
    {
      public:

        eFexEventDumper(const std::string& name, ISvcLocator* pSvcLocator);
        ~eFexEventDumper() = default;

        virtual StatusCode initialize();
        virtual StatusCode execute(const EventContext& ctx) const;

      private:

        Gaudi::Property<std::string> m_outputFileName {this, "OutputFile", "eFexEvents.root", "Name of the output file to dump to"};
        SG::ReadHandleKey<xAOD::eFexTowerContainer> m_towerKey {this, "TowersKey", "", "Name of the eFexTowers to dump, if any"};
        SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_emTOBKey {this, "EMRoIKey", "", "Name of the eFexEMRoIContainer to dump, if any"};
        SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_tauTOBKey {this, "TauRoIKey", "", "Name of the eFexTauRoIContainer to dump, if any"};
        SG::ReadCondHandleKey<CondAttrListCollection> m_noiseCutsKey{this,"NoiseCutsKey","/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts",
                                                                     "Key to noise cuts (AttrListCollection)"};

        std::shared_ptr<TFile> m_file;

    };

} // end of LVL1 namespace
#endif
