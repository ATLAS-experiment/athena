/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuCTPIPhase1ByteStreamAlgo.h"
#include "TrigT1Result/MuCTPI_Phase1_RDO.h"
#include "TrigT1MuctpiBits/MuCTPI_Bits.h"
#include "TrigT1MuctpiBits/HelpersPhase1.h"
#include "PathResolver/PathResolver.h"

#include <atomic>

static std::atomic<unsigned int> barrelROIFailCounter{0};

//also inspired by Rafal's word decoding code from:
//https://gitlab.cern.ch/atlas/athena/blob/release/22.0.91/Trigger/TrigT1/TrigT1ResultByteStream/src/MuonRoIByteStreamTool.cxx

/**
 * The constructor takes care of correctly constructing the base class and
 * declaring the tool's interface to the framework.
 */
MuCTPIPhase1ByteStreamAlgo::MuCTPIPhase1ByteStreamAlgo( const std::string& name, ISvcLocator* svcLoc )
    : AthReentrantAlgorithm( name, svcLoc) {}

StatusCode MuCTPIPhase1ByteStreamAlgo::initialize()
{
    ATH_MSG_DEBUG("Initialising " << name());

    ATH_CHECK( m_MuCTPI_Phase1_RDOKey.initialize(/*m_processMuctpi=*/true) );

    //needed to enable the decoding of eta and phi
	ATH_MSG_INFO("--- ENABLING THE DECODING");
	const std::string barrelFileName   = PathResolverFindCalibFile( m_barrelRoIFile );
	ATH_MSG_INFO("--- - CHECK BARREL FILE NAME" << barrelFileName);
	const std::string ecfFileName      = PathResolverFindCalibFile( m_ecfRoIFile );
	ATH_MSG_INFO("--- - CHECK ECF FILE NAME" << ecfFileName);
	const std::string side0LUTFileName = PathResolverFindCalibFile( m_side0LUTFile );
	ATH_MSG_INFO("--- - CHECK SIDE0 LUT FILE NAME" << side0LUTFileName);
	const std::string side1LUTFileName = PathResolverFindCalibFile( m_side1LUTFile );
	ATH_MSG_INFO("--- - INFO SIDE1 LUT FILE NAME" << side1LUTFileName);

	CHECK( m_l1topoLUT.initializeLUT(barrelFileName,
									 ecfFileName,
									 side0LUTFileName,
									 side1LUTFileName) );

    //return here while looking for fix for this
    return StatusCode::SUCCESS;
	
    //this didn't work yet locally (offline). Waiting for feedback and should cleanup or reinclude soon.
    //not critical; can run without it.
    /**

    //fetch nbits vector from l1menu (todo: fix)
    bool success=false;
    TrigConf::L1CTPFiles ctpfiles;
    SG::ReadHandleKey<TrigConf::L1Menu>  L1MenuKey{this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu", "L1 Menu"};

    //    ATH_CHECK(L1MenuKey.initialize());
    if(L1MenuKey.initialize())
    {
        SG::ReadHandle<TrigConf::L1Menu> l1Menu = SG::makeHandle(L1MenuKey);

        if (!l1Menu.isValid())
        {
            ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES L1 menu INvalid! Cannot read nbits for mlt word");
        }
        else
        {
            ATH_MSG_DEBUG("MUCTPI DQ DEBUG: CTPFILES L1 menu valid");
            uint smk = l1Menu->smk();
            TrigConf::TrigDBCTPFilesLoader db_loader(m_alias_db);
            //options below added according to
            //TrigConf::L1CTPFiles header file
            ATH_MSG_DEBUG("MUCTPI DQ DEBUG: CTPFILES L1 menu load files with smk="<<smk);
            try{success = db_loader.loadHardwareFiles(smk, ctpfiles,0x08);}
            catch(std::exception& e){ ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES loadHardwareFiles exception: "<<e.what()); }
            catch(...){ ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES loadHardwareFiles: Unknown exception"); }

            ATH_MSG_DEBUG("MUCTPI DQ DEBUG: CTPFILES success="<<success);
        }
        if(!ctpfiles.hasCompleteMuctpiData())
        {
            ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES Incomplete MUCTPI data from TriggerDB");
        }
        else if(success)
        {
            ATH_MSG_DEBUG("MUCTPI DQ DEBUG: CTPFILES L1 menu load files success!");
            try
            {
                m_muctpi_Nbits = (std::vector<uint32_t>) ctpfiles.muctpi_Nbits();
                if(m_muctpi_Nbits.size()!=32)
                    ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES MUCTPI data from TriggerDB - nbits wrong size: "<<std::dec << m_muctpi_Nbits.size());
                else
                    ATH_MSG_WARNING("MUCTPI DQ DEBUG: CTPFILES MUCTPI data from TriggerDB - nbits GOOD SIZE!");

                ATH_MSG_DEBUG("MUCTPI DQ DEBUG: CTPFILES L1 menu GOT muctpi nbits");

            }
            catch(std::exception& e){ ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES exception: "<<e.what()); }
            catch(...){ ATH_MSG_ERROR("MUCTPI DQ DEBUG: CTPFILES: Unknown exception"); }
        }

    }
    return StatusCode::SUCCESS;

    */
}

