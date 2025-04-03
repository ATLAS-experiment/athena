/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArHVPathologyDbAlg.h"

#include "LArRecConditions/LArHVPathologiesDb.h"
#include "RegistrationServices/IIOVRegistrationSvc.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "LArIdentifier/LArOnlineID.h"

#include "CaloDetDescr/CaloDetectorElements.h"
#include "LArReadoutGeometry/EMBCell.h"
#include "LArHV/EMBHVElectrode.h"
#include "LArHV/EMBPresamplerHVModule.h"
#include "LArReadoutGeometry/EMECCell.h"
#include "LArHV/EMECHVElectrode.h"
#include "LArHV/EMECPresamplerHVModule.h"
#include "LArReadoutGeometry/HECCell.h"
#include "LArHV/HECHVSubgap.h"
#include "LArReadoutGeometry/FCALTile.h"
#include "LArHV/FCALHVLine.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <fstream>
#include <cstdlib>

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "CoralBase/Blob.h"

#include "TBufferFile.h"
#include "TClass.h"


StatusCode LArHVPathologyDbAlg::initialize()
{
  ATH_MSG_INFO(" in initialize()");

  if(m_writeCondObjs && m_folder.value().empty()) {
    ATH_MSG_ERROR("Folder property not set. Exiting ... ");
    return StatusCode::FAILURE;
  }

  // Get HVPathology tool
  //ATH_CHECK(m_pathologyTool.retrieve());

  // retrieve LArEM id helpers
  ATH_CHECK(detStore()->retrieve(m_caloIdMgr));

  m_larem_id   = m_caloIdMgr->getEM_ID();
  m_larhec_id   = m_caloIdMgr->getHEC_ID();
  m_larfcal_id   = m_caloIdMgr->getFCAL_ID();

  ATH_CHECK(detStore()->retrieve(m_laronline_id,"LArOnlineID"));

  ATH_CHECK( m_hvCablingKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_caloMgrKey.initialize() );
  ATH_CHECK( m_hvPathologyKey.initialize(!m_writeCondObjs) );

  return StatusCode::SUCCESS;
}

StatusCode LArHVPathologyDbAlg::execute()
{
  ATH_MSG_INFO(" in execute()");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  int nevt = ctx.eventID().event_number();
  if (nevt!=1) return StatusCode::SUCCESS;

  
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  ATH_CHECK(caloMgrHandle.isValid());
  const CaloDetDescrManager* calodetdescrmgr = *caloMgrHandle;
  

  const std::vector<LArHVPathologiesDb::LArHVElectPathologyDb>* pathologyContainer=nullptr;
  if(m_writeCondObjs) {
    ATH_MSG_INFO("Creating conditions objects");

    // Create cond objects
    auto pathologies=createCondObjects(ctx,calodetdescrmgr);
    if(!pathologies) {
      ATH_MSG_ERROR("Could not create cond objects ");
      m_writeCondObjs = false;
      return StatusCode::FAILURE;
    }
    pathologyContainer=&(pathologies->m_v);
    std::unique_ptr<AthenaAttributeList> attrlist = hvPathology2AttrList(*pathologies);
    ATH_MSG_INFO("Created Attribute List");
    coral::Blob& blob=(*attrlist)["Constants"].data<coral::Blob>();
    ATH_MSG_DEBUG("Blob size=" << blob.size());
    StatusCode sc = detStore()->record(std::move(attrlist),m_folder.value());
    if(!sc.isSuccess()) {
      ATH_MSG_ERROR("Could not record " << m_folder.value());
      return sc;
    }
    else
      ATH_MSG_INFO("Recorded " << m_folder.value());
  }
  // Dump cond objects
  ATH_CHECK(printCondObjects (ctx,calodetdescrmgr,pathologyContainer));
  return StatusCode::SUCCESS;
}


