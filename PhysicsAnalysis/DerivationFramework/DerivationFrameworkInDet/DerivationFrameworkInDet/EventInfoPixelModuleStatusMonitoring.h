/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_EVENTINFOPIXELMODULESTATUSMONITORING_H
#define DERIVATIONFRAMEWORK_EVENTINFOPIXELMODULESTATUSMONITORING_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthLinks/ElementLink.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "StoreGate/ReadHandleKey.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/PixelID.h"

#include "PixelConditionsData/PixelDCSHVData.h"
#include "PixelConditionsData/PixelDCSTempData.h"
#include "PixelConditionsData/PixelDCSStateData.h"
#include "PixelConditionsData/PixelDCSStatusData.h"
#include "PixelConditionsData/PixelDeadMapCondData.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"

#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"
#include "PixelReadoutGeometry/PixelFEUtils.h"

namespace DerivationFramework {

  class EventInfoPixelModuleStatusMonitoring : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      EventInfoPixelModuleStatusMonitoring(const std::string& type, const std::string& name, const IInterface* parent);

      StatusCode initialize();
      StatusCode finalize();
      virtual StatusCode addBranches(const EventContext& ctx) const;

    private:

      Gaudi::Property<std::string> m_prefix
      { this,"DecorationPrefix", "", "" };

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      { this, "ContainerName", "EventInfo", ""};

      SG::ReadCondHandleKey<PixelDCSTempData> m_readKeyTemp
      {this, "ReadKeyeTemp", "PixelDCSTempCondData", "Key of input sensor temperature conditions folder"};

      SG::ReadCondHandleKey<PixelDCSHVData> m_readKeyHV
      {this, "ReadKeyHV",    "PixelDCSHVCondData", "Key of input bias voltage conditions folder"};

      SG::ReadCondHandleKey<PixelDCSStateData> m_condDCSStateKey
      {this, "PixelDCSStateCondData", "PixelDCSStateCondData", "Pixel FSM state key"};

      SG::ReadCondHandleKey<PixelDCSStatusData> m_condDCSStatusKey
      {this, "PixelDCSStatusCondData", "PixelDCSStatusCondData", "Pixel FSM status key"};

      SG::ReadCondHandleKey<PixelDeadMapCondData> m_condDeadMapKey
      {this, "PixelDeadMapCondData", "PixelDeadMapCondData", "Pixel deadmap conditions key"};

      ToolHandle<IInDetConditionsTool> m_pixelSummary
      {this, "PixelConditionsSummaryTool", "PixelConditionsSummaryTool", "Tool for PixelConditionsSummaryTool"};

      SG::ReadHandleKey<IDCInDetBSErrContainer> m_idcErrContKey
      {this, "PixelByteStreamErrs", "PixelByteStreamErrs", "PixelByteStreamErrs container key"};

      Gaudi::Property<bool> m_useByteStreamFEI4
      {this, "UseByteStreamFEI4", true, "Switch of the ByteStream error for FEI4"};
      Gaudi::Property<bool> m_useByteStreamFEI3
      {this, "UseByteStreamFEI3", true, "Switch of the ByteStream error for FEI3"};
      Gaudi::Property<bool> m_useByteStreamRD53
      {this, "UseByteStreamRD53", false, "Switch of the ByteStream error for RD53"};

      const PixelID* m_pixelID;
      mutable std::atomic_uint m_lbCounter{0};
      unsigned int m_readoutTechnologyMask{};

      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_moduleConditionKeys;
      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_moduleFEmaskKeys;
      std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > m_moduleBSErrKeys;

    protected:

      SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatusActiveOnly
      {this, "PixelDetElStatusActiveOnly", "", "Key of SiDetectorElementStatus for Pixel which reflects only whether modules or chips are active rather than delivering good data"};

      SG::ReadHandle<InDet::SiDetectorElementStatus> getPixelDetElStatus(const SG::ReadHandleKey<InDet::SiDetectorElementStatus> &key, const EventContext& ctx) const {
        SG::ReadHandle<InDet::SiDetectorElementStatus> pixelDetElStatus;
        if (!key.empty()) {
          pixelDetElStatus = SG::ReadHandle<InDet::SiDetectorElementStatus>(key, ctx);
          if (!pixelDetElStatus.isValid()) {
            std::stringstream msg;
            msg << "Failed to get " << key.key() << " from StoreGate in " << name();
            throw std::runtime_error(msg.str());
          }
        }
        return pixelDetElStatus;
      };

  };
}

namespace InDet {
   /** Retrieve the bytestream error word for the given module if the readout technology of the module is contained in in the mask.
    * @param elementStatus the detector element status information.
    * @param bsErrorContainer the container with bytestream error words.
    * @param the hash of the module in question.
    * @param the index of the error word where the index of the module error is equal to the hash.
    * @param readOutTechnologyMask a mask which contains a bit for each readout technology.
    */
   inline unsigned int getBSErrorWord(const InDet::SiDetectorElementStatus &elementStatus,
                                      const IDCInDetBSErrContainer &bsErrorContainer,
                                      const IdentifierHash &moduleIdHash,
                                      unsigned int index,
                                      unsigned int readOutTechnologyMask = (    Pixel::makeReadoutTechnologyBit(InDetDD::PixelReadoutTechnology::FEI4)
                                                                            | ( Pixel::makeReadoutTechnologyBit(InDetDD::PixelReadoutTechnology::FEI3) ) ))
   {
      if ( Pixel::matchingReadoutTechnology(elementStatus, moduleIdHash, readOutTechnologyMask )) {
         constexpr uint64_t missingErrorInfo{std::numeric_limits<uint64_t>::max()-3000000000};
         uint64_t word = static_cast<uint64_t>(bsErrorContainer.retrieve(index));
         return word < missingErrorInfo ? word : 0;
      }
      else {
         return 0;
      }
   }
}

#endif
