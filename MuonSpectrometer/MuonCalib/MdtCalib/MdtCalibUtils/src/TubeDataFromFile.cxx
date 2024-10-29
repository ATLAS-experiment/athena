/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibUtils/TubeDataFromFile.h"

#include <TString.h>  // for Form
#include <math.h>
#include <stdio.h>

#include <sstream>
#include <string>


namespace MuonCalib {

    std::ostream& TubeDataFromFile::write(std::ostream& os) const {
        if (m_regions != m_tubeData.size()) {
            MsgStream log(Athena::getMessageSvc(), "MdtTubeFitContainer");
            log << MSG::WARNING << "TubeDataFromFile::write: <inconsistent count>" << endmsg;
        }

        os << "v0.0 " << m_regions << " ";
        unsigned int counter{0};
        for (const MdtTubeFitContainer* container : m_tubeData) {
            if (!container) continue;
            const unsigned int nml = container->numMultilayers();
            const unsigned int nl = container->numLayers();
            const unsigned int nt = container->numTubes();
            const std::string datatype = container->name();
            const std::string implementation = container->implementation();
            const int stnEta = container->idHelperSvc()->stationEta(container->identify());
            const int stnPhi = container->idHelperSvc()->stationPhi(container->identify());
            const std::string stnName = container->idHelperSvc()->stationNameString(container->identify());
            os << datatype << " " << implementation << " " << counter << " " << container->size() << std::endl;
            ++counter;
            const MdtIdHelper& id_helper{container->idHelperSvc()->mdtIdHelper()};
            for (unsigned int km = 1; km <= nml; ++km) {
                for (unsigned int kl = 1; kl <= nl; ++kl) {
                    for (unsigned int kt = 1; kt <= nt; ++kt) {
                        const Identifier chId{id_helper.channelID(container->identify(),km ,kl ,kt)};
                        const MdtTubeFitContainer::SingleTubeCalib* stc = container->getCalib(chId);
                        const MdtTubeFitContainer::SingleTubeFit* stf = container->getFit(chId);
                        double t0 = 999.;
                        if (stc) {
                            t0 = stc->t0;
                            if (std::isnan(t0) != 0) t0 = -99999.;
                        }
                        constexpr int technology = 0;
                        os << "  " << stnName << "  " << stnPhi << "  " << stnEta 
                           << "  " << technology << "  " << km  << "  " << kl << "  " << kt;
                        if (stc) {
                            os << " " << t0 << " " << stc->adcCal << " " << stc->statusCode;
                        } else{
                            os << " " << 0 << " " << 0 << " " << 0;
                           
                        }
                        if (stf)
                            os << " " << stf->statistics << " " << stf->chi2Tdc << " " << stf->cov[4]  // stf->t0Err
                               << " " << stf->par[5]                                                   // stf->tMax
                               << " " << stf->cov[5]                                                   // stf->tMaxErr
                               << " " << stf->par[0]                                                   // stf->noise
                               << " " << stf->cov[0]                                                   // stf->noiseErr
                               << " " << stf->par[6]                                                   // stf->slope
                               << " " << stf->cov[6];                                                  // stf->slopeErr;
                        else {
                            os << " " << -1 << " " << 0 << " " << 0 << " " << 0 << " " 
                                      << 0 << " " << 0 << " " << 0 << " " << 0 << " "
                                      << 0;
                        }
                        os << std::endl;
                    }
                }
            }
        }
        return os;
    }


