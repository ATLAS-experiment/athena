/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSMUONDETECTOR_MUONDETECTORBUILDERTOOL_H
#define ACTSMUONDETECTOR_MUONDETECTORBUILDERTOOL_H

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/IDetectorVolumeBuilderTool.h>
#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

namespace ActsTrk{
    class MuonDetectorBuilderTool: public AthAlgTool, virtual public IDetectorVolumeBuilderTool {
    
    public:
        /** @brief Standard tool constructor **/
        MuonDetectorBuilderTool( const std::string& type, const std::string& name, const IInterface* parent );

        virtual ~MuonDetectorBuilderTool() = default;

        StatusCode initialize() override final;
 
        Acts::Experimental::DetectorComponent construct(const Acts::GeometryContext& context) const override final; 
        

    private:

        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        Gaudi::Property<bool> m_dumpVisual{this, "DumpVisualization", true, "If set to true the DetectorVolumes are dumped into a visualization file format"};

        
        //private method for the readout element construction
        std::pair<std::vector<std::shared_ptr<Acts::Experimental::DetectorVolume>>,std::vector<std::shared_ptr<Acts::Surface>>> constructElements(const ActsGeometryContext& gctx, 
        const MuonGMR4::MuonChamber& mChamber, std::pair<unsigned int, unsigned int> chId) const;

    };

}
#endif
