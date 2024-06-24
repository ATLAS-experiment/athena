/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GLOBALSIM_L1MENURESOURCES_H
#define GLOBALSIM_L1MENURESOURCES_H

/*
 * L1TopoSimulation initialisation is done by reading in a L1Menu
 * object from the store, and using the information therein to
 * initialiaze L1TopoAlgorithms.
 *
 * In GlobalL1TopoSim, most of the initialiation data is obtained
 * during the python configuration step, using data found in the python
 * menu object, or lightly modified.
 *
 * However, there is some  data, for example the threshold used for the
 * cTauMultiplicity algorithm, that is the result of some non-trivial
 * processing. Instead of copying the code to do this processing to python,
 * the L1Menu object is read in at run time and the threshold is obtained
 * from that source.
 *
 */


#include <string>
#include <map>

namespace TrigConf {
  class L1Threshold;
  class L1Menu;
}

namespace GlobalSim {
  
  class L1MenuResources {
  public:
    L1MenuResources(const TrigConf::L1Menu& l1menu,
		    const std::string& confAlgName,
		    const std::string& confAlgTypeLabel);


    std::map<std::string, int> isolationFW_CTAU() const;
    std::map<std::string, int> isolationFW_CTAU_jTAUCoreScale() const;
    const TrigConf::L1Threshold& threshold() const;
    
    const std::string& menuName();
    std::string to_string() const;

  private:
    
    const TrigConf::L1Menu&  m_l1menu;
    std::string m_confAlgName;

    // confAlgTypeLabel: string such as "TOPO" or "MULTTOPO"
    std::string m_confAlgTypeLabel; 

  };
}

#endif
