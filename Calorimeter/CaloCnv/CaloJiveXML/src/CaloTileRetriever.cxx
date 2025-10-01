/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTileRetriever.h"

#include "AthenaKernel/Units.h"

#include "EventContainers/SelectAllObject.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "TileEvent/TileCell.h"
#include "TileEvent/TileCellContainer.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBID.h"
#include "TileConditions/TileInfo.h"
#include "TileConditions/TileCablingService.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include <vector>
#include <cstddef>
#include <map>
#include <stdexcept>

using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  CaloTileRetriever::CaloTileRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_calocell_id(nullptr)
  {
   //Only declare the interface
   declareInterface<IDataRetriever>(this);

   declareProperty("CellThreshold", m_cellThreshold = 50.);
   declareProperty("RetrieveTILE" , m_tile = true);
   declareProperty("DoTileDigit",   m_doTileDigit = false);
   declareProperty("DoBadTile",     m_doBadTile = false);
   declareProperty("DoTileCellDetails",  m_doTileCellDetails = false);
   declareProperty("CellEnergyPrec", m_cellEnergyPrec = 3);
   declareProperty("CellTimePrec", m_cellTimePrec = 3);

   // TileDigitsContainer names: {"TileDigitsCnt","TileDigitsFlt"};
   declareProperty("TileDigitsContainer" ,m_sgKeyTileDigits = "",
        "Input collection to retrieve Tile digits, used when doTileDigit is True");

   // TileRawChannelContainer names: {"TileRawChannelOpt2","TileRawChannelOpt","TileRawChannelFixed",
   //                                 "TileRawChannelFitCool","TileRawChannelFit",
   //                                 "TileRawChannelCnt","TileRawChannelFlt"};
   declareProperty("TileRawChannelContainer" ,m_sgKeyTileRawChannel = "",
        "Input collection to retrieve Tile raw channels, used when doTileCellDetails is True.");
  }

  /**
   * Initialise the ToolSvc
   */

  StatusCode CaloTileRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool" );
    ATH_CHECK( detStore()->retrieve (m_calocell_id, "CaloCell_ID") );

    //=== get TileCondToolTiming
    ATH_CHECK( m_tileToolTiming.retrieve() );

    //=== get TileCondToolEmscale
    ATH_CHECK( m_tileToolEmscale.retrieve() );

    //=== get TileBadChanTool
    ATH_CHECK( m_tileBadChanTool.retrieve() );
    
    ATH_CHECK( m_sgKey.initialize() );

    ATH_CHECK( m_sgKeyTileDigits.initialize(m_doTileDigit) );

    ATH_CHECK( m_sgKeyTileRawChannel.initialize(m_doTileCellDetails) );

    return StatusCode::SUCCESS;
  }

  /**
   * Tile data retrieval from chosen collection
   */
  StatusCode CaloTileRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG( "in retrieve()"  );

    SG::ReadHandle<CaloCellContainer> cellContainer(m_sgKey);
    if (!cellContainer.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve Calorimeter Cells "  );
    }
    else{
      if (m_tile) {
        DataMap data = getCaloTileData(&(*cellContainer));
        ATH_CHECK( FormatTool->AddToEvent("TILE", m_sgKey.key(), &data) );
        ATH_MSG_DEBUG( "Tile retrieved"  );
      }
    }

    //Tile cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve Tile cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloTileRetriever::getCaloTileData(const CaloCellContainer* cellContainer) {

    ATH_MSG_DEBUG( "getTileData()"  );
    char rndStr[30];
    DataMap DataMap;

    DataVect phi; phi.reserve(cellContainer->size());
    DataVect eta; eta.reserve(cellContainer->size());
    DataVect idVec; idVec.reserve(cellContainer->size());
    DataVect energyVec; energyVec.reserve(cellContainer->size());
    DataVect adcCounts1Vec; adcCounts1Vec.reserve(cellContainer->size() * 10);
    DataVect adcCounts2Vec; adcCounts2Vec.reserve(cellContainer->size() * 10);
    DataVect BadCell; BadCell.reserve(cellContainer->size());

    DataVect pmt1Energy; pmt1Energy.reserve(cellContainer->size());
    DataVect pmt1Time; pmt1Time.reserve(cellContainer->size());
    DataVect pmt1Chi2; pmt1Chi2.reserve(cellContainer->size());
    DataVect pmt1Gain; pmt1Gain.reserve(cellContainer->size());
    DataVect pmt2Energy; pmt2Energy.reserve(cellContainer->size());
    DataVect pmt2Time; pmt2Time.reserve(cellContainer->size());
    DataVect pmt2Chi2; pmt2Chi2.reserve(cellContainer->size());
    DataVect pmt2Gain; pmt2Gain.reserve(cellContainer->size());

    DataVect pmt1RawAmplitude; pmt1RawAmplitude.reserve(cellContainer->size());
    DataVect pmt1RawTime; pmt1RawTime.reserve(cellContainer->size());
    DataVect pmt1Pedestal; pmt1Pedestal.reserve(cellContainer->size());
    DataVect pmt1Number; pmt1Number.reserve(cellContainer->size());
    DataVect pmt1ADCStatus; pmt1ADCStatus.reserve(cellContainer->size());
    DataVect pmt2RawAmplitude; pmt2RawAmplitude.reserve(cellContainer->size());
    DataVect pmt2RawTime; pmt2RawTime.reserve(cellContainer->size());
    DataVect pmt2Pedestal; pmt2Pedestal.reserve(cellContainer->size());
    DataVect pmt2Number; pmt2Number.reserve(cellContainer->size());
    DataVect pmt2ADCStatus; pmt2ADCStatus.reserve(cellContainer->size());

