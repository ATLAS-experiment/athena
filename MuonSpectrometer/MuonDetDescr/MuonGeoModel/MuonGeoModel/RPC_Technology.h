/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPC_H
#define RPC_H

#include "MuonGeoModel/Technology.h"

#define RPCprint false
namespace MuonGM {
    class MYSQL;

    class RPC : public Technology {
      public:
        // the internal RPC geometry parameters{
        // the external support panel is the pre-bent panel: the honeycomb material can depend of the
        // station type !!!!
        double externalSupPanelThickness{0.};
        double externalAlSupPanelThickness{0.}; // given by AMDB  W RPC
        // for Lay.P lower=external and upper does not exist
        // for Lay.M lower and upper are given separatly by AMDB
        double lowerSupPanelThickness{0.};
        double lowerAlSupPanelThickness{0.};
        double upperSupPanelThickness{0.};
        double upperAlSupPanelThickness{0.};
        // the central support panel
        double centralSupPanelThickness{0.};
        double centralAlSupPanelThickness{0.}; // given by AMDB  W RPC

        // A rpcLayer consists of a bakelite gas volume,
        // a pet foil (on top[bottom] of the bakelite gas volume for the lower[upper] gas gap in a doublet),
        // two strip panel layers, one on each side of the bakelite gas gap
        double rpcLayerThickness{0.}; // given by AMDB
        // TotalThickness = 2*rpcLayerThickness + centralSupPanelThickness + externalSupPanelThickness
        // for layout P and following
        // TotalThickness = 2*rpcLayerThickness + centralSupPanelThickness + lower + upperSupPanelThickness
        // for layout M
        double TotalThickness{0.}; // given by AMDB

        // GasGap consists of a bakelite gas volume,
        // a pet foil (on top[bottom] of the bakelite gas volume for the lower[upper] gas gap in a doublet)
        int NGasGaps_in_s{0};           // from AMDB
        int NGasGaps_in_z{0};           // from AMDB
        double GasGapThickness{0.};      // must be computed, if needed, from the following data
        double bakeliteThickness{0.};    // from AMDB P
        double bakeliteframesize{0.};    // from where ?????????????
        double bakelitePetThickness{0.}; // from amdb
        double gasThickness{0.};         // from AMDB
        double totalAirThickness{0.};    // from AMDB P

        // The spacers reinforcing the gasgap structure
        double spacerDiameter{0.};
        double spacerPitch{0.}; // from AMDB P

        // extra pet foil
        double petFoilThickness{0.};

        // StripPanels
        int NstripPanels_in_s{0};
        int NstripPanels_in_z{0}; // from AMDB
        double MidChamberDeadRegion_in_s{0.};
        double MidChamberDeadRegion_in_z{0.}; // from AMDB
        double stripPitchS{0.};
        double stripPitchZ{0.};
        double stripSeparation{0.};               // from AMDB
        double stripPanelThickness{0.};           // to be computed from the following quantities
        double stripPanelFoamThickness{0.};       // from AMDB
        double stripPanelCopperSkinThickness{0.}; // from AMDB
        double stripPanelStripSidePetThickness{0.};
        double stripPanelGroundSidePetThickness{0.}; // from AMDB
        double frontendBoardWidth{0.};
        double backendBoardWidth{0.};

        double maxThickness{0.}; // dictated by sup clearance !!!!

        // end of the internal RPC geometry parameters}

        double supportThickness{0.};
        double supportThicknessWidth{0.};

        double upperHoneycombLayerThickness{0.};
        double alThickness{0.};

        double RPCLayerThickness{0.};
        double foamSpacerThickness{0.};
        double xx{0.};
        double lowerHoneycombLayerThickness{0.};
        double yy{0.};
        double totalThickness{0.};
        double pitchs{0.};
        double pitchz{0.};
        double deadSeparation{0.};
        double gazGapThickness{0.};
        double stripsSupportThickness{0.};
        double stripsThickness{0.};
        double internalMidchamberDeadRegion{0.};
        double firstStripsOffset{0.};
        double secondStripsOffset{0.};
        double numberOfStripsReadouts{0.};