std::optional<LArHVPathologiesDb> LArHVPathologyDbAlg::createCondObjects (const EventContext & ctx, const CaloDetDescrManager* calodetdescrmgr) const
{
  ATH_MSG_INFO(" in createCondObjects() ");

    SG::ReadCondHandle<LArHVIdMapping> hvIdMapping (m_hvCablingKey, ctx);

    SG::ReadCondHandle<LArOnOffIdMapping> cabHdl (m_cablingKey, ctx);
    const LArOnOffIdMapping *cabling = *cabHdl;
    if(!cabling) {
       ATH_MSG_ERROR("Do not have cabling object with key " << m_cablingKey.key());
       return std::nullopt;
    }
    // Read input file and construct LArHVPathologiesDb for given folder
    std::ifstream infile;
    infile.open(m_inpFile.value().c_str());

    if(!infile.is_open()) {
      ATH_MSG_ERROR("Unable to open " << m_inpFile << " for reading");
      return std::nullopt;
    }

    char checkChar;
    char commentSign('#');
    std::string commentLine(""), foldername("");
    unsigned int cellID;
    unsigned short electInd, pathologyType;

    // Look for the folder name in the file
    while(!infile.eof()) {
      infile >> foldername;
      if(foldername==m_folder.value())
	break;
    }

    if(foldername!=m_folder.value()) {
      ATH_MSG_ERROR("Unable to find data for the folder " << m_folder.value() 
		      << " in the input file");
      return std::nullopt;
    }
    else
      ATH_MSG_INFO("Found folder " << foldername << " in the input file");
      
    // Get data corresponding to the folder and put it into LArHVPathologiesDb object
    LArHVPathologiesDb pathologies;
    
    ATH_MSG_INFO(" start reading input file ");
    while(!infile.eof()) {
      // Number or string?
      checkChar = static_cast<char> (infile.get());
      if(checkChar=='\n')
	continue;
      if((checkChar >= '0') && (checkChar <= '9')) {
	// Number - read three values
	infile.unget();
        std::vector<unsigned int> elecList;
        elecList.reserve(2);
        if (m_mode==0) {
	  infile >> cellID >> electInd >> pathologyType;
          elecList.push_back(electInd);
        } else {
          unsigned int bec,pos_neg,FT,slot,channel,hvModule,hvLine;
          infile >> bec >> pos_neg >> FT >> slot >> channel >> hvModule >> hvLine >> pathologyType;
          ATH_MSG_INFO(" read " << bec << " " << pos_neg << " " << FT << " " << slot << " " << channel << " " << hvModule << " " << hvLine << " " << pathologyType);
          HWIdentifier hwid = m_laronline_id->channel_Id(bec,pos_neg,FT,slot,channel);
          Identifier id = cabling->cnvToIdentifier( hwid);
          cellID = (unsigned int)(id.get_identifier32().get_compact());
          elecList=getElectInd(**hvIdMapping, id,hvModule,hvLine,calodetdescrmgr);
          ATH_MSG_INFO(" cellId , elecList size " << cellID << " " << elecList.size());
        }
        for (unsigned int i=0;i<elecList.size();i++) {
	 LArHVPathologiesDb::LArHVElectPathologyDb electPath{};
	 electPath.cellID = cellID;
	 electPath.electInd = elecList[i];
         if(m_mode==2) electPath.pathologyType = ((pathologyType&0x0FFF)<<4);
         else electPath.pathologyType = pathologyType;
	 pathologies.m_v.push_back(electPath);
	 ATH_MSG_INFO("Created electrode pathology (" << cellID
		       << "," << elecList[i]
		       << "," << pathologyType << ")");
        }
      }
      else if(checkChar==commentSign) {
	// Skip the comment
	std::getline(infile,commentLine);
      }
      else {
	// We found another folder. Stop parsing
	break;
      }
    }

    infile.close();
    ATH_MSG_INFO("Finished parsing input file");    
    return std::make_optional<LArHVPathologiesDb>(pathologies);
}

StatusCode LArHVPathologyDbAlg::printCondObjects (const EventContext& ctx, const CaloDetDescrManager* calodetdescrmgr, 
                                                  const std::vector<LArHVPathologiesDb::LArHVElectPathologyDb>* pathologyContainer) const
{
  ATH_MSG_INFO(" in printCondObjects() ");

  SG::ReadCondHandle<LArHVIdMapping> hvIdMapping (m_hvCablingKey, ctx);
  SG::ReadCondHandle<LArOnOffIdMapping> cabHdl (m_cablingKey, ctx);
  const LArOnOffIdMapping *cabling = *cabHdl;
  if(!cabling) {
     ATH_MSG_ERROR("Do not have cabling object with key " << m_cablingKey.key());
     return StatusCode::FAILURE;
  }

  std::ofstream *fout=nullptr;
  if (!m_hvPathologyKey.empty() &&  pathologyContainer==nullptr) {
    SG::ReadCondHandle<LArHVPathology> pathHdl(m_hvPathologyKey,ctx);
    pathologyContainer=&(pathHdl->getPathology());
  }
  if (!pathologyContainer) {
    ATH_MSG_WARNING("No input data "); 

    return StatusCode::SUCCESS;
  }
  
  else {
    if(!m_outFile.value().empty()) {
       fout = new std::ofstream(m_outFile.value().c_str());
       if((!fout) || (fout && !(fout->good()))) {
             ATH_MSG_WARNING("Could not open output file: " << m_outFile.value());
             fout=nullptr;
             }
       if(fout) *fout<<m_folder.value()<<std::endl;
    }
    for (const LArHVPathologiesDb::LArHVElectPathologyDb& electPath : *pathologyContainer) {
      if(m_mode==0) {
         ATH_MSG_INFO("Got pathology for cell ID: " << electPath.cellID
      	     << "(" << electPath.electInd 
      	     << "," << electPath.pathologyType << ") ");
         if(fout) *fout<<electPath.cellID<<"\t"<<electPath.electInd<<"\t"<<electPath.pathologyType<<std::endl;    
      } else {
         ATH_MSG_INFO("Got pathology for cell ID: " << electPath.cellID);
         HWIdentifier hwid = cabling->createSignalChannelID(Identifier32(electPath.cellID));
         int HVLine=getHVline(**hvIdMapping,Identifier(electPath.cellID),electPath.electInd,calodetdescrmgr);
         if(HVLine<0) {
	   ATH_MSG_ERROR("No HVline for cell "<<electPath.cellID);
         } else {
            int hvmodule=HVLine/1000;
            int hvline=HVLine%1000;
            if(m_mode==1) {
              ATH_MSG_INFO(m_laronline_id->barrel_ec(hwid) << " " << m_laronline_id->pos_neg(hwid) << " " << m_laronline_id->feedthrough(hwid) << " " << m_laronline_id->slot(hwid) << " " << m_laronline_id->channel(hwid) << " " << hvmodule << " " << hvline << " " << electPath.pathologyType);
              if(fout) *fout << m_laronline_id->barrel_ec(hwid) << " " << m_laronline_id->pos_neg(hwid) << " " << m_laronline_id->feedthrough(hwid) << " " << m_laronline_id->slot(hwid) << " " << m_laronline_id->channel(hwid) << " " << hvmodule << " " << hvline << " " << electPath.pathologyType << std::endl;
            } else if (m_mode==2){
              ATH_MSG_INFO(m_laronline_id->barrel_ec(hwid) << " " << m_laronline_id->pos_neg(hwid) << " " << m_laronline_id->feedthrough(hwid) << " " << m_laronline_id->slot(hwid) << " " << m_laronline_id->channel(hwid) << " " << hvmodule << " " << hvline << " " << ((electPath.pathologyType&0xFF0)>>4));
              if(fout) *fout << m_laronline_id->barrel_ec(hwid) << " " << m_laronline_id->pos_neg(hwid) << " " << m_laronline_id->feedthrough(hwid) << " " << m_laronline_id->slot(hwid) << " " << m_laronline_id->channel(hwid) << " " << hvmodule << " " << hvline << " " << ((electPath.pathologyType&0xFFF0)>>4) << std::endl;

            }
         }
      }
    }
  }
  if(fout) fout->close();
  return StatusCode::SUCCESS;
}

std::vector<unsigned int>
LArHVPathologyDbAlg::getElectInd(const LArHVIdMapping& hvIdMapping,
                                 const Identifier & id,
                                 unsigned int module,
                                 unsigned int line,
				 const CaloDetDescrManager* calodetdescrmgr) const
{

  std::vector<unsigned int> list;
  int HVline = 1000*module + line;
// EM calo
  if (m_larem_id->is_lar_em(id)) {
// LAr EMB
     if (abs(m_larem_id->barrel_ec(id))==1 &&  m_larem_id->sampling(id) > 0)  {
       if (const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id))) {
         const EMBCellConstLink cell = embElement->getEMBCell();
         unsigned int nelec = cell->getNumElectrodes();
         for (unsigned int i=0;i<nelec;i++) {
            const EMBHVElectrode& electrode = cell->getElectrode(i);
            for (unsigned int igap=0;igap<2;igap++) {
              if (electrode.hvLineNo(igap, &hvIdMapping)==HVline) {
                  list.push_back(2*i+igap);
              }
            } 
         }
       }
     }
