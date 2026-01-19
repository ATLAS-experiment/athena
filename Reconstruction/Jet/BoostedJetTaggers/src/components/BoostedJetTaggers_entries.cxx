#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/SmoothedTopTagger.h"
#include "BoostedJetTaggers/JetQGTagger.h"
#include "BoostedJetTaggers/JetQGTaggerBDT.h"
#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"
#include "BoostedJetTaggers/JSSWTopTaggerANN.h"
#include "BoostedJetTaggers/JSSMLTool.h"
#include "BoostedJetTaggers/JSSTaggerUtils.h"
#include "BoostedJetTaggers/LundJetOnnxAlg.h"

DECLARE_COMPONENT(SmoothedWZTagger)
DECLARE_COMPONENT(SmoothedTopTagger)
DECLARE_COMPONENT(JSSWTopTaggerDNN)
DECLARE_COMPONENT(JSSWTopTaggerANN)
DECLARE_COMPONENT(CP::JetQGTagger)
DECLARE_COMPONENT(CP::JetQGTaggerBDT)
DECLARE_COMPONENT(AthONNX::JSSMLTool)
DECLARE_COMPONENT(JSSTaggerUtils)
#ifndef XAOD_ANALYSIS
DECLARE_COMPONENT( LundJetOnnxAlg )
#endif
