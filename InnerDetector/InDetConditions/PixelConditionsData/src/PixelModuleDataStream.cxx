/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelModuleDataStream.h"
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelConditionsDataStringUtils.h"
#include "PathResolver/PathResolver.h"
#include <vector>
#include <type_traits>
#include <sstream>

using namespace PixelConditionsData;

namespace{
  template <typename T>
  struct is_container : std::false_type { };
  
  template <typename T> 
  struct is_container<std::vector<T> > : std::true_type { };
  
  template <class T>
  std::ostream &
  operator << (std::ostream & out, const std::vector<T> & v){
    const std::string delim = is_container<T>()?"":" ";
    const std::string LF = is_container<T>()?"":"\n";
    for (size_t i{};i!=v.size();++i){
      out<<v[i]<<delim;
    }
    out<<LF;
    return out;
  }
  
  
  std::istream &
  operator >> (std::istream & in, std::vector<std::vector<float>> & v){
    std::istream::sentry s(in);
    if (s){
      v.clear();
      constexpr size_t bufferSize=500;
      char buffer[bufferSize];
      in.getline(buffer, bufferSize-1, '\n');
      if (buffer[0] =='#') in.getline(buffer, bufferSize);
      std::istringstream is(buffer);
      //assumes (for now) only three entries...which is currently the case
      v.emplace_back(std::istream_iterator<float>(is),std::istream_iterator<float>());
      in.getline(buffer, bufferSize-1, '\n');
      std::istringstream is2(buffer);
      v.emplace_back(std::istream_iterator<float>(is2),std::istream_iterator<float>());
      in.getline(buffer, bufferSize-1, '\n');
      std::istringstream is3(buffer);
      v.emplace_back(std::istream_iterator<float>(is3),std::istream_iterator<float>());
    }
    return in;
  }
  template <typename T>
  std::istream &
  operator >> (std::istream & in, std::vector<T> & v){
    std::istream::sentry s(in);
    if (s){
      v.clear();
      constexpr size_t bufferSize=500;
      char buffer[bufferSize]={};
      in.getline(buffer, bufferSize-1,'\n');
      if (buffer[0] =='#') in.getline(buffer, bufferSize-1,'\n');
      std::string s(buffer);
      std::istringstream iss(s);
      v.assign(std::istream_iterator<T>(iss), std::istream_iterator<T>());
      if (in.fail()){
        throw std::runtime_error("error on streaming input to PixelModuleData");
      }
    }
    return in;
  }
}

