/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWCALIBDBTHRESHOLDDATA_H
#define MUONCONDDATA_NSWCALIBDBTHRESHOLDDATA_H


// Athena includes
#include "MuonCondData/Defs.h" //CalibTechType
#include "MuonStationIndex/MuonStationIndex.h" //enum
#include "AthenaKernel/CondCont.h"  
#include "AthenaBaseComps/AthMessaging.h"
#include "Identifier/Identifier.h"
// STL includes
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <optional>
#include <unordered_map>



namespace Muon{
  class IMuonIdHelperSvc;
}


/** @brief Conditions data to model a channel dependent energy deposit threshold such that
 *         the electronics returns a signal. The object is only used by the digitization tools
 *          of the NSW */
class NswCalibDbThresholdData: public AthMessaging {

 

public:
    using ThrsldTechType = MuonCond::CalibTechType;
  
    NswCalibDbThresholdData(const Muon::IMuonIdHelperSvc* idHelperSvc);
    virtual ~NswCalibDbThresholdData() = default;

	  // setting functions
	  void setData(const Identifier& channelId, const float);
	  void setZero(const ThrsldTechType tech  , const float);

	  // retrieval functions
	  std::vector<Identifier> getChannelIds(std::string_view = {}, std::string_view = {}) const;
	  std::optional<float> getThreshold (const Identifier& channelId) const;

 
private:

	// containers
    using ChannelMap = std::unordered_map<Identifier, float>;
    using ZeroMap = std::array<std::optional<float>, 
                               Muon::MuonStationIndex::toInt(ThrsldTechType::nTypes)>;
    ChannelMap m_data{};
    ZeroMap m_zero{};

	// ID helpers
  const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
};

CLASS_DEF( NswCalibDbThresholdData , 108292495 , 1 );
CONDCONT_DEF( NswCalibDbThresholdData , 169109811 );

#endif
