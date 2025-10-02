/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */
#ifndef ISOLATIONSELECTION_ISOLATIONSELECTIONTOOL_H
#define ISOLATIONSELECTION_ISOLATIONSELECTIONTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/ReadDecorHandleKeyArray.h>

// Local include(s):
#include "IsolationSelection/IIsolationSelectionTool.h"
#include "IsolationSelection/IsolationWP.h"

// Forward declaration(s):
class TFile;
class TH1F;

class Interp3D;

namespace CP {

    class IsolationSelectionTool : public virtual CP::IIsolationSelectionTool, public asg::AsgTool {
        /// Create a proper constructor for Athena
        ASG_TOOL_CLASS(IsolationSelectionTool, CP::IIsolationSelectionTool)

    public:
        /// Constructor for standalone usage, but please do use the interface and ToolHandle
        IsolationSelectionTool(const std::string& name);
        /// Destructor
        virtual ~IsolationSelectionTool();

        /// Function initialising the tool
        virtual StatusCode initialize() override;
        /// Function finalizing the tool

        enum IsoWPType { Efficiency, Cut };
        virtual asg::AcceptData accept(const xAOD::Photon& x) const override;
        virtual asg::AcceptData accept(const xAOD::Electron& x) const override;
        virtual asg::AcceptData accept(const xAOD::Muon& x) const override;
        virtual asg::AcceptData accept(const strObj& x) const override;
        virtual asg::AcceptData accept(const xAOD::IParticle& x) const override;  // for tracks, and others?

        virtual const asg::AcceptInfo& getPhotonAcceptInfo() const override;
        virtual const asg::AcceptInfo& getElectronAcceptInfo() const override;
        virtual const asg::AcceptInfo& getMuonAcceptInfo() const override;
        virtual const asg::AcceptInfo& getObjAcceptInfo() const override;

        virtual const std::vector<std::unique_ptr<IsolationWP>>& getMuonWPs() const override;
        virtual const std::vector<std::unique_ptr<IsolationWP>>& getElectronWPs() const override;
        virtual const std::vector<std::unique_ptr<IsolationWP>>& getPhotonWPs() const override;
        virtual const std::vector<std::unique_ptr<IsolationWP>>& getObjWPs() const override;

        
        virtual StatusCode setIParticleCutsFrom(xAOD::Type::ObjectType ObjType) override;
        
        StatusCode addWP(const std::string& WP, xAOD::Type::ObjectType type);
        StatusCode addWP(std::unique_ptr<IsolationWP> wp, xAOD::Type::ObjectType type);
        StatusCode addMuonWP(const std::string& wpname);
        StatusCode addPhotonWP(const std::string& wpname);
        StatusCode addElectronWP(const std::string& wpname);
        StatusCode addUserDefinedWP(const std::string& WPname, xAOD::Type::ObjectType ObjType,
                                    std::vector<std::pair<xAOD::Iso::IsolationType, std::string>>& cuts, std::string key = "",
                                    IsoWPType type = Efficiency);
        StatusCode addCutToWP(IsolationWP* wp, const std::string& key_in, const xAOD::Iso::IsolationType t, const std::string& expression,
                              const xAOD::Iso::IsolationType isoCutRemap);
        StatusCode addCutToWP(IsolationWP* wp, const std::string& key, const xAOD::Iso::IsolationType t, const std::string& expression);

      
    private:
        // same interface for xAOD::IParticle and StrObj -> use  template
        template <typename T> void evaluateWP(const T& x, const std::vector<std::unique_ptr<IsolationWP>>& WP, asg::AcceptData& accept) const;
      
        Gaudi::Property<std::string> m_muWPname{this, "MuonWP", "Undefined", "Working point for muon"};
        Gaudi::Property<std::string> m_elWPname{this, "ElectronWP", "Undefined", "Working point for electron"};
        Gaudi::Property<std::string> m_phWPname{this, "PhotonWP", "Undefined", "Working point for photon"};
        Gaudi::Property<std::vector<std::string>> m_muWPvec{this, "MuonWPVec", {}, "Vector of working points for muon"};
        Gaudi::Property<std::vector<std::string>> m_elWPvec{this, "ElectronWPVec", {}, "Vector of working points for electron"};
        Gaudi::Property<std::vector<std::string>> m_phWPvec{this, "PhotonWPVec", {}, "Vector of working points for photon"};