//declared as a friend in the PixelModuleData class
std::ostream & 
operator << (std::ostream &out, const PixelModuleData &c){
  const std::string LF{"\n"};
  //NB: in the following, vector variables are not followed by a linefeed;
  // it's already in the vector stream insertion operator in anonymous namespace (above)
  out<<"#defaultBarrelAnalogThreshold"<<LF;
  out<<c.m_defaultBarrelAnalogThreshold;
  out<<"#defaultEndcapAnalogThreshold"<<LF;
  out<<c.m_defaultEndcapAnalogThreshold;
  out<<"#defaultDBMAnalogThreshold"<<LF;
  out<<c.m_defaultDBMAnalogThreshold;
  //
  out<<"#defaultBarrelAnalogThresholdSigma"<<LF;
  out<<c.m_defaultBarrelAnalogThresholdSigma;
  out<<"#defaultEndcapAnalogThresholdSigma"<<LF;
  out<<c.m_defaultEndcapAnalogThresholdSigma;
  out<<"#defaultDBMAnalogThresholdSigma"<<LF;
  out<<c.m_defaultDBMAnalogThresholdSigma;
  //
  out<<"#defaultBarrelAnalogThresholdNoise"<<LF;
  out<<c.m_defaultBarrelAnalogThresholdNoise;
  out<<"#defaultEndcapAnalogThresholdNoise"<<LF;
  out<<c.m_defaultEndcapAnalogThresholdNoise;
  out<<"#defaultDBMAnalogThresholdNoise"<<LF;
  out<<c.m_defaultDBMAnalogThresholdNoise;
  //
  out<<"#defaultBarrelInTimeThreshold"<<LF;
  out<<c.m_defaultBarrelInTimeThreshold;
  out<<"#defaultEndcapInTimeThreshold"<<LF;
  out<<c.m_defaultEndcapInTimeThreshold;
  out<<"#defaultDBMInTimeThreshold"<<LF;
  out<<c.m_defaultDBMInTimeThreshold;
  //
  out<<"#barrelToTThreshold"<<LF;
  out<<c.m_barrelToTThreshold;
  out<<"#endcapToTThreshold"<<LF;
  out<<c.m_endcapToTThreshold;
  out<<"#DBMToTThreshold"<<LF;
  out<<c.m_DBMToTThreshold;
  //
  out<<"#barrelCrossTalk"<<LF;
  out<<c.m_barrelCrossTalk; 
  out<<"#endcapCrossTalk"<<LF;
  out<<c.m_endcapCrossTalk; 
  out<<"#DBMCrossTalk"<<LF;
  out<<c.m_DBMCrossTalk;
  //
  out<<"#barrelThermalNoise"<<LF;
  out<<c.m_barrelThermalNoise;
  out<<"#endcapThermalNoise"<<LF;
  out<<c.m_endcapThermalNoise;
  out<<"#DBMThermalNoise"<<LF;
  out<<c.m_DBMThermalNoise;
  //
  out<<"#barrelNoiseOccupancy"<<LF;
  out<<c.m_barrelNoiseOccupancy;
  out<<"#endcapNoiseOccupancy"<<LF;
  out<<c.m_endcapNoiseOccupancy;
  out<<"#DBMNoiseOccupancy"<<LF;
  out<<c.m_DBMNoiseOccupancy;
  //
  out<<"#barrelDisableProbability"<<LF;
  out<<c.m_barrelDisableProbability;
  out<<"#endcapDisableProbability"<<LF;
  out<<c.m_endcapDisableProbability;
  out<<"#DBMDisableProbability"<<LF;
  out<<c.m_DBMDisableProbability;
  //
  out<<"#barrelNoiseShape"<<LF;
  out<<c.m_barrelNoiseShape;
  out<<"#endcapNoiseShape"<<LF;
  out<<c.m_endcapNoiseShape;
  out<<"#DBMNoiseShape"<<LF;
  out<<c.m_DBMNoiseShape;
  //
  out<<"#FEI3BarrelLatency"<<LF;
  out<<c.m_FEI3BarrelLatency;
  out<<"#FEI3EndcapLatency"<<LF;
  out<<c.m_FEI3EndcapLatency;
  //
  out<<"#FEI3BarrelTimingSimTune"<<LF;
  out<<c.m_FEI3BarrelTimingSimTune;
  out<<"#FEI3EndcapTimingSimTune"<<LF;
  out<<c.m_FEI3EndcapTimingSimTune;

  //
  out<<"#paramA"<<LF;
  out<<c.m_paramA<<LF;
  out<<"#paramE"<<LF;
  out<<c.m_paramE<<LF;
  out<<"#paramC"<<LF;
  out<<c.m_paramC<<LF;
  out<<"#doLinearExtrapolation"<<LF;
  out<<c.m_doLinearExtrapolation<<LF;
  //
  out<<"#biasVoltage"<<LF;
  out<<c.m_biasVoltage<<LF;
  //
  out<<"#fluenceLayer"<<LF;
  out<<c.m_fluenceLayer;
  out<<"#radSimFluenceMapList"<<LF;
  out<<c.m_radSimFluenceMapList;
  //
  out<<"#fluenceLayer3D"<<LF;
  out<<c.m_fluenceLayer3D;
  out<<"#radSimFluenceMapList3D"<<LF;
  out<<c.m_radSimFluenceMapList3D;
  //
  out<<"#cablingMapToFile"<<LF;
  out<<c.m_cablingMapToFile<<LF;
  out<<"#cablingMapFileName"<<LF;
  out<<c.m_cablingMapFileName;
  
  return out;
}

