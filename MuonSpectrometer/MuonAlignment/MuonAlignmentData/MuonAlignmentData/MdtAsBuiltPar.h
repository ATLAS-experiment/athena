/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_MDTASBUILTPAR_H
#define MUONALIGNMENTDATA_MDTASBUILTPAR_H

#include <string>
#include <array>
#include <iosfwd>

#include "MuonAlignmentData/MuonAlignmentPar.h"

/**
 * Container classifier the MDT as-built parameters
 * See parameter description in
 * http://atlas-muon-align.web.cern.ch/atlas-muon-align/endplug/asbuilt.pdf
 */
class MdtAsBuiltPar : public MuonAlignmentPar {
public:
    // Default constructor
    MdtAsBuiltPar() = default;
    // destructor
    virtual ~MdtAsBuiltPar() override = default;

    /**
     * MDT multi-layer index
     */
    enum class multilayer_t { ML1 = 0, ML2, NMLTYPES };

    /**
     * MDT tube side
     */
    enum class tubeSide_t {
        POS = 0,  // s>0 side
        NEG,      // s<0 side
        NTUBESIDETYPES
    };

    /**
     * Set the alignment parameters for a ML and a tube side
     */
    void setAlignmentParameters(multilayer_t iML, tubeSide_t iTubeSide, float y0, float z0, float alpha, float ypitch, float zpitch,
                                int stagg);


    // Getters
    double y0(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).y0; }
    double z0(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).z0; }
    double alpha(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).alpha; }
    double ypitch(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).ypitch; }
    double zpitch(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).zpitch; }
    int stagg(multilayer_t iML, tubeSide_t iTubeSide) const { return meas(iML, iTubeSide).stagg; }


    // Alignment parameters
    struct AlignmentParameters {
        AlignmentParameters() = default;
        float y0{0.f};              // y position of first wire w.r.t. reference point
        float z0{0.f};              // z position of first wire w.r.t. reference point
        float alpha{0.f};           // rotation around tube axis
        float ypitch{0.f};          // y pitch
        float zpitch{0.f};          // z pitch
        int stagg{1};             // is tube staggering ATLAS-standard or reversed?
    };
private:
    static constexpr unsigned int NMEAS = static_cast<unsigned int>(multilayer_t::NMLTYPES) * 
                                          static_cast<unsigned int>(tubeSide_t::NTUBESIDETYPES);
    std::array<AlignmentParameters, NMEAS>  m_meas{};  // in this order: ML1_HV, ML1_RO, ML2_HV, ML2_RO

    AlignmentParameters& meas(multilayer_t iML, tubeSide_t iTubeSide) { 
        const unsigned int idx = static_cast<unsigned int>(tubeSide_t::NTUBESIDETYPES) *
                                 static_cast<unsigned int>(iML) + static_cast<unsigned>(iTubeSide);
        return m_meas.at(idx); 
    }
    const AlignmentParameters& meas(multilayer_t iML, tubeSide_t iTubeSide) const { 
        const unsigned int idx = static_cast<unsigned int>(tubeSide_t::NTUBESIDETYPES) *
                                 static_cast<unsigned int>(iML) + static_cast<unsigned>(iTubeSide);
        return m_meas.at(idx);  
    }
};

std::ostream& operator<<(std::ostream& ostr, const MdtAsBuiltPar& par);
#endif  // MUONALIGNMENTDATA_MDTASBUILTPAR_H
