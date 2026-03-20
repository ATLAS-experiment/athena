/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TileCellContainerCnv.h"
#include "CaloIdentifier/TileTBID.h"
#include "CaloDetDescr/MbtsDetDescrManager.h"

#include "GaudiKernel/StatusCode.h"


TileCellContainerCnv::TileCellContainerCnv(ISvcLocator* svcloc)
  : TileCellContainerCnvBase::T_AthenaPoolCustomCnv(svcloc, "TileCellContainerCnv"),
    m_storeGate("StoreGateSvc", "TileCellContainerCnv")
{
}

TileCellContainerCnv::~TileCellContainerCnv()
{
}

StatusCode TileCellContainerCnv::initialize()
{
    // Call base clase initialize
    ATH_CHECK( AthenaPoolConverter::initialize() );

    // version 2 by default
    m_version = 2;
    
    // Get the messaging service, print where you are
    ATH_MSG_INFO("TileCellContainerCnv::initialize(), packing format version " << m_version);

    // get StoreGate service
    StatusCode sc = m_storeGate.retrieve();
    if (sc.isFailure()) {
      this->initIdToIndex();
      ATH_MSG_ERROR("StoreGate service not found !");
    }

    sc = detStore()->retrieve(m_tileTBID);
    if (sc.isFailure()) {
      this->initIdToIndex();
      ATH_MSG_ERROR("No TileTBID helper");
    } else {
      for (int side=0; side<NSIDE; ++side) {
        for (int phi=0; phi<NPHI; ++phi) {
          for (int eta=0; eta<NETA; ++eta) {
            m_id[cell_index(side,phi,eta)] = m_tileTBID->channel_id((side>0)?1:-1,phi,eta);
          }
        }
      }
    }
    
    sc = detStore()->retrieve(m_mbtsMgr);
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Unable to retrieve MbtsDetDescrManager from DetectorStore");
      memset(m_dde,0,sizeof(m_dde));
    } else {
      for (int side=0; side<NSIDE; ++side) {
        for (int phi=0; phi<NPHI; ++phi) {
          for (int eta=0; eta<NETA; ++eta) {
            m_dde[cell_index(side,phi,eta)] = m_mbtsMgr->get_element(m_id[cell_index(side,phi,eta)]);
          }
        }
      }
    }
    

    // set CaloGain <-> gain index mapping for all possible TileCal gains
    for (int i=0; i<17; ++i)  m_gainIndex[i] = 8;
    m_gain[0] = -2; // put non-existing gain here
    m_gainIndex[-CaloGain::TILELOWLOW]   = 8 + 1;  m_gain[1] = (int)CaloGain::TILELOWLOW;
    m_gainIndex[-CaloGain::TILELOWHIGH]  = 8 + 2;  m_gain[2] = (int)CaloGain::TILELOWHIGH;
    m_gainIndex[-CaloGain::TILEHIGHLOW]  = 8 + 3;  m_gain[3] = (int)CaloGain::TILEHIGHLOW;
    m_gainIndex[-CaloGain::TILEHIGHHIGH] = 8 + 4;  m_gain[4] = (int)CaloGain::TILEHIGHHIGH;
    m_gainIndex[-CaloGain::TILEONELOW]   = 8 + 5;  m_gain[5] = (int)CaloGain::TILEONELOW;
    m_gainIndex[-CaloGain::TILEONEHIGH]  = 8 + 6;  m_gain[6] = (int)CaloGain::TILEONEHIGH;
    m_gainIndex[-CaloGain::INVALIDGAIN]  = 8 + 7;  m_gain[7] = (int)CaloGain::INVALIDGAIN;
    
    return StatusCode::SUCCESS;
}

void TileCellContainerCnv::initIdToIndex()
{
  for (int side=0; side<NSIDE; ++side) {
    for (int phi=0; phi<NPHI; ++phi) {
      for (int eta=0; eta<NETA; ++eta) {
        m_id[cell_index(side,phi,eta)] = (Identifier)cell_index(side,phi,eta);
      }
    }
  }
}

