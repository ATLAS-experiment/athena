#ifndef MUONCSVDUMP_MuonStripCsvDumperAlg_H
#define MUONCSVDUMP_MuonStripCsvDumperAlg_H
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AthenaBaseComps/AthAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <StoreGate/ReadHandleKey.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <xAODMeasurementBase/UncalibratedMeasurementContainer.h>

/** The MuonStripCsvDumperAlg reads the RpcStripContainer and dumps information to csv files
 *  The files are used for the algorithm development in acts **/

class MuonStripCsvDumperAlg: public AthAlgorithm {

   public:

     MuonStripCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator);
    ~MuonStripCsvDumperAlg() = default;

     StatusCode initialize() override;
     StatusCode execute() override;

   private:

    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    
    SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_stripContainerKey{this, "ContainerKey", 
                                                                                  "", "Key to the Rpc/Tgc container"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    Gaudi::Property<std::string> m_preFix{this, "PreFix", "", "Prefix to the csv file name"};
    size_t m_event{0};

};
#endif