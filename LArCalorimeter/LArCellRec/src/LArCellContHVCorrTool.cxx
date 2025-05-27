/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCellContHVCorrTool.h" 
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"




StatusCode LArCellContHVCorrTool::initialize() {

  ATH_CHECK(m_offlineHVScaleCorrKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode LArCellContHVCorrTool::process(CaloCellContainer* cellCollection, const EventContext& ctx) const {

   // get offline HVScaleCorr
   SG::ReadCondHandle<LArHVCorr> oflHVCorrHdl(m_offlineHVScaleCorrKey, ctx);
   const LArHVCorr *oflHVCorr = *oflHVCorrHdl;
   if(!oflHVCorr) {
       ATH_MSG_ERROR("Do not have ofline HV corr. conditions object !!!!");
       return StatusCode::FAILURE;
   }

   if (!cellCollection) {
     ATH_MSG_ERROR( "Cell Correction tool receives invalid cell Collection"  );
     return StatusCode::FAILURE;
   }
    
   
   for (CaloCell* theCell : *cellCollection) {
     const float hvcorr = oflHVCorr->HVScaleCorr(theCell->ID());
     theCell->scaleEnergy(hvcorr);
   }// End loop over cell-container
   return StatusCode::SUCCESS;
}
