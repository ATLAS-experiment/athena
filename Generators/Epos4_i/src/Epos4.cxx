/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ----------------------------------------------------------------------
// Generators/Epos4.cxx
//
//
// AuthorList:
//   Andrii Verbytskyi
// ----------------------------------------------------------------------

#include "GaudiKernel/MsgStream.h"
#include "CLHEP/Random/RandFlat.h"
#include "AthenaKernel/RNGWrapper.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/HeavyIon.h"
#include "AtlasHepMC/SimpleVector.h"

#include "Epos4.h"
#include <iostream>
#include <filesystem>
#include "HepMC3/Writer.h"
#include "HepMC3/GenEvent.h"


// Match the Fortran COMMON block layout

struct jobfnametype {
    char fnjob[1000];  // Fortran CHARACTER*1000
    int nfnjob;        // Fortran INTEGER
} ;

// Declare the Fortran COMMON block symbol
extern struct jobfnametype jobfname_;
// C function to set values in the COMMON block
void set_job_common(const char *filename) {
    size_t len = strlen(filename);
    if (len > 1000) len = 1000;
    // Copy filename and pad with spaces
    memcpy(jobfname_.fnjob, filename, len);
    for (size_t i = len; i < 1000; ++i) {
        jobfname_.fnjob[i] = ' ';
    }
    jobfname_.nfnjob = (int)len;
}

namespace {
static std::string epos_rndm_stream = "EPOS4_INIT";
static CLHEP::HepRandomEngine* p_rndmEngine{};
}

extern std::shared_ptr<HepMC3::Writer> writer;
namespace HepMC3 {
class WriterEPOS: public Writer  {
public:
    WriterEPOS([[maybe_unused]] const std::string& filename, std::shared_ptr<GenRunInfo> run = std::shared_ptr<GenRunInfo>()) {
        set_run_info(run);
    }
    WriterEPOS([[maybe_unused]] std::ostream& stream, std::shared_ptr<GenRunInfo> run = std::shared_ptr<GenRunInfo>()) {
        set_run_info(run);
    }
    WriterEPOS([[maybe_unused]] std::shared_ptr<std::ostream> s_stream, std::shared_ptr<GenRunInfo> run = std::shared_ptr<GenRunInfo>()) {
        set_run_info(run);
    }
    ~WriterEPOS() {};
    void write_event(const GenEvent& evt) override {
        m_event = evt;
    };
    const GenEvent& current_event() const {
        return m_event;
    }
    bool failed() override {
        return false;
    };
    void close() override {}
private:
    GenEvent m_event;
};
}


void checkTime();
void defineStorageSettings();
void initializeElectronProtonPart();
void eposEnd();
void eposStart();
void finalizeSimulation();
void generateEposEvent(int&);
void listParticles(int&);
void initializeEpos();
void initializeEventCounters();
void initializeHeavyQuarkPart();
int numberOfEnergyValues();
int numberOfEvents();
void readInputFile();
void rewindInputFile();
int setEnergyIndex(int&);
void showMemoryAtStart();
void showMemoryAtEnd();
void writeStatistics();

// ----------------------------------------------------------------------
Epos4::Epos4( const std::string &name, ISvcLocator *pSvcLocator ): GenModule( name, pSvcLocator ) {

    epos_rndm_stream = "EPOS4_INIT";
    declareProperty( "BeamMomentum",    m_beamMomentum    = -6500.0 );      // GeV
    declareProperty( "TargetMomentum",  m_targetMomentum  = 6500.0 );
    m_events = 0; // current event number (counted by interface)
}