StatusCode MuCTPIPhase1ByteStreamAlgo::execute(const EventContext& eventContext) const {
    ATH_MSG_DEBUG("Executing " << name());

    // Retrieve the BS data for all tools in one request to parallelise DCM->ROS network requests
    IROBDataProviderSvc::VROBFRAG vrobf;
    std::vector<uint32_t> robID = { m_robId };// Source ID of MIROD
    //get rob fragment(s)
    m_robDataProviderSvc->getROBData(eventContext, robID, vrobf, name());
    //should only receive exactly 1 fragment; make sure:
    if(vrobf.size()!=1)
    {
        ATH_MSG_ERROR("Wrong number of MUCTPI fragment in event: vrobf.size()="<<vrobf.size());
        return StatusCode::FAILURE;
    }

    //make the conversion, i.e. make the RDO, record it
    SG::WriteHandle<MuCTPI_Phase1_RDO> outputHandle{m_MuCTPI_Phase1_RDOKey, eventContext};
    ATH_CHECK(convert(vrobf[0],outputHandle));
    return StatusCode::SUCCESS;
}

/**
 * Conversion from eformat::ROBFragment to RDO.
 * This is called from the MuCTPIByteStreamCnv::createObj method.
 */
StatusCode MuCTPIPhase1ByteStreamAlgo::convert( const IROBDataProviderSvc::ROBF* rob,  SG::WriteHandle<MuCTPI_Phase1_RDO>& outputHandle ) const {

  ATH_MSG_DEBUG("executing convert() from ROBFragment to RDO");
  // check ROD source ID
  const uint32_t rodId = rob->rod_source_id();
  // check BC ID
  const uint32_t bcId = rob->rod_bc_id();

  ATH_MSG_DEBUG(" expected ROD sub-detector ID: " << std::hex << m_robId << " ID found: " << std::hex << rodId << std::dec);  

  if( rodId != m_robId ) {
    ATH_MSG_ERROR("Wrong ROD ID found in the MuCTPI ROB fragment!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_VERBOSE(" ROD Header BCID " << bcId << ", dumping MuCTPI words:");

  const uint32_t* it_data;
  rob->rod_data( it_data );
  const uint32_t ndata = rob->rod_ndata();
  ATH_MSG_DEBUG("MUCTPI DQ DEBUG: number of ROB data words: " << std::dec << ndata);

  //slices
  std::vector< LVL1::MuCTPIBits::Slice > slices;
  LVL1::MuCTPIBits::Slice slice;
  bool firstSlice=true;
  std::vector<size_t> errorBits;    
  uint64_t sliceMultiplicity=0;//grouping the 3 multiplicity words, to be processed at the end of the slice

  for( uint32_t iWord = 0; iWord < ndata; ++iWord, ++it_data ) {

      //for each word, get it, find type, and add in Slice struct.
      uint32_t word =  static_cast< uint32_t >( *it_data );
      ATH_MSG_DEBUG("MUCTPI raw word " << iWord << ": 0x" << std::hex << word << std::dec);
      LVL1::MuCTPIBits::WordType wordType = LVL1::MuCTPIBits::getWordType(word);

      switch (wordType) {
      case LVL1::MuCTPIBits::WordType::Timeslice: {

          ATH_MSG_DEBUG(" MUCTPI DQ DEBUG: Timeslice found: "<< std::hex << word);

          //add previous slice if any
          if(!firstSlice)
          {
              ATH_MSG_DEBUG(" MUCTPI DQ DEBUG: new timeslice found (pushing)");
              slices.push_back(slice);
          }
          else
              firstSlice=false;

          //make new slice (to be improved, since "new" will give pointer)
          LVL1::MuCTPIBits::Slice s;
          slice =  s;

          const auto header = LVL1::MuCTPIBits::timesliceHeader(word);
          ATH_MSG_DEBUG("This is a timeslice header word with BCID=" << header.bcid
                        << ", NTOB=" << header.tobCount << ", NCAND=" << header.candCount);
          slice.bcid  = header.bcid;
          slice.nCand = header.candCount;
          slice.nTOB  = header.tobCount;
          break;
      }
      case LVL1::MuCTPIBits::WordType::Multiplicity: {
          uint32_t tmNum = LVL1::MuCTPIBits::multiplicityWordNumber(word);
          ATH_MSG_DEBUG("This is a multiplicity word #" << tmNum);

          if(m_muctpi_Nbits.size()==32)
          {
              //fill mult word into temp container until 3rd word is found
              if(tmNum==1)
                  sliceMultiplicity |= ( (uint64_t)LVL1::MuCTPIBits::maskedWord(word,LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART1_SHIFT, LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART1_MASK) ) << LVL1::MuCTPIBits::RUN3_MULTIPLICITY_ENC_PART1_SHIFT;
              else if(tmNum==2)
                  sliceMultiplicity |= ( (uint64_t)LVL1::MuCTPIBits::maskedWord(word,LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART2_SHIFT, LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART2_MASK) ) << LVL1::MuCTPIBits::RUN3_MULTIPLICITY_ENC_PART2_SHIFT;
              else if(tmNum==3)
                  sliceMultiplicity |= ( (uint64_t)LVL1::MuCTPIBits::maskedWord(word,LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART3_SHIFT, LVL1::MuCTPIBits::RUN3_MULTIPLICITY_PART3_MASK) ) << LVL1::MuCTPIBits::RUN3_MULTIPLICITY_ENC_PART3_SHIFT;

              //flags from third word
              //AND: process multiplicity for the slice!!!
              if(tmNum==3)
              {
                  slice.mlt.nswMon       = LVL1::MuCTPIBits::maskedWord(word,LVL1::MuCTPIBits::RUN3_NSW_MONITORING_TRIGGER_SHIFT, LVL1::MuCTPIBits::RUN3_NSW_MONITORING_TRIGGER_MASK);
                  slice.mlt.candOverflow = LVL1::MuCTPIBits::maskedWord(word,LVL1::MuCTPIBits::RUN3_MULTIPLICITY_OVERFLOW_SHIFT,  LVL1::MuCTPIBits::RUN3_MULTIPLICITY_OVERFLOW_MASK);
                  slice.mlt.bits         = sliceMultiplicity;

                  //process the long mult word into 32 mlt thr counters
                  for(uint iThr=0;iThr<m_muctpi_Nbits.size();iThr++)
                  {
                      uint thismask=0;
                      if(m_muctpi_Nbits[iThr]==1)
                          thismask=0x1;
                      else if(m_muctpi_Nbits[iThr]==2)
                          thismask=0x3;
                      else if(m_muctpi_Nbits[iThr]==3)
                          thismask=0x7;

                      //keep only the part of the 64bit word corresponding to the nbits value
                      slice.mlt.cnt.push_back( sliceMultiplicity & thismask);
                      //"throw away" the part of the 64bit word that we just used
                      sliceMultiplicity >>= m_muctpi_Nbits[iThr];
                  }

                  sliceMultiplicity=0;//cleaning just in case..
              }

          }
          else
          {
              //if nbits size !=32, then it's not set
              //for now, ignore, and can fill the histos with the Mult bits, as they come
              //without decoding
              //=>suppress this warning
              //ATH_MSG_WARNING("MUCTPI DQ DEBUG: skipping Mult processing, no nbits defined");

              //todo: add code + histos for Mult bits
          }

          break;
      }
      case LVL1::MuCTPIBits::WordType::Candidate: {
        ATH_MSG_DEBUG("This is a RoI candidate word");

        LVL1::MuCTPIBits::Candidate thiscand(word);

		// We calculate eta/phi coordinates for each candidate with the 
		// full resolution available.
		if(thiscand.type == LVL1::MuCTPIBits::SubsysID::Barrel)
		{
			thiscand.eta = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).eta;
			thiscand.phi = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).phi;
			//ATH_MSG_INFO("Candidate Pt " << thiscand.pt);
			thiscand.mappedPt = LVL1::MuCTPIBits::RPCtoTGC_pt_map[thiscand.pt];
			//ATH_MSG_INFO("Candidate mapped Pt " << thiscand.mappedPt);
		}
		else if(thiscand.type == LVL1::MuCTPIBits::SubsysID::Endcap)
		{
			thiscand.eta = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).eta;
			thiscand.phi = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).phi;
		}
		else if(thiscand.type == LVL1::MuCTPIBits::SubsysID::Forward)
		{
			thiscand.eta = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).eta;
			thiscand.phi = m_l1topoLUT.getCoordinates(thiscand.side, thiscand.subsystem, thiscand.num, thiscand.roi).phi;
		}
        slice.cand.push_back(thiscand);
        break;
      }
      case LVL1::MuCTPIBits::WordType::Topo: {
		ATH_MSG_DEBUG("This is a Topo TOB word "<< std::hex << word);
        LVL1::MuCTPIBits::TopoTOB thistob(word);

		if(thistob.det == 0) // BA
		{
          try
          {
            thistob.roi = m_l1topoLUT.getBarrelROI(thistob.side, thistob.sec, thistob.barrel_eta_lookup, thistob.barrel_phi_lookup);
          }
          catch (const std::out_of_range& e) // Occurs when getBarrelROI fails to find entry in the map
          { 
            ++barrelROIFailCounter;
            ATH_MSG_WARNING("TopoTOB word not found in LUT!"
                            << " (" << e.what() << " exception)"
                            << " Word info: det = " << thistob.det
                            << ", subsystem = " << thistob.subsystem
                            << ", side = " << thistob.side
                            << ", sector = " << thistob.sec
                            << ", ieta = " << thistob.barrel_eta_lookup
                            << ", iphi = " << thistob.barrel_phi_lookup);
            break;
          }
		  thistob.etaDecoded = m_l1topoLUT.getCoordinates(thistob.side, thistob.subsystem, thistob.sec, thistob.roi).eta;
		  thistob.phiDecoded = m_l1topoLUT.getCoordinates(thistob.side, thistob.subsystem, thistob.sec, thistob.roi).phi;
		}
		else // FW or EC
		{
		  // FW and EC have the ROI initialized in the constructor of the Topo word, because it is encoded in eta_raw and phi_raw 
		  thistob.etaDecoded = m_l1topoLUT.getCoordinates(thistob.side, thistob.subsystem, thistob.sec, thistob.roi).eta;
		  thistob.phiDecoded = m_l1topoLUT.getCoordinates(thistob.side, thistob.subsystem, thistob.sec, thistob.roi).phi;
		}        
		slice.tob.push_back(thistob);
        break;
      }
      case LVL1::MuCTPIBits::WordType::Status: {
        ATH_MSG_DEBUG("This is a status word"<< std::hex << word);
        errorBits = LVL1::MuCTPIBits::getDataStatusWordErrors(word);
        if (!errorBits.empty()) {
          ATH_MSG_DEBUG("MUCTPI ROD data flagged with errors. The data status word is 0x" << std::hex << word << std::dec);
          for (size_t bit : errorBits) {
            ATH_MSG_DEBUG("Error bit " << bit << ": " << LVL1::MuCTPIBits::DataStatusWordErrors.at(bit));
          }
        }
        break;
      }
      default: {
        ATH_MSG_ERROR("The MUCTPI word 0x" << std::hex << word << std::dec << " does not match any known word type");
        return StatusCode::FAILURE;
      }//default
      }//switch
  }//for each word

  //add last timeslice in vector, since there is no end-slice flag
  ATH_MSG_DEBUG(" MUCTPI DQ DEBUG: out of words (pushing last slice)");
  slices.push_back( slice );

  // Check that the number of exceptions is below a limit based on the number of slices.
  // Exceptions can occur if word is present in multiple timeslices for same event.
  // This allows for cases where we have one or two bad words per event, but not where lots of map errors arise. 
  if (barrelROIFailCounter == slices.size() + 2 )
  {
    ATH_MSG_ERROR("TopoTOB word exception count exceeded limit!");
    return StatusCode::FAILURE;
  }

  // create MuCTPI RDO
  ATH_CHECK(outputHandle.record(
      std::make_unique<MuCTPI_Phase1_RDO>(std::move(slices), std::move(errorBits))
  ));
  return StatusCode::SUCCESS;
}

StatusCode MuCTPIPhase1ByteStreamAlgo::finalize()
{
    if (barrelROIFailCounter != 0) ATH_MSG_WARNING(barrelROIFailCounter << " TopoTOB words could not retrieve RoIs in the barrel");
    return StatusCode::SUCCESS;
}
