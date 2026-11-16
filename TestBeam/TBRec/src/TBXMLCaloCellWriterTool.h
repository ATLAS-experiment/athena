/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBXMLCALOCELLWRITERTOOL_H
#define TBREC_TBXMLCALOCELLWRITERTOOL_H
///////////////////////////////////////////////////////////////////////////////
// \brief writes CaloCell for TB event display
///////////////////////////////////////////////////////////////////////////////

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloGeoHelpers/CaloSampling.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "TBXMLWriterToolBase.h"

#include <iosfwd>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>

class TBXMLWriter;

class TBXMLCaloCellWriterTool : public TBXMLWriterToolBase
{

 public:

  /////////////////////////////////
  // Constructors and Destructor //
  /////////////////////////////////

  /// \brief tool constructor
  TBXMLCaloCellWriterTool(const std::string& type,
			  const std::string& name,
			  const IInterface* parent);

  ~TBXMLCaloCellWriterTool();

  virtual StatusCode initialize() override;

  ////////////
  // Action //
  ////////////

 protected:

  virtual StatusCode writeRunFiles(const std::string& fileDir,
				   unsigned int runNumber) override;
  virtual StatusCode writeEvent(std::ostream& outFile, 
				std::string_view /* entryTag */ ) override;

  virtual StatusCode convertProperties();

 private:

  static const unsigned int m_nCols;

  Gaudi::Property<std::vector<std::string>> m_includedCalos{this, "IncludedCalos", {}, "List of included calorimeters"};
  Gaudi::Property<std::vector<std::string>> m_includedSamplings{this, "IncludedSamplings", {}, "List of included calorimeter samplings"};

  Gaudi::Property<double> m_etaMin{this, "EtaMin", -5.0, "Minimum cell eta"};
  Gaudi::Property<double> m_etaMax{this, "EtaMax", 5.0, "Maximum cell eta"};
  Gaudi::Property<double> m_phiMin{this, "PhiMin", 0.*CLHEP::deg, "Minimum cell phi"};
  Gaudi::Property<double> m_phiMax{this, "PhiMax", 360.*CLHEP::deg, "Maximum cell phi"};

  bool                                  m_firstEvent{true};

  std::vector<CaloCell_ID::SUBCALO>     m_caloIndices;
  std::vector<CaloSampling::CaloSample> m_caloSamplings;
  Gaudi::Property<std::string>          m_cellContainer{this, "InputCellContainer", "", "Input CaloCellContainer name"};

  const CaloCell_ID*                    m_idHelper{};

  const TBXMLWriter*                    m_mother{};

  std::unordered_set<EventIDBase::number_type> m_runNumbers;

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

};
#endif
