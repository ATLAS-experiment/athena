/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SimpleMMClusterBuilderTool_h
#define SimpleMMClusterBuilderTool_h

#include "AthenaBaseComps/AthAlgTool.h"
#include "MMClusterization/IMMClusterBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonCondData/NswErrorCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"
//
// Simple clusterization tool for MicroMegas
//
namespace Muon {

    class SimpleMMClusterBuilderTool : public extends<AthAlgTool, IMMClusterBuilderTool> {
    public:
        using base_class::base_class;
        virtual ~SimpleMMClusterBuilderTool() = default;
        
        StatusCode initialize() override;

        StatusCode getClusters(const EventContext& ctx,
                               std::vector<Muon::MMPrepData>&& stripsVect,
                               std::vector<std::unique_ptr<Muon::MMPrepData>>& clustersVect) const override;

        

        RIO_Author getCalibratedClusterPosition(const EventContext& ctx, 
                                                const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                const Amg::Vector3D& directionEstimate, 
                                                Amg::Vector2D& clusterLocalPosition,
                                                Amg::MatrixX& covMatrix) const override;

    private:
        StatusCode getClusterPosition(const EventContext& ctx,
                                      const Identifier& clustId, 
                                      std::vector<Muon::MMPrepData>& stripsVect, 
                                      Amg::Vector2D& clusterLocalPosition,
                                      Amg::MatrixX& covMatrix) const;
        

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                            "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        
        SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                                 "Key of the parametrized NSW uncertainties"};
    

        Gaudi::Property<bool> m_writeStripProperties{this, "writeStripProperties", true};
        Gaudi::Property<uint> m_maxHoleSize{this, "maxHoleSize", 1};
        Gaudi::Property<unsigned int> m_maxClusSize{this, "maxClusSize", 50};
    };

}  // namespace Muon
#endif