        Gaudi::Property<std::string> m_muWPKey{this, "MuonKey", "/Muons/DFCommonGoodMuon/mu_cutValues_", "path of the cut map for muon"};
        Gaudi::Property<std::string> m_elWPKey{this, "ElectronKey", "/ElectronPhoton/LHTight/el_cutValues_",
                                               "path of the cut map for electron"};
        Gaudi::Property<std::string> m_phWPKey{this, "PhotonKey", "/ElectronPhoton/LHTight/el_cutValues_",
                                               "path of the cut map for photon"};
        Gaudi::Property<std::string> m_isoDecSuffix{
            this, "IsoDecSuffix", "", "Suffix added to output isolation variables to allow to NOT overwrite existing values"};


        /// input file
        Gaudi::Property<std::string> m_calibFileName{this, "CalibFileName", "", " The config to use"};
        std::unique_ptr<TFile> m_calibFile{nullptr};

        /// internal use
        std::vector<std::unique_ptr<IsolationWP>> m_muWPs{};
        std::vector<std::unique_ptr<IsolationWP>> m_elWPs{};
        std::vector<std::unique_ptr<IsolationWP>> m_phWPs{};
        std::vector<std::unique_ptr<IsolationWP>> m_objWPs{};

        /// AcceptInfo's
        asg::AcceptInfo m_photonAccept{"IsolationSelectionToolPhotonAcceptInfo"};
        asg::AcceptInfo m_electronAccept{"IsolationSelectionToolElectronAcceptInfo"};
        asg::AcceptInfo m_muonAccept{"IsolationSelectionToolMuonAcceptInfo"};
        asg::AcceptInfo m_objAccept{"IsolationSelectionToolObjAcceptInfo"};

        /// Iparticle interface
        std::vector<std::unique_ptr<IsolationWP>>* m_iparWPs{nullptr};
        asg::AcceptInfo* m_iparAcceptInfo{nullptr};

        // for cut interpolation
        Gaudi::Property<bool> m_doInterpM{this, "doCutInterpolationMuon", false, "flag to perform cut interpolation, muon"};
        Gaudi::Property<bool> m_doInterpE{this, "doCutInterpolationElec", true, "flag to perform cut interpolation, electron"};
        std::shared_ptr<Interp3D> m_Interp{nullptr};

        void addDependencies(const std::string& container, const IsolationWP& wp);
        SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_isoDecors{this, "IsolationDecors", {}, "List of decorations needed by the tool"};

        /// Properties to declare the data dependencies to the avalanche scheduler
        Gaudi::Property<std::string> m_inMuonContainer{this, "InMuonContainer", "" , "Name of the muon container parsed to the tool."};
        Gaudi::Property<std::string> m_inElecContainer{this, "InElectronContainer", "" , "Name of the electron container parsed to the tool."};
        Gaudi::Property<std::string> m_inPhotContainer{this, "InPhotonContainer", "", "Name of the photon container parsed to the tool."};

        // Helper function to create a piece wise polinomial function
        std::string createPieceWisePolinomialFunction(const std::vector<double>& boundaries, const std::vector<std::vector<double>>& parameters, bool isOpen = false) const;
        // The function returns a piecewise polinomial function, where each segment is defined by a polynomial of the form:
        // f(x) = p0 + p1*x + p2*x^2 + ... + pn*x^n
        // where n is the degree of the polynomial, and p0, p1, p2, ..., pn are the coefficients of the polynomial.
        // The function is defined on the intervals [x0, x1), [x1, x2), ..., [xn-1, xn).
        // boundaries: {x0, x1, x2, ...}: are the boundaries of the polinomial functions
        // parameters: {(p0_0, p1_0, p2_0, ...), (p0_1, p1_1, p2_1, ...), ...}: are the parameters of the polinomial functions
        // isOpen: if true, first and last segments are open, i.e. intervals are  (-inf, x0), [x0, x1), [x1, x2), ..., [xn-1, xn), [xn, +inf)
        // if isOpen==false, boundaries.size() == parameters.size()+1, else boundaries.size() == parameters.size()-1
    };
}  // namespace CP

#endif
