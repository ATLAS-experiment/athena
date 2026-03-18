/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArByteStream/LArABBADecoder.h"
#include "LArByteStream/ABBAMapping.h"

static const InterfaceID IID_ILArABBADecoder("LArABBADecoder", 1, 0);


using namespace OFFLINE_FRAGMENTS_NAMESPACE;

LArABBADecoder::LArABBADecoder(const std::string& type, const std::string& name,
			       const IInterface* parent ) 
  : AthAlgTool(type,name,parent) 
{
  declareInterface< LArABBADecoder  >( this );

}

const InterfaceID& LArABBADecoder::interfaceID( )
{ return IID_ILArABBADecoder;  }



LArABBADecoder::~LArABBADecoder() {}


StatusCode LArABBADecoder::initialize() {
  return StatusCode::SUCCESS;
}

StatusCode LArABBADecoder::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode LArABBADecoder::convert(const RawEvent* re, LArDigitContainer* coll) const
{

  bool ret=false;
  // Check fragment validity:
  try {
    ret=re->check();
  } 
  catch (eformat::Issue& ex) {
    ATH_MSG_WARNING( "Exception while checking eformat fragment validity: " << ex.what() ); 
    ret=false;
  }
  if (!ret) {
    ATH_MSG_ERROR( "Got invalid RawEvent fragment" );
    return StatusCode::FAILURE;
  }
  
  //Build TOC
  std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> > robIndex;
  eformat::helper::build_toc(*re, robIndex );
  std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> >::const_iterator robIt = robIndex.find(eformat::LAR);
  if (robIt!=robIndex.end()) {
      const std::vector<const uint32_t*>& robs = robIt->second;
      for (const uint32_t* pRob :robs) {
	try {
	  ROBFragment robFrag(pRob);
	    fillCollection(&robFrag,coll);
	}	
	catch (eformat::Issue& ex) {
	  ATH_MSG_WARNING ( " exception thrown by ROBFragment, badly corrupted event. Abort decoding " );
	  coll->clear();
	  break;
	}
      }//end loop over robs
  }//end if got LAr-RODs   
  return StatusCode::SUCCESS;
}


void LArABBADecoder::fillCollection(const ROBFragment* robFrag, LArDigitContainer* coll) const
{
  const CaloGain::CaloGain dummyGain=CaloGain::LARHIGHGAIN;
  LArDigit * scDigit=0 ;

  //FIXME: Check validity
  uint32_t nData = robFrag->rod_ndata(); 
  const uint32_t* p=robFrag->rod_data();
  if (nData < 3) {
	    ATH_MSG_WARNING ( "Error reading bytestream event: "
			      << "Empty ROD block (less than 3 words) for source ID " << robFrag->rod_source_id() );
   return;
	  }
  else
  robFrag->rod_source_id();
  unsigned int n = robFrag->payload_size_word();

     std::vector<std::string> string; 
     int x=0x0;
      int nsamples = p[0];
      for(unsigned int s=1;s<n-13;s+=6) {
        std::string ss;
	for (unsigned int k=0;k<6;++k){
	x = p[s+k];
        std::ostringstream oss;
	oss << std::hex << x;
        if (oss.str().size() == 7){
	   ss.append("0");
	}
	ss += oss.str();
          }
        string.push_back(std::move(ss));
     } 



constexpr int nFibers = 20;
constexpr int nchannels = 8;
constexpr int Nabbachannels = nFibers * nchannels;

constexpr int fiberSeq[16] = {0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1};
constexpr int adcSeq[16] =   {1,0,1,0,3,2,3,2,5,4,5,4,7,6,7,6};

std::vector< std::vector< std::vector<int> > > abbasamples;
abbasamples.resize(nFibers);

std::vector<std::vector<short>> abba_channel_samples;
abba_channel_samples.resize(Nabbachannels);

for (int fib=0;fib < nFibers; ++fib){
    abbasamples[fib].resize(nchannels);
	for (int ch = 0; ch < nchannels; ++ch){
	    abbasamples[fib][ch].resize(nsamples);
	    int Nchannel = fib*8 + ch;
	    abba_channel_samples[Nchannel].resize(nsamples);    
	}
   }
	
 for (int fib=0;fib < nFibers; fib+=2){
        for (int sam=0; sam < nsamples; sam++){ 
            int zx = sam + (fib/2)*nsamples;
            const std::string &wx = string[zx];
            int k = 0;
            for (int m = 0; m < 16; ++m){ //16 size of fiberSeq array
		int fiber_number = fiberSeq[m];
		int adc_number = adcSeq[m];
                int idx = k*3;
		std::stringstream ss;
		char mysample[3];
		for (int j = idx; j < idx+3; ++j){
		    int i = j%3;
                    mysample[i] = wx[j];
 		}
		ss << mysample;
		short mysample_int;
		ss >> std::hex >> mysample_int;
                int fb = fiber_number+fib;
                int ch = adc_number;
		int abbachannel = fb*8+ch;
		k++;
     	        abbasamples[fb][ch][sam] = mysample_int;
		abba_channel_samples[abbachannel][sam] = mysample_int;
            }
        }
    }

std::map<int, HWIdentifier> abba_mapping;


int iphi = 0;
switch (robFrag->rod_source_id())
{
case 0x410511:
  iphi=19;
  break;
////modified for data taken between march 31 and apr 13
case 0x410512:
  iphi=18;
  break;
case 0x410521:
  iphi=20;
  break;
case 0x410522:
  iphi=21;
  break;

default:
msg(MSG::ERROR) << "ROD_source_id not found" << endmsg;
}


ABBAMapping::fill(&abba_mapping, iphi);


for (std::size_t index_abba_channel_samples = 0; index_abba_channel_samples < abba_channel_samples.size(); ++index_abba_channel_samples) {
  auto abba_mapping_search = abba_mapping.find(index_abba_channel_samples);
  if (abba_mapping_search == abba_mapping.end()) {
    msg(MSG::WARNING) << "No mapping for index_abba_channel_samples: "<<std::dec<<index_abba_channel_samples << endmsg;
    continue;
  }
    
    HWIdentifier channelID = (*abba_mapping_search).second;
    std::vector<short> adcValues = abba_channel_samples[index_abba_channel_samples];
    scDigit=new LArDigit(channelID,dummyGain,std::move(adcValues));

    coll->push_back(scDigit);
  }

abbasamples.clear();

  return;


}