TileCellVec* TileCellContainerCnv::createPersistent(TileCellContainer* cont)
{
    // Convert every TileCell to 3 32-bit integers: ID,Ene, and (time,qual,qain)

    std::string name = m_storeGate->proxy(cont)->name();
    auto vecCell = std::make_unique<TileCellVec>();
    vecCell->reserve(NCELLMBTS);

    ATH_MSG_DEBUG("storing TileCells from " << name << " in POOL");

    vecCell->push_back(m_version);
    int nMBTSfound=0;

    std::vector<const TileCell *> allCells;
    
    switch (m_version) {

    case 1: // 3 words per cell, energy scale factor is 1000, time scale factor is 100
      for (const TileCell* cell : *cont) {
        ATH_MSG_VERBOSE("ene=" << cell->energy()
                        << " time=" << cell->time()
                        << " qual=" << (int)cell->qual1()
                        << " gain=" << (int)cell->gain());
        unsigned int id  = cell->ID().get_identifier32().get_compact();
        int          ene = round32(cell->energy() * 1000.);
        unsigned int tim = 0x8000 + round16(cell->time()*100.);
        unsigned int qua = std::max(0, std::min(0xFF, (int)cell->qual1()));
        unsigned int gai = std::max(0, std::min(0xFF,   0x80   + (int)(cell->gain())));
        unsigned int tqg = (tim<<16) | (qua<<8) | gai;
        vecCell->push_back(id);
        vecCell->push_back((unsigned int)ene);
        vecCell->push_back(tqg);
        ATH_MSG_VERBOSE("packing cell in three words " << MSG::hex << id <<
                        " " << ene << " " << tqg << MSG::dec);
      }
      break;

    case 2: // 1 or 2 words for MBTS cells, 3 words for others, energy scale factor is 1000, time scale factor is 100

      // prepare vector with all cells first, expect at least 32 MBTS cells
      allCells.resize(NCELLMBTS);
      for (const TileCell* cell : *cont) {
        Identifier id = cell->ID();
        if (m_tileTBID->is_tiletb(id)) {
          int side = std::max(0,m_tileTBID->type(id));
          int phi  = m_tileTBID->module(id);
          int eta  = m_tileTBID->channel(id);
          int ind  = cell_index(side,phi,eta);
          if (eta<NETA && phi<NPHI && ind < NCELLMBTS) {
            allCells[ind] = cell;
            ++nMBTSfound;
          } else {
            allCells.push_back(cell);
          }
        } else {
          allCells.push_back(cell);
        }
      }

     if (nMBTSfound>0) {

      // save first 32 cells (MBTS) without identifiers, 2 words per cell, put zeros for empty cells
      // if MBTS energy is in pCb, then LSB corresponds to 1/12 ADC count of high gain
      for (int ind=0; ind<NCELLMBTS; ++ind) {
        int energy = 0;
        int time   = 0;
        int quality= 0;
        int gain   = m_gain[0]; // non-existing gain in CaloGain - to mark non-existing cells
        const TileCell* cell = allCells[ind];
        if (cell) {
          energy = round32(cell->energy() * 1000.);
          time   = round16(cell->time() * 100.);
          quality= cell->qual1();
          gain   = cell->gain();

          ATH_MSG_VERBOSE("ind="  << ind <<
                          " ene=" << cell->energy() <<
                          " time=" << cell->time() <<
                          " qual=" << (int)cell->qual1() <<
                          " gain=" << (int)cell->gain());
        }
        else {
          ATH_MSG_VERBOSE("ind="  << ind << " create MBTS cell with zero energy");
        }
        
        // put correct MBTS cells in one word
        if (time == 0 && // expect time to be equal to zero
            -0x10000 < energy && energy < 0xEFFFF && // expect energy within (-65,980) pCb 
            -17 < gain && gain < 0 ) { // expext only gains in TileCal range

          unsigned int ene = energy+0x10000;  // shift by 65 pCb (65*10^3 because of scaling)
          unsigned int qua = std::max(0, std::min(0xFF, quality)); // 8 bits for quality
          unsigned int gai = m_gainIndex[-gain];
          unsigned int gqe = (gai << 28) | (qua<<20) | ene; // upper most bit is always 1 here
          vecCell->push_back(gqe);

          ATH_MSG_VERBOSE("packing cell " << ind << " in one word " <<
                          MSG::hex << gqe << MSG::dec);

        } else { // cells with time, use 2 words for channel
                 // but make sure that upper most bit in energy word is zero

          unsigned int ene = std::max(0, std::min(0x7FFFFFFF, 0x40000000 + energy));
          unsigned int tim = std::max(0, std::min(0xFFFF, 0x8000 + time));
          unsigned int qua = std::max(0, std::min(0xFF, quality)); // 8 bits for quality
          unsigned int gai = std::max(0, std::min(0xFF,   0x80   + gain));
          unsigned int tqg = (tim<<16) | (qua<<8) | gai;
          vecCell->push_back(ene);
          vecCell->push_back(tqg);

          ATH_MSG_VERBOSE("packing cell " << ind << " in two words " <<
                          MSG::hex << ene << " " << tqg << MSG::dec);
        }
      }

     } else {

       (*vecCell)[0] = 1; // no MBTS found - use version 1 for packing
     }

      // keep all other cells (if any) with identifiers, 3 words per cell
      for (unsigned int ind=NCELLMBTS; ind<allCells.size(); ++ind) {
        
        const TileCell* cell = allCells[ind];

        ATH_MSG_VERBOSE("ind="  << ind <<
                        " ene=" << cell->energy() <<
                        " time=" << cell->time() <<
                        " qual=" << (int)cell->qual1() <<
                        " gain=" << (int)cell->gain());

        unsigned int id  = cell->ID().get_identifier32().get_compact();
        int          ene = round32(cell->energy() * 1000.);
        unsigned int tim = 0x8000 + round16(cell->time()*100.);
        unsigned int qua = std::max(0, std::min(0xFF, (int)cell->qual1()));
        unsigned int gai = std::max(0, std::min(0xFF,   0x80   + (int)(cell->gain())));
        unsigned int tqg = (tim<<16) | (qua<<8) | gai;
        vecCell->push_back(id);
        vecCell->push_back((unsigned int)ene);
        vecCell->push_back(tqg);

        ATH_MSG_VERBOSE("packing cell " << ind << " in three words " <<
                        MSG::hex << id << " " << ene << " " << tqg << MSG::dec);
      }
      break;

    default:

      ATH_MSG_ERROR("Unknown version of TileCellVec, ver="<<m_version);

    }

    ATH_MSG_DEBUG("Storing data vector of size " << vecCell->size() << " with version " << vecCell->front());

    return vecCell.release();
}