//declared as a friend in the PixelModuleData class (dangerously skirting the interface)
std::istream & 
operator >> (std::istream &in, PixelModuleData &c){
  [[maybe_unused]] std::vector<float> ignoreFloatVec;
  [[maybe_unused]] std::vector<double> ignoreDoubleVec;
  [[maybe_unused]] std::vector<double> ignoreIntVec;
  [[maybe_unused]] std::vector<bool> ignoreBoolVec;
  [[maybe_unused]] float ignoreFloat{};
  [[maybe_unused]] bool ignoreBool{};
  [[maybe_unused]] double ignoreDouble{};
  [[maybe_unused]] int ignoreInt{};
  [[maybe_unused]] std::string ignoreString{};
  std::istream::sentry s(in);
  if (s){
    //this is rather unforgiving, and should only be used with the format given by the ostream
    //insertion operator
    std::string label;
    in.ignore(100, '\n');
    in>>ignoreDouble;
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    //
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    in.ignore(100,'\n');
    in>>ignoreDoubleVec;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThresholdSigma;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThresholdSigma;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThresholdSigma;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelAnalogThresholdNoise;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapAnalogThresholdNoise;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMAnalogThresholdNoise;
    //
    in.ignore(100,'\n');
    in>>c.m_defaultBarrelInTimeThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultEndcapInTimeThreshold;
    in.ignore(100,'\n');
    in>>c.m_defaultDBMInTimeThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelToTThreshold;
    in.ignore(100,'\n');
    in>>c.m_endcapToTThreshold;
    in.ignore(100,'\n');
    in>>c.m_DBMToTThreshold;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelCrossTalk; 
    in.ignore(100,'\n');
    in>>c.m_endcapCrossTalk; 
    in.ignore(100,'\n');
    in>>c.m_DBMCrossTalk;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelThermalNoise;
    in.ignore(100,'\n');
    in>>c.m_endcapThermalNoise;
    in.ignore(100,'\n');
    in>>c.m_DBMThermalNoise;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelNoiseOccupancy;
    in.ignore(100,'\n');
    in>>c.m_endcapNoiseOccupancy;
    in.ignore(100,'\n');
    in>>c.m_DBMNoiseOccupancy;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelDisableProbability;
    in.ignore(100,'\n');
    in>>c.m_endcapDisableProbability;
    in.ignore(100,'\n');
    in>>c.m_DBMDisableProbability;
    //
    in.ignore(100,'\n');
    in>>c.m_barrelNoiseShape;
    in.ignore(100,'\n');
    in>>c.m_endcapNoiseShape;
    in.ignore(100,'\n');
    in>>c.m_DBMNoiseShape;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelLatency;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapLatency;
    //
    in.ignore(100,'\n');
    in>>ignoreBoolVec;
    in.ignore(100,'\n');
    in>>ignoreBoolVec;
    //
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    //
    in.ignore(100,'\n');
    in>>c.m_FEI3BarrelTimingSimTune;
    in.ignore(100,'\n');
    in>>c.m_FEI3EndcapTimingSimTune;
    //
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    in.ignore(100,'\n');
    in>>ignoreIntVec;
    //something magic about having to stream to a string at this point, 
    //instead of using 'ignore'
    in>>label;
    in>>ignoreFloat;
    in>>label;
    in>>ignoreBool;
   
    in>>label;
    in>>ignoreDoubleVec;
    //
    in>>label;
    in>>c.m_paramA;
    in>>label;
    in>>c.m_paramE;
    in>>label;
    in>>c.m_paramC;
    in>>label;
    in>>c.m_doLinearExtrapolation;
    //
    in>>label;
   
    in>>ignoreDoubleVec;
    in>>label;
    in>>ignoreDoubleVec;
    //
    in>>label;
    in>>c.m_biasVoltage;
    in>>label;
    in>>ignoreFloat;
    //
    in>>label;
    in>>ignoreFloatVec;
    in>>label;
    in>>ignoreFloatVec;

    in>>label;
    in>>ignoreFloatVec;
    //
    in>>label;
    in>>c.m_fluenceLayer;
    in>>label;
    in>>c.m_radSimFluenceMapList;
    //
    in>>label;
    in>>c.m_fluenceLayer3D;
    in>>label;
    in>>c.m_radSimFluenceMapList3D;
    //
    in>>label;
    in>>c.m_cablingMapToFile;
    in>>label;
    in>>c.m_cablingMapFileName;
    //
    in>>label;
    in>>ignoreInt;
    in>>label;
    in>>ignoreInt;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreDouble;
    in>>label;
    in>>ignoreBool;
    in>>label;
    in>>ignoreString;
  }
  return in;
}

