/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_RAW2DIGITOOL_H
#define AFP_RAW2DIGITOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AFP_Raw2Digi/IAFP_Raw2DigiTool.h"
#include "AFP_RawEv/AFP_RawContainer.h"
#include "xAODForward/AFPToFHit.h"
#include "xAODForward/AFPToFHitContainer.h"
#include "xAODForward/AFPSiHitContainer.h"
#include <string>
#include "TF1.h"

class AFP_Raw2DigiTool : public extends<AthAlgTool, IAFP_Raw2DigiTool>
{
public:
  AFP_Raw2DigiTool(const std::string &type, const std::string &name, const IInterface *parent);

  /// Does nothing 
  virtual ~AFP_Raw2DigiTool() override;

  /// Does nothing
  virtual StatusCode initialize() override;

  /// Creates xAOD for silicon detector
  StatusCode recoSiHits(const EventContext &ctx) const override;

  /// Creates xAOD for time-of-flight detector
  StatusCode recoToFHits(const EventContext &ctx) const override;

  StatusCode recoAll(const EventContext &ctx) const override;

  
  /// Does nothing
  virtual StatusCode finalize() override;

  /// Method that decodes raw information about time-over-threshold to
  /// number of clock ticks
  unsigned int decodeTimeOverThresholdSi (const unsigned int input, const unsigned int discConfig) const;
  
protected:
  SG::ReadHandleKey<AFP_RawContainer> m_rawDataContainerName{this, "rawDataContainerName", "AFP_RawData"};
  SG::WriteHandleKey<xAOD::AFPSiHitContainer> m_AFPSiHitsContainerName{this, "AFPSiHitsContainerName", "AFPSiHitContainer"};
  SG::WriteHandleKey<xAOD::AFPToFHitContainer> m_AFPHitsContainerNameToF{this, "AFPHitsContainerNameToF", "AFPToFHitContainer"};

  /// @brief Factor converting signal to time
  ///
  /// The value of the factor is 25/8192 nanoseconds (3.05 ps)
  static constexpr double s_timeConversionFactor = 25./8192.;

  /// @brief Factor converting pulse length to time
  ///
  /// The value of the factor is 0.09765625 nanoseconds, starting with 5th LSB
  static constexpr double s_pulseLengthFactor = 25./(8192>>5);

  /// @brief Function that transforms time-over-threshold to charge
  ///
  /// Transformation function can be set in steering cards
  Gaudi::Property<std::string> m_totToChargeTransfExpr{this, "TotToChargeTransfExpr", "1909 + x*363 + x*x*141", "Function that transforms time-over-threshold to charge"};
  Gaudi::Property<std::string> m_totToChargeTransfName{this, "TotToChargeTransfName", "TotToChargeTransfFunction", "Name of the function that transforms time-over-threshold to charge"};
  TF1 m_totToChargeTransformation;	

  /// Method that creates a new AFPToFHit and sets it valus according to #data
  void newXAODHitToF (xAOD::AFPToFHitContainer* tofHitContainer, const AFP_ToFRawCollection& collection, const AFP_ToFRawData& data, const EventContext& ctx) const;
  
  /// Method that creates a new AFPSiHit and sets it valus according to #data
  void newXAODHitSi (xAOD::AFPSiHitContainer* xAODSiHit, const AFP_SiRawCollection& collection, const AFP_SiRawData& data) const;

  /// @brief Method mapping hptdcID and hptdcChannel to train ID and bar in train ID
  ///
  /// The method requires that hptdcID and hptdcChannel are set in the
  /// tofHit passed as argument.  Mapping is implemented according to
  /// https://twiki.cern.ch/twiki/bin/view/Atlas/AFPHPTDC#Channel_Mapping
  void setBarAndTrainID(xAOD::AFPToFHit* tofHit, const EventContext& ctx) const;
  
private:
  
  // TODO: this should go to the database at some moment
  // @brief mapping of channels to trainID and barID
  //
  // indices are: runID (Run2, Run3 till 12 Oct 2002, Run3 since 12 Oct 2022); hptdcID (1,2); channel (0...11)
  // channels 1,4,7,10 shouldn't exist; Run2 data has channels 1 and 4 as well, this is a known problem
  const int m_channel2train[3][2][12]={{{0,-2,1,0,-2,1, 0,-1,1,0,-1,1}, {2,-2,3,2,-2,3, 2,-1,3,2,-1,3}},
                                       {{1,-1,0,1,-1,0, 1,-1,0,1,-1,0}, {3,-1,2,3,-1,2, 3,-1,2,3,-1,2}},
                                       {{1,-1,3,1,-1,3, 1,-1,3,1,-1,3}, {0,-1,2,0,-1,2, 0,-1,2,0,-1,2}}};
  const int m_channel2bar[3][2][12] = {{{0,-2,2,3,-2,1, 2,-1,0,1,-1,3}, {0,-2,2,3,-2,1, 2,-1,0,1,-1,3}},
                                       {{0,-1,2,3,-1,1, 2,-1,0,1,-1,3}, {0,-1,2,3,-1,1, 2,-1,0,1,-1,3}},
                                       {{0,-1,2,3,-1,1, 2,-1,0,1,-1,3}, {0,-1,2,3,-1,1, 2,-1,0,1,-1,3}}};
};
#endif 
