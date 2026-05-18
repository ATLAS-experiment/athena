/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtAsBuiltJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <format>
#include <string_view>

MdtAsBuiltJsonDumpAlg::MdtAsBuiltJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
      AthAlgorithm{name, pSvcLocator} {}

StatusCode MdtAsBuiltJsonDumpAlg::initialize() {
    ATH_CHECK(m_readKey.initialize());   
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }
StatusCode MdtAsBuiltJsonDumpAlg::execute(){
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<MdtAsBuiltContainer> asBuiltContainer{m_readKey, ctx};
  if (!asBuiltContainer.isValid()){
    ATH_MSG_FATAL("Failed to load ALine container "<<m_readKey.fullKey());
    return StatusCode::FAILURE;
  }
  std::ofstream ostr{m_jsonFile};
  if (!ostr.good()) {
     ATH_MSG_FATAL("Failed to create output file "<<m_jsonFile);
     return StatusCode::FAILURE;
  }
  ostr<<"["<<std::endl;
  unsigned int nLines{0};
  using multilayer_t = MdtAsBuiltPar::multilayer_t;
  using tubeSide_t = MdtAsBuiltPar::tubeSide_t;
  for (const MdtAsBuiltPar& asBuilt : **asBuiltContainer) {
    ++nLines;
    ostr<<"    {"<<std::endl;
    /// Identifier of the A Line
    ostr<<"     \"typ\": \""<<asBuilt.AmdbStation()<<"\","<<std::endl;
    ostr<<"     \"jzz\": "<<asBuilt.AmdbEta()<<", "<<std::endl;
    ostr<<"     \"jff\": "<<asBuilt.AmdbPhi()<<", "<<std::endl;
    for (const multilayer_t ml : {multilayer_t::ML1, multilayer_t::ML2}){
      for (const tubeSide_t side: {tubeSide_t::POS, tubeSide_t::NEG}){
          const std::string prefix = std::format("Ml{}{}TubeSide",
                                       static_cast<unsigned>(ml) + 1,
                                       side == tubeSide_t::POS ? "Pos" : "Neg");

          const auto dumpValue = [&ostr, &prefix](std::string_view field,
                                                  float val,
                                                  bool last = false) {
            ostr << "     \"" << prefix << field << "\": "
                 << val << (last ? "" : ", ") << '\n';
          };
          
          dumpValue("y0", asBuilt.y0(ml, side));
          dumpValue("z0", asBuilt.z0(ml, side));
          dumpValue("alpha", asBuilt.alpha(ml, side));
          dumpValue("ypitch", asBuilt.ypitch(ml, side));
          dumpValue("zpitch", asBuilt.zpitch(ml, side));
          dumpValue("stagg", asBuilt.stagg(ml, side),
                    ml == multilayer_t::ML2 && side == tubeSide_t::NEG);
        }
    }
    ostr<<"    }"<< (nLines != asBuiltContainer->size() ? "," : "")<<std::endl;
  }
  ostr<<"]"<<std::endl;
  ostr.close();
  
  
  return StatusCode::SUCCESS;
}
