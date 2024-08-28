/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCelldeadOTXTool.h" 
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArElecCalib/LArProvenance.h"



StatusCode LArCelldeadOTXTool::initialize() {

  ATH_CHECK(m_SCKey.initialize());
  ATH_CHECK(m_MFKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_cablingSCKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_onlineID,"LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_calo_id,"CaloCell_ID"));

  ATH_CHECK(m_scidtool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode LArCelldeadOTXTool::process(CaloCellContainer* cellCollection, const EventContext& ctx) const {

   ATH_MSG_VERBOSE (" in process...");
   if (!cellCollection) {
     ATH_MSG_ERROR( "Cell Correction tool receives invalid cell Collection"  );
     return StatusCode::FAILURE;
   }
    
   SG::ReadCondHandle<LArBadFebCont> mfHdl(m_MFKey, ctx);
   if(!mfHdl.isValid()) {
       ATH_MSG_ERROR("Do not have Missing FEBs container !!!!");
       return StatusCode::FAILURE;
   }

   SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey, ctx);
   if(!cablingHdl.isValid()) {
       ATH_MSG_ERROR("Do not have Onl-Ofl cabling map !!!!");
       return StatusCode::FAILURE;
   }

   SG::ReadCondHandle<LArOnOffIdMapping> cablingSCHdl(m_cablingSCKey, ctx);
   if(!cablingSCHdl.isValid()) {
       ATH_MSG_ERROR("Do not have Onl-Ofl cabling map for SuperCells !!!!");
       return StatusCode::FAILURE;
   }

   // vector of all missing cells Id
   std::vector<Identifier> cell_array;
   cell_array.reserve(512);
   std::vector<Identifier> sc_array;
   sc_array.reserve(512);
   
   LArBadFebCont::const_iterator firstFeb = mfHdl->begin();
   LArBadFebCont::const_iterator lastFeb  = mfHdl->end();


   // Step ONE - Loop Over FEB, find missing FEB
   for (LArBadFebCont::const_iterator i= firstFeb; i != lastFeb; ++i) {
       if(i->second.deadReadout()){
           HWIdentifier febId(i->first);
           //Find the corresponding Cell(s)
           for (int ch=0; ch<m_onlineID->channelInSlotMax(febId); ++ch) {
               HWIdentifier hwid = m_onlineID->channel_Id(febId, ch);
               if (cablingHdl->isOnlineConnected(hwid)) {
                   Identifier fid = cablingHdl->cnvToIdentifier( hwid);
                   //Save the CellIds 
                   cell_array.push_back(fid);
                   // and find corresponding SC
                   Identifier scId = m_scidtool->offlineToSuperCellID(fid);
                   if (!scId.is_valid()) {
                      sc_array.push_back(Identifier(0));
                   } else {
                      sc_array.push_back(scId);
                   }
               } //If Online
           }//Cell Loop
       }//Missing FEB
   }   // loop over bad Febs


   ATH_MSG_DEBUG (" Number of missing cells " << cell_array.size());
   // nothing to do if no missing Febs
   if (cell_array.size()==0) return StatusCode::SUCCESS;

   // get SuperCellContainer
   SG::ReadHandle<LArRawSCContainer> scHdl(m_SCKey, ctx);
   if(!scHdl.isValid()) {
       ATH_MSG_WARNING("Do not have SuperCell container no patching !!!!");
       return StatusCode::SUCCESS;
   } else {
       ATH_MSG_DEBUG("Reading SuperCell container with key "<<m_SCKey.key());
   }

   const unsigned int bcid = ctx.eventID().bunch_crossing_id();

   // loop over missing cells
   std::vector<float> energy_arr(cell_array.size(),0.);
   //get the SC, container is unordered, so have to loop
   const LArRawSCContainer* scells = scHdl.cptr();
   for(const auto *sc: *scells) {
      if ( !sc ) continue;
      Identifier off_id = cablingSCHdl->cnvToIdentifier(sc->hardwareID()); 
      auto it = std::find(sc_array.begin(), sc_array.end(), off_id);
      if(it !=  sc_array.end()) {// we have to use this SC
         int idx = std::distance(sc_array.begin(), it);
         const std::vector< unsigned short >& bcids = sc->bcids();
         const std::vector< int >& energies = sc->energies();
         const std::vector< bool>& satur = sc->satur();
         for(unsigned int i=0;i<bcids.size();i++) {
               if ( bcids[i]==bcid && !satur[i]) { // that's our bcid and not saturated
                    energy_arr[idx] = energies[i];
               }
         } 
         // and now divide by number of cells in the SC
         unsigned nCell=(m_scidtool->superCellToOfflineID(off_id)).size();
         if(nCell) energy_arr[idx] /= nCell;
         ATH_MSG_DEBUG("Got ET "<<energy_arr[idx]<<" from 0x"<<std::hex<<off_id.get_identifier32().get_compact()<<std::dec<<" out of "<<nCell<<" cells...");
         for(unsigned l=idx+1; l<sc_array.size();++l) {
            if(sc_array[l] == sc_array[idx]) energy_arr[l]=energy_arr[idx];
         }
      } // find SC
   } // over all SC

   // Assign energy
   for(unsigned i=0; i<cell_array.size(); ++i) {

      IdentifierHash theCellHashID = m_calo_id->calo_cell_hash(cell_array[i]);
      int index = cellCollection->findIndex(theCellHashID);
 
      if(index!=-1){
          CaloCell* thisCell = cellCollection->at(index);
 
          if (thisCell ) {
             // convert ET (coming from LATOMEs) into Energy and apply magic 12.5 factor
             thisCell->setEnergy(12.5*energy_arr[i]*cosh(thisCell->eta()));
             thisCell->setProvenance(thisCell->provenance() | LArProv::PATCHED); 
             ATH_MSG_DEBUG ("   assign " << thisCell->energy()<< " to the cell 0x"<<std::hex<<cell_array[i].get_identifier32().get_compact()<<std::dec);
          }
      }
   } // over cell_array
   return StatusCode::SUCCESS;
}