// LAr EMEC
     if (abs(m_larem_id->barrel_ec(id))>1 && m_larem_id->sampling(id) > 0) {
       if (const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id))) {
         const EMECCellConstLink cell = emecElement->getEMECCell();
         unsigned int nelec = cell->getNumElectrodes();
         for (unsigned int i=0;i<nelec;i++) {
            const EMECHVElectrode& electrode = cell->getElectrode(i);
            for (unsigned int igap=0;igap<2;igap++) {
              if (electrode.hvLineNo(igap, &hvIdMapping)==HVline) {
                  list.push_back(2*i+igap);
              }       
            }       
         }    
       }
     }
// EMBPS
     if (abs(m_larem_id->barrel_ec(id))==1 &&  m_larem_id->sampling(id)==0) {
       if (const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id))) {
        const EMBCellConstLink cell = embElement->getEMBCell();
        const EMBPresamplerHVModule& hvmodule =  cell->getPresamplerHVModule ();
        for (unsigned int igap=0;igap<2;igap++) {
           if (hvmodule.hvLineNo(igap, &hvIdMapping)==HVline) {
             list.push_back(igap);
           }
        }
       }
     }
// EMECPS
    if (abs(m_larem_id->barrel_ec(id))>1 && m_larem_id->sampling(id)==0) {
      if (const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id))) {
       const EMECCellConstLink cell = emecElement->getEMECCell();
       const EMECPresamplerHVModule& hvmodule = cell->getPresamplerHVModule ();
       for (unsigned int igap=0;igap<2;igap++) {
        if (hvmodule.hvLineNo(igap, &hvIdMapping)==HVline) {
          list.push_back(igap);
        }
       }
      }
    }
  }
//HEC
  if (m_larhec_id->is_lar_hec(id)) {
    if (const HECDetectorElement* hecElement = dynamic_cast<const HECDetectorElement*>(calodetdescrmgr->get_element(id))) {
      const HECCellConstLink cell = hecElement->getHECCell();
      unsigned int nsubgaps = cell->getNumSubgaps();
      for (unsigned int i=0;i<nsubgaps;i++) {
          const HECHVSubgap& subgap = cell->getSubgap(i);
          if (subgap.hvLineNo(&hvIdMapping)==HVline) {
            list.push_back(i);
          }
      }
    }
  }
//FCAL
  if (m_larfcal_id->is_lar_fcal(id)) {
    if (const FCALDetectorElement* fcalElement = dynamic_cast<const FCALDetectorElement*>(calodetdescrmgr->get_element(id))) {
       const FCALTile* tile = fcalElement->getFCALTile();
       unsigned int nlines = FCALTile::getNumHVLines();
       for (unsigned int i=0;i<nlines;i++) {
         const FCALHVLine* line2 = tile->getHVLine(i);
	 if(line2) {
	   if (line2->hvLineNo(&hvIdMapping)==HVline) {
	     list.push_back(i);
	   }
	 }
       }
    }
  }

  return list;

}

int LArHVPathologyDbAlg::getHVline(const LArHVIdMapping& hvIdMapping,
                                   const Identifier & id,
                                   short unsigned int ElectInd,
				   const CaloDetDescrManager* calodetdescrmgr) const
{

  unsigned int igap, ielec;
// EM calo
  if (m_larem_id->is_lar_em(id)) {
// LAr EMB
     if (abs(m_larem_id->barrel_ec(id))==1 &&  m_larem_id->sampling(id) > 0)  {
       if (const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id))) {
         const EMBCellConstLink cell = embElement->getEMBCell();
         unsigned int nelec = cell->getNumElectrodes();
         igap = ElectInd % 2;
         ielec = std::div(ElectInd - igap, 2).quot;
         if (ielec > nelec) {
            ATH_MSG_ERROR("Wrong electrode number " << ielec << " for cell "<< id.get_identifier32().get_compact());
            return -1;
         } else { 
            return cell->getElectrode(ielec).hvLineNo(igap, &hvIdMapping);
         }
       }
     }
// LAr EMEC
     if (abs(m_larem_id->barrel_ec(id))>1 && m_larem_id->sampling(id) > 0) {
       if (const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id))) {
         const EMECCellConstLink cell = emecElement->getEMECCell();
         unsigned int nelec = cell->getNumElectrodes();
         igap = ElectInd % 2;
         ielec = std::div(ElectInd - igap, 2).quot;
         if (ielec > nelec) {
            ATH_MSG_ERROR("Wrong electrode number " << ielec << " for cell "<< id.get_identifier32().get_compact());
            return -1;
         } else { 
            return cell->getElectrode(ielec).hvLineNo(igap, &hvIdMapping);
         }
       }
     }
