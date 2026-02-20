/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EGAMMA_LAYER_RECALIB_TOOL
#define EGAMMA_LAYER_RECALIB_TOOL

/////////////////////////////////////////////////////////////////////////////////
/// Name    : egammaLayerRecalibTool.h
/// Package : egammaLayerRecalibTool
/// Author  : R. Turra
/// Created : May 2013
/// Twiki   : https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/egammaLayerRecalibTool
///
/// DESCRIPTION:
///
/// This class provide corrected layer energies.

/////////////////////////////////////////////////////////////////////////////////

// TODO: rewrite with 1 modifier <-> 1 class (not 1 modifier <-> 2 classes amount + modifier)
// TODO: remove all not used cases

#include <string>
#include <vector>
#include <memory>

#include "AsgTools/AsgTool.h"
#include "AsgMessaging/AsgMessaging.h"
#include "xAODEgamma/Egamma.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEventInfo/EventInfo.h"
#include "PATInterfaces/CorrectionCode.h"

#include <TH1.h>
#include <TH2.h>
#include <TFormula.h>
#include "egammaLayerRecalibTool/corr_HV_EMBPS.h"
#include "egammaLayerRecalibTool/corr_HV_EMECPS.h"
#include "egammaLayerRecalibTool/corr_pileupShift.h"


struct StdCalibrationInputs
{
  float averageInteractionsPerCrossing;  // only for pileup correction
  unsigned int RunNumber;   // only for HV presampler correction and accordion energy correction
  double eta;
  double phi;               // only for HV presampler correction
  double E0raw;
  double E1raw;
  double E2raw;
  double E3raw;
  double etaCalo;
};


// TODO: add protections for invalid inputs (histogram overflows)
struct GetAmountBase
{
  virtual float operator()(const StdCalibrationInputs & input) const = 0;
  virtual ~GetAmountBase() { };
};


// object adaptor (prefer composition vs inheritance)
struct GetAmountHVPSGuillaume : public GetAmountBase
{
  virtual float operator()(const StdCalibrationInputs & input) const;
private:
  corr_HV_EMBPS m_tool;
};

struct GetAmountHVEMECPS207 :  public GetAmountBase
{
  virtual float operator()(const StdCalibrationInputs & input) const;
private:
  corr_HV_EMECPS m_toolEMECPS;
};

struct GetAmountPileupE0 : public GetAmountBase
{
  GetAmountPileupE0(corr_pileupShift* tool) : m_tool(tool) { };
  virtual float operator()(const StdCalibrationInputs & inputs) const;
private:
  corr_pileupShift* m_tool;
};

struct GetAmountPileupE1 : public GetAmountBase
{
  GetAmountPileupE1(corr_pileupShift* tool) : m_tool(tool) { };
  virtual float operator()(const StdCalibrationInputs & inputs) const;
private:
  corr_pileupShift* m_tool;
};

struct GetAmountPileupE2 : public GetAmountBase
{
  GetAmountPileupE2(corr_pileupShift* tool) : m_tool(tool) { };
  virtual float operator()(const StdCalibrationInputs & inputs) const;
private:
  corr_pileupShift* m_tool;
};

struct GetAmountPileupE3 : public GetAmountBase
{
  GetAmountPileupE3(corr_pileupShift* tool) : m_tool(tool) { };
  virtual float operator()(const StdCalibrationInputs & inputs) const;
private:
  corr_pileupShift* m_tool;
};



struct GetAmountFormula : public GetAmountBase
{
  GetAmountFormula(const TFormula & formula) : m_formula(formula) { };
  virtual float operator()(const StdCalibrationInputs & input) const;
protected:
  TFormula m_formula;
};


struct GetAmountHisto1D : public GetAmountBase
{
  GetAmountHisto1D(const TH1& histo)
      : m_histo(static_cast<TH1*>(histo.Clone())) {
    m_histo->SetDirectory(nullptr);
  };
  virtual float operator()(const StdCalibrationInputs & input) const;
protected:
  std::unique_ptr<TH1> m_histo;
};


struct GetAmountHisto1DUp : public GetAmountHisto1D
{
  GetAmountHisto1DUp(const TH1& histo) : GetAmountHisto1D(histo) { };
  virtual float operator()(const StdCalibrationInputs & input) const;
};


struct GetAmountHisto1DDown : public GetAmountHisto1D
{
  GetAmountHisto1DDown(const TH1& histo) : GetAmountHisto1D(histo) { };
  virtual float operator()(const StdCalibrationInputs & input) const;
};


struct GetAmountHisto1DErrorUp : public GetAmountHisto1D
{
  GetAmountHisto1DErrorUp(const TH1& histo) : GetAmountHisto1D(histo) { };
  virtual float operator()(const StdCalibrationInputs & input) const;
};