        double centralalThickness{0.};
        double petUpperThickness{0.};
        double petLowerThickness{0.};
        double sInternalMidchamberDeadRegion{0.};
        double zInternalMidchamberDeadRegion{0.};
        double sStripsOffset{0.};
        double zFirstStripsOffset{0.};
        double zSecondStripsOffset{0.};
        double sNumberOfStripsReadouts{0.};
        double zNumberOfStripsReadouts{0.};
        double sGasGapsNumber{0.};

        RPC(MYSQL& mysql, const std::string& s)
            : Technology(mysql, s){}

        double GetCentralalThickness() const { return centralalThickness; }          
        double GetRPCLayerThickness() const { return RPCLayerThickness; }
        double GetFoamSpacerThickness() const { return foamSpacerThickness; }
        double GetUpperHoneycombLayerThickness() const { return upperHoneycombLayerThickness; }
        double GetAlThickness() const { return alThickness; }
        double GetPitchs() const { return pitchs; }
        double GetPitchz() const { return pitchz; }
        double GetDeadSeparation() const { return deadSeparation; }
        double GetBakeliteThickness() const { return bakeliteThickness; }
        double GetGazGapThickness() const { return gazGapThickness; }
        double GetAirThickness() const { return totalAirThickness; }
        double GetStripsSupportThickness() const { return stripsSupportThickness; }
        double GetStripsThickness() const { return stripsThickness; }
        double GetPetUpperThickness() const { return petUpperThickness; }
        double GetPetLowerThickness() const { return petLowerThickness; }
        double GetSInternalMidchamberDeadRegion() const { return sInternalMidchamberDeadRegion; }
        double GetZInternalMidchamberDeadRegion() const { return zInternalMidchamberDeadRegion; }
        double GetSpacerDiameter() const { return spacerDiameter; }
        double GetSpacerPitch() const { return spacerPitch; }
        double GetSStripsOffset() const { return sStripsOffset; }
        double GetZFirstStripsOffset() const { return zFirstStripsOffset; }
        double GetZSecondStripsOffset() const { return zSecondStripsOffset; }
        double GetSNumberOfStripsReadouts() const { return sNumberOfStripsReadouts; }
        double GetZNumberOfStripsReadouts() const { return zNumberOfStripsReadouts; }
        double GetSGasGapsNumber() const { return sGasGapsNumber; }
        // set methods

        void SetCentralalThickness(double d) { centralalThickness = d; };
        void SetRPCLayerThickness(double d) { RPCLayerThickness = d; };
        void SetFoamSpacerThickness(double d) { foamSpacerThickness = d; };
        void SetUpperHoneycombLayerThickness(double d) { upperHoneycombLayerThickness = d; };
        void SetAlThickness(double d) { alThickness = d; };
        void SetPitchs(double d) { pitchs = d; };
        void SetPitchz(double d) { pitchz = d; };
        void SetDeadSeparation(double d) { deadSeparation = d; };
        void SetBakeliteThickness(double d) { bakeliteThickness = d; };
        void SetGazGapThickness(double d) { gazGapThickness = d; };
        //    void SetPetThickness(double d){petThickness=d;};
        void SetAirThickness(double d) { totalAirThickness = d; };
        void SetStripsSupportThickness(double d) { stripsSupportThickness = d; };
        void SetStripsThickness(double d) { stripsThickness = d; };
        void SetPetUpperThickness(double d) { petUpperThickness = d; };
        void SetPetLowerThickness(double d) { petLowerThickness = d; };
        void SetSInternalMidchamberDeadRegion(double d) { sInternalMidchamberDeadRegion = d; };
        void SetZInternalMidchamberDeadRegion(double d) { zInternalMidchamberDeadRegion = d; };
        void SetSpacerDiameter(double d) { spacerDiameter = d; };
        void SetSpacerPitch(double d) { spacerPitch = d; };
        void SetSStripsOffset(double d) { sStripsOffset = d; };
        void SetZFirstStripsOffset(double d) { zFirstStripsOffset = d; };
        void SetZSecondStripsOffset(double d) { zSecondStripsOffset = d; };
        void SetSNumberOfStripsReadouts(double d) { sNumberOfStripsReadouts = d; };
        void SetZNumberOfStripsReadouts(double d) { zNumberOfStripsReadouts = d; };
        void SetSGasGapsNumber(double d) { sGasGapsNumber = d; };
    };
} // namespace MuonGM

#endif