//    m_sub; m_sub.reserve(cellContainer->size());
    m_sub.clear();

    std::string adcCounts1Str = "adcCounts1 multiple=\"0\"";
    std::string adcCounts2Str = "adcCounts2 multiple=\"0\"";

    const TileID* tileID = nullptr;
    const TileHWID* tileHWID = nullptr;
    const TileInfo* tileInfo = nullptr;
    const TileCablingService* cabling=nullptr;
    TileRawChannelUnit::UNIT RChUnit = TileRawChannelUnit::ADCcounts;  //!< Unit for TileRawChannels (ADC, pCb, etc.)
    cabling = TileCablingService::getInstance();
    double energyGeV;
    double amplitude = 0.;
    const int vsize = cellContainer->size();  //5184;
    int Index,pmtInd, cellInd, nTileSamples=0;
    std::vector<double> pmt1ped(vsize,0.0);
    std::vector<double> pmt2ped(vsize,0.0);
    std::vector<double> pmt1rawamp(vsize,0.0);
    std::vector<double> pmt2rawamp(vsize,0.0);
    std::vector<double> pmt1rawtime(vsize,0.0);
    std::vector<double> pmt2rawtime(vsize,0.0);
    std::vector<int> pmt1number(vsize,0);
    std::vector<int> pmt2number(vsize,0);
    std::vector<unsigned long int> pmt1status(vsize,0);
    std::vector<unsigned long int> pmt2status(vsize,0);
    std::map<int,std::vector<float> > pmt1digit;
    std::map<int,std::vector<float> > pmt2digit;
    bool offlineRch = false;

    //===== retrieving everything which is needed for Tile

    if (detStore()->retrieve(tileID).isFailure()) {
      ATH_MSG_ERROR( "in getCaloTileData(), Could not retrieve TileID"  );

    }

    if (detStore()->retrieve(tileHWID).isFailure()) {
      ATH_MSG_ERROR( "in getCaloTileData(), Could not retrieve TileHWID"  );
    }

    if (detStore()->retrieve(tileInfo, "TileInfo").isFailure()) {
      ATH_MSG_ERROR( "in getCaloTileData(), Could not retrieve TileInfo" );
    }

    SG::ReadHandle<TileDigitsContainer> tileDigits;
    if (m_doTileDigit) {
      tileDigits = SG::makeHandle(m_sgKeyTileDigits);
      if (!tileDigits.isValid()){
         ATH_MSG_WARNING( "Could not retrieve TileDigits "  );
      }
    }

    SG::ReadHandle<TileRawChannelContainer> RawChannelCnt;
    if (m_doTileCellDetails) {
      RawChannelCnt = SG::makeHandle(m_sgKeyTileRawChannel);
      if (!RawChannelCnt.isValid()){
         ATH_MSG_WARNING( "Could not retrieve TileRawChannel "  );
      }
      else {
          RChUnit = RawChannelCnt->get_unit();
          offlineRch = (RChUnit<TileRawChannelUnit::OnlineADCcounts && 
                        RawChannelCnt->get_type() != TileFragHash::OptFilterDsp);
      }
    }


    // Loop Over TileRawChannelContainer to retrieve raw information. Keep the values in vectors

    if (m_doTileCellDetails && RawChannelCnt.isValid()) {
     if (offlineRch) {

      for (const auto rawChannel : *RawChannelCnt) {

         for (const auto cell : *rawChannel) {

          /*Identifier cell_id =*/ cell->cell_ID_index(Index,pmtInd);
          if (Index <= -1 ) continue;  //disconnect channel index is -1 and MBTS is -2. They do not have an idhash
          IdentifierHash cell_hash = m_calocell_id->calo_cell_hash( cell->cell_ID() );
          cellInd = cellContainer->findIndex(cell_hash);  //find Cell Index
          if (cellInd < 0) continue;

          HWIdentifier hwid=cell->adc_HWID();
          int adc       = tileHWID->adc(hwid);
          int channel   = tileHWID->channel(hwid);
          int drawer    = tileHWID->drawer(hwid);
          int ros       = tileHWID->ros(hwid);
          int PMT = abs( cabling->channel2hole(ros,channel) );
          int drawerIdx = TileCalibUtils::getDrawerIdx(ros,drawer);
          uint32_t tileAdcStatus = m_tileBadChanTool->encodeStatus(m_tileBadChanTool->getAdcStatus(drawerIdx,channel,adc));

          amplitude = cell->amplitude();
          //Change amplitude units to ADC counts
          if (TileRawChannelUnit::ADCcounts < RChUnit && RChUnit < TileRawChannelUnit::OnlineADCcounts) {
            amplitude /= m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, 1.0, TileRawChannelUnit::ADCcounts, RChUnit);
          } else if (RChUnit > TileRawChannelUnit::OnlineADCcounts) {
            // Should never get here due to offlineRch test above.
            //amplitude = m_tileToolEmscale->undoOnlCalib(drawerIdx, channel, adc, amplitude, RChUnit);
            std::abort();
          }

          if ( pmtInd == 0 ) { // first PMT

            pmt1ped[cellInd] = cell->pedestal();
            pmt1rawamp[cellInd] = amplitude;
            pmt1rawtime[cellInd] = cell->uncorrTime();
            pmt1number[cellInd] = PMT;
            pmt1status[cellInd] = tileAdcStatus;
          }
          else { // second PMT

            pmt2ped[cellInd] = cell->pedestal();
            pmt2rawamp[cellInd] = amplitude;
            pmt2rawtime[cellInd] = cell->uncorrTime();
            pmt2number[cellInd] = PMT;
            pmt2status[cellInd] = tileAdcStatus;

          }
        }
      }//for TileRawChannelContainer loop
     }
    } // end of doTileCellDetails


    //Loop over TileDigitsContainer to retrieve digits. Keep the digits values in a map

    if (m_doTileDigit && tileDigits.isValid()) {

      //----- get tile digits--------------------------

      // tile digits loop
      for (const auto digitChannel : *tileDigits) {

        for (const auto cell : *digitChannel) {

          /*Identifier cell_id =*/ cell->cell_ID_index(Index,pmtInd);
          if (Index <= -1 ) continue; //disconnect channel index is -1 and MBTS is -2. They do not have an idhash
          IdentifierHash cell_hash = m_calocell_id->calo_cell_hash( cell->cell_ID() );
          cellInd = cellContainer->findIndex(cell_hash);  //find Cell Index
          if (cellInd < 0) continue;
          if ( (*cellContainer)[cellInd]->energy() < m_cellThreshold) continue;

          nTileSamples = cell->NtimeSamples();
          std::vector<float> tileSamples = cell->samples();

          if (pmtInd == 0 ) { // first PMT
            pmt1digit.insert(std::make_pair( cellInd, tileSamples ) );
          }
          else { // second PMT
            pmt2digit.insert(std::make_pair( cellInd, tileSamples ) );
          }

        }
      }//for TileDigitContainer loop
    } // end if doTileDigit


    //Loop Over CaloCellContainer to retrieve TileCell information

    CaloCellContainer::const_iterator it1 = cellContainer->beginConstCalo(CaloCell_ID::TILE);
    CaloCellContainer::const_iterator it2 = cellContainer->endConstCalo(CaloCell_ID::TILE);

    double energyAllTile = 0.; 

    for (;it1!=it2;++it1) {

      if ((*it1)->badcell()) BadCell.push_back(1);
      else if ((*it1)->energy()>= m_cellThreshold) BadCell.push_back(0);
      else BadCell.push_back(-1);

      Identifier cellid = (*it1)->ID();
      IdentifierHash cell_hash = m_calocell_id->calo_cell_hash( cellid );
      cellInd = cellContainer->findIndex(cell_hash);
      if (cellInd < 0) continue;
      calcTILELayerSub(cellid);

      energyGeV = (*it1)->energy()*(1./GeV);
      energyVec.push_back(DataType( gcvt( energyGeV, m_cellEnergyPrec, rndStr) ));
      energyAllTile += energyGeV;

      idVec.push_back(DataType( (Identifier::value_type)(*it1)->ID().get_compact() ));
      phi.push_back(DataType((*it1)->phi()));
      eta.push_back(DataType((*it1)->eta()));

      if (m_doTileDigit && tileDigits.isValid()) {

        if ( !pmt1digit[cellInd].empty()) {
          for (int i=0; i<nTileSamples; i++) {
            adcCounts1Str="adcCounts1 multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts1Vec.push_back(DataType( int(pmt1digit[cellInd][i])));
          }
        }
        else {
          for (int i=0; i<nTileSamples; i++) {
            adcCounts1Str="adcCounts1 multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts1Vec.push_back(DataType(0));
          }
        }


        if ( !pmt2digit[cellInd].empty()) {
          for (int i=0; i<nTileSamples; i++) {
            adcCounts2Str="adcCounts2 multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts2Vec.push_back(DataType( int(pmt2digit[cellInd][i])));
          }
        }
        else {
          for (int i=0; i<nTileSamples; i++) {
            adcCounts2Str= "adcCounts2 multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts2Vec.push_back(DataType(0));
          }
        }
      }

      if (m_doTileCellDetails) {

        const TileCell* theTileCell = dynamic_cast<const TileCell *>(*it1);
        if (not theTileCell) {
          //should never happen
          throw std::runtime_error("Could not cast pointer to TileCell in CaloTileRetriever::getCaloTileData");
        }
        int  gain1  = theTileCell->gain1();
        int  qual1  = theTileCell->qual1();
        bool badch1 = theTileCell->badch1();
        bool noch1  = (gain1<0 || gain1>1);

        int  qual2  = theTileCell->qual2();
        int  gain2  = theTileCell->gain2();
        bool badch2 = theTileCell->badch2();
        bool noch2  = (gain2<0 || gain2>1);

        if (badch1 != badch2 && qual1 != qual2 && qual1 < 255 && qual2 < 255) {
          if (badch1 && !noch1) gain1 = 1 - gain1;
          if (badch2 && !noch1) gain2 = 1 - gain2;
        }
        if (badch1) qual1 = -qual1;
        if (badch2) qual2 = -qual2;

        pmt1Energy.push_back(DataType(theTileCell->ene1()*(1./GeV)));
        pmt1Time.push_back(DataType(theTileCell->time1()));
        pmt1Chi2.push_back(DataType(qual1));
        pmt1Gain.push_back(DataType(gain1));

        pmt2Energy.push_back(DataType(theTileCell->ene2()*(1./GeV)));
        pmt2Time.push_back(DataType(theTileCell->time2()));
        pmt2Chi2.push_back(DataType(qual2));
        pmt2Gain.push_back(DataType(gain2));

        if (offlineRch && RawChannelCnt.isValid() 
            && (noch1 || pmt1number[cellInd]!=0) && (noch2 || pmt2number[cellInd]!=0)) {

          uint32_t tileAdcStatus = pmt1status[cellInd];
          if (badch1) tileAdcStatus += 10;

          pmt1RawAmplitude.push_back(DataType(pmt1rawamp[cellInd]));
          pmt1RawTime.push_back(DataType(pmt1rawtime[cellInd]));
          pmt1Pedestal.push_back(DataType(pmt1ped[cellInd]));
          pmt1Number.push_back(DataType(pmt1number[cellInd]));
          pmt1ADCStatus.push_back(DataType(tileAdcStatus));

          tileAdcStatus = pmt2status[cellInd];
          if (badch2) tileAdcStatus += 10;

          pmt2RawAmplitude.push_back(DataType(pmt2rawamp[cellInd]));
          pmt2RawTime.push_back(DataType(pmt2rawtime[cellInd]));
          pmt2Pedestal.push_back(DataType(pmt2ped[cellInd]));
          pmt2Number.push_back(DataType(pmt2number[cellInd]));
          pmt2ADCStatus.push_back(DataType(tileAdcStatus));
        }
        else {  //// don't have TileRawChannel container, but want raw amplitude (for DPD input)

          float maxTime = (tileInfo->NdigitSamples()/2) * 25;

          if (noch1 /* || pmt1digit[cellInd].empty() */ ) { //invalid gain - channel missing or digits not include in DPD
            pmt1RawAmplitude.push_back(DataType(0));
            pmt1RawTime.push_back(DataType(0));
            pmt1ADCStatus.push_back(DataType(0));
            pmt1Number.push_back(DataType(0));
            pmt1Pedestal.push_back(DataType(0));  //The is no pedestal in DPD.
          } else {
            HWIdentifier hwid = cabling->s2h_adc_id(tileID->adc_id(cellid,0,gain1));

            int adc       = tileHWID->adc(hwid);
            int channel   = tileHWID->channel(hwid);
            int drawer    = tileHWID->drawer(hwid);
            int ros       = tileHWID->ros(hwid);
            int PMT = abs( cabling->channel2hole(ros,channel) );
            int drawerIdx = TileCalibUtils::getDrawerIdx(ros,drawer);
            float scale = m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, 1.0,
                                                          TileRawChannelUnit::ADCcounts, TileRawChannelUnit::MegaElectronVolts);
            float amp = theTileCell->ene1() / scale;
            float time = theTileCell->time1();

            int qbit = (theTileCell->qbit1() & TileCell::MASK_TIME);
            if ((qual1 != 0 || qbit != 0 || amp != 0.0) && (fabs(time) < maxTime && time != 0.0)) {
              time += m_tileToolTiming->getSignalPhase(drawerIdx, channel, adc);
            }

            uint32_t tileAdcStatus = m_tileBadChanTool->encodeStatus(m_tileBadChanTool->getAdcStatus(drawerIdx,channel,adc));
            if (badch1) tileAdcStatus += 10;

            pmt1RawAmplitude.push_back(DataType(amp));
            pmt1RawTime.push_back(DataType(time));
            pmt1Number.push_back(DataType(PMT));
            pmt1ADCStatus.push_back(DataType( tileAdcStatus ));
            pmt1Pedestal.push_back(DataType(0));  //The is no pedestal in DPD . This line is temporary.
          }

          if (noch2 /* || pmt2digit[cellInd].empty() */ ) { //invalid gain - channel missing
            pmt2RawAmplitude.push_back(DataType(0));
            pmt2RawTime.push_back(DataType(0));
            pmt2ADCStatus.push_back(DataType(0));
            pmt2Number.push_back(DataType(0));
            pmt2Pedestal.push_back(DataType(0));  //The is no pedestal in DPD
          } else {
            HWIdentifier hwid = cabling->s2h_adc_id(tileID->adc_id(cellid,1,gain2));

            int adc       = tileHWID->adc(hwid);
            int channel   = tileHWID->channel(hwid);
            int drawer    = tileHWID->drawer(hwid);
            int ros       = tileHWID->ros(hwid);
            int PMT = abs( cabling->channel2hole(ros,channel) );
            int drawerIdx = TileCalibUtils::getDrawerIdx(ros,drawer);
            float scale = m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, 1.0,
                                                          TileRawChannelUnit::ADCcounts, TileRawChannelUnit::MegaElectronVolts);
            float amp = theTileCell->ene2() / scale;
            float time = theTileCell->time2();

            int qbit = (theTileCell->qbit2() & TileCell::MASK_TIME);
            if ((qual2 != 0 || qbit != 0 || amp != 0.0) && (fabs(time) < maxTime && time != 0.0)) {
              time += m_tileToolTiming->getSignalPhase(drawerIdx, channel, adc);
            }

            uint32_t tileAdcStatus = m_tileBadChanTool->encodeStatus(m_tileBadChanTool->getAdcStatus(drawerIdx,channel,adc));
            if (badch2) tileAdcStatus += 10;

            pmt2RawAmplitude.push_back(DataType(amp));
            pmt2RawTime.push_back(DataType(time));
            pmt2ADCStatus.push_back(DataType( tileAdcStatus ));
            pmt2Number.push_back(DataType(PMT));
            pmt2Pedestal.push_back(DataType(0));  //The is no pedestal in DPD .
          }
        } //don't have TileRawChannel
      } //if (m_doTileCellDetails)
    } // end cell iterator

    ATH_MSG_DEBUG( " Total energy in Tile in GeV : " <<  energyAllTile );

    if ( !pmt1digit.empty() ) pmt1digit.clear();
    if ( !pmt2digit.empty() ) pmt2digit.clear();

    // write values into DataMap
    auto n =  phi.size();
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["sub"] = m_sub;
    DataMap["id"] = std::move(idVec);
    DataMap["energy"] = std::move(energyVec);


    //BadCells
    if (m_doBadTile==true) {
      DataMap["BadCell"]= std::move(BadCell);
    }

    if (m_doTileCellDetails) {
      DataMap["pmt1Energy"] = std::move(pmt1Energy);
      DataMap["pmt1Time"] = std::move(pmt1Time);
      DataMap["pmt1Chi2"] = std::move(pmt1Chi2);
      DataMap["pmt1Gain"] = std::move(pmt1Gain);

      DataMap["pmt2Energy"] = std::move(pmt2Energy);
      DataMap["pmt2Time"] = std::move(pmt2Time);
      DataMap["pmt2Chi2"] = std::move(pmt2Chi2);
      DataMap["pmt2Gain"] = std::move(pmt2Gain);

      DataMap["pmt1RawAmplitude"] = std::move(pmt1RawAmplitude);
      DataMap["pmt1RawTime"] = std::move(pmt1RawTime);
      DataMap["pmt1ADCStatus"] = std::move(pmt1ADCStatus);
      DataMap["pmt1Number"] = std::move(pmt1Number);
      DataMap["pmt1Pedestal"] = std::move(pmt1Pedestal);

      DataMap["pmt2RawAmplitude"] = std::move(pmt2RawAmplitude);
      DataMap["pmt2RawTime"] = std::move(pmt2RawTime);
      DataMap["pmt2ADCStatus"] = std::move(pmt2ADCStatus);
      DataMap["pmt2Number"] = std::move(pmt2Number);
      DataMap["pmt2Pedestal"] = std::move(pmt2Pedestal);
    }

    DataMap[adcCounts1Str] = std::move( adcCounts1Vec);
    DataMap[adcCounts2Str] = std::move(adcCounts2Vec);

    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << n << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // getTileData

  //-----------------------------------------------------------------------------------------------------

  void CaloTileRetriever::calcTILELayerSub(Identifier& cellid)
  {
    if (m_calocell_id->is_tile_barrel(cellid))
    {
      if (m_calocell_id->is_tile_negative(cellid))
        m_sub.push_back(DataType(2));
      else
        m_sub.push_back(DataType(3));
    }
    else if (m_calocell_id->is_tile_extbarrel(cellid))
    {
      if (m_calocell_id->is_tile_negative(cellid))
        m_sub.push_back(DataType(0));
      else
        m_sub.push_back(DataType(5));
    }
    //else in ITC or scint
    else
    {
      if (m_calocell_id->is_tile_negative(cellid))
        m_sub.push_back(DataType(1));
      else
        m_sub.push_back(DataType(4));
    }
  }

  //--------------------------------------------------------------------------

} // JiveXML namespace