TileCellContainer* TileCellContainerCnv::createTransient(const Token* /*token*/)
{
    // Fill TileCellContainer from vector, creating cells from 3 integers 

    std::unique_ptr<TileCellVec> vec(this->poolReadObject<TileCellVec>());

    ATH_MSG_DEBUG("Read TileCell Vec, size " << vec->size());

    // create the TileCellContainer
    auto cont = std::make_unique<TileCellContainer>();

    TileCellVec::const_iterator it   = vec->begin();
    TileCellVec::const_iterator last = vec->end();

    unsigned int version = *it++;
    int iCell = 0;
    
    switch (version) {
    case 1:

      for (; it != last; ) {

        Identifier id(Identifier32(*it++));
        int ene = (int)(*it++);
        unsigned int tqg = *it++;
      
        float ener = ene*1e-3;
        float time = ((int)(tqg>>16) - 0x8000 ) * 0.01;
        uint16_t qual = ((tqg>>8) & 0xFF);
        uint16_t qbit = TileCell::MASK_CMPC | TileCell::MASK_TIME;
        int   gain = (int)(tqg & 0xFF) - 0x80;

        ATH_MSG_VERBOSE("reading cell " << (iCell++) << " " <<
                        MSG::hex << id << MSG::dec << " " << ene << " " <<
                        MSG::hex << tqg << MSG::dec);

        ATH_MSG_VERBOSE("ene=" << ener << " time=" << time <<
                        " qual=" << qual << " gain=" << gain);

        TileCell * cell = new TileCell(NULL,id,ener,time,qual,qbit,(CaloGain::CaloGain)gain);
        cont->push_back(cell);
      }
      break;

    case 2:
      
      for (; it != last; ) {

        Identifier id;
        CaloDetDescrElement * dde = NULL;
        float ener = 0.0;
        float time = 0.0;
        uint16_t qual = 0;
        uint16_t qbit = TileCell::MASK_CMPC | TileCell::MASK_TIME;
        int gain = m_gain[0]; // non-existing gain in CaloGain - to mark non-existing cells

        if (msgLvl(MSG::VERBOSE))
          msg() << MSG::VERBOSE << "reading cell " << iCell << " ";

        if (iCell < NCELLMBTS) { // first 32 cells are MBTS cells without identifier

          id = m_id[iCell]; // identifier is taken from array
          dde = m_dde[iCell]; // mbtsDDE is taken from array

          int ene = (int)(*it++); // first word is energy

          if (msgLvl(MSG::VERBOSE))
            msg() << MSG::hex << id << " " << ene << " " << MSG::dec;

          if (ene < 0 ) { // upper most bit is set, it means that everything is packed in one word

            if (msgLvl(MSG::VERBOSE))
              msg() << endmsg;

            time = 0.0;   // time was zero and it was not saved
            ener = ((ene & 0xFFFFF) - 0x10000) * 1e-3;
            qual = ((ene>>20) & 0xFF);
            gain = m_gain[((ene>>28) & 0x7)]; // gain is taken from array
            
          } else { // two words packing
          
            unsigned int tqg = *it++;
            ATH_MSG_VERBOSE(MSG::hex << tqg << MSG::dec);

            ener = (ene - 0x40000000) * 1e-3;
            time = ((int)(tqg>>16) - 0x8000 ) * 0.01;
            qual = ((tqg>>8) & 0xFF);
            gain = (int)(tqg & 0xFF) - 0x80;
          }

        } else { // three words packing for remaining cells

          id = Identifier(Identifier32(*it++));
          int ene = (int)(*it++);
          unsigned int tqg = *it++;

          ATH_MSG_VERBOSE(MSG::hex << id << MSG::dec << " " << ene <<
                          " " << MSG::hex << tqg << MSG::dec);

          ener = ene*1e-3;
          time = ((int)(tqg>>16) - 0x8000 ) * 0.01;
          qual = ((tqg>>8) & 0xFF);
          gain = (int)(tqg & 0xFF) - 0x80;
        }

        ATH_MSG_VERBOSE("ene=" << ener << " time=" << time
                        << " qual=" << qual << " gain=" << gain);

        if (gain != m_gain[0]) { // don't create cells with non-existing gain
          TileCell * cell = new TileCell(dde,id,ener,time,qual,qbit,(CaloGain::CaloGain)gain);
          cont->push_back(cell);
        }
        else {
          ATH_MSG_VERBOSE("Don't create MBTS cell with invalid gain");
        }
        ++iCell;
      }
      break;
      
    default:

      ATH_MSG_ERROR("Unknown version of TileCellVec, ver="<<version);
    }
    
    return cont.release();
}
