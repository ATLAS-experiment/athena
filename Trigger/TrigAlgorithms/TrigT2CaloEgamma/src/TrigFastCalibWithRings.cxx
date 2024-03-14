/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



#include "TrigFastCalibWithRings.h"
#include <AsgMessaging/MessageCheck.h>

TrigFastCalibWithRings::TrigFastCalibWithRings([[maybe_unused]] const std::string& type, const std::string& myname, [[maybe_unused]] const IInterface* parent):asg::AsgTool(myname){} 


TrigFastCalibWithRings::~TrigFastCalibWithRings(){}

StatusCode TrigFastCalibWithRings::initialize() {

    ATH_CHECK(m_ringerKey.initialize());

    //Setup the BDTs ...
    ATH_CHECK(setupBDTFastCalo(PathResolverFindCalibFile(m_CalibPath)));
   
 
    return StatusCode::SUCCESS;
}

StatusCode TrigFastCalibWithRings::execute() const {

    return StatusCode::SUCCESS;
}

StatusCode TrigFastCalibWithRings::checkRings(const EventContext& ctx ) const {
   SG::ReadHandle<xAOD::TrigRingerRingsContainer> rgCont( m_ringerKey, ctx);
   ATH_CHECK(rgCont.isValid());
   const xAOD::TrigRingerRings_v2 *ring=rgCont->at(0);


    if(!ring->emCluster()){
      ATH_MSG_WARNING("There is no link to emCluster.");
      return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;

}


StatusCode TrigFastCalibWithRings::setupBDTFastCalo(const std::string& fileName){



  std::unique_ptr<TFile> f(TFile::Open(fileName.c_str()));
  if (!f || f->IsZombie()) {
    ATH_MSG_FATAL("Could not open " << fileName);
    return StatusCode::FAILURE;
  }

  // Load hPoly
  TH2Poly *hPoly = nullptr;
  f->GetObject("hPoly", hPoly);
  if (!hPoly) {
    ATH_MSG_FATAL("Could not find hPoly");
    return StatusCode::FAILURE;
  }
  //pass ownership to class variable
  m_hPoly.reset(static_cast<TH2Poly*>(hPoly));
  m_hPoly->SetDirectory(nullptr);

  // Load variables
  TObjArray *variablesTmp = nullptr;
  f->GetObject("variables", variablesTmp);
  if (!variablesTmp) {
    ATH_MSG_FATAL("Could not find variables");
    return StatusCode::FAILURE;
  }
  auto variables = std::unique_ptr<TObjArray>(variablesTmp);
  variables->SetOwner(); // to delete the objects when d-tor is called

  // Load shifts
  TObjArray *shiftsTmp = nullptr;
  f->GetObject("shifts", shiftsTmp);
  if (!shiftsTmp) {
    ATH_MSG_FATAL("Could not find shifts");
    return StatusCode::FAILURE;
  }
  auto shifts = std::unique_ptr<TObjArray>(shiftsTmp);
  shifts->SetOwner(); // to delete the objects when d-tor is called

  // Load trees
  TObjArray *treesTmp = nullptr;
  //std::unique_ptr<TObjArray> trees;
  TObjArray *trees = nullptr;
  f->GetObject("trees", treesTmp);
  if (treesTmp) {
    trees = treesTmp; 
    trees->SetOwner(); // to delete the objects when d-tor is called
    ATH_MSG_DEBUG("setupBDT " << "BDTs read from TObjArray");
  } else {
    ATH_MSG_DEBUG("setupBDT " << "Reading trees individually");
    trees = new TObjArray();
    trees->SetOwner(); // to delete the objects when d-tor is called
    for (int i = 0; i < variables->GetEntries(); ++i)
    {
      TTree *tree = nullptr;
      f->GetObject(Form("BDT%d", i), tree);
      if (tree) tree->SetCacheSize(0);
      trees->AddAtAndExpand(tree, i);
    }
  }

  // Ensure the objects have (the same number of) entries
  if (!trees->GetEntries() || !(trees->GetEntries() == variables->GetEntries())) {
    ATH_MSG_FATAL("Tree has size " << trees->GetEntries()
		  << " while variables has size " << variables->GetEntries());
    return StatusCode::FAILURE;
  }

  // Loop simultaneously over trees, variables and shifts
  // Define the BDTs, the list of variables and the shift for each BDT
  TObjString *str2;

  TTree *tree;
  TIter nextTree(trees);
  TIter nextVariables(variables.get());
  TIter nextShift(shifts.get());
  for (int i=0; (tree = (TTree*) nextTree()) && ((TObjString*) nextVariables()); ++i)
  {
    m_BDTs.emplace_back(tree);

    std::vector<std::function<float(const xAOD::Egamma*, const xAOD::CaloCluster*)> > funcs;
    // Loop over variables, which are separated by comma
    char separator_var = ';';
    if (getString(variables->At(i)).Index(";") < 1) separator_var = ','; // old versions
    std::unique_ptr<TObjArray> tokens(getString(variables->At(i)).Tokenize(separator_var));
    TIter nextVar(tokens.get());
    while ((str2 = (TObjString*) nextVar()))
    {
      const TString& varName = getString(str2);
      if (!varName.Length()) {
        ATH_MSG_FATAL("There was an empty variable name!");
        return StatusCode::FAILURE;
      }
    }
  }
return StatusCode::SUCCESS;
}


const TString& TrigFastCalibWithRings::getString(TObject* obj) 
{
  TObjString *objS = dynamic_cast<TObjString*>(obj);
  if (!objS) {
    throw std::runtime_error("egammaMVACalibTool::getString was passed something that was not a string object");
  }
  return objS->GetString();
}



float TrigFastCalibWithRings::makeCalibWRings(const EventContext& ctx) const {
    SG::ReadHandle<xAOD::TrigRingerRingsContainer> rgCont( m_ringerKey, ctx);
    const xAOD::TrigRingerRings_v2 *ring=rgCont->at(0);


    //Open the EventContext and create a BDT input vector: Rings + Et + eta
    float eta_cluster=ring->emCluster()->eta();
    float et_cluster=ring->emCluster()->et();
    const static std::vector<float>rings=rgCont->at(0)->rings();

    //Define the Rings to be used as inputs
    const std::vector<int>inputRingsIndex{0,1,2,3,8,9,10,11,12,13,14,15,72,73,74,75,76,77,78,79,81,82,83,84,88,89,90,91};

    if (!(static_cast<int>(ring ->size()) > inputRingsIndex.back())){
        throw std::runtime_error("The last ring index is bigger than the ring's lenght");
    }

    std::vector<float>ringsInput;
    for(auto index:inputRingsIndex)ringsInput.push_back(rings[index]);

    const TH2Poly* hPoly = m_hPoly.get();
    const int bin = hPoly->FindFixBin(eta_cluster, et_cluster/Gaudi::Units::GeV) - 1; // poly bins are shifted by one

    ATH_MSG_DEBUG("Using bin: " << bin);

    if (bin < 0) {
      ATH_MSG_DEBUG("The bin is under/overflow; just return the energy");
      return et_cluster;
    }

    if (bin >= static_cast<int>(m_BDTs.size())) {
      ATH_MSG_WARNING("The bin is outside the range, so just return the energy");
      return et_cluster;
    }

    // select the bdt and functions. (shifts are done later if needed)
    // if there is only one BDT just use that
    const int bin_BDT = m_BDTs.size() != 1 ? bin : 0;
    const auto& bdt = m_BDTs[bin_BDT];

    // evaluate the BDT response
    const float mvaOutput = bdt.GetResponse(ringsInput);



    return et_cluster*mvaOutput;
}