    std::ostream& TubeDataFromFile::write_forDB(std::ostream& ftube, int mdt_head, int lowrun, int uprun) const {
        //////////////////////////////////////////////
        // write out ascii files for Calibration DB //
        //////////////////////////////////////////////
        if (m_regions != m_tubeData.size()) {
            MsgStream log(Athena::getMessageSvc(), "MdtTubeFitContainer");
            log << MSG::WARNING << "TubeDataFromFile::write_forDB: <inconsistent count>" << endmsg;
        }

        for (const MdtTubeFitContainer* container : m_tubeData) {
            if (!container) continue;
            const unsigned int nml = container->numMultilayers();
            const unsigned int nl = container->numLayers();
            const unsigned int nt = container->numTubes();
            const std::string datatype = container->name();
            const std::string implementation = container->implementation();
            const int stnEta = container->idHelperSvc()->stationEta(container->identify());
            const int stnPhi = container->idHelperSvc()->stationPhi(container->identify());
            const std::string stnName = container->idHelperSvc()->stationNameString(container->identify());

            const MdtIdHelper& id_helper{container->idHelperSvc()->mdtIdHelper()};
            for (unsigned int km = 1; km <= nml; ++km) {
                for (unsigned int kl = 1; kl <= nl; ++kl) {
                    for (unsigned int kt = 1; kt <= nt; ++kt) {
                        const Identifier chId{id_helper.channelID(container->identify(),km ,kl ,kt)};
                        const MdtTubeFitContainer::SingleTubeCalib* stc = container->getCalib(chId);
                        const MdtTubeFitContainer::SingleTubeFit* stf = container->getFit(chId);
                        constexpr int technology = 0;

                        MuonFixedId fixId{};
                        int sc{0}, stat{-1};
                        double t0{0.}, adcm{0.}, chi2tdc{0.}, t0err{0.}, 
                               tmax{0.}, tmaxerr{0.}, noise{0.}, noiseerr{0.}, 
                               slope{0.}, sloperr{0.};
                        int tube_id{0}, runflag{0}, validflag{0};

                        if (!fixId.setTechnology(technology) || 
                            !fixId.setStationName(fixId.stationStringToFixedStationNumber(stnName)) ||
                            !fixId.setStationEta(stnEta) || 
                            !fixId.setStationPhi(stnPhi) || 
                            !fixId.setMdtTube(kt) ||
                            !fixId.setMdtTubeLayer(kl) || 
                            !fixId.setMdtMultilayer(km)) {
                                std::stringstream except{};
                                except<<__FILE__<<":"<<__LINE__<<"nTubeDataFromFile::write_forDB() - Setting identifier failed!";
                                throw std::runtime_error(except.str());
                            }
                            tube_id = fixId.getIdInt();

                            if (stc) {
                                t0 = stc->t0;
                                adcm = stc->adcCal;
                                sc = stc->statusCode;
                            } 
                            if (stf) {
                                stat = stf->statistics;
                                chi2tdc = stf->chi2Tdc;
                                t0err = stf->cov[4];
                                tmax = stf->par[5];
                                tmaxerr = stf->cov[5];
                                noise = stf->par[0];
                                noiseerr = stf->cov[0];
                                slope = stf->par[6];
                                sloperr = stf->cov[6];
                            } 

                            if (std::isnan(t0) != 0) t0 = -99999.;

                            ftube<<mdt_head<<","<<tube_id<<","<<fixId.mdtChamberId().getIdInt()<<","
                                 <<","<<lowrun<<","<<uprun<<","<<runflag<<","<<sc
                                 <<validflag<<","<<stat<<",";

                            for (int ii = -2; ii < 35; ii++) {
                                if (ii == -2) {
                                   ftube<<chi2tdc<<",";
                                } else if (ii == 0) {
                                    ftube<<noise<<",";
                                } else if (ii == 4) {
                                    ftube<<t0<<",";
                                } else if (ii == 5) {
                                    ftube<<tmax<<",";
                                } else if (ii == 6) {
                                    ftube<<slope<<",";
                                } else if (ii == 10) {
                                    ftube<<t0err<<",";
                                } else if (ii == 11) {
                                    ftube<<tmaxerr<<",";
                                } else if (ii == 12) {
                                    ftube<<noiseerr<<",";
                                } else if (ii == 13) {
                                    ftube<<sloperr<<",";
                                } else if (ii == 14) {
                                    ftube<<adcm<<",";
                                } else {
                                    ftube<<0.0<<",";
                                }
                            }
                            ftube<<"0,0,algoflag,tubegrpgr"<<std::endl;
                        }
                    }
                }
        }
        return ftube;
    }

}  // namespace MuonCalib

std::ostream& operator<<(std::ostream& os, const MuonCalib::TubeDataFromFile& data) { return data.write(os); }
