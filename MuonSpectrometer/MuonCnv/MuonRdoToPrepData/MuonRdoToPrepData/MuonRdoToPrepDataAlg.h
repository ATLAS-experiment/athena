/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDOTOPREPDATA_MUONRDOTOPREPDATA_H
#define MUONRDOTOPREPDATA_MUONRDOTOPREPDATA_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "IRegionSelector/IRegSelTool.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"


////////////////////////////////////////////////////////////////////////////////////////
/// algorithm to decode RDO into MdtPrepData
/// get the RDO container from Storegate
/// loop over the RDO
/// Decode RDO into PrepRawData
/// loop over the PrepRawData and build the PrepRawData container
/// store the PrepRawData container in StoreGate
////////////////////////////////////////////////////////////////////////////////////////

class MuonRdoToPrepDataAlg : public AthReentrantAlgorithm {
public:
    MuonRdoToPrepDataAlg(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

private:
    // EJWM - where is this implemented? Removing to avoid missing symbol
    // void printRpcPrepRawData(); //!< Prints information about the resultant PRDs.

    ToolHandle<Muon::IMuonRdoToPrepDataTool> m_tool{this, "DecodingTool", "", "RdoToPrepDataConversionTool"};
    ToolHandle<IRegSelTool> m_regsel{this, "RegSelector", ""};

    Gaudi::Property<bool> m_print_inputRdo{this, "PrintInputRdo", false, 
                                          "If true, will dump information about the input RDOs."};
    Gaudi::Property<bool> m_print_prepData{this, "PrintPrepData", false , 
                                          "If true, will dump information about the resulting PRDs."};
    Gaudi::Property<bool> m_seededDecoding{this, "DoSeededDecoding", false, "If true decode only in RoIs"};
    Gaudi::Property<bool> m_robDecoding{this, "useROBs" , true, "Pipe the ROBS directly to the decoder"};
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{this, "RoIs", "OutputRoIs", "RoIs to read in "};
};

#endif  /// MuonRdoToPrepDataAlg_H
