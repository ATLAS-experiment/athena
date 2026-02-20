/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// =================================================================================

#include "CaloStripsRingsBuilder.h"
#include "AthenaKernel/errorcheck.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloUtils/CaloCellList.h"
#include "xAODBase/IParticle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloRings/CaloRingsAuxContainer.h"
#include "xAODCaloRings/CaloRingsContainer.h"
#include "xAODCaloRings/RingSetAuxContainer.h"
#include "xAODCaloRings/RingSetConf.h"
#include "xAODCaloRings/RingSetContainer.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <sstream>

namespace Ringer {

// =================================================================================
CaloStripsRingsBuilder::CaloStripsRingsBuilder(const std::string& type,
                                 const std::string& name,
                                 const ::IInterface* parent)
  : CaloRingsBuilder(type,name,parent)
{
  
  declareInterface<ICaloRingsBuilder>(this);
  declareProperty("Axis", m_axis=0, "Which axis to use when building the strips");
  declareProperty("doEtaAxesDivision", m_doEtaAxesDivision = false, "Eta Axes can be divide in two.");
  declareProperty("doPhiAxesDivision", m_doPhiAxesDivision = false, "Phi Axes can be divide in two.");
}

// =====================================================================================
CaloStripsRingsBuilder::~CaloStripsRingsBuilder() = default;

// =====================================================================================
StatusCode CaloStripsRingsBuilder::initialize()
{
  ATH_MSG_WARNING("Initializing " << name() );
  m_nRingSets = m_nRings.size();
  auto itr = m_layers.begin();

  for (size_t rsConfIdx = 0; rsConfIdx < m_nRingSets; ++rsConfIdx) {
    
    const auto rsNLayers = m_nLayers[rsConfIdx];
    auto end_itr = itr + rsNLayers;

    const auto& caloSampleItr = reinterpret_cast< std::vector<CaloSampling::CaloSample>::iterator& >(itr);
    const auto& caloSampleEndItr = reinterpret_cast< std::vector<CaloSampling::CaloSample>::iterator& >(end_itr);

    std::vector<CaloSampling::CaloSample> rsLayers( caloSampleItr , caloSampleEndItr);
    itr += rsNLayers;

    const auto rawConf = xAOD::RingSetConf::RawConf(
          m_nRings[rsConfIdx],
          rsLayers,
          m_etaWidth[rsConfIdx], m_phiWidth[rsConfIdx],
          m_cellMaxDEtaDist, m_cellMaxDPhiDist,
          xAOD::RingSetConf::whichLayer(rsLayers),
          xAOD::RingSetConf::whichSection(rsLayers)
        );
    m_rsRawConfCol.push_back(std::move(rawConf));
  }

  try {
    xAOD::RingSetConf::addRawConfColBounderies(m_rsRawConfCol);
  } catch ( const std::runtime_error &e) {
    ATH_MSG_ERROR("Could not add collection bounderies due to: " << e.what() );
    ATH_MSG_ERROR("RawConfCollection is: ");
    std::ostringstream str;
    xAOD::RingSetConf::print(m_rsRawConfCol, str);
    ATH_MSG_ERROR(str.str());
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_crContName.initialize() );
  ATH_CHECK( m_rsContName.initialize() );
  ATH_CHECK( m_cellsContName.initialize() );
  ATH_CHECK( m_caloMgrKey.initialize() );
  return StatusCode::SUCCESS;
}

// =====================================================================================
StatusCode CaloStripsRingsBuilder::finalize()
{
  return StatusCode::SUCCESS;
}
// =================================================================================

StatusCode CaloStripsRingsBuilder::buildRingSet(
    const xAOD::RingSetConf::RawConf &rawConf,
    const AtlasGeoPoint &seed,
    xAOD::RingSet *rs)
{
  const auto nStrips = rawConf.nRings;
  int midPoint = nStrips/2;

  SG::ReadHandle<CaloCellContainer> cellsCont(m_cellsContName);
  if(!cellsCont.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve "<< m_cellsContName.key());
    return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  const CaloDetDescrManager* caloMgr=*caloMgrHandle;
  CaloCellList cells(caloMgr, cellsCont.ptr() );

  std::function<double(const CaloCell*)> calcDelta;
  if (m_axis){
    calcDelta = [&rawConf, &seed](const CaloCell* cell){
        return CaloPhiRange::diff(seed.phi(), cell->phi())/rawConf.phiWidth;
    };
  } else {
    calcDelta = [&rawConf, &seed](const CaloCell* cell){
        return (seed.eta() - cell->eta())/rawConf.etaWidth;
    };
  }
  // loop over cells
  for ( const int layer : rawConf.layers) {
    cells.select(seed.eta(), seed.phi(), m_cellMaxDEtaDist, m_cellMaxDPhiDist, layer );
    for ( const CaloCell *cell : cells ) {
      float deltaPhi = CaloPhiRange::diff(cell->phi(), seed.phi());
      bool phiPositive = deltaPhi > 0;
      const auto delta = calcDelta( cell );
      int idx =copysign(static_cast<int>(std::floor( delta + .5)),delta);
      unsigned int stripIdx(0);
      
      if(!phiPositive){
          stripIdx = midPoint - (idx*2);
          if (stripIdx>100000){stripIdx = 0;}
      }else{
          stripIdx = midPoint - (idx*2 + 1);
          if (stripIdx>100000){stripIdx = 0;}
      }
      if ( stripIdx < nStrips ){
        if(m_doTransverseEnergy){
          rs->at(stripIdx) += cell->energy()/std::cosh(cell->eta());
        }else{
          rs->at(stripIdx) += cell->energy();
        }
      }
    }
  }
  return StatusCode::SUCCESS; 
}

}

