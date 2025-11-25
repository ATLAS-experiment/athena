/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include "LArHVCorrToSCHVCorr.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArElecCalib/LArCalibErrorCode.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "PathResolver/PathResolver.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/IOVInfiniteRange.h"



LArHVCorrToSCHVCorr::LArHVCorrToSCHVCorr( const std::string& name, 
				      ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{
}

StatusCode LArHVCorrToSCHVCorr::initialize()
{

  ATH_CHECK(m_scidTool.retrieve());
  ATH_CHECK(m_cablingKeySC.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_contKey.initialize());
  ATH_CHECK(m_outKey.initialize(!m_outKey.empty()));

  return StatusCode::SUCCESS;
}


StatusCode LArHVCorrToSCHVCorr::stop()
{  
  
  //Retrieve HVCorr for regular cells
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<ILArHVScaleCorr> hvHdl(m_contKey, ctx);
  if(!hvHdl.isValid()) {
      ATH_MSG_ERROR( "Do not have HVScaleCorr from key " << m_contKey.key() );
      return StatusCode::FAILURE;
  }

  //Retrieve SuperCell online id
  const LArOnline_SuperCellID* onlSCID = nullptr;
  CHECK(detStore()->retrieve(onlSCID));

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling{*cablingHdl};
  if(!cabling) {
      ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKey.key() );
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdlSC{m_cablingKeySC, ctx};
  const LArOnOffIdMapping* cablingSC{*cablingHdlSC};
  if(!cablingSC) {
      ATH_MSG_ERROR( "Do not have cabling mapping from key " << m_cablingKeySC.key() );
      return StatusCode::FAILURE;
  }

  const CaloCell_SuperCell_ID* calosccellID=nullptr;
  ATH_CHECK( detStore()->retrieve (calosccellID, "CaloCell_SuperCell_ID") );
  const unsigned hashMax=calosccellID->calo_cell_hash_max();
  ATH_MSG_INFO("SuperCell hash max: " << hashMax);

  // get the HEC layer weights
  std::string file = PathResolver::find_file (m_weightsName.value(), "JOBOPTSEARCHPATH");
  ATH_MSG_INFO( "Reading file  " << file << " configured: " << m_weightsName.value() );
  std::ifstream fin(file.c_str());
  if (!fin.is_open()) {
       ATH_MSG_ERROR( "Could not open input file " << file );
       return StatusCode::FAILURE;
  }
  std::string line;
  int ttbin;
  float em0,em1,em2,em3,em4;
  float had0,had1,had2,had3;
  std::map<int,std::vector<float> > wmap;
  while (std::getline(fin, line)){
      int nread = sscanf(line.c_str(), "%d %f %f %f %f %f %f %f %f %f", 
                                       &ttbin,&em0,&em1,&em2,&em3,&em4,&had0,&had1,&had2,&had3);
      if(nread != 10) continue;
      wmap[ttbin]=std::vector<float>{had0,had1,had2,had3};
  }
  // need also online ID helper
  const LArHEC_ID* hecID = nullptr;
  ATH_CHECK(detStore()->retrieve(hecID,"LArHEC_ID"));

  //Set up AttributeListCollection 
  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification();
  spec->extend("HVScaleCorr", "blob");
  spec->extend<unsigned>("version");
  
  coral::AttributeList attrList(*spec);   
  attrList["version"].setValue(0U);
  coral::Blob& hvBlob=attrList["HVScaleCorr"].data<coral::Blob>();

  spec->release();
  // cppcheck-suppress memleak
  spec = nullptr;

  // Important, blob is ordered by LAr online hash, but LArHVCorr by cell offline hash !!!!
  hvBlob.resize(onlSCID->channelHashMax()*sizeof(float));
    
  float *pHV=static_cast<float*>(hvBlob.startingAddress());
  std::vector<float> vScale;
  //Initialize blobs to ERRORCODE
  for (unsigned i=0;i<onlSCID->channelHashMax();++i) {
    pHV[i]=LArElecCalib::ERRORCODE;
  }
  if(!m_outKey.empty()) vScale.resize(hashMax,(float)1.0);
   
  unsigned nFilledIds=0;
  unsigned nTotalIds=0;
  for (unsigned i=0; i < hashMax; ++i) {
    if(calosccellID->sub_calo(IdentifierHash(i)) > 2) continue; // not a LAr channel  
    const Identifier scId=calosccellID->cell_id(IdentifierHash(i));
    nTotalIds += 1;
    const std::vector<Identifier> &cellIds=m_scidTool->superCellToOfflineID(scId);
    if (cellIds.empty()) {
      ATH_MSG_ERROR("Got empty vector of cell ids for super cell id 0x" 
		      << std::hex << scId.get_compact()<<std::dec);
		      return StatusCode::FAILURE;
    }
    float hvcorr=0.;
    float weight=1.;
    float wsum=0.;
    for(const Identifier cellId : cellIds) {
        if(hecID->is_lar_hec(cellId)) { // find a weight
          weight = getWeight(hecID,cellId,wmap);
        }
        hvcorr += weight * hvHdl->HVScaleCorr(cabling->createSignalChannelID(cellId));
        wsum += weight;
    }
    if(wsum>0.) hvcorr /= wsum;
    nFilledIds += 1;
    if (m_isHI) {
       // Hack for excessive corr. in EMBC
       // this part is eventually needed for HI running
       //if(hvcorr >= 1.6 && calosccellID->is_em_barrel(scId) ) {
       if(hvcorr >= 1.6 ) {
          ATH_MSG_INFO("Set manually HVCorr to "<<m_patchHI<<" for SC "<< scId.get_identifier32().get_compact()<<" "<<calosccellID->cell_name(scId));
          ATH_MSG_INFO( calosccellID->is_em_barrel(scId) << " " << calosccellID->pos_neg(scId));
          hvcorr=m_patchHI;
       }
    }

    // Important, blob is ordered by LAr online hash, but LArHVCorr by cell offline hash !!!!
    pHV[onlSCID->channel_Hash(cablingSC->createSignalChannelID(scId))]=hvcorr;

    if(!m_outKey.empty()) vScale[i]=hvcorr;
    
  }//end loop over super-cell hash
  
  //Add to collection
  CondAttrListCollection* coll=new CondAttrListCollection(true);
  CHECK(detStore()->record(coll,m_folderName));
  coll->add(0,attrList);

  ATH_MSG_INFO("Total number of SuperCells:" << nTotalIds << ", filled:" << nFilledIds);

  if(!m_outKey.empty()) {
     SG::WriteCondHandle<LArHVCorr> writeHandle{m_outKey, ctx};
     const EventIDRange fullRange=IOVInfiniteRange::infiniteMixed();
     writeHandle.addDependency (fullRange);
     writeHandle.addDependency(hvHdl);
     writeHandle.addDependency(cablingHdl);
     writeHandle.addDependency(cablingHdlSC);

     auto scHvCorr = std::make_unique<LArHVCorr>(std::move(vScale), cablingSC, calosccellID);
     if (writeHandle.record(std::move(scHvCorr)).isFailure()) {
       ATH_MSG_ERROR("Could not record LArHVCorr object with " << m_outKey.key()
                   << " with EventRange " << writeHandle.getRange() << " into Conditions Store");
       return StatusCode::FAILURE;
     }
     ATH_MSG_INFO("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");
  }
  return StatusCode::SUCCESS;
}

float LArHVCorrToSCHVCorr::getWeight(const LArHEC_ID *hecID, const Identifier &id, std::map<int,std::vector<float> > &wmap) {
   int side = hecID->pos_neg(id);
   int reg = hecID->region(id);
   int eta = hecID->eta(id);
   int ttbin=0;
   switch(side){
      case -2: if(reg==0) ttbin=-16-eta; else ttbin=-27-2*eta; if(ttbin==-33) ttbin=-32; break;
      case 2: if(reg==0) ttbin=15+eta; else ttbin=25+2*eta;
   }
   if(ttbin!=0 && wmap.find(ttbin) != wmap.end()){
          ATH_MSG_DEBUG("Found weight "<<wmap[ttbin][hecID->sampling(id)] <<" for "<<side<<" "<<reg<<" "<<eta);
          return wmap[ttbin][hecID->sampling(id)];
   } else {
          ATH_MSG_DEBUG("Returning 1 for "<<side<<" "<<reg<<" "<<eta);
      return 1.;
   }
}