// EMBPS
     if (abs(m_larem_id->barrel_ec(id))==1 &&  m_larem_id->sampling(id)==0) {
       if (const EMBDetectorElement* embElement = dynamic_cast<const EMBDetectorElement*>(calodetdescrmgr->get_element(id))) {
        const EMBCellConstLink cell = embElement->getEMBCell();
        const EMBPresamplerHVModule& hvmodule =  cell->getPresamplerHVModule ();
        if(ElectInd >= 2) {
            ATH_MSG_ERROR("Wrong igap "<<ElectInd<<" for EMBPS cell "<<id.get_identifier32().get_compact());
            return -1;
        } else {
            return hvmodule.hvLineNo(ElectInd, &hvIdMapping);
        }
       }
     }
// EMECPS
    if (abs(m_larem_id->barrel_ec(id))>1 && m_larem_id->sampling(id)==0) {
      if (const EMECDetectorElement* emecElement = dynamic_cast<const EMECDetectorElement*>(calodetdescrmgr->get_element(id))) {
       const EMECCellConstLink cell = emecElement->getEMECCell();
       const EMECPresamplerHVModule& hvmodule = cell->getPresamplerHVModule ();
        if(ElectInd >= 2) {
            ATH_MSG_ERROR("Wrong igap "<<ElectInd<<" for EMECPS cell "<<id.get_identifier32().get_compact());
            return -1;
        } else {
            return hvmodule.hvLineNo(ElectInd, &hvIdMapping);
        }
      }
    }
  }
//HEC
  if (m_larhec_id->is_lar_hec(id)) {
    if (const HECDetectorElement* hecElement = dynamic_cast<const HECDetectorElement*>(calodetdescrmgr->get_element(id))) {
      const HECCellConstLink cell = hecElement->getHECCell();
      unsigned int nsubgaps = cell->getNumSubgaps();
      if( ElectInd >= nsubgaps) {
         ATH_MSG_ERROR("Wrong igap "<<ElectInd<<" for HEC cell "<<id.get_identifier32().get_compact());
         return -1;
      } else {
         return cell->getSubgap(ElectInd).hvLineNo(&hvIdMapping);
      }
    }
  }
//FCAL
  if (m_larfcal_id->is_lar_fcal(id)) {
    if (const FCALDetectorElement* fcalElement = dynamic_cast<const FCALDetectorElement*>(calodetdescrmgr->get_element(id))) {
       const FCALTile* tile = fcalElement->getFCALTile();
       unsigned int nlines = FCALTile::getNumHVLines();
      if( ElectInd >= nlines) {
         ATH_MSG_ERROR("Wrong line "<<ElectInd<<" for FCAL cell "<<id.get_identifier32().get_compact());
         return -1;
      } else {
         const FCALHVLine* line2 = tile->getHVLine(ElectInd);
         if(line2) {
	   return line2->hvLineNo(&hvIdMapping);
         } else {
	   ATH_MSG_ERROR("Do not have HVLine for "<<ElectInd<<" for FCAL cell "<<id.get_identifier32().get_compact());
	   return -1;
	 }
      }
    }
  }

  // should not get up to this point....
  return -1;

}

std::unique_ptr<AthenaAttributeList> LArHVPathologyDbAlg::hvPathology2AttrList(const LArHVPathologiesDb& pathologyContainer) const {

  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification();
  spec->extend("blobVersion",
               "unsigned int");       // Should allow schema evolution if needed
  spec->extend("Constants", "blob");  // Holds the container

  std::unique_ptr<AthenaAttributeList> attrList = std::make_unique<AthenaAttributeList>(*spec);

  (*attrList)["blobVersion"].data<unsigned int>() = (unsigned int)0;
  coral::Blob& blob = (*attrList)["Constants"].data<coral::Blob>();

  TClass* klass = TClass::GetClass("LArHVPathologiesDb");
  if (klass == nullptr) {
    ATH_MSG_ERROR("Can't find TClass LArHVPathologiesDb");
    return nullptr;
  } else
    ATH_MSG_DEBUG("Got TClass LArHVPathologiesDb");

  TBufferFile buf(TBuffer::kWrite);

  if (buf.WriteObjectAny(&pathologyContainer, klass) != 1) {
    ATH_MSG_ERROR("Failed to stream LArHVPathologiesDb");
    return nullptr;
  }

  blob.resize(buf.Length());
  void* adr = blob.startingAddress();
  memcpy(adr, buf.Buffer(), buf.Length());
  return attrList;
}