std::istream & operator >>(SoshiFormat & f, PixelModuleData & md){
  std::istream *i = f.m_is;
  std::string sline;
  std::vector<std::string> lBuffer;
  std::string multiline = "";
  std::vector<std::string> mapsPath_list;
  std::vector<std::string> mapsPath_list3D;
  while (getline(*i,sline)) {
    if (!sline.empty()) {
      if (sline.find("//")==std::string::npos) {
        if (sline.find("{{")!=std::string::npos && sline.find("}")!=std::string::npos) {
          multiline = sline;
        }
        else if (sline.find("{")!=std::string::npos && sline.find("}}")!=std::string::npos) {
          multiline += sline;
          lBuffer.push_back(multiline);
        }
        else if (sline.find("{")!=std::string::npos && sline.find("}")!=std::string::npos) {
          multiline += sline;
          lBuffer.push_back(sline);
        }
        else if (sline.find("{")!=std::string::npos) {
          multiline = sline;
        }
        else if (sline.find("}")!=std::string::npos) {
          multiline += sline;
          lBuffer.push_back(multiline);
        }
        else {
          multiline += sline;
        }
      }
    }
  }

  md.setBarrelToTThreshold(getParameter<int>("BarrelToTThreshold", lBuffer));
  md.setFEI3BarrelLatency(getParameter<int>("FEI3BarrelLatency", lBuffer));
  std::vector<bool> ignoreBoolVec;
  ignoreBoolVec = getParameter<bool>("FEI3BarrelHitDuplication", lBuffer);
  auto ignoreIntVec = getParameter<int>("FEI3BarrelSmallHitToT", lBuffer);
  md.setFEI3BarrelTimingSimTune(getParameter<int>("FEI3BarrelTimingSimTune", lBuffer));
  md.setBarrelCrossTalk(getParameter<double>("BarrelCrossTalk", lBuffer));
  md.setBarrelNoiseOccupancy(getParameter<double>("BarrelNoiseOccupancy", lBuffer));
 
  md.setBarrelDisableProbability(getParameter<double>("BarrelDisableProbability", lBuffer));
  std::vector<double> ignoreDoubleVec{};
  ignoreDoubleVec = getParameter<double>("BarrelLorentzAngleCorr", lBuffer);
  
  md.setEndcapToTThreshold(getParameter<int>("EndcapToTThreshold", lBuffer));
  md.setFEI3EndcapLatency(getParameter<int>("FEI3EndcapLatency", lBuffer));
  
  ignoreBoolVec = getParameter<bool>("FEI3EndcapHitDuplication", lBuffer);
  ignoreIntVec = getParameter<int>("FEI3EndcapSmallHitToT", lBuffer);
  
  md.setFEI3EndcapTimingSimTune(getParameter<int>("FEI3EndcapTimingSimTune", lBuffer));
  md.setEndcapCrossTalk(getParameter<double>("EndcapCrossTalk", lBuffer));
  md.setEndcapNoiseOccupancy(getParameter<double>("EndcapNoiseOccupancy", lBuffer));
  md.setEndcapDisableProbability(getParameter<double>("EndcapDisableProbability", lBuffer));
  ignoreDoubleVec = getParameter<double>("EndcapLorentzAngleCorr", lBuffer);

  md.setEndcapNoiseShape({getParameter<float>("PixelNoiseShape", lBuffer),
                                   getParameter<float>("PixelNoiseShape", lBuffer),
                                   getParameter<float>("PixelNoiseShape", lBuffer)});

  md.setBLayerTimingIndex(getParameter<float>("BLayerTimingIndex", lBuffer));
  md.setBLayerTimingProbability(getParameter<float>("BLayerTimingProbability", lBuffer));

  md.setLayer1TimingIndex(getParameter<float>("Layer1TimingIndex", lBuffer));
  md.setLayer1TimingProbability(getParameter<float>("Layer1TimingProbability", lBuffer));

  md.setLayer2TimingIndex(getParameter<float>("Layer2TimingIndex", lBuffer));
  md.setLayer2TimingProbability(getParameter<float>("Layer2TimingProbability", lBuffer));

  md.setEndcap1TimingIndex(getParameter<float>("Endcap1TimingIndex", lBuffer));
  md.setEndcap1TimingProbability(getParameter<float>("Endcap1TimingProbability", lBuffer));

  md.setEndcap2TimingIndex(getParameter<float>("Endcap2TimingIndex", lBuffer));
  md.setEndcap2TimingProbability(getParameter<float>("Endcap2TimingProbability", lBuffer));

  md.setEndcap3TimingIndex(getParameter<float>("Endcap3TimingIndex", lBuffer));
  md.setEndcap3TimingProbability(getParameter<float>("Endcap3TimingProbability", lBuffer));

  // Radiation damage simulation
  md.setFluenceLayer(getParameter<double>("BarrelFluence", lBuffer));
  std::vector<std::string> barrelFluenceFile = getParameterString("BarrelRadiationFile", lBuffer);
  for (const auto & fluence : barrelFluenceFile) {
    mapsPath_list.push_back(PathResolverFindCalibFile(fluence));
  }

  if (f.run1) {    // RUN1
    md.setBarrelNoiseShape({getParameter<float>("BLayerNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer)});
  } else {     // RUN2
    md.setDBMToTThreshold(getParameter<int>("DBMToTThreshold", lBuffer));
    md.setDBMCrossTalk(getParameter<double>("DBMCrossTalk", lBuffer));
    md.setDBMNoiseOccupancy(getParameter<double>("DBMNoiseOccupancy", lBuffer));
    md.setDBMDisableProbability(getParameter<double>("DBMDisableProbability", lBuffer));

    md.setBarrelNoiseShape({getParameter<float>("IBLNoiseShape", lBuffer),
                                     getParameter<float>("BLayerNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer),
                                     getParameter<float>("PixelNoiseShape", lBuffer)});

    md.setDBMNoiseShape({getParameter<float>("IBLNoiseShape", lBuffer),
                                  getParameter<float>("IBLNoiseShape", lBuffer),
                                  getParameter<float>("IBLNoiseShape", lBuffer)});

    // Radiation damage simulation for 3D sensor
    md.setFluenceLayer3D(getParameter<double>("3DFluence", lBuffer));
    std::vector<std::string> barrel3DFluenceFile = getParameterString("3DRadiationFile", lBuffer);
    for (const auto & fluence3D : barrel3DFluenceFile) {
      mapsPath_list3D.push_back(PathResolverFindCalibFile(fluence3D));
    }
  }
  md.setRadSimFluenceMapList(mapsPath_list);
  md.setRadSimFluenceMapList3D(mapsPath_list3D);
  return *i;

}

