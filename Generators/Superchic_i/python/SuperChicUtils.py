#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


import subprocess, os, shlex, re, shutil, tarfile, json, difflib
from packaging.version import Version

from AthenaCommon import Logging

## Get handle to Athena logging
logger = Logging.logging.getLogger("SuperChicUtils")

GRIDPACK_SIG = "gridpack_signature.json"
GRIDPACK_DAT = "gridpack_input.DAT"
GRIDPACK_VER = "gridpack_versions.json"

class SuperChicConfig:

    def __init__(self, runArgs=None):
        self.superchicpath = os.environ['SUPERCHIC_SOURCE_PATH']
        
        #SuperChic specific variables for input.DAT, see writeInputDAT function for more elaboration
        self.rts = 13000. #collision energy (GeV)
        if runArgs is not None:
            if hasattr(runArgs,"ecmEnergy"):
                self.rts = runArgs.ecmEnergy

        self.isurv = 4 #Model of soft survival (from 1 -> 4, corresponding to arXiv:1306.2149)
        self.intag = 'in' #tag for input files
        self.PDFname = 'MMHT2014lo68cl'
        self.PDFmember = 0
        self.proc = 5 #Please consult Superchic Manual https://superchic.hepforge.org/
        self.beam = 'prot'
        self.outtg = 'out'
        self.sfaci = True
        self.diff = 'el' #interaction: elastic ('el'), single ('sd','sda','sdb') and double ('dd') dissociation.
        self.an = 208
        self.az = 82
        self.rz = 6.68
        self.dz = 0.447
        self.rn = 6.7
        self.dn = 0.55
        self.ionqcd = 'coh'
        self.ionbreakup = True
        self.fAA = '00'
        self.fracsigX = 1
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.7")):
            self.veto = False
            self.veto_ecent = 53.9655
            self.veto_eden = 263.313
            self.rshad = 0.4
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.5.1")):
            self.wrho = False
            self.yrho = 2.5
            self.accrho = 1.0
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.6.1")):
            self.ion_incoh = False
            self.ion_incoh_type = 'inel'
        self.ncall = 10000
        self.itmx = 10
        self.prec = 0.5
        self.ncall1 = 10000
        self.inccall = 10000
        self.itend = 1000

        self.iseed = 34
        if runArgs is not None:
            if hasattr(runArgs,"randomSeed"):
                self.iseed  = runArgs.randomSeed

        self.genunw = True

        self.nev = "500"
        if runArgs is not None:
            if hasattr(runArgs,"maxEvents"):
                self.nev = runArgs.maxEvents

        self.erec = 'lhe' #output file type
        self.readwt = False
        self.wtmax = 0
        self.ymin = -2.4 # Minimum object rapidity Y_X
        self.ymax = 2.4 # Maximum object rapidity Y_X
        self.mmin = 6. # Minimum object mass M_X
        self.mmax = 500. # Maximum object mass M_X
        self.gencuts = True # Generate cuts below
        self.scorr = True
        self.fwidth = True
        self.ptxmax = 100.
        self.ptamin = 3.0 # Minimum pT of outgoing object a (gamma)
        self.ptbmin = 3.0 # Minimum pT of outgoing object b (gamma)
        self.etaamin = -2.4 # Minimum eta of outgoing object a
        self.etaamax = 2.4 # Maximum eta of outgoing object a
        self.etabmin = -2.4 # Minimum eta of outgoing object b
        self.etabmax = 2.4 # Maximum eta of outgoing object b
        self.acoabmax = 100.
        self.ptcmin = 0
        self.etacmin = -2.5
        self.etacmax = 2.5
        self.ptdmin = 0
        self.etadmin = -2.5
        self.etadmax = 2.5
        self.ptemin = 0
        self.etaemin = -2.5
        self.etaemax = 2.5
        self.ptfmin = 0
        self.etafmin = -2.5
        self.etafmax = 2.5
        self.rjet = 0.6
        self.jalg = 'antikt'
        self.m2b = 0.133
        self.pdgid1 = 211
        self.pdgid2 = -211
        self.malp = 1000
        self.gax = 0.001
        self.alpt = 'ps'
        self.mpol = 500
        self.mmon = 933
        self.gamm = 10
        self.mcharg = 100
        self.mneut = 80
        self.wlp = 'el'
        self.wlm = 'el'
        self.tau = 0.04
        self.mxs = 100
        self.atau = 0.0
        self.dtau = 0.0
        self.calc_tau_coeff = False
        self.tau_mom = 'atau'
        self.tau_coeff = 1

        # Paterns to intercept the cross-section and write to MetaData
        self._NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"
        self._PATTERN = re.compile(
            rf"Cross\s+section\s*=\s*({self._NUM})\s*\+/-\s*({self._NUM})\s*(\w+)",
            re.IGNORECASE,
        )

    def toFortran(self):

        def fortDouble(x):
            return str(x)+"d0"
        def fortInt(x):
            return str(x)
        def fortBool(x):
            return '.true.' if x else '.false.'
        def fortStr(x):
            return "'{}'".format(x)
        
        conf = ""
        conf+="***********************************************************************************\n"
        conf+="****** Initialize again if FIRST FIVE PARAMETERS ARE CHANGED (and beam = 'prot'):******\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.rts) + "                     ! [rts] : CMS collision energy (GeV) \n"
        conf+=fortInt(self.isurv) + "                         ! [isurv] : Model of soft survival (from 1 -> 4)\n"
        conf+=fortStr(self.intag) + "                     ! [intag] for input files  \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortStr(self.PDFname) + "                ! [PDFname] : PDF set \n"
        conf+=fortInt(self.PDFmember)  + "                       ! [PDFmember] : PDF member \n"
        conf+="***********************************************************************************\n"
        conf+=fortInt(self.proc) + "                             ! [proc] : Process number \n"
        conf+=fortStr(self.beam) + "                           ! [beam] : Beam type ('prot', 'ion', 'ionp' or 'el') \n"
        conf+=fortStr(self.outtg) + "                           ! [outtg] : for output file \n"
        conf+=fortBool(self.sfaci) + "                          ! [sfaci] : Include soft survival effects \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortStr(self.diff) + "                          ! [diff] : dissociation flag \n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.an) + "                                  ! [an] : Ion mass number \n"
        conf+=fortDouble(self.az) + "                                   ! [az] : Ion atomic number \n"
        conf+=fortDouble(self.rz) + "                                 ! [rz] : Ion proton density - radius \n"
        conf+=fortDouble(self.dz) + "                                ! [dz] : Ion proton density - skin thickness \n"
        conf+=fortDouble(self.rn) + "                                  ! [rn] : Ion neutron density - radius \n"
        conf+=fortDouble(self.dn) + "                                  ! [dn] : Ion neutron density - skin thickness \n"
        conf+=fortStr(self.ionqcd) + "                               ! [ionqcd] : Coherent ('coh') or incoherent ('incoh') for QCD-induced processes \n"
        conf+=fortBool(self.ionbreakup) + "                            ! [ionbreakup] \n"
        conf+=fortStr(self.fAA) + "                                    ! [fAA] \n"
        conf+=fortDouble(self.fracsigX) + "                            ! [fracsigX] : multiply sig_(gamA) by this factor (1d0 - default) \n"
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.7")):
            conf+=fortBool(self.veto) + "                            ! [veto] : impose exclusivity veto in ion-ion case due to EMD \n"
            conf+=fortDouble(self.veto_ecent) + "                    ! [veto_ecent] \n"
            conf+=fortDouble(self.veto_eden) + "                     ! [veto_eden] \n"
            conf+=fortDouble(self.rshad) + "                         ! [rshad] \n"
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.5.1")):
            conf+=fortBool(self.wrho) + "                            ! [wrho] : generate concident rho production in AA collisions \n"
            conf+=fortDouble(self.yrho) + "                          ! [yrho] : maximum rho |y| \n"
            conf+=fortDouble(self.accrho) + "                        ! [accrho] : additional correction factor to account for rho->pipi acceptance, non-resonant dipions and different beams \n"
        if (Version(os.environ['SUPERCHICVER']) >= Version("5.6.1")):
            conf+=fortBool(self.ion_incoh) + "                            ! [ion_incoh] \n"
            conf+=fortStr(self.ion_incoh_type) + "                        ! [ion_incoh_type]   \n"
        conf+="***********************************************************************************\n"
        conf+="*************Integration parameters************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortInt(self.ncall) + "                              ! [ncall] : Number of calls for preconditioning \n"
        conf+=fortInt(self.itmx) + "                                  ! [itmx] : Number of iterations for preconditioning \n"
        conf+=fortDouble(self.prec) + "                                 ! [prec] :  Relative accuracy (in %) in main run \n"
        conf+=fortInt(self.ncall1) + "                               ! [ncall1] : Number of calls in first iteration \n"
        conf+=fortInt(self.inccall) + "                              ! [inccall] : Number of increase calls per iteration \n"
        conf+=fortInt(self.itend) + "                                ! [itend] : Maximum number of iterations \n"
        conf+=fortInt(self.iseed) + "                                 ! [iseed] : Random number seed (integer > 0) \n"
        conf+="***********************************************************************************\n"
        conf+="********************Unweighted events**********************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortBool(self.genunw) + "                              ! [genunw] : Generate unweighted events \n"
        conf+=fortInt(int(self.nev*1.01)) + "                                 ! [nev] : Number of events (preferably controlled by maxEvents option in Gen_tf command) \n"
        conf+=fortStr(self.erec) + "                               ! [erec] : Event record format ('hepmc','lhe','hepevt') \n"
        conf+=fortBool(self.readwt) + "                             ! [readwt] : Set to true to read in pre-calculated maxium weight below \n"
        conf+=fortDouble(self.wtmax) +  "                               ! [wtmax] : Maximum weight \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+="*******************   general cuts ************************************************\n"
        conf+=fortDouble(self.ymin) + "                               ! [ymin] : Minimum object rapidity Y_X \n"
        conf+=fortDouble(self.ymax) + "                               ! [ymax] : Maximum object rapidity Y_X \n"
        conf+=fortDouble(self.mmin) + "                               ! [mmin] : Minimum object mass M_X (redundant for resonance production) \n"
        conf+=fortDouble(self.mmax) + "                               ! [mmax] : Maximum object mass M_X (redundant for resonance production) \n"
        conf+=fortBool(self.gencuts) + "                             ! [gencuts] : Generate cuts below \n"
        conf+=fortBool(self.scorr) + "                             ! [scorr] : Include spin correlations (for chi_c/b decays) \n"
        conf+=fortBool(self.fwidth) + "                             ! [fwidth] : Include finite width (for chi_c decays)  \n"
        conf+="***********************************************************************************\n"
        conf+="************ See manual for momentum assignments***********************************\n"
        conf+="***********************************************************************************\n"
        conf+="******************* Proton Cuts ***************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptxmax) + "                               ! [ptxmax] : max pT of the system  \n"
        conf+="***********************************************************************************\n"
        conf+="**********2 body final states : p(a) + p(b) ***************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptamin) + "                               ! [ptamin] \n"
        conf+=fortDouble(self.ptbmin) + "                               ! [ptbmin] \n"
        conf+=fortDouble(self.etaamin) + "                              ! [etaamin] \n"
        conf+=fortDouble(self.etaamax) + "                              ! [etaamax] \n"
        conf+=fortDouble(self.etabmin) + "                              ! [etabmin] \n"
        conf+=fortDouble(self.etabmax) + "                              ! [etabmax] \n"
        conf+=fortDouble(self.acoabmax) + "                             ! [acoabmax] \n"
        conf+="***********************************************************************************\n"
        conf+="****** 3 body final states : p(a) + p(b) + p(c) ***********************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptamin) + "                                ! [ptamin] \n"
        conf+=fortDouble(self.ptbmin)  + "                               ! [ptbmin] \n"
        conf+=fortDouble(self.ptcmin) + "                                ! [ptcmin] \n"
        conf+=fortDouble(self.etaamin) + "                               ! [etaamin] \n"
        conf+=fortDouble(self.etaamax) + "                               ! [etaamax] \n"
        conf+=fortDouble(self.etabmin)  + "                              ! [etabmin] \n"
        conf+=fortDouble(self.etabmax) + "                               ! [etabmax]	 \n"
        conf+=fortDouble(self.etacmin) + "                               ! [etacmin] \n"
        conf+=fortDouble(self.etacmax) + "                               ! [etacmax]	 \n"
        conf+="***********************************************************************************\n"
        conf+="****** 4 body final states : p(a) + p(b) + p(c) + p(d) ****************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptamin) + "                                ! [ptamin] \n"
        conf+=fortDouble(self.ptbmin) + "                                ! [ptbmin] \n"
        conf+=fortDouble(self.ptcmin) + "                                ! [ptcmin] \n"
        conf+=fortDouble(self.ptdmin) + "                                ! [ptdmin] \n"
        conf+=fortDouble(self.etaamin) + "                               ! [etaamin] \n"
        conf+=fortDouble(self.etaamax) + "                               ! [etaamax] \n"
        conf+=fortDouble(self.etabmin) + "                               ! [etabmin] \n"
        conf+=fortDouble(self.etabmax) + "                               ! [etabmax] \n"
        conf+=fortDouble(self.etacmin) + "                               ! [etacmin] \n"
        conf+=fortDouble(self.etacmax) + "                               ! [etacmax] \n"
        conf+=fortDouble(self.etadmin) + "                               ! [etadmin] \n"
        conf+=fortDouble(self.etadmax) + "                               ! [etadmax] \n"
        conf+="***********************************************************************************\n"
        conf+="****** 6 body final states : p(a) + p(b) + p(c) + p(d) + p(e) + p(f) **************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.ptamin)  + "                                ! [ptamin] \n"
        conf+=fortDouble(self.ptbmin)  + "                                ! [ptbmin] \n"
        conf+=fortDouble(self.ptcmin)  + "                                ! [ptcmin]  \n"
        conf+=fortDouble(self.ptdmin)  + "                                ! [ptdmin] \n"
        conf+=fortDouble(self.ptemin)  + "                                ! [ptemin] \n"
        conf+=fortDouble(self.ptfmin)  + "                                ! [ptfmin]  \n"
        conf+=fortDouble(self.etaamin)  + "                               ! [etaamin] \n"
        conf+=fortDouble(self.etaamax)  + "                               ! [etaamax] \n"
        conf+=fortDouble(self.etabmin) + "                               ! [etabmin] \n"
        conf+=fortDouble(self.etabmax) + "                               ! [etabmax]	\n"
        conf+=fortDouble(self.etacmin) + "                               ! [etacmin] \n"
        conf+=fortDouble(self.etacmax) + "                               ! [etacmax] \n"
        conf+=fortDouble(self.etadmin) + "                               ! [etadmin]  \n"
        conf+=fortDouble(self.etadmax) + "                               ! [etadmax]  \n"
        conf+=fortDouble(self.etaemin) + "                               ! [etaemin]  \n"
        conf+=fortDouble(self.etaemax) + "                               ! [etaemax] \n"
        conf+=fortDouble(self.etaemin) + "                               ! [etafmin] \n"
        conf+=fortDouble(self.etafmax) + "                               ! [etafmax]	\n"
        conf+="***********************************************************************************\n"
        conf+="******************* jet algorithm parameters **************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.rjet) + "                                  ! [rjet] : Jet Radius \n"
        conf+=fortStr(self.jalg) + "                                  ! [jalg] : Jet algorithm ('antikt','kt') \n"
        conf+="***********************************************************************************\n"
        conf+="***** chi_c/b two-body decays *****************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.m2b) + "                                   ! [m2b] : mass of decay particles \n"
        conf+=fortInt(self.pdgid1) + "                                ! [pdgid1] : PDG number particle 1 \n"
        conf+=fortInt(self.pdgid2) + "                                ! [pdgid2] : PDG number particle 2 \n"
        conf+="***********************************************************************************\n"
        conf+="*******  ALP Parameters ***********************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.malp) + "                                  ! [malp] : ALP mass (GeV) \n"
        conf+=fortDouble(self.gax) + "                                   ! [gax] : ALP coupling (GeV^-1) \n"
        conf+=fortStr(self.alpt) + "                                  ! [alpt] : AlP type (scalar - 'sc', pseudoscalar - 'ps') \n"
        conf+="***********************************************************************************\n"
        conf+="**** Monopole Parameters **********************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.mpol) + "                                  ! [mpol] : Monopole mass \n"
        conf+=fortDouble(self.mmon) + "                                  ! [mmon] : Monopolium mass \n"
        conf+=fortDouble(self.gamm) + "                                  ! [gamm] : Monopolium width \n"
        conf+="***********************************************************************************\n"
        conf+="****   SUSY Parameters ************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortDouble(self.mcharg) + "                                ! [mcharg] : Chargino/Slepton mass \n"
        conf+=fortDouble(self.mneut)  + "                                ! [mneut]  : Neutralino mass \n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+="***********************************************************************************\n"
        conf+=fortStr(self.wlp) + "                                  ! [wlp] : leptonic decay (either 'mu' or 'el') for Wplus \n"
        conf+=fortStr(self.wlm) + "                                  ! [wlm] : leptonic decay (either 'mu' or 'el') for Wminus \n"

        conf+="****************************************************************************************\n"
        conf+="****** V+X simplified model \n"   
        conf+="****************************************************************************************\n" 
        conf+=fortDouble(self.tau) + "                                   ! [tau] : mass distribution decay constant (GeV^-1) \n"
        conf+=fortDouble(self.mxs) + "                                 ! [mxs] : mass of MX \n"

        conf+="****************************************************************************************\n" 
        conf+="******  tau anomalous moments\n"
        conf+="****************************************************************************************\n" 
        conf+=fortDouble(self.atau) + "                                 ! [atau] : magnetic dipole moment\n"
        conf+=fortDouble(self.dtau) + "                                 ! [dtau] : electric dipole moment [e cm]\n"
        conf+="******  flags for calculating individual coeffs - SEE MANUAL for explanation\n" 
        conf+=fortBool(self.calc_tau_coeff) + "                         ! [calc_tau_coeff] : if true calculate  O(a_tau^n) or O(d_tau^n) coefficients\n"
        conf+=fortStr(self.atau) + "                                  ! [tau_mom] : 'atau','dtau' - coeffecients for magnetic/electric dipole moments\n"
        conf+=fortInt(self.tau_coeff) + "                                 ! [tau_coeff] : order 'n' in coefficient (0-4)\n"

        return conf 

    def outputLHEFile(self):
        return "evrecs/evrec"+self.outtg+".dat"

    def from_dict(self, params: dict):
        for key, value in params.items():
            if hasattr(self, key):
                setattr(self, key, value)
            else:
                raise AttributeError(f"'{type(self).__name__}' has no attribute '{key}'")
        return self

    def writeInputDAT(self):
        
        with open("input.DAT", "w") as outF:
            outF.write(self.toFortran())
        
        return

    def intercept_cross_section(self, text):
        """
        Look for a cross section pattern in SuperChic output
        and write it to MetaData if succesfully intercepted.
        """

        match = self._PATTERN.search(text)
        if match:
            xsec, unc, unit = match.groups()
            print(f"MetaData: cross-section ({unit}) = {float(xsec)}")
            print(f"MetaData: generator = SuperChic {os.environ['SUPERCHICVER']}")

    def run_command(self, command, stdin = None):
        """
        Run a command and print output continuously
        """
        process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stdin=stdin)
        while True:
            output = process.stdout.readline().decode("utf-8")
            if output == '' and process.poll() is not None:
                break
            if output:
                # remove ANSI escape formatting characters
                reaesc = re.compile(r'(\x9B|\x1B\[)[0-?]*[ -\/]*[@-~]')
                text = reaesc.sub('', output.strip())
                logger.info(text)
                self.intercept_cross_section(text)

        rc = process.poll()
        return rc

    def _ensureCards(self):
        """Copy param_card.dat and ident_card.dat from the installation if absent."""
        for card in ("param_card.dat", "ident_card.dat"):
            if not os.path.exists(card):
                src = os.path.join(self.superchicpath, "share/doc/SuperChic/Cards", card)
                if os.path.exists(src):
                    shutil.copyfile(src, card)
                else:
                    raise Exception("Unexpected error in superchic execution: probably absent " + card)


    # ------------------------------------------------------------------
    # Gridpack support
    #
    # A gridpack contains:
    #   inputs/                  grids written by bin/init
    #   gridpack_input.DAT       input.DAT of the job that created it
    #   gridpack_signature.json  parameters that determine the validity of the grids
    #   gridpack_versions.json   SuperChic and AthGeneration versions used to make it
    #
    # When a gridpack is used:
    #   - SuperChic version differs        -> error
    #   - AthGeneration version differs    -> warning
    #   - signature differs                -> error
    #   - rest of input.DAT (ignoring iseed and nev) differs
    #                                      -> warning, or error if strict
    # ------------------------------------------------------------------

    def initSignature(self):
        """
        Parameters that determine whether the grids in inputs/ are valid for a job.
        A gridpack is only accepted if all of these match (hard check).

        Conservative choice: besides the parameters the input.DAT header lists as
        requiring a new init (rts, isurv, intag, PDFname, PDFmember; beam='prot'),
        also proc, diff and sfaci are included, i.e. one gridpack per process.
        Remove entries only after validating that the grids do not depend on them.
        """
        return {
            "rts": float(self.rts),
            "isurv": int(self.isurv),
            "intag": self.intag,
            "PDFname": self.PDFname,
            "PDFmember": int(self.PDFmember),
            "beam": self.beam,
            "proc": int(self.proc),
            "diff": self.diff,
            "sfaci": bool(self.sfaci),
        }

    @staticmethod
    def athGenerationVersion():
        """AthGeneration release of the running job, e.g. 'AthGeneration-23.6.40'."""
        proj = os.environ.get("AtlasProject", "unknown")
        ver = os.environ.get("AtlasVersion", "unknown")
        return "%s-%s" % (proj, ver)

    def currentVersions(self):
        return {
            "superchic": os.environ["SUPERCHICVER"],
            "athgeneration": self.athGenerationVersion(),
        }

    def gridpackFileName(self):
        """Standard DSID gridpack name: mc_13TeV.<physics_short>.GRID.tar.gz"""
        from MCJobOptionUtils.JOsupport import get_physics_short
        e = self.rts / 1000.
        estr = str(int(e)) if e == int(e) else ("%g" % e).replace('.', 'p')
        return "mc_%sTeV.%s.GRID.tar.gz" % (estr, get_physics_short())

    def findGridpack(self):
        """Look for the DSID gridpack in the cwd and on the JO search path."""
        try:
            name = self.gridpackFileName()
        except Exception as e:
            logger.warning("Could not determine gridpack name (%s); running init in this job", e)
            return None
        if os.path.isfile(name):
            return name
        for d in re.split(r'[:,]', os.environ.get("JOBOPTSEARCHPATH", "")):
            p = os.path.join(d, name)
            if d and os.path.isfile(p):
                return p
        return None

    @staticmethod
    def _normaliseDAT(text):
        """input.DAT lines with the per-job values (iseed, nev) removed."""
        return [' '.join(l.split()) for l in text.splitlines()
                if '! [iseed]' not in l and '! [nev]' not in l]

    def makeGridpack(self, name=None):
        """Pack inputs/ together with input.DAT and the metadata (run after init)."""
        if not os.path.isdir("inputs") or not os.listdir("inputs"):
            raise Exception("inputs/ is empty after init; refusing to make an empty gridpack")
        if name is None:
            name = self.gridpackFileName()
        with open(GRIDPACK_SIG, "w") as f:
            json.dump(self.initSignature(), f, sort_keys=True, indent=1)
        with open(GRIDPACK_DAT, "w") as f:
            f.write(self.toFortran())
        with open(GRIDPACK_VER, "w") as f:
            json.dump(self.currentVersions(), f, sort_keys=True, indent=1)
        with tarfile.open(name, "w:gz") as tar:
            tar.add("inputs")
            tar.add(GRIDPACK_SIG)
            tar.add(GRIDPACK_DAT)
            tar.add(GRIDPACK_VER)
        logger.info("Created SuperChic gridpack %s", name)
        return name

    def useGridpack(self, path, strict=False):
        """Unpack a gridpack and check it is compatible with this job."""
        logger.info("Unpacking SuperChic gridpack %s", path)
        with tarfile.open(path) as tar:
            if hasattr(tarfile, "data_filter"):
                tar.extractall(".", filter="data")
            else:
                tar.extractall(".")

        # version checks
        if not os.path.isfile(GRIDPACK_VER):
            raise Exception("Gridpack %s has no %s: cannot verify SuperChic version" % (path, GRIDPACK_VER))
        with open(GRIDPACK_VER) as f:
            storedVer = json.load(f)
        nowVer = self.currentVersions()
        if storedVer.get("superchic") != nowVer["superchic"]:
            raise Exception("SuperChic version mismatch: gridpack made with %s, this job uses %s"
                            % (storedVer.get("superchic"), nowVer["superchic"]))
        if storedVer.get("athgeneration") != nowVer["athgeneration"]:
            logger.warning("AthGeneration version differs: gridpack made with %s, this job uses %s",
                           storedVer.get("athgeneration"), nowVer["athgeneration"])

        # hard check: parameters that determine the grids
        with open(GRIDPACK_SIG) as f:
            stored = json.load(f)
        current = self.initSignature()
        if stored != current:
            diff = {k: (stored.get(k), current.get(k))
                    for k in set(stored) | set(current)
                    if stored.get(k) != current.get(k)}
            raise Exception("Gridpack incompatible with this job (gridpack, job): %s" % diff)

        if not os.path.isdir("inputs") or not os.listdir("inputs"):
            raise Exception("Gridpack %s did not provide any files in inputs/" % path)

        # soft check: everything else in input.DAT except iseed and nev
        with open(GRIDPACK_DAT) as f:
            old = self._normaliseDAT(f.read())
        new = self._normaliseDAT(self.toFortran())
        if old != new:
            delta = [l for l in difflib.unified_diff(old, new, "gridpack", "job", lineterm="", n=0)
                     if l[:1] in "+-" and l[:3] not in ("+++", "---")]
            msg = "input.DAT differs from the one used to make the gridpack:\n" + "\n".join(delta)
            if strict:
                raise Exception(msg)
            logger.warning(msg)

    def prepareGrids(self, gridpack="auto", makeGridpackFile=False, strict=True):
        """
        Provide the grids in inputs/, either from a gridpack or by running init.

        gridpack="auto"  : look for the DSID gridpack (mc_<E>TeV.<physics_short>.GRID.tar.gz);
                           if none is found, run init in this job (previous behaviour)
        gridpack=<path>  : use this gridpack
        gridpack=False   : ignore any gridpack and run init
        makeGridpackFile : run init (ignoring any gridpack) and pack inputs/ into a gridpack
        strict           : treat differences in input.DAT (other than iseed, nev) as errors
        """
        if makeGridpackFile:
            gridpack = None
        elif gridpack == "auto":
            gridpack = self.findGridpack()
            if gridpack is None:
                logger.info("No gridpack found, running init in this job")
        elif gridpack is False:
            gridpack = None

        if gridpack:
            self.useGridpack(gridpack, strict=strict)
        else:
            self.superChicInitialize()
            if makeGridpackFile:
                self.makeGridpack()


    def superChicInitialize(self, stdin=None):

        logger.info("Starting SuperChic Initialization")

        os.makedirs('inputs', exist_ok=True)
        os.makedirs('evrecs', exist_ok=True)
        os.makedirs('outputs', exist_ok=True)
        self._ensureCards()

        try:
            inputDAT = open('input.DAT')

        except IOError:
            raise Exception("problem with file IO; potentially input.DAT not created correctly")
        else:

            try:
                rc = self.run_command(self.superchicpath+"/bin/init", inputDAT) 

            except OSError:
                raise Exception("init executable or file not found")

            except Exception:
                raise Exception("Non-OSError or IOError in init execution block")

        if rc:
            raise Exception('Unexpected error in superchic init execution')
            
        return

    def superChicExecute(self):

        logger.info("Starting SuperChic Itself")
        os.makedirs('inputs', exist_ok=True)
        os.makedirs('evrecs', exist_ok=True)
        os.makedirs('outputs', exist_ok=True)
        self._ensureCards()

        try:
            inputDAT = open('input.DAT')
        except IOError:
            raise  Exception ("problem with IO; potentially input.DAT not created correctly")
        else: 

            try: 
                rc = self.run_command(self.superchicpath+'/bin/superchic', stdin=inputDAT)

            except OSError:
                raise Exception("Superchic executable or file not found")

            except Exception:
                raise Exception("Non-OSError or IOError in Superchic execution block")

        if rc:
            raise Exception('Unexpected error in superchic execution')

        return

def SuperChicRun(Init, genSeq, gridpack="auto", makeGridpackFile=False, strictGridpack=True):

    # dump the job configuration for fortran code
    print(Init.toFortran())

    # attach config to genSequence for later usin in showering
    genSeq.SuperChicConfig = Init
    
    Init.writeInputDAT()
    Init.prepareGrids(gridpack=gridpack, makeGridpackFile=makeGridpackFile, strict=strictGridpack)
    Init.superChicExecute()
    
    return