namespace fs = std::filesystem;
std::string  Epos4::create_file(const std::string&  filein) {

    fs::path source = filein;
    fs::path destination = fs::current_path() / fs::path(std::string("z-")+source.filename().string());

    std::ifstream src(source);             // Open in text mode
    std::ofstream dst(destination);        // Open in text mode

    if (!src) {
        std::cerr << "Error: Cannot open source file.\n";
        return "";
    }

    if (!dst) {
        std::cerr << "Error: Cannot create destination file.\n";
        return "";
    }

    std::string line;
    while (std::getline(src, line)) {
        dst << line << '\n';
    }
    dst<<"set ihepmc 1"<< '\n';

    std::string file = source.filename().string().size()>6 ? source.filename().string().substr(0,source.filename().string().size()-6) : "";

    std::string num="0";
    std::string one = file;
    if (num != "0") {
        one = file + "-" + num;
    }

    std::string clinput = "z-" + one + ".clinput";
    std::ofstream ofile(clinput);

    std::string CHK     = std::getenv("CHK")     ? std::getenv("CHK")     : (std::cerr << "Warning: CHK not set\n", "");
    std::string seedi   = std::to_string(m_seeds.at(0));// Should be something like "222222222";
    std::string seedj   = std::to_string(m_seeds.at(1));// Should be something like "111111111";
    std::string rootcproot = "nono";
    std::string system  = "i";
    std::string ext1    = "-";
    std::string ext3    = "-";
    std::string ext4    = "-";
    std::string gefac   = "1";
    std::string EPO     = std::getenv("EPO")     ? std::getenv("EPO")     : (std::cerr << "Warning: EPO not set\n", "");
    std::string SRCEXT  = std::getenv("SRCEXT")  ? std::getenv("SRCEXT")  : (std::cerr << "Warning: SRCEXT not set\n", "");
    std::string HTO     = std::getenv("HTO")     ? std::getenv("HTO")     : (std::cerr << "Warning: HTO not set\n", "");
    std::string SRC     = std::getenv("SRC")     ? std::getenv("SRC")     : (std::cerr << "Warning: SRC not set\n", "");
    std::string CONF    = std::getenv("CONF")    ? std::getenv("CONF")    : (std::cerr << "Warning: CONF not set\n", "");
    std::string OPT    = std::getenv("OPT")    ? std::getenv("OPT")    : (std::cerr << "Warning: OPT not set\n", "");
    std::string OPX="";
    if (std::string(OPT) == "./") {
        OPX = std::filesystem::current_path().string() + "/";
    } else {
        OPX = OPT;
    }

    ofile << "!fname mtr " << CHK << "z-" << one << ".mtr\n";
    ofile << "set seedj " << seedj << "  set seedi " << seedi << "\n";
    ofile << "echo off\n";
    ofile << "rootcproot " << rootcproot << "\n";
    ofile << "system " << system << "\n";
    ofile << "ext1 " << ext1 << "\n";
    ofile << "ext3 " << ext3 << "\n";
    ofile << "ext4 " << ext4 << "\n";
    ofile << "set gefac " << gefac << "\n";
    ofile << "!!!beginoptns spherio !No longer used.\n";
    ofile << "!!!...                !See version < 3118  \n";
    ofile << "!!!endoptns spherio   !if needed again         \n";
    ofile << "fname pathep " << EPO << "\n";
    ofile << "!-------HQ--------\n";
    ofile << "fname user1  " << SRCEXT << "/HQt/\n";
    ofile << "fname user2  " << HTO << "z-" << one << ".hq\n";
    ofile << "fname user3  " << SRCEXT << "/URt/\n";
    ofile << "!-------HQ END--------\n";
    ofile << "fname pathpdf " << SRC << "/TPt/\n";
    ofile << "fname histo  " << HTO << "z-" << one << ".histo\n";
    ofile << "fname check  " << CHK << "z-" << one << ".check\n";
    ofile << "fname copy   " << CHK << "z-" << one << ".copy\n";
    ofile << "fname log    " << CHK << "z-" << one << ".log\n";
    ofile << "fname data   " << CHK << "z-" << one << ".data\n";
    ofile << "fname initl  " << SRC << "/KWt/aa.i\n";
    ofile << "fname inidi  " << SRC << "/TPt/di.i\n";
    ofile << "fname inidr  " << SRC << "/KWt/dr.i\n";
    ofile << "fname iniev  " << SRC << "/KWt/ev.i\n";
    ofile << "fname inirj  " << SRC << "/KWt/rj.i\n";
    ofile << "fname inics  " << SRC << "/TPt/cs.i\n";
    ofile << "fname inigrv " << SRC << "/grv.i\n";
    ofile << "fname partab " << SRCEXT << "/YK/ptl6.data\n";
    ofile << "fname dectab " << SRCEXT << "/YK/dky6.data\n";
    ofile << "fname hpf    " << SRCEXT << "/UR/tables.dat \n";
    ofile << "!fqgsjet dat   " << EPO << SRC << "qgsjet/qgsjet.dat !No longer used.\n";
    ofile << "!fqgsjet ncs   " << EPO << SRC << "qgsjet/qgsjet.ncs !No longer used.\n";
    ofile << "!fqgsjetII dat   " << EPO << SRC << "qgsjetII/qgsdat-II-03 !No longer used.\n";
    ofile << "!fqgsjetII ncs   " << EPO << SRC << "qgsjetII/sectnu-II-03 !No longer used.\n";
    ofile << "nodecay 1220\n";
    ofile << "nodecay -1220\n";
    ofile << "nodecay 120\n";
    ofile << "nodecay -120\n";
    ofile << "nodecay 130\n";
    ofile << "nodecay -130\n";
    ofile << "nodecay -20\n";
    ofile << "nodecay 14\n";
    ofile << "nodecay -14\n";
    ofile << "nodecay 16\n";
    ofile << "nodecay -16\n";
    ofile << "echo on\n";
    ofile << "input " << CONF << "/parbf.i\n";
    ofile << "input " << OPX << "z-" << one << ".optns\n";
    ofile << "input " << SRC << "/KWn/paraf.i\n";
    ofile << "input " << CONF << "/partx.i\n";
    ofile << "runprogram\n";
    ofile << "stopprogram\n";

    ofile.close();
    return clinput;
}

