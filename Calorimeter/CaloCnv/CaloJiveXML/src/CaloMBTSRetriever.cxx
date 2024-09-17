/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloMBTSRetriever.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "EventContainers/SelectAllObject.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "TileEvent/TileCell.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBID.h"
#include "TileConditions/TileInfo.h"
#include "TileConditions/TileCablingService.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  CaloMBTSRetriever::CaloMBTSRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent),
    m_tileTBID(nullptr)
  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("MBTSThreshold", m_mbtsThreshold = 0.05);
    declareProperty("RetrieveMBTS" , m_mbts = true);
    declareProperty("DoMBTSDigits",  m_mbtsdigit=false);
    declareProperty("DoMBTSCellDetails",  m_mbtsCellDetails = false);

    // TileDigitsContainer names: {"TileDigitsCnt","TileDigitsFlt"};
    declareProperty("TileDigitsContainer" ,m_sgKeyTileDigits = "",
        "Input collection to retrieve Tile digits, used when doTileDigit is True");

    // TileRawChannelContainer names: {"TileRawChannelOpt2","TileRawChannelOpt","TileRawChannelFixed",
    //                                 "TileRawChannelFitCool","TileRawChannelFit",
    //                                 "TileRawChannelCnt","TileRawChannelFlt"};
    declareProperty("TileRawChannelContainer" ,m_sgKeyTileRawChannel = "",
        "Input collection to retrieve Tile raw channels, used when DoMBTSCellDetails is True.");
  }

  /**
   * Initialise the ToolSvc
   */

  StatusCode CaloMBTSRetriever::initialize() {

    ATH_MSG_DEBUG( "Initialising Tool" );

    ATH_CHECK(m_sgKeyMBTS.initialize());

    //=== get TileCondToolTiming
    ATH_CHECK( m_tileToolTiming.retrieve() );

    //=== get TileCondToolEmscale
    ATH_CHECK( m_tileToolEmscale.retrieve() );

    ATH_CHECK( m_sgKeyTileDigits.initialize(m_mbtsdigit) );

    ATH_CHECK( m_sgKeyTileRawChannel.initialize(m_mbtsCellDetails) );

    return StatusCode::SUCCESS;
  }

  /**
   * MBTS data retrieval from chosen collection
   */
  StatusCode CaloMBTSRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    ATH_MSG_DEBUG( "in retrieve()"  );

    SG::ReadHandle<TileCellContainer> cellContainerMBTS(m_sgKeyMBTS);
    if (!cellContainerMBTS.isValid()){
	    ATH_MSG_WARNING( "Could not retrieve MBTS Cells "  );
    }
    else{
      if (m_mbts) {
        DataMap data = getMBTSData(&(*cellContainerMBTS));
        ATH_CHECK( FormatTool->AddToEvent(dataTypeName(), m_sgKeyMBTS.key(), &data) );
        ATH_MSG_DEBUG( "MBTS retrieved"  );
      }
    }

    //MBTS cells retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve MBTS cell location and details
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  const DataMap CaloMBTSRetriever::getMBTSData(const TileCellContainer* tileMBTSCellContainer) {

    ATH_MSG_DEBUG( "getMBTSData()"  );

    DataMap DataMap;

    DataVect energy; energy.reserve(tileMBTSCellContainer->size());
    DataVect label; label.reserve(tileMBTSCellContainer->size());
    DataVect phi; phi.reserve(tileMBTSCellContainer->size());
    DataVect eta; eta.reserve(tileMBTSCellContainer->size());
    DataVect sampling; sampling.reserve(tileMBTSCellContainer->size());
    DataVect timeVec; timeVec.reserve(tileMBTSCellContainer->size());
    DataVect quality; quality.reserve(tileMBTSCellContainer->size());
    DataVect type; type.reserve(tileMBTSCellContainer->size());
    DataVect channel; channel.reserve(tileMBTSCellContainer->size());
    DataVect module; module.reserve(tileMBTSCellContainer->size());
    DataVect cellPedestal; cellPedestal.reserve(tileMBTSCellContainer->size());
    DataVect cellRawAmplitude; cellRawAmplitude.reserve(tileMBTSCellContainer->size());
    DataVect cellRawTime; cellRawTime.reserve(tileMBTSCellContainer->size());
    DataVect adcCounts; adcCounts.reserve(tileMBTSCellContainer->size() * 10);

    std::string adcCountsStr="adcCounts multiple=\"0\"";
    const TileHWID* tileHWID = nullptr;
    const TileInfo* tileInfo = nullptr;
    const TileCablingService* cabling=nullptr;
    TileRawChannelUnit::UNIT RChUnit = TileRawChannelUnit::ADCcounts;  //!< Unit for TileRawChannels (ADC, pCb, etc.)
    cabling = TileCablingService::getInstance();
    bool offlineRch = false;

    if ( detStore()->retrieve(m_tileTBID).isFailure() ) {
      ATH_MSG_ERROR( "in getMBTSData(), Could not retrieve m_tileTBID"  );
    }

    if ( detStore()->retrieve(tileHWID).isFailure() ) {
      ATH_MSG_ERROR( "in getMBTSData(), Could not retrieve TileHWID"  );
    }

    if ( detStore()->retrieve(tileInfo, "TileInfo").isFailure() ) {
      ATH_MSG_ERROR( "in getMBTSData(), Could not retrieve TileInfo" );
    }

    SG::ReadHandle<TileRawChannelContainer> RawChannelCnt;
    if (m_mbtsCellDetails) {
      RawChannelCnt = SG::makeHandle(m_sgKeyTileRawChannel);
      if (!RawChannelCnt.isValid()){
          ATH_MSG_WARNING( "Could not retrieve TileRawChannel "  );
      }
      else{
          RChUnit = RawChannelCnt->get_unit();
          offlineRch = (RChUnit<TileRawChannelUnit::OnlineADCcounts &&
                        RawChannelCnt->get_type() != TileFragHash::OptFilterDsp);
      }
    }

    SG::ReadHandle<TileDigitsContainer> tileDigits;
    if (m_mbtsdigit) {
      tileDigits = SG::makeHandle(m_sgKeyTileDigits);
      if (!tileDigits.isValid()){
         ATH_MSG_WARNING( "Could not retrieve TileDigits "  );
      }
    }

    // from: TileCalorimeter/TileRec/src/TileCellToNtuple.cxx

    std::string MBTS_ID;
    int nchan =0;
    int nTileSamples=0;
    const int max_chan=5216;
    double energyMeV = 0.;
    double phiMBTS = 0.;
    double amplitude = 0.;
    unsigned long int cellid;
    std::map<unsigned long int,double> theMbtspedestal;
    std::map<unsigned long int,double> theMbtsrawamp;
    std::map<unsigned long int,double> theMbtsrawtime;
    std::map<unsigned long int,std::vector<float> > theMbtsdigit;
    std::string myCellRawTimeStr = "0.";

    //Loop over TileRawChannel to get Pedestal and raw amplitude and time

    if (m_mbtsCellDetails && RawChannelCnt.isValid()) {
      if (offlineRch) {

        for (const auto rawChannel : *RawChannelCnt) {
          for (const auto cell : *rawChannel) {
            Identifier pmt_id = cell->pmt_ID();
            if (!m_tileTBID->is_tiletb(pmt_id)) continue;
            Identifier id = cell->cell_ID();
            cellid = id.get_identifier32().get_compact();
            HWIdentifier hwid=cell->adc_HWID();
            int adc       = tileHWID->adc(hwid);
            int channel   = tileHWID->channel(hwid);
            int drawer    = tileHWID->drawer(hwid);
            int ros       = tileHWID->ros(hwid);
            int drawerIdx = TileCalibUtils::getDrawerIdx(ros,drawer);

            amplitude = cell->amplitude();
            //Change amplitude units to ADC counts
            if (TileRawChannelUnit::ADCcounts < RChUnit && RChUnit < TileRawChannelUnit::OnlineADCcounts) {
              amplitude /= m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, 1.0, TileRawChannelUnit::ADCcounts, RChUnit);
            } else if (RChUnit > TileRawChannelUnit::OnlineADCcounts) {
              amplitude = m_tileToolEmscale->undoOnlCalib(drawerIdx, channel, adc, amplitude, RChUnit);
            }
            theMbtspedestal.insert(std::make_pair( cellid, cell->pedestal() ) );
            theMbtsrawamp.insert(std::make_pair( cellid, amplitude ));
            theMbtsrawtime.insert(std::make_pair( cellid, cell->time(cell->uncorrTime()) ));
            break;

          }
        }
      }
    }

    //Loop over TileDigits to retrieve MBTS digits

    if (m_mbtsdigit && tileDigits.isValid()) {

      //----- get tile digits--------------------------

      // tile digits loop
      for (const auto digitChannel : *tileDigits) {

        for (const auto cell : *digitChannel) {

          Identifier pmt_id = cell->pmt_ID();
          if (!m_tileTBID->is_tiletb(pmt_id)) continue;

          Identifier id = cell->cell_ID();
          cellid = id.get_identifier32().get_compact();

          nTileSamples = cell->NtimeSamples();
          std::vector<float> tileSamples = cell->samples();
          theMbtsdigit.insert(std::make_pair( cellid, tileSamples));
          break;

        }
      }//for TileDigitContainer loop
    } //if (m_mbtsdigit)

    //Loop Over TileCellContainer to retrieve MBTSCell information

    for (const auto cell : *tileMBTSCellContainer) {

      int qual = cell->quality();
      if (cell->badcell()) qual = -qual;

      timeVec.push_back(DataType( cell->time() ));
      quality.push_back(DataType( qual ));

      if (  cell->energy() >= m_mbtsThreshold ) {
        energyMeV = cell->energy(); // roughly correct: energy unit is pC, which is 95% MeV
      }else{
        energyMeV = 0.;
      }
      energy.push_back(DataType( energyMeV )); // write in MeV

      Identifier id=cell->ID();

      //TileCell/type is  "side"  +/- 1
      //TileCell/module is "phi"  0-7
      //TileCell/channel is "eta"  0-1   zero is closer to beam pipe

      type.push_back(DataType( m_tileTBID->type(id) ));
      channel.push_back(DataType( m_tileTBID->channel(id) ));
      module.push_back(DataType( m_tileTBID->module(id) ));

      MBTS_ID = "type_" + DataType( m_tileTBID->type(id) ).toString() + "_ch_" +
          DataType( m_tileTBID->channel(id) ).toString() + "_mod_" +
          DataType( m_tileTBID->module(id) ).toString();

      label.push_back(DataType( MBTS_ID ));

      eta.push_back(DataType( 5.0*m_tileTBID->type(id) ));

      phiMBTS = (M_PI/8)+(2*M_PI/8)*int(m_tileTBID->module(id));
      phi.push_back(DataType( phiMBTS ));
      sampling.push_back(DataType( m_tileTBID->channel(id) ));

      if (m_mbtsCellDetails && RawChannelCnt.isValid()) {

        cellPedestal.push_back(DataType( theMbtspedestal[id.get_identifier32().get_compact()] ));
        cellRawAmplitude.push_back(DataType( theMbtsrawamp[id.get_identifier32().get_compact()] ));
        myCellRawTimeStr =  DataType(theMbtsrawtime[id.get_identifier32().get_compact()]).toString();

        if ( myCellRawTimeStr.find('n') == 1 )  myCellRawTimeStr="0.";
        cellRawTime.push_back( myCellRawTimeStr );

          // this can rarely be '-nan', but checking this each time may make code slow ?
      }
      else { // don't have TileRawChannel container (for DPF input)

        float maxTime = (tileInfo->NdigitSamples()/2) * 25;
        int gain = cell->gain();

        if (gain<0 || gain>1) { //invalid gain - channel missing
          cellRawAmplitude.push_back(DataType(0));
          cellRawTime.push_back(DataType(0));
          cellPedestal.push_back(DataType(0));  //There is no pedestal in DPD .
        }
        else {

          HWIdentifier hwid = tileHWID->adc_id(cabling->s2h_channel_id(id),gain);
          int adc       = tileHWID->adc(hwid);
          int channel   = tileHWID->channel(hwid);
          int drawer    = tileHWID->drawer(hwid);
          int ros       = tileHWID->ros(hwid);
          int drawerIdx = TileCalibUtils::getDrawerIdx(ros,drawer);
          float scale = m_tileToolEmscale->channelCalib(drawerIdx, channel, adc, 1.0,
                                                        TileRawChannelUnit::ADCcounts, TileRawChannelUnit::MegaElectronVolts);
          float amp;

          if (  cell->energy() >= m_mbtsThreshold ) amp = cell->energy()/scale;
          else amp = 0.0;
          float time = cell->time();

          if ((qual != 0 || amp != 0.0) && (fabs(time) < maxTime && time != 0.0)) {
            time += m_tileToolTiming->getSignalPhase(drawerIdx, channel, adc);
          }

          cellRawAmplitude.push_back(DataType(amp));
          cellRawTime.push_back(DataType(time));
          cellPedestal.push_back(DataType(0));  //There is no pedestal in DPD . This line is temporary.
        }
      }


      if (m_mbtsdigit && tileDigits.isValid()) {

        if ( !theMbtsdigit[id.get_identifier32().get_compact()].empty() ) {
          for (int i=0; i<nTileSamples; i++) {
            adcCountsStr="adcCounts multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts.push_back(DataType( int(theMbtsdigit[id.get_identifier32().get_compact()][i]) ));
          }
        }
        else {
          for (int i=0; i<nTileSamples; i++) {
            adcCountsStr="adcCounts multiple=\""+DataType(nTileSamples).toString()+"\"";
            adcCounts.push_back(DataType(0));
          }
        }
      }


      ATH_MSG_DEBUG( "MBTS no: " << nchan << ", type_chan_mod: " << MBTS_ID
                     << ", energy MeV pC: " << energyMeV  );

      nchan++;

      if (nchan >= max_chan) break;

    }//TileCell Loop

    if (!theMbtspedestal.empty()) theMbtspedestal.clear();
    if (!theMbtsrawamp.empty() )  theMbtsrawamp.clear();
    if (!theMbtsrawtime.empty())  theMbtsrawtime.clear();

    // write values into DataMap
    const auto nentries = phi.size();
    DataMap["energy"] = std::move(energy);
    DataMap["label"] = std::move(label);
    DataMap["phi"] = std::move(phi);
    DataMap["eta"] = std::move(eta);
    DataMap["sampling"] = std::move(sampling);
    DataMap["time"] = std::move(timeVec);
    DataMap["quality"] = std::move(quality);
    DataMap["type"] = std::move(type);
    DataMap["channel"] = std::move(channel);
    DataMap["module"] = std::move(module);
    DataMap["cellPedestal"] = std::move(cellPedestal);
    DataMap["cellRawAmplitude"] = std::move(cellRawAmplitude);
    DataMap["cellRawTime"] = std::move(cellRawTime);
    DataMap[adcCountsStr] = std::move(adcCounts);

    //Be verbose
    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << nentries << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // getMBTSData

  //--------------------------------------------------------------------------

} // JiveXML namespace
