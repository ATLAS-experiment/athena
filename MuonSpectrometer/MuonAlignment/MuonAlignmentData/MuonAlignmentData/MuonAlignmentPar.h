/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_MUONALIGNMENTPAR_H
#define MUONALIGNMENTDATA_MUONALIGNMENTPAR_H

#include <string>
#include <Identifier/Identifier.h>

/**
 * Basic class to map the MuonAlignment parameters to the different subdetectors 
 * inside the muon system.
*/
class MuonAlignmentPar {
public:
    MuonAlignmentPar() = default;
    virtual ~MuonAlignmentPar() = default;
    
    /// Setters and getters for the Athena Identifier
    void setIdentifier(const Identifier& id);
    const Identifier& identify() const;
    /// Odering operater using the Identifier
    bool operator<(const MuonAlignmentPar& other) const;
    
    /// @brief AMDB identifiers. They're often not the same as the ATLAS ones (TGCs)
    void setAmdbId(const std::string& stName, int stEta, int stPhi, int stJob);
    /// Seems to correspond to the multilayer but sometimes also stEta.
    int AmdbJob() const; 
    /// @brief station Eta
    int AmdbEta() const;
    /// @brief station phi
    int AmdbPhi() const;
    /// @brief station Name
    const std::string& AmdbStation() const;

private:
    Identifier m_id{0};
    std::string m_station{};
    int m_eta{0};
    int m_phi{0};
    int m_job{0};
};
/// Operator for 
bool operator<(const Identifier& a, const MuonAlignmentPar& b);
bool operator<(const MuonAlignmentPar& a, const Identifier& b);

#endif  // MUONALIGNMENTDATA_MUONALIGNMENTPAR_H