// ----------------------------------------------------------------------
StatusCode Epos4::genInitialize()
{
    p_rndmEngine = getRandomEngineDuringInitialize(epos_rndm_stream, m_randomSeed, m_dsid); // NOT THREAD-SAFE
    epos_rndm_stream = "EPOS4";
    m_events = 0;

    std::string in("someinputfoo.txt");
    writer = std::make_shared<HepMC3::WriterEPOS>("foo");
    auto x = this->create_file(in);
    set_job_common(x.c_str());

    showMemoryAtStart();
    checkTime();
    eposStart();
    readInputFile();
    initializeHeavyQuarkPart();
    initializeElectronProtonPart();

    int currentnumberOfEnergyValues = numberOfEnergyValues();
    /// This value should be 1!
    for (int k=1; k<=currentnumberOfEnergyValues; k++) {
        setEnergyIndex(k);
        initializeEpos();
        initializeEventCounters();
        defineStorageSettings();
    }

    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos4::callGenerator()
{
    //Re-seed the random number stream
    long seeds[7];
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATHRNG::calculateSeedsMC21(seeds, epos_rndm_stream, ctx.eventID().event_number(), m_dsid, m_randomSeed);
    p_rndmEngine->setSeeds(seeds, 0); // NOT THREAD-SAFE

    // save the random number seeds in the event
    const long *s = p_rndmEngine->getSeeds();
    m_seeds.assign({s[0], s[1]});
    ++m_events;
    generateEposEvent(m_events);
    listParticles(m_events);
//    auto e = std::dynamic_pointer_cast<HepMC3::WriterEPOS>(writer)->current_event();
    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos4::genFinalize()
{
    ATH_MSG_INFO("EPOS4 finalizing.");
    ATH_MSG_INFO("MetaData: generator = Epos4 .");
    writeStatistics();
    finalizeSimulation();
    rewindInputFile();
    readInputFile();
    eposEnd();
    showMemoryAtEnd();
    return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
StatusCode Epos4::fillEvt( HepMC::GenEvent* evt )
{
    auto e = std::dynamic_pointer_cast<HepMC3::WriterEPOS>(writer)->current_event();
    /// Here we should put e into evt
    *evt = e;
    return StatusCode::SUCCESS;
}

