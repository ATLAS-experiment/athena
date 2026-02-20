/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_MmCTPClusterCalibData_H
#define MUONCONDDATA_MmCTPClusterCalibData_H

#include "GeoPrimitives/GeoPrimitives.h"
// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaBaseComps/AthMessaging.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "GeoModelUtilities/TransientConstSharedPtr.h"

namespace Muon{
class MmCTPClusterCalibData: public AthMessaging {
    public:

        using CTPParameters = std::array<double, 2>;
        
        MmCTPClusterCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc);
         ~MmCTPClusterCalibData() = default;

        StatusCode storeConstants(const Identifier& gasGapId, CTPParameters&& newConstants);
        double getCTPCorrectedDriftVelocity(const Identifier& identifier, const double theta) const;
    
    private:
        /** @brief Converts the identifier to a continious hash used to access
         *         the stored parameters
         *  @param gasgGapId: Identifier to convert */
        std::uint32_t convertHash(const Identifier& gasGapId) const;
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using parameterMap_t = std::vector<std::unique_ptr<CTPParameters>>; 
        parameterMap_t m_database{};
};

} // end for Muon namespace


CLASS_DEF( Muon::MmCTPClusterCalibData , 2631242 , 1 );
CONDCONT_DEF( Muon::MmCTPClusterCalibData , 200440612 );

#endif