struct GetAmountHisto1DErrorDown : public GetAmountHisto1D
{
  GetAmountHisto1DErrorDown(const TH1& histo) : GetAmountHisto1D(histo) { };
  virtual float operator()(const StdCalibrationInputs & input) const;
};

struct GetAmountHisto2D : public GetAmountBase
{
  GetAmountHisto2D(const TH2F& histo) : m_histo(histo) { m_histo.SetDirectory(nullptr); };
  virtual float operator()(const StdCalibrationInputs & input) const;
private:
  TH2F m_histo;
};


struct GetAmountHisto2DEtaCaloRunNumber : public GetAmountBase
{
  GetAmountHisto2DEtaCaloRunNumber(const TH2F& histo) : m_histo(histo) { m_histo.SetDirectory(0); };
  virtual float operator()(const StdCalibrationInputs & input) const;
protected:
  TH2F m_histo;
};


struct GetAmountFixed : public GetAmountBase
{
public:
  GetAmountFixed(float amount) : m_amount(amount) { }
  virtual float operator()(const StdCalibrationInputs & input) const;
private:
  float m_amount;
};

struct InputModifier
{
  enum NullPoint {ZEROBASED, ONEBASED, ZEROBASED_ALPHA, ONEBASED_ALPHA, SHIFT, SCALE, SUBTRACT};

  InputModifier(NullPoint base) : m_base(base) { };
  CP::CorrectionCode operator()(StdCalibrationInputs&, float amount) const;
  virtual ~InputModifier() { };
private:
  InputModifier() { }; // privatize default constructor
  // here we are one based (amount == 1 <=> null scale)
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const=0;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const=0;
  NullPoint m_base;
};


struct ScaleE0 : public InputModifier
{
  ScaleE0(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleE1 : public InputModifier
{
  ScaleE1(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleE2 : public InputModifier
{
  ScaleE2(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleE3 : public InputModifier
{
  ScaleE3(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleE1overE2 : public InputModifier
{
  ScaleE1overE2(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleEaccordion : public InputModifier
{
  ScaleEaccordion(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


struct ScaleEcalorimeter : public InputModifier
{
  ScaleEcalorimeter(NullPoint base) : InputModifier(base) { };
private:
  virtual void scale_inputs(StdCalibrationInputs&, float amount) const;
  virtual void shift_inputs(StdCalibrationInputs&, float amount) const;
};


class egammaLayerRecalibTool : public asg::AsgMessaging
{
public:
  typedef std::vector<std::pair<InputModifier*, GetAmountBase*> > ModifiersList;
  /**
   * @param tune string to configure the tuning
   - "" the tool has no effect
   - as default it is "current_default"
   - "test1" just for testing
  **/
  egammaLayerRecalibTool(const std::string& name, const std::string& tune, int SaccEnable = 1);
  egammaLayerRecalibTool(const std::string& tune, int SaccEnable = 1);
  ~egammaLayerRecalibTool() { clear_corrections(); delete m_pileup_tool; }

  CP::CorrectionCode applyCorrection(xAOD::Egamma &, const xAOD::EventInfo& event_info) const;

  /**
   * helper to create a tool from a string (useful for command line arguments)
   **/
  static std::pair<std::string, egammaLayerRecalibTool*> create(const std::string& type,
								const std::string& args);

  /**
   * apply layer calibration to the @param inputs
   **/
  CP::CorrectionCode scale_inputs(StdCalibrationInputs & inputs) const;
  /**
   * add custom layer scale correction. Can be called multiple times.
   **/
  void add_scale(InputModifier * modifier, GetAmountBase * amount);
  /**
   * add scale correction from string. Can be called multiple times.
   * The list of valid values is on the twiki
   **/
  void add_scale(const std::string& scale);
  /**
   * remove all the scale corrections
   **/
  void clear_corrections();

  void fixForMissingCells(bool fix = true) { m_aodFixMissingCells = fix; }
  void scaleMC(bool scaleMC = true) { m_scaleMC = scaleMC; }
  void disable_PSCorrections() {m_doPSCorrections=false;}
  void disable_S12Corrections() {m_doS12Corrections=false;}
  void disable_SaccCorrections() {m_doSaccCorrections=false;}

private:

  static const unsigned int m_Run2Run3runNumberTransition = 400000;

  std::string m_tune;
  bool m_doPSCorrections = true;
  bool m_doS12Corrections = true;
  bool m_doSaccCorrections = true;
  const std::string resolve_path(std::string filename) const;
  static std::string resolve_alias(const std::string& tune) ;
  ModifiersList m_modifiers;

  corr_pileupShift* m_pileup_tool = nullptr;

  bool m_aodFixMissingCells = false;
  bool m_scaleMC = false;
};

#endif // EGAMMA_LAYER_RECALIB_TOOL
