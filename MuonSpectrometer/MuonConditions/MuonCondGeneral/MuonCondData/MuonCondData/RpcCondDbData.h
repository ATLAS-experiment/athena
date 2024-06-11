/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_RPCCONDDBDATA_H
#define MUONCONDDATA_RPCCONDDBDATA_H

//STL includes
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

//Athena includes
#include "Identifier/Identifier.h"
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 


//forward declarations
class Identifier;


class RpcCondDbData {

  friend class RpcCondDbAlg;

public:

    RpcCondDbData() = default;
    ~RpcCondDbData() = default;

    void setEfficiency      (Identifier, double             );
    void setFracClusterSize1(Identifier, double             );
    void setFracClusterSize2(Identifier, double             );
    void setFracDeadStrip   (Identifier, double             );
    void setGapEfficiency   (Identifier, double             );
    void setMeanClusterSize (Identifier, double             );
    void setProjectedTrack  (Identifier, int                );
    void setStripTime       (Identifier, const std::vector<double>&);

    std::optional<double> getEfficiency      (const Identifier& ) const;
    std::optional<double> getFracClusterSize1(const Identifier& ) const;
    std::optional<double> getFracClusterSize2(const Identifier& ) const;
    std::optional<double> getFracClusterSize3(const Identifier& ) const;
    std::optional<double> getFracDeadStrip   (const Identifier& ) const;
    std::optional<double> getGapEfficiency   (const Identifier& ) const;
    std::optional<double> getMeanClusterSize (const Identifier& ) const;
    std::optional<int>    getProjectedTrack  (const Identifier& ) const;
    std::optional<double> getStripTime(const Identifier& ) const;
 
private:
    /// Used to describe the panel Efficiency
    std::unordered_map<Identifier, double> m_cachedEfficiency;
    /// Used for cluster size evaluation
    std::unordered_map<Identifier, double> m_cachedFracClusterSize1;
    /// Used for cluster size evaluation
    std::unordered_map<Identifier, double> m_cachedFracClusterSize2;
    /// Used in digitization for efficiency calculation
    std::unordered_map<Identifier, double> m_cachedFracDeadStrip;
    /// Used to describe the gap efficiency
    std::unordered_map<Identifier, double> m_cachedGapEfficiency;
    /// Used for cluster size evaluation
    std::unordered_map<Identifier, double> m_cachedMeanClusterSize;
    /// Used in digitization for efficiency / mean cluster evaluation
    std::unordered_map<Identifier, int> m_cachedProjectedTracks;
    /// Used in Rdo -> Prd conversion
    std::unordered_map<Identifier, double> m_cachedStripTime;

};

CLASS_DEF( RpcCondDbData, 25128902, 1)
CLASS_DEF( CondCont<RpcCondDbData>, 178635428, 0)

#endif
