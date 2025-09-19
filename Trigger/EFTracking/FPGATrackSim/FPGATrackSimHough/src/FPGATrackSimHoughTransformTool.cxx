// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimHoughTransformTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date October 31st, 2020
 * @brief See header file.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHoughTransformTool.h"

#include <sstream>
#include <cmath>
#include <algorithm>

static inline int quant(double min, double max, unsigned nSteps, double val);
static inline double unquant(double min, double max, unsigned nSteps, int step);
template <typename T>
static inline std::string to_string(const std::vector<T> &v);


StatusCode FPGATrackSimHoughTransformTool::initialize()
{

  // Move temp variables over from properties to struct
  m_parMin.phi = m_tempMin_phi;
  m_parMin.qOverPt = m_tempMin_qOverPt;
  m_parMin.d0 = m_tempMin_d0;
  m_parMax.phi = m_tempMax_phi;
  m_parMax.qOverPt = m_tempMax_qOverPt;
  m_parMax.d0 = m_tempMax_d0;


  // Debug
  ATH_MSG_INFO("Image size: " << m_imageSize_x << " x " << m_imageSize_y);
  ATH_MSG_INFO("Convolution size: " << m_convSize_x << " x " << m_convSize_y);
  ATH_MSG_INFO("Convolution: " << to_string(const_cast<std::vector<int>&>(m_conv.value())));
  ATH_MSG_INFO("Hit Extend: " << to_string(const_cast<std::vector<unsigned>&>(m_hitExtend_x.value())));

  // Retrieve info
  if (m_idealGeoRoads || m_useSectors) ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();

  // Error checking
  // TODO check bounds are set correctly
  bool ok = false;
  if (!m_imageSize_x || !m_imageSize_y)
    ATH_MSG_FATAL("initialize() Image size must be greater than 0");
  else if (m_conv.size() != m_convSize_x * m_convSize_y)
    ATH_MSG_FATAL("initialize() Convolution sizes don't match");
  else if (!m_conv.empty() && (m_convSize_x % 2 == 0 || m_convSize_y % 2 == 0))
    ATH_MSG_FATAL("initialize() Convolution sizes must be odd");
  else if (m_hitExtend_x.size() % m_nLayers)
    ATH_MSG_FATAL("initialize() Hit extentsion list must have size % nLayers");
  else if (!m_combineLayers.empty() && m_combineLayers.size() != m_nLayers)
    ATH_MSG_FATAL("initialize() Combine layers list must have size = nLayers");
  else if (m_threshold.size() % 2 != 1)
    ATH_MSG_FATAL("initialize() Threshold size must be odd");
  else if (!m_binScale.empty() && m_binScale.size() != m_nLayers)
    ATH_MSG_FATAL("initialize() Bin scale list must have size = nLayers");
  else if (std::any_of(m_binScale.begin(), m_binScale.end(), [&](unsigned i){ return m_imageSize_y % i != 0; }))
    ATH_MSG_FATAL("initialize() The imagesize is not divisible by scale");
  else
    ok = true;
  if (!ok) return StatusCode::FAILURE;

  // Warnings / corrections
  if (m_localMaxWindowSize && !m_traceHits)
    {
      ATH_MSG_WARNING("initialize() localMaxWindowSize requires tracing hits, turning on automatically");
      m_traceHits = true;
    }
  if (m_idealGeoRoads)
    {
      if (m_useSectors)
        {
	  ATH_MSG_WARNING("initialize() idealGeoRoads conflicts with useSectors, switching off FPGATrackSim sector matching");
	  m_useSectors = false;
        }
      if (!m_traceHits)
        {
	  ATH_MSG_WARNING("initialize() idealGeoRoads requires tracing hits, turning on automatically");
	  m_traceHits = true;
        }
    }
  if (m_binScale.empty()) m_binScale.value().resize(m_nLayers, 1);
		 
  // Fill convenience variables
  m_step_x = (m_parMax[m_par_x] - m_parMin[m_par_x]) / m_imageSize_x;
  m_step_y = (m_parMax[m_par_y] - m_parMin[m_par_y]) / m_imageSize_y;
  for (unsigned i = 0; i <= m_imageSize_x; i++)
    m_bins_x.push_back(unquant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, i));
  for (unsigned i = 0; i <= m_imageSize_y; i++)
    m_bins_y.push_back(unquant(m_parMin[m_par_y], m_parMax[m_par_y], m_imageSize_y, i));

  // Initialize combine layers
  if (!m_combineLayers.empty())
    {
      m_nCombineLayers = *std::max_element(m_combineLayers.begin(), m_combineLayers.end()) + 1;
      m_combineLayer2D.resize(m_nCombineLayers);
      for (unsigned i = 0; i < m_combineLayers.size(); i++)
	      m_combineLayer2D[m_combineLayers[i]].push_back(i);
    }
  else
    {
      m_nCombineLayers = m_nLayers;
      for (unsigned i = 0; i < m_nLayers; i++)
	      m_combineLayer2D.push_back({ i });
    }

  if (m_houghType == "LowResource"){
    makeLUT(m_LUT, m_h_rfix, Form("%d-%d", m_imageSize_y.value(), m_imageSize_x.value()));
  } else if (m_houghType == "Flexible") {
    double phi0_max_bit_th = (m_parMax.phi * m_phi_coord_max * m_bitwise_phi0_conv) / (m_phi_range);
    double qApt_max_bit_th = (m_parMax.qOverPt * fpgatracksim::A * m_phi_coord_max * m_r_max_mm * m_bitwise_qApt_conv) / (m_r_max * m_phi_range);

    m_one_r_const_twoexp = std::pow(2, static_cast<int>(2 * std::log2(m_r_max + 1)));
    m_phi0_min_bit = (m_parMin.phi * m_phi_coord_max * m_bitwise_phi0_conv) / (m_phi_range);
    m_qApt_min_bit = (m_parMin.qOverPt * fpgatracksim::A * m_phi_coord_max * m_r_max_mm * m_bitwise_qApt_conv) / (m_r_max * m_phi_range);
    m_DBinQApt_bit_int = (qApt_max_bit_th - m_qApt_min_bit) / m_imageSize_y;
    m_DBinPhi0_bit_int = (phi0_max_bit_th - m_phi0_min_bit) / m_imageSize_x;
    double phi0_max_bit = m_phi0_min_bit + (m_imageSize_x * m_DBinPhi0_bit_int);
    double qApt_max_bit = m_qApt_min_bit + (m_imageSize_y * m_DBinQApt_bit_int);
    m_qAptBins_bit = (qApt_max_bit - m_qApt_min_bit) / m_DBinQApt_bit_int;
    m_phi0Bins_bit = (phi0_max_bit - m_phi0_min_bit) / m_DBinPhi0_bit_int;
    m_phi0_bins_first_sector = m_phi0Bins_bit / (m_phi0_sectors * m_pipes_phi0);
    m_qApt_bins_first_sector = m_qAptBins_bit / (m_pipes_qApt * m_qApt_sectors);    
    m_tot_bitwise_conv = m_bitwise_phi0_conv * m_bitwise_qApt_conv;
    m_one_r_const_twoexp_post_conv = m_bitwise_phi0_conv * m_one_r_const_twoexp;
    m_qApt_min_post_conv = (m_bitwise_phi0_conv * m_qApt_min_bit) / (m_bitwise_phi0_conv * m_DBinQApt_bit_int);

    double DBinQApt_clean = ((qApt_max_bit_th - m_qApt_min_bit) / m_imageSize_y) / m_bitwise_qApt_conv;
    double DBinPhi0_clean = ((phi0_max_bit_th - m_phi0_min_bit) / m_imageSize_x) / m_bitwise_phi0_conv;
    m_HT_sel = static_cast<int>(DBinPhi0_clean / DBinQApt_clean);
  }
  return StatusCode::SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimHoughTransformTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) 
{
  roads.clear();
  m_roads.clear();

  m_image = createImage(hits);

  //objects for road merge, all roads go through them, except m_traceHits is disabled
  std::vector<std::pair<unsigned, unsigned>> roadListXY;
  std::unordered_set<std::shared_ptr<const FPGATrackSimHit>> merged_image;
  ///

  if (!m_conv.empty()) m_image = convolute(m_image);

  for (unsigned y = 0; y < m_imageSize_y; y++){
    for (unsigned x = 0; x < m_imageSize_x; x++){
      if (passThreshold(m_image, x, y)){
        if (m_traceHits){
	  //first road is pushed to the roadList
	  roadListXY.push_back({x, y});
	}else{
	  addRoad(hits, x, y);
        }
      }
    }
  }

  for (const auto & roadXY : roadListXY){
    ATH_MSG_DEBUG("roadList x : " << roadXY.first << ", y : " << roadXY.second);
  }

  if(!m_traceHits){
    if(m_roadMerge)
      ATH_MSG_DEBUG("Trace hits is disabled. Road merge doesn't work with this state at the moment.");
  }

  if(!m_roadMerge){//in case road marge is disabled, add road as usual
    for (const auto & roadXY : roadListXY){
      unsigned x = roadXY.first;
      unsigned y = roadXY.second;
      addRoad(m_image(y, x).second, x, y);
    }
  }else{
    if(!roadListXY.empty()){
      std::vector<std::pair<unsigned, unsigned>> mergedRoadListXY;//merged road liste
      size_t roadCounter = 0; // the number of roads after road merge
      /////
      //Road marge works as follows
      //1. Take a road from the input list, add it to the merged road list, and remove it from the input list
      //2. If any road in the input list is neighbouring to the road in the merged road list, then add it to the merged road list and remove it from the input list
      //3. Once it reaches the end of the input list, select the second road in the merged road list, and repeat the check with the input list
      //4. Until it reaches the end of the merged road list, repeat 2.-3.
      //5. Once it reaches the end of the merged road list, all the hits associated with the road in the merged road list are re-associated with the first road in the merged road list
      //6. As a representative, only the first road is added as a road
      //7. Select the next road in the input list, then repeat 2.-6.
      //8. Repeat until all the roda in the input list are considered
      /////
      while(!roadListXY.empty()){
	      mergedRoadListXY.push_back(roadListXY[0]);
	      roadListXY.erase(roadListXY.begin());
        for(size_t i = 0; i < mergedRoadListXY.size(); i++){
          for(int j = 0; j < std::ssize(roadListXY); j++){
	          if(std::abs(static_cast<int>(roadListXY[j].first) - static_cast<int>(mergedRoadListXY[i].first)) < 2 && 
		         std::abs(static_cast<int>(roadListXY[j].second) - static_cast<int>(mergedRoadListXY[i].second)) < 2){
	            mergedRoadListXY.push_back(roadListXY[j]);
	            roadListXY.erase(roadListXY.begin() + j);
	            //note: on first iteration, j=0 so j-- will produce negative number
              j--;
            }
          }
        }
        if (!mergedRoadListXY.empty()){
          ATH_MSG_DEBUG( mergedRoadListXY.size() -1 <<" roads are merged to road(" << mergedRoadListXY[0].first << "," << mergedRoadListXY[0].second << ")");
        }
	for (const auto & roadXY : mergedRoadListXY){
	  unsigned x = roadXY.first;
	  unsigned y = roadXY.second;
          merged_image.insert(m_image(y, x).second.begin(), m_image(y, x).second.end());
	}
        addRoad(merged_image, mergedRoadListXY[0].first, mergedRoadListXY[0].second);
        mergedRoadListXY.clear();
        merged_image.clear();
        roadCounter++;
      }
      ATH_MSG_DEBUG("There is/are " << roadCounter << " roads after road merge");
    }
  }

  roads.reserve(m_roads.size());
  for (FPGATrackSimRoad & r : m_roads) roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
    
  return StatusCode::SUCCESS;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::createLayerImage(std::vector<unsigned> const & layers, const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned const scale) const
{
  Image image(m_imageSize_y, m_imageSize_x);

  for (const auto& hit : hits) {
    if (std::find(layers.begin(), layers.end(), hit->getLayer()) == layers.end()) continue;

    if (m_houghType == "LowResource"){
      //FIXME at the moment, only the barrel is considered. And as the r is mostly the same for two strip layers, those two layers share the LUTs of acceptable input phi values. Here, LUT_layer is used to switch the list of LUTs 
      int LUT_layer = -1;
      switch (hit->getLayer()){
        case 0 :
          LUT_layer = 0;
          break;
        case 1 :
        case 2 :
          LUT_layer = 1;
          break;
        case 3 :
        case 4 :
          LUT_layer = 2;
          break;
        case 5 :
        case 6 :
          LUT_layer = 3;
          break;
        case 7 :
        case 8 :
          LUT_layer = 4;
          break;
        default :
          ATH_MSG_FATAL("Debug: something wrong! layer: " << hit->getLayer());
          break;
      }
      if (LUT_layer <0){
        ATH_MSG_ERROR("FPGATrackSimHoughTransformTool: array index is negative");
        return image;
      }
      int phi_L_bin = m_h_rfix.at(LUT_layer)->FindBin(hit->getGPhi());
      int MSB = (phi_L_bin >> (m_bitlength -6));
      if(MSB >= 64) MSB = 63;//for barrel, MSB should be up to 63, but this line force it to not crash
      for(const auto& LUT_i: m_LUT.at(LUT_layer).at(MSB)){//check LUTs only correspoinding MSB
	if(LUT_i.input_begin <= phi_L_bin && phi_L_bin <= LUT_i.input_end){//If in the range, then fire it
	  for(const auto& pos: LUT_i.output){
	    image(pos.y -1, pos.x -1).first++;//NOTE : pos is 1 start
	    if (m_traceHits) image(pos.y -1, pos.x -1).second.insert(hit);
	  }
	}
      }
    } else if (m_houghType == "Flexible") {
	double r = hit->getR();
	int r_bit = static_cast<int>(r * m_r_max / m_r_max_mm);	
	int input_bins_vector_size = 0;
	
	//Bologna Flexible HT uses qA/Pt=.... or Phi0=.... depending from the axis with more bins (qA/Pt axis #bin > Phi0 then Phi0=..... and viceversa)
	if (r_bit > m_HT_sel) { //formula for qA/Pt HT
      input_bins_vector_size = m_imageSize_x;
  } else { //formula for Phi0 HT
      input_bins_vector_size = m_imageSize_y;
  }
        
  std::vector<std::vector<int>> hitLineQAPtPhi0(input_bins_vector_size);
  hitLineQAPtPhi0 = lineGenLay(hit);
  
	for (const std::vector<int>& it_base : hitLineQAPtPhi0) {
	    if (it_base[0] != -1 && it_base[1] != -1) {   
		      image(it_base[0] , it_base[1]).first++;
		      if (m_traceHits) {
		          image(it_base[0] , it_base[1]).second.insert(hit);
		      }
	    }
	}
    }else{
      // This scans over y (pT) because that is more efficient in memory, in C.
      // Unknown if firmware will want to scan over x instead.
      unsigned new_size_y  = m_imageSize_y / scale;
      for (unsigned y_ = 0; y_ < new_size_y; y_++) {
	unsigned y_bin_min = scale * y_;
	unsigned y_bin_max = scale * (y_ + 1);

	// Find the min/max x bins
	auto xBins = yToXBins(y_bin_min, y_bin_max, hit);

	// Update the image
	for (unsigned y = y_bin_min; y < y_bin_max; y++){
	  for (unsigned x = xBins.first; x < xBins.second; x++) {
	    image(y, x).first++;
	    if (m_traceHits) image(y, x).second.insert(hit);
	  }
	}
      }
    }
  }

  return image;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::createImage(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits) const
{
  Image image(m_imageSize_y, m_imageSize_x);

  for (unsigned i = 0; i < m_nCombineLayers; i++)
    {
      Image layerImage = createLayerImage(m_combineLayer2D[i], hits, m_binScale[i]);
      for (unsigned x = 0; x < m_imageSize_x; ++x)
	for (unsigned y = 0; y < m_imageSize_y; ++y)
	  if (layerImage(y, x).first > 0)
	    {
	      image(y, x).first++;
	      image(y, x).second.insert(layerImage(y, x).second.begin(), layerImage(y, x).second.end());
	    }
    }
  return image;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::convolute(Image const & image) const
{
  Image out(m_imageSize_y, m_imageSize_x);

  for (unsigned y0 = 0; y0 < m_imageSize_y; y0++)     // Loop over out
    for (unsigned x0 = 0; x0 < m_imageSize_x; x0++) 
      for (unsigned r = 0; r < m_convSize_y; r++)     // Loop over conv
      	for (unsigned c = 0; c < m_convSize_x; c++) {
	        int y = -static_cast<int>(m_convSize_y) / 2 + r + y0; // Indices of input
	        int x = -static_cast<int>(m_convSize_x) / 2 + c + x0; //
	  
      	  if (y >= 0 && y < static_cast<int>(m_imageSize_y) && x >= 0 && x < static_cast<int>(m_imageSize_x)) {
	          int val = m_conv[r * m_convSize_x + c] * image(y, x).first;
	          if (val > 0) {
	            out(y0, x0).first += val;
	            out(y0, x0).second.insert(image(y, x).second.begin(), image(y, x).second.end());
	          }
	        }
	      }  
  return out;
}

bool FPGATrackSimHoughTransformTool::passThreshold(Image const & image, unsigned x, unsigned y) const
{
  // Pass window threshold
  unsigned width = m_threshold.size() / 2;
  if (x < width || (image.size(1) - x) < width) return false;
  for (unsigned i = 0; i < m_threshold.size(); i++) {
    if (image(y, x - width + i).first < m_threshold[i]) return false;
  }
  
  // Pass local-maximum check
  if (m_localMaxWindowSize) {
    for (int j = -m_localMaxWindowSize; j <= m_localMaxWindowSize; j++) {
      for (int i = -m_localMaxWindowSize; i <= m_localMaxWindowSize; i++) {
      	if (i == 0 && j == 0) continue;
	      if (y + j < image.size(0) && x + i < image.size(1)) {
	        if (image(y+j, x+i).first > image(y, x).first) return false;
	        if (image(y+j, x+i).first == image(y, x).first) {
	          if (image(y+j, x+i).second.size() > image(y, x).second.size()) return false;
	          if (image(y+j, x+i).second.size() == image(y, x).second.size() && j <= 0 && i <= 0) return false; // favor bottom-left (low phi, low neg q/pt)
	        }
	      }
      }
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Helpers


// Quantizes val, given a range [min, max) split into nSteps. Returns the bin below.
static inline int quant(double min, double max, unsigned nSteps, double val)
{
  return static_cast<int>((val - min) / (max - min) * nSteps);
}

// Returns the lower bound of the bin specified by step
static inline double unquant(double min, double max, unsigned nSteps, int step)
{
  return min + (max - min) * step / nSteps;
}

template <typename T>
static inline std::string to_string(const std::vector<T> &v)
{
  std::ostringstream oss;
  oss << "[";
  if (!v.empty())
    {
      std::copy(v.begin(), v.end()-1, std::ostream_iterator<T>(oss, ", "));
      oss << v.back();
    }
  oss << "]";
  return oss.str();
}

double FPGATrackSimHoughTransformTool::yToX(double y, const std::shared_ptr<const FPGATrackSimHit> &hit) const
{
  double x = 0;

  if (m_par_x == FPGATrackSimTrackPars::IPHI && m_par_y == FPGATrackSimTrackPars::IHIP)
    {
      double r = hit->getR(); // mm
      double phi_hit = hit->getGPhi(); // radians
      double d0 = std::isnan(m_parMin.d0) ? 0 : m_parMin.d0; // mm, assume min = max
      x = asin(r * fpgatracksim::A * y - d0 / r) + phi_hit;

      if (m_fieldCorrection) x += fieldCorrection(m_EvtSel->getRegionID(), y, r);
    }
  else
    {
      ATH_MSG_ERROR("yToX() not defined for the current m_par selection");
    }

  return x;
}



std::vector<std::vector<int>> FPGATrackSimHoughTransformTool::lineGenLay(const std::shared_ptr<const FPGATrackSimHit> &hit) const
{
    /*
    Chosen strategy: 
    The Bologna Flexible HT firmware does mathematical operations to implement the HT and utilizes only integer with variable bits range at the necessary minimum to do so.
    In the firmware some variables and flags are used to select 1)the scale factors the lead the float values to integer values and 2)the bits range of the internal variables.
    Because to lead the firmware to behave exactly as a "regular" software would have required to many work and resources on the FPGA, it has been chosen to:
        - make the firmware behave as a HT and adapt the software to the firmware to fully reproduce firmware performance;
        - change the HT software to properly behave as the firmware in some operations (as the fact that the bits range is not infinite and so on);
    Strategies to fill the accumulator:
        - 1 input bin to 1 output bin;
        - Assuming [Nx, Ny] binning accumulator is used, here the accumulator is separated into M "pipes," which each of them are[Nx/M, Ny] or [Nx, Ny/M]accumulator. In each pipe the line is drawn as described below:
        - only part of the "line" coming from a hit is drawn using the HT formula, the rest is done "shifting and replacing" the drawn line to the rest of the pipe
            - the lines drawn in the pipes start each one from a different point, following the monotonouse behavior of the line;
            - this reduce the amount of multiplication in the firmware but is not "exactly" as it would be in software, that is why it ahs been decided to keep it;
            - the accumulator in separated in "sectors" (not overlapped) alongside 1 axis. The first sector is filled with the line drawn with the HT formula, the others with the "shift and replace";
    */

    std::vector<int64_t> bins_x_new;
    std::vector<int64_t> bins_y_new;
    
    //Input values of the bins for the HT formula. The value is the center of the bin (in dev code the latter can be changed)  
    int64_t x_offset = m_phi0_min_bit + m_DBinPhi0_bit_int / 2;
    int64_t y_offset = m_qApt_min_bit + m_DBinQApt_bit_int / 2;
    for (unsigned i = 0; i < m_imageSize_x; i++) {
      bins_x_new.push_back(x_offset + m_DBinPhi0_bit_int * i);
    }
    for (unsigned i = 0; i < m_imageSize_y; i++) {
      bins_y_new.push_back(y_offset + m_DBinQApt_bit_int * i);
    }

      
    std::vector<int64_t> qApt_ht_array;
    std::vector<int64_t> phi0_ht_array;
    std::vector<int> zeros = {-1, -1};
       
    std::vector<std::vector<int>> hitLineQAPtPhi0;
    
    const unsigned bins_along_phi0_sector = m_phi0Bins_bit / m_phi0_sectors;
    const unsigned bins_along_qApt_sector = m_qAptBins_bit / m_qApt_sectors;
    
    for (unsigned i = 0; i < bins_along_phi0_sector; i++) {
        qApt_ht_array.push_back(0); 
    }
    for (unsigned i = 0; i < bins_along_qApt_sector; i++) {
        phi0_ht_array.push_back(0); 
    }

    int64_t phi_conv_offseted = 0;
    int64_t Delta_qApt_after_first_sector = 0;
    int64_t qApt_ht = -1;
    int64_t phi0_ht = -1;
    int64_t qApt_ht_pre = -1;
    int64_t phi0_ht_pre = -1;
    int64_t qApt_ht_sector = 0;
    int64_t phi0_ht_sector = 0;
    int64_t Delta_phi0_after_first_sector = 0;

    std::vector<double> d_qApt_ht_array;
    std::vector<double> d_phi0_ht_array;

    for (unsigned i = 0; i < bins_along_phi0_sector; i++) {
        d_qApt_ht_array.push_back(0.0); 
    }
    for (unsigned i = 0; i < bins_along_qApt_sector; i++) {
        d_phi0_ht_array.push_back(0.0); 
    }

   double d_phi_conv_offseted = 0.0;
   double d_qApt_ht_sector = 0.0;
   double d_phi0_ht_sector = 0.0;
   double d_one_over_r_by_const = 0.0;
   double d_phi_conv_over_DBinQApt = 0.0;
   double d_Delta_phi0_after_first_sector = 0.0;

    if (m_par_x == FPGATrackSimTrackPars::IPHI && m_par_y == FPGATrackSimTrackPars::IHIP) {
	double r = hit->getR();
        double phi_hit = hit->getGPhi(); // radians
        int r_bit = static_cast<int>(r * m_r_max / m_r_max_mm);
        int64_t phi_bit = phi_hit * m_phi_coord_max / m_phi_range;
        if (r_bit > m_HT_sel) { //Case for qA/Pt HT formula
            for (unsigned i = 0; i < m_imageSize_x; i++) {
                hitLineQAPtPhi0.push_back(zeros); 
            } 
            //formula first sector
                
            d_one_over_r_by_const = m_one_r_const_twoexp / r_bit;
            d_phi_conv_over_DBinQApt = phi_bit * m_tot_bitwise_conv / m_DBinQApt_bit_int;

	    for (int pp = 0; pp < m_pipes_phi0; pp++) {//loop over pipes pipes alongside phi0 axis
                unsigned start_phi0_bins_first_sector = static_cast <unsigned>(pp * m_phi0Bins_bit / m_pipes_phi0);
                unsigned end_phi0_bins_first_sector = m_phi0_bins_first_sector + static_cast <unsigned>(pp * m_phi0Bins_bit / m_pipes_phi0);

                for (unsigned j = start_phi0_bins_first_sector; j < end_phi0_bins_first_sector; j++) {
                    double d_c1 = m_bitwise_qApt_conv * bins_x_new[j] / m_DBinQApt_bit_int;
                    int64_t c2 = (static_cast<int64_t>(d_c1) - static_cast<int64_t>(d_phi_conv_over_DBinQApt)) * static_cast<int64_t>(d_one_over_r_by_const) / m_one_r_const_twoexp_post_conv;
                    int64_t c3 = (static_cast<int64_t>(d_c1) - static_cast<int64_t>(d_phi_conv_over_DBinQApt)) * static_cast<int64_t>(d_one_over_r_by_const);
                    
                    double d_c2 = (d_c1 - d_phi_conv_over_DBinQApt) * d_one_over_r_by_const / m_one_r_const_twoexp_post_conv;
                    double d_c3 = (d_c1 - d_phi_conv_over_DBinQApt) * d_one_over_r_by_const;
                    double d_c4 = m_bitwise_phi0_conv * m_qApt_min_bit / (m_bitwise_phi0_conv * m_DBinQApt_bit_int);

                    if (static_cast<int64_t>(d_c1) <= static_cast<int64_t>(d_phi_conv_over_DBinQApt)) {
                      qApt_ht_pre = c2 - m_qApt_min_post_conv;
                    }else{
                      qApt_ht_pre = 1 + c2 - m_qApt_min_post_conv;
                    }
                    qApt_ht_sector = c3;
                    d_qApt_ht_sector = d_c3;
                    qApt_ht_array[j - start_phi0_bins_first_sector] = qApt_ht_sector;
                    d_qApt_ht_array[j - start_phi0_bins_first_sector] = d_qApt_ht_sector;
                    
                    if (static_cast<int64_t>(d_c1) <= static_cast<int64_t>(d_phi_conv_over_DBinQApt) && (d_c2 - m_qApt_min_post_conv - d_c4) >= 0) {
                        qApt_ht = qApt_ht_pre;
                    }
                    if (static_cast<int64_t>(d_c1) > static_cast<int64_t>(d_phi_conv_over_DBinQApt) && 1 + (d_c2 - m_qApt_min_post_conv - d_c4) >= 0) {
                        qApt_ht = qApt_ht_pre;
                    }
                    
                    if (qApt_ht >= 0 && qApt_ht <= m_imageSize_y -1) {
                        hitLineQAPtPhi0[j] = {static_cast<int>(qApt_ht), static_cast<int>(j)};
                    }
                    qApt_ht = -1;
                }
                if (m_phi0_sectors > 1) { //formula after first sector
                    for (int ps = 0; ps < m_phi0_sectors -1; ps++) {
                        //note: this is integer division, e.g. 3/2 = 1
                        int64_t d_c6 = (m_DBinPhi0_bit_int * ((ps + 1) * m_phi0_bins_first_sector) * m_bitwise_qApt_conv) / m_DBinQApt_bit_int;

                        Delta_phi0_after_first_sector = d_c6 * static_cast<int64_t>(d_one_over_r_by_const);
                        //want a double here
                        d_Delta_phi0_after_first_sector = static_cast<double>(d_c6) * d_one_over_r_by_const;
                        for (unsigned psi = start_phi0_bins_first_sector; psi < end_phi0_bins_first_sector; psi++) {
                            int64_t c7 = (qApt_ht_array[psi - start_phi0_bins_first_sector] + Delta_phi0_after_first_sector) / m_one_r_const_twoexp_post_conv;
                            
                            double d_c9 = m_bitwise_phi0_conv * m_qApt_min_bit / (m_bitwise_phi0_conv * m_DBinQApt_bit_int);

                            if (-qApt_ht_array[psi - start_phi0_bins_first_sector] < Delta_phi0_after_first_sector) {
				qApt_ht = 1 + c7 - m_qApt_min_post_conv;
                            }else{
				qApt_ht = 0 + c7 - m_qApt_min_post_conv;
                            }
                            
                            double d_c7 = (d_qApt_ht_array[psi - start_phi0_bins_first_sector] + d_Delta_phi0_after_first_sector) / m_one_r_const_twoexp_post_conv;
                            
                            if (qApt_ht >= 0 && qApt_ht <= m_imageSize_y -1 && (1 + d_c7 - d_c9) >= 0 && (0 + d_c7 - d_c9) >= 0 ) {

				hitLineQAPtPhi0[((ps + 1) * m_phi0_bins_first_sector) + psi] = {static_cast<int>(qApt_ht), static_cast<int>(((ps + 1) * m_phi0_bins_first_sector) + psi)};
                            }
			    qApt_ht = -1;
                        }
                    }
                }
            }
        } else { //Case for Phi0 HT formula
            for (unsigned i = 0; i < m_imageSize_y; i++) {
                hitLineQAPtPhi0.push_back(zeros);
            }
            phi_conv_offseted = (m_bitwise_phi0_conv * phi_bit - static_cast<int64_t>(m_phi0_min_bit)) * m_bitwise_qApt_conv / m_DBinPhi0_bit_int;
            d_phi_conv_offseted = (m_bitwise_phi0_conv * phi_bit - m_phi0_min_bit) * m_bitwise_qApt_conv / m_DBinPhi0_bit_int;

            for (int pq = 0; pq < m_pipes_qApt; pq++) { //loop over pipes alongside qA/Pt axis 

                unsigned start_qApt_bins_first_sector = static_cast <unsigned>(pq * m_qAptBins_bit / m_pipes_qApt);
                unsigned end_qApt_bins_first_sector = m_qApt_bins_first_sector + static_cast <unsigned>(pq * m_qAptBins_bit / m_pipes_qApt);           
              
                for (unsigned n = start_qApt_bins_first_sector; n < end_qApt_bins_first_sector; n++) {
                  //formula for first sector
                  //note: this is integer division e.g. 3/2 = 1
                  int64_t d2 =m_bitwise_phi0_conv * bins_y_new[n] / m_DBinPhi0_bit_int;

                  phi0_ht_sector = phi_conv_offseted + r_bit * d2; 
                  phi0_ht_array[n - start_qApt_bins_first_sector] = phi0_ht_sector;
                  //want a double result here
                  d_phi0_ht_sector = d_phi_conv_offseted + r_bit * static_cast<double>(d2);
                  d_phi0_ht_array[n - start_qApt_bins_first_sector] = d_phi0_ht_sector;

                  if (phi0_ht_sector >= 0) {
                    phi0_ht_pre = phi0_ht_sector / m_bitwise_qApt_conv;
                  } else {
                    phi0_ht_pre = -1 + phi0_ht_sector / m_bitwise_qApt_conv;
                  }
                  
                  if (phi0_ht_sector >= 0 && phi0_ht_pre >= 0 && (phi0_ht_sector >= m_bitwise_qApt_conv - m_imageSize_x)) {
                      phi0_ht = phi0_ht_pre;
                  }
                  /** This condition is never true
                  if (phi0_ht_sector < 0 && phi0_ht_pre >= 0 && (-1 + phi0_ht_sector >= m_bitwise_qApt_conv - m_imageSize_x)) {
                      phi0_ht = phi0_ht_pre;
                  }
                  **/
                  if (phi0_ht <= m_imageSize_x - 1 && phi0_ht >= 0){
                      hitLineQAPtPhi0[n] = {static_cast<int>(n), static_cast<int>(phi0_ht)};
                  }
                  phi0_ht = -1;
                }
                if (m_qApt_sectors > 1) { //formula after first sector
                    for (int ts = 0; ts < m_qApt_sectors - 1; ts++) {   
                        int64_t d3 = m_DBinQApt_bit_int * (ts + 1) * m_qApt_bins_first_sector * m_bitwise_phi0_conv / m_DBinPhi0_bit_int;
                        Delta_qApt_after_first_sector = r_bit * d3;
                        for (unsigned tsi =  start_qApt_bins_first_sector; tsi < end_qApt_bins_first_sector; tsi++) {   
                            phi0_ht = (phi0_ht_array[tsi - start_qApt_bins_first_sector] + Delta_qApt_after_first_sector) / m_bitwise_qApt_conv;
                            double d_phi0_ht = (d_phi0_ht_array[tsi - start_qApt_bins_first_sector] + Delta_qApt_after_first_sector) / m_bitwise_qApt_conv;

                            if (phi0_ht <= m_imageSize_x - 1 && phi0_ht >= 0 && d_phi0_ht >= 0 && phi0_ht >= m_bitwise_qApt_conv - m_imageSize_x) {
                                hitLineQAPtPhi0[((ts + 1) * m_qApt_bins_first_sector) + tsi] = {static_cast<int>((ts + 1) * m_qApt_bins_first_sector + tsi), static_cast<int>(phi0_ht)};  
                            }
			    phi0_ht = -1;
                        }
                    }
                }
            } 
        }
    } else {
        ATH_MSG_ERROR("lineGenLay() not defined for the current m_par selection");
    }

    //return { HitLine, BinInPos };
    return  hitLineQAPtPhi0;
}

// Find the min/max x bins of the hit's line, in each y bin. Max is exclusive.
// Note this assumes yToX is monotonic. Returns {0, 0} if hit lies out of bounds.
std::pair<unsigned, unsigned> FPGATrackSimHoughTransformTool::yToXBins(size_t yBin_min, size_t yBin_max, const std::shared_ptr<const FPGATrackSimHit> & hit) const
{
  // Get float values
  double x_min = yToX(m_bins_y[yBin_min], hit);
  double x_max = yToX(m_bins_y[yBin_max], hit);
  if (x_min > x_max) std::swap(x_min, x_max);
  if (x_max < m_parMin[m_par_x] || x_min > m_parMax[m_par_x])
    return { 0, 0 }; // out of bounds

  // Get bins
  int x_bin_min = quant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, x_min);
  int x_bin_max = quant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, x_max) + 1; // exclusive

  // Extend bins
  unsigned extend = getExtension(yBin_min, hit->getLayer());
  x_bin_min -= extend;
  x_bin_max += extend;

  // Clamp bins
  if (x_bin_min < 0) x_bin_min = 0;
  if (x_bin_max > static_cast<int>(m_imageSize_x)) x_bin_max = m_imageSize_x;

  return { x_bin_min, x_bin_max };
}

// We allow variable extension based on the size of m_hitExtend_x. See comments below.
unsigned FPGATrackSimHoughTransformTool::getExtension(unsigned y, unsigned layer) const
{
  if (m_hitExtend_x.size() == m_nLayers) return m_hitExtend_x[layer];
  if (m_hitExtend_x.size() == m_nLayers * 2)
    {
      // different extension for low pt vs high pt, split in half but irrespective of sign
      // first nLayers entries of m_hitExtend_x is for low pt half, rest are for high pt half
      if (y < m_imageSize_y / 4 || y > 3 * m_imageSize_y / 4) return m_hitExtend_x[layer];
      return m_hitExtend_x[m_nLayers + layer];
    }
  return 0;
}

// Creates a road from hits that pass through the given bin (x, y), and pushes it onto m_roads
void FPGATrackSimHoughTransformTool::addRoad(const std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> & hits, layer_bitmask_t hitLayers, unsigned x, unsigned y)
{
  m_roads.emplace_back();
  FPGATrackSimRoad & r = m_roads.back();

  r.setRoadID(m_roads.size() - 1);
  r.setPID(y * m_imageSize_y + x);
  r.setHits( std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>(hits)); //copy hits

  // We use the y coordinate in matchIdealGeoSectors
  // and so it needs to be available before setting the sector.

  r.setSubRegion(m_subRegion);
  r.setX(m_bins_x[x] + m_step_x/2);
  r.setY(m_bins_y[y] + m_step_y/2);
  r.setXBin(x);
  r.setYBin(y);
  r.setHitLayers(hitLayers);
  r.setSubRegion(m_subRegion);
}


// Creates a road from hits that pass through the given bin (x, y), and pushes it onto m_roads
void FPGATrackSimHoughTransformTool::addRoad(const std::unordered_set<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned x, unsigned y)
{
  layer_bitmask_t hitLayers = 0;
  for (auto const & hit : hits)
    hitLayers |= 1 << hit->getLayer();

  auto sorted_hits = ::sortByLayer(hits);
  sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short

  addRoad(sorted_hits, hitLayers, x, y);
}

// Use this version of addRoad when hit tracing is turned off
void FPGATrackSimHoughTransformTool::addRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned x, unsigned y)
{
  // Get the road hits
  std::vector<std::shared_ptr<const FPGATrackSimHit>> road_hits;
  layer_bitmask_t hitLayers = 0;
  for (const auto & hit : hits)
    {
      // Find the min/max y bins (after scaling)
      unsigned int y_bin_min = (y / m_binScale[hit->getLayer()]) * m_binScale[hit->getLayer()];
      unsigned int y_bin_max = y_bin_min + m_binScale[hit->getLayer()];

      // Find the min/max x bins
      auto xBins = yToXBins(y_bin_min, y_bin_max, hit);
      if (x >= xBins.first && x < xBins.second)
        {
	  road_hits.push_back(hit);
	  hitLayers |= 1 << hit->getLayer();
        }
    }

  auto sorted_hits = ::sortByLayer(road_hits);
  sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short

  addRoad(sorted_hits, hitLayers, x, y);
}

// A pre-calculated list of LUTs is installed. Each LUT represents the range of input phi value to fire one bin (x,y) on a given layer l. 
void FPGATrackSimHoughTransformTool::makeLUT(std::vector<std::vector<std::vector<LUT>>> &v_LUT, std::vector<TH1D*> &v_h, const std::string& tag){
  const std::string filepath = m_requirements.value() + "requirement-" + tag + ".root";
  TFile* fin = TFile::Open(filepath.c_str());
  ATH_MSG_INFO("open: " << tag);
  m_bitlength = 14;//FIXME length needed to represent input phi. Should be define from requirement file
  TTree* requirement = fin->Get<TTree>("requirement");
  v_LUT.resize(5);
  for(int i=0; i<5; i++){//FIXME as this target only for barrel so far, hardcoded
    v_LUT.at(i).resize(64);
    TH1D* h = (TH1D*)fin->Get(Form("in%d",i));//This is 1D hist to convert float phi to bitwise, depending on layer
    v_h.push_back(h);
  }
  int nrequirement = requirement->GetEntries();
  int in_min;
  int in_max;
  int xi;
  int yi;
  int ri;
  requirement->SetBranchAddress("input_begin", &in_min);
  requirement->SetBranchAddress("input_end", &in_max);
  requirement->SetBranchAddress("output_x", &xi);
  requirement->SetBranchAddress("output_y", &yi);
  requirement->SetBranchAddress("output_l", &ri);
  for(int i=0; i<nrequirement; i++){
    requirement->GetEntry(i);
    //process for min range
    int MSB = (in_min >> (m_bitlength -6));
    bool toBeFilled = true;
    if(!v_LUT.at(ri).at(MSB).empty()){
      for(auto& LUT_i : v_LUT.at(ri).at(MSB)){// For the sake of process speed and reproducibility of FPGA performance, LUTs are pushed to different vectors depending on their input MSB value.
        if(LUT_i.input_begin == in_min && LUT_i.input_end == in_max){
          LUT_i.output.push_back({xi, yi, ri});
          toBeFilled = false;
          break;
        }
      }
    }
    if(toBeFilled){
      LUT LUT_i;
      LUT_i.input_begin = in_min;
      LUT_i.input_end = in_max;
      LUT_i.layer = ri;
      LUT_i.output.push_back({xi, yi, ri});
      v_LUT.at(ri).at(MSB).push_back(std::move(LUT_i));
    }
    //process for max range
    int MSB2 = (in_max >> (m_bitlength -6));
    if(MSB != MSB2){
      MSB = MSB2;
      toBeFilled = true;
      if(!v_LUT.at(ri).at(MSB).empty()){
        for(auto& LUT_i : v_LUT.at(ri).at(MSB)){
          if(LUT_i.input_begin == in_min && LUT_i.input_end == in_max){
            LUT_i.output.push_back({xi, yi, ri});
            toBeFilled = false;
            break;
          }
        }
      }
      if(toBeFilled){
        LUT LUT_i;
        LUT_i.input_begin = in_min;
        LUT_i.input_end = in_max;
        LUT_i.layer = ri;
        LUT_i.output.push_back({xi, yi, ri});
        v_LUT.at(ri).at(MSB).push_back(LUT_i);
      }
    }
  }
  int LUTsize = v_LUT.size();
  for(int layer = 0; layer < LUTsize; layer++){
    for(int msb = 0; msb < 64; msb++){
      ATH_MSG_DEBUG("LUT size L(" << layer << ") msb(" << msb << "): " << v_LUT.at(layer).at(msb).size());
    }
  }
}
