// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODEVENTINFOCNV_EVENTINFOBEAMSPOTDECORATORALG_H
#define XAODEVENTINFOCNV_EVENTINFOBEAMSPOTDECORATORALG_H

// EDM include(s).
#include "xAODEventInfo/EventInfo.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

// Gaudi/Athena include(s).
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace xAODMaker {

   /// Algorithm for filling the beam position variables on @c xAOD::EventInfo
   ///
   /// These variables need to be filled as decorations for data. This algorithm
   /// takes care of doing that.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class EventInfoBeamSpotDecoratorAlg : public AthReentrantAlgorithm {

   public:
      // Inherit the base class's constructor.
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      /// @name Function(s) implemented from @c AthReentrantAlgorithm
      /// @{

      /// Function initialising the algorithm
      virtual StatusCode initialize() override;
      /// Function executing the algorithm
      virtual StatusCode execute( const EventContext& ctx ) const override;

      /// @}

   private:
      /// @name Algorithm properties
      /// @{

      /// Conditions object with the beamspot information
      SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{ this,
         "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

     /// Read handle for EventInfo object to be decorated
      SG::ReadHandleKey< xAOD::EventInfo > m_eventInfoKey{ this,
         "EventInfoKey", "EventInfo" };

     /// Decorator handle for @c beamPosX
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosXKey{ this,
         "beamPosXKey", m_eventInfoKey, "beamPosX",
         "Key for the beamPosX decoration" };
      /// Decorator handle for @c beamPosY
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosYKey{ this,
         "beamPosYKey", m_eventInfoKey, "beamPosY",
         "Key for the beamPosY decoration" };
      /// Decorator handle for @c beamPosZ
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosZKey{ this,
         "beamPosZKey", m_eventInfoKey, "beamPosZ",
         "Key for the beamPosZ decoration" };

      /// Decorator handle for @c beamPosSigmaX
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosSigmaXKey{ this,
         "beamPosSigmaXKey", m_eventInfoKey, "beamPosSigmaX",
         "Key for the beamPosSigmaX decoration" };
      /// Decorator handle for @c beamPosSigmaY
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosSigmaYKey{ this,
         "beamPosSigmaYKey", m_eventInfoKey, "beamPosSigmaY",
         "Key for the beamPosSigmaY decoration" };
      /// Decorator handle for @c beamPosSigmaZ
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosSigmaZKey{ this,
         "beamPosSigmaZKey", m_eventInfoKey, "beamPosSigmaZ",
         "Key for the beamPosSigmaZ decoration" };
      /// Decorator handle for @c beamPosSigmaXY
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamPosSigmaXYKey{ this,
         "beamPosSigmaXYKey", m_eventInfoKey, "beamPosSigmaXY",
         "Key for the beamPosSigmaXY decoration" };

      /// Decorator handle for @c beamTiltXZ
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamTiltXZKey{ this,
         "beamTiltXZKey", m_eventInfoKey, "beamTiltXZ",
         "Key for the beamTiltXZ decoration" };
      /// Decorator handle for @c beamTiltYZ
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamTiltYZKey{ this,
         "beamTiltYZKey", m_eventInfoKey, "beamTiltYZ",
         "Key for the beamTiltYZ decoration" };

      /// Decorator handle for @c beamStatus
      SG::WriteDecorHandleKey< xAOD::EventInfo > m_beamStatusKey{ this,
         "beamStatusKey", m_eventInfoKey, "beamStatus",
         "Key for the beamStatus decoration" };

      /// @}

   }; // class EventInfoBeamSpotDecoratorAlg

} // namespace xAODMaker

#endif // XAODEVENTINFOCNV_EVENTINFOBEAMSPOTDECORATORALG_H
