#ifndef JETSELECTORTOOLS_HSTPSKIMMINGTOOL_H
#define JETSELECTORTOOLS_HSTPSKIMMINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

#include "xAODJet/JetContainer.h"

class HSTPSkimmingTool
  : public extends<AthAlgTool, DerivationFramework::ISkimmingTool>
{
public:

  using base_class::base_class;

  virtual StatusCode initialize() override;

  virtual bool eventPassesFilter(const EventContext& ctx) const override;

private:

  Gaudi::Property<bool> m_doHSTPFiltering{ this, "doHSTPFiltering", true, "Perform HSTP filtering"};

};

#endif