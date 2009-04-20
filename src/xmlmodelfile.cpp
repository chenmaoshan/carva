#include <fstream>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "lib/global_def.h"
#include "lib/lib_misc.h"

#include "nrlib/iotools/logkit.hpp"
#include "nrlib/segy/segy.hpp"

#include "src/inputfiles.h"
#include "src/modelsettings.h"
#include "src/xmlmodelfile.h"
#include "src/vario.h"
#include "src/definitions.h"
#include "src/fftgrid.h"
#include "src/vario.h"

XmlModelFile::XmlModelFile(const char * fileName)
{
  modelSettings_         = new ModelSettings();
  inputFiles_            = new InputFiles();
  failed_                = false;

  TiXmlDocument doc;
	bool loadOkay = doc.LoadFile(fileName);

  if (loadOkay == false) {
    LogKit::LogFormatted(LogKit::ERROR,"Error: Reading of xml-file %s failed. %s Line %d, column %d\n", 
      fileName, doc.ErrorDesc(), doc.ErrorRow(), doc.ErrorCol());
    exit(1);
  }

  std::string errTxt = "";
  if(parseCrava(&doc, errTxt) == false)
    errTxt = "Error: '"+std::string(fileName)+"' is not a crava model file (lacks the 'crava' keyword.\n";

  checkForJunk(&doc, errTxt);

  checkConsistency(errTxt);


  if(errTxt != "") {
    LogKit::LogMessage(LogKit::ERROR, errTxt);
    failed_ = true;
  }
}

XmlModelFile::~XmlModelFile()
{
  //Both modelSettings and inputFiles are taken out and deleted outside.
}

bool
XmlModelFile::parseCrava(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("crava");
  if(root == 0)
    return(false);

  parseWellData(root, errTxt);
  parseSurvey(root, errTxt);
  parseProjectSettings(root, errTxt);
  parsePriorModel(root, errTxt);
  parseActions(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}

  
bool
XmlModelFile::parseActions(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("actions");
  if(root == 0)
    return(false);
  
  std::string mode;
  if(parseValue(root, "mode", mode, errTxt) == false)
    errTxt = errTxt+"Error: Command 'mode' must be given under command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  else {
    if(mode == "forward")
      modelSettings_->setGenerateSeismic(true);
    else if(mode == "estimation")
      modelSettings_->setEstimationMode(true);
    else if(mode != "inversion")
      errTxt = errTxt+"Error: String after 'mode' must be either 'inversion', 'estimation' or 'forward', found'"+
        mode+"' under command '"+root->ValueStr()+"'"+lineColumnText(root)+".\n";
  }

  parseInversionSettings(root, errTxt);
  
  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseInversionSettings(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("inversion-settings");
  if(root == 0)
    return(false);

  bool value;
  if(parseBool(root, "prediction", value, errTxt) == true)
    modelSettings_->setWritePrediction(value);
  
  parseSimulation(root, errTxt);

  if(parseBool(root, "condition-to-wells", value, errTxt) == true) {
    if(value == true) {
      if(modelSettings_->getKrigingParameter() == 0)
        modelSettings_->setKrigingParameter(250); //Default value.
    }
    else
      modelSettings_->setKrigingParameter(-1);
  }

  std::string facprob;
  if(parseValue(root, "facies-probabilities", facprob, errTxt) == true) {
    int flag = modelSettings_->getGridOutputFlag();
    if(facprob == "absolute")
      flag = (flag | ModelSettings::FACIESPROB);
    else if(facprob == "relative")
      flag = (flag | ModelSettings::FACIESPROBRELATIVE);
    modelSettings_->setGridOutputFlag(flag);
  }

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseSimulation(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("simulation");
  if(root == 0)
    return(false);

  int seed;
  bool seedGiven = parseValue(root, "seed", seed, errTxt);
  if(seedGiven == true)
    modelSettings_->setSeed(seed);

  std::string filename;
  if(parseFileName(root, "seed-file", filename, errTxt) == true) {
    if(seedGiven == true)
      errTxt = errTxt+"Error: Both seed and seed file given in command '"+
        root->ValueStr()+"'"+lineColumnText(root)+".\n";
  }

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseWellData(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("well-data");
  if(root == 0)
    return(false);
  

  parseLogNames(root, errTxt);

  int nWells = 0;
  while(parseWell(root, errTxt) == true)
    nWells++;
  modelSettings_->setNumberOfWells(nWells);

  float value;
  if(parseValue(root, "high-cut-seismic-resolution", value, errTxt) == true)
    modelSettings_->setHighCut(value);

  parseAllowedParameterValues(root, errTxt);

  if(parseValue(root, "maximum-deviation-angle", value, errTxt) == true)
    modelSettings_->setMaxDevAngle(value);

  if(parseValue(root, "maximum-rank-correlation", value, errTxt) == true)
    modelSettings_->setMaxRankCorr(value);

  if(parseValue(root, "maximum-merge-distance", value, errTxt) == true)
    modelSettings_->setMaxMergeDist(value);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseLogNames(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("log-names");
  if(root == 0)
    return(false);

  std::string value;
  if(parseValue(root, "time-log-name", value, errTxt) == true)
    modelSettings_->setLogName(0, value);

  bool vp = parseValue(root, "vp-log-name", value, errTxt);
  if(vp == true) {
    modelSettings_->setLogName(1, value);
    modelSettings_->setInverseVelocity(0, false);
  }
  if(parseValue(root, "dt-log-name", value, errTxt) == true) {
    if(vp == true)
      errTxt = errTxt+"Error: Both vp and dt given as logs in command '"
        +root->ValueStr()+"'"+lineColumnText(root)+".\n";
    else {
      modelSettings_->setLogName(1, value);
      modelSettings_->setInverseVelocity(0, true);
    }
  }

  bool vs = parseValue(root, "vs-log-name", value, errTxt);
  if(vp == true) {
    modelSettings_->setLogName(3, value);
    modelSettings_->setInverseVelocity(1, false);
  }
  if(parseValue(root, "dts-log-name", value, errTxt) == true) {
    if(vs == true)
      errTxt = errTxt+"Error: Both vs and dts given as logs in command '"
        +root->ValueStr()+"'"+lineColumnText(root)+".\n";
    else {
      modelSettings_->setLogName(3, value);
      modelSettings_->setInverseVelocity(1, true);
    }
  }

  if(parseValue(root, "density-log-name", value, errTxt) == true)
    modelSettings_->setLogName(2, value);
  if(parseValue(root, "facies-log-name", value, errTxt) == true)
    modelSettings_->setLogName(4, value);
  
  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseWell(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("well");
  if(root == 0)
    return(false);

  std::string tmpErr = "";
  std::string value;
  if(parseValue(root, "filename", value, tmpErr) == true) {
    inputFiles_->addWellFile(value);
    if(tmpErr == "")
      checkFileOpen(value, root, tmpErr);
  }
  else
    inputFiles_->addWellFile(""); //Dummy to keep tables balanced.

  bool use;
  if(parseBool(root, "use-in-estimate-of-wavelet", use, tmpErr) == true && use == false)
    modelSettings_->addIndicatorWavelet(0);
  else
    modelSettings_->addIndicatorWavelet(1);

  if(parseBool(root, "use-in-estimate-of-background-trend", use, tmpErr) == true && use == false)
    modelSettings_->addIndicatorBGTrend(0);
  else
    modelSettings_->addIndicatorBGTrend(1);

  if(parseBool(root, "use-in-estimate-of-facies-probabilities", use, tmpErr) == true && use == false)
    modelSettings_->addIndicatorFacies(0);
  else
    modelSettings_->addIndicatorFacies(1);

  checkForJunk(root, errTxt, true); //Allow duplicates

  errTxt += tmpErr;
  return(true);
}

bool
XmlModelFile::parseAllowedParameterValues(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("allowed-parameter-values");
  if(root == 0)
    return(false);

  float value;
  if(parseValue(root, "minimum-vp", value, errTxt) == true)
    modelSettings_->setAlphaMin(value);
  if(parseValue(root, "maximum-vp", value, errTxt) == true)
    modelSettings_->setAlphaMax(value);
  if(parseValue(root, "minimum-vs", value, errTxt) == true)
    modelSettings_->setBetaMin(value);
  if(parseValue(root, "maximum-vs", value, errTxt) == true)
    modelSettings_->setBetaMax(value);
  if(parseValue(root, "minimum-density", value, errTxt) == true)
    modelSettings_->setRhoMin(value);
  if(parseValue(root, "maximum-density", value, errTxt) == true)
    modelSettings_->setRhoMax(value);

  if(parseValue(root, "minimum-variance-vp", value, errTxt) == true)
    modelSettings_->setVarAlphaMin(value);
  if(parseValue(root, "maximum-variance-vp", value, errTxt) == true)
    modelSettings_->setVarAlphaMax(value);
  if(parseValue(root, "minimum-variance-vs", value, errTxt) == true)
    modelSettings_->setVarBetaMin(value);
  if(parseValue(root, "maximum-variance-vs", value, errTxt) == true)
    modelSettings_->setVarBetaMax(value);
  if(parseValue(root, "minimum-variance-density", value, errTxt) == true)
    modelSettings_->setVarRhoMin(value);
  if(parseValue(root, "maximum-variance-density", value, errTxt) == true)
    modelSettings_->setVarRhoMax(value);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseSurvey(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("survey");
  if(root == 0)
    return(false);

  Vario * vario = NULL;
  if((parseVariogram(root, "angular-correlation", vario, errTxt) == true) && (vario != NULL)) {
    vario->convertRangesFromDegToRad();
    modelSettings_->setAngularCorr(vario);
  }

  float value;
  if(parseValue(root, "segy-start-time", value, errTxt) == true)
    modelSettings_->setSegyOffset(value);

  while(parseAngleGather(root, errTxt) == true);

  if(modelSettings_->getNumberOfAngles() == 0)
    errTxt = errTxt + "Error: Need at least one angle gather in command '"+root->ValueStr()+"'"+
    lineColumnText(root)+".\n";

  parseWaveletEstimationInterval(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseAngleGather(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("angle-gather");
  if(root == 0)
    return(false);

  float value;
  if(parseValue(root, "offset-angle", value, errTxt) == true)
    modelSettings_->addAngle(value*float(M_PI/180));
  else
    errTxt = errTxt+"Error: Need offset angle for gather"+lineColumnText(root)+".\n";
  
  if(parseSeismicData(root, errTxt) == false) {
    //Go for defaults. Assume forward model (check later)
    inputFiles_->addSeismicFile("");
    modelSettings_->addSeismicType(ModelSettings::STANDARDSEIS);
  }

  if(parseWavelet(root, errTxt) == false) {
    inputFiles_->addWaveletFile("*");
    modelSettings_->addEstimateWavelet(true);
    modelSettings_->addWaveletScale(RMISSING);
  }

  bool bVal = false;
  if(parseBool(root, "match-energies", bVal, errTxt) == true)
    modelSettings_->addMatchEnergies(bVal);
  else
    modelSettings_->addMatchEnergies(false);

  if(parseValue(root, "signal-to-noise-ratio", value, errTxt) == true) {
    modelSettings_->addEstimateSNRatio(false);
    modelSettings_->addSNRatio(value);
  }
  else {
    modelSettings_->addEstimateSNRatio(true);
    modelSettings_->addSNRatio(RMISSING);
  }
    
  checkForJunk(root, errTxt, true);
  return(true);
}


bool
XmlModelFile::parseSeismicData(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("seismic-data");
  if(root == 0)
    return(false);

  std::string value = "";
  if(parseFileName(root, "file-name", value, errTxt) == true)
    inputFiles_->addSeismicFile(value);
  else
    inputFiles_->addSeismicFile(""); //Keeping tables balanced.

  if(parseValue(root, "type", value, errTxt) == true) {
    if(value == "pp") 
      modelSettings_->addSeismicType(ModelSettings::STANDARDSEIS);
    else if(value == "ps")
      modelSettings_->addSeismicType(ModelSettings::PSSEIS);
    else
      errTxt = errTxt+"Error: Only 'pp' and 'ps' are valid seismic types, found '"+value+
        "' on line "+NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
  }
  else
    modelSettings_->addSeismicType(ModelSettings::STANDARDSEIS);

  float fVal;
  if(parseValue(root, "start-time", fVal, errTxt) == true)
    modelSettings_->addLocalSegyOffset(fVal);
  else
    modelSettings_->addLocalSegyOffset(-1);

  TraceHeaderFormat * thf = NULL;
  if(parseTraceHeaderFormat(root, "segy-format", thf, errTxt) == true) {
    modelSettings_->addTraceHeaderFormat(thf);
  }
  else
    modelSettings_->addTraceHeaderFormat(NULL);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseWavelet(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("wavelet");
  if(root == 0)
    return(false);

  std::string value;
  if(parseFileName(root, "file-name", value, errTxt) == true) {
    inputFiles_->addWaveletFile(value);
    modelSettings_->addEstimateWavelet(false);
  }
  else {
    inputFiles_->addSeismicFile(""); //Keeping tables balanced.
    modelSettings_->addEstimateWavelet(true);
  }

  float scale;
  if(parseValue(root, "scale", scale, errTxt) == true)
    modelSettings_->addWaveletScale(scale);
  else
    modelSettings_->addWaveletScale(1);

  parseLocalWavelet(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseLocalWavelet(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("local-wavelet");
  if(root == 0)
    return(false);

  std::string filename;
  bool shiftGiven = parseFileName(root, "shift-file", filename, errTxt);
  if(shiftGiven)
    inputFiles_->addShiftFile(filename);
  else
    inputFiles_->addShiftFile("");

  bool scaleGiven = parseFileName(root, "scale-file", filename, errTxt);
  if(scaleGiven)
    inputFiles_->addScaleFile(filename);
  else
    inputFiles_->addScaleFile("");

  bool estimate;
  std::string tmpErr;
  if(parseBool(root, "estimate-shift",estimate,tmpErr) == false || tmpErr != "") {
    modelSettings_->addEstimateLocalShift(false);
    errTxt += tmpErr;
  }
  else 
    if(estimate == true && shiftGiven == true) {
      errTxt = errTxt +"Error: Can not give both file and ask for estimation of local wavelet shift"+
        lineColumnText(node);
      modelSettings_->addEstimateLocalShift(false);
    }
    else
      modelSettings_->addEstimateLocalShift(estimate);

  tmpErr = "";
  if(parseBool(root, "estimate-scale",estimate,tmpErr) == false || tmpErr != "") {
    modelSettings_->addEstimateLocalScale(false);
    errTxt += tmpErr;
  }
  else 
    if(estimate == true && scaleGiven == true) {
      errTxt = errTxt +"Error: Can not give both file and ask for estimation of local wavelet scale"+
        lineColumnText(node);
      modelSettings_->addEstimateLocalScale(false);
    }
    else
      modelSettings_->addEstimateLocalScale(estimate);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseWaveletEstimationInterval(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("wavelet-estimation-interval");
  if(root == 0)
    return(false);

  std::string filename;
  if(parseFileName(root, "top-surface-file", filename, errTxt) == true)
    inputFiles_->setWaveletEstIntFile(0, filename);
  if(parseFileName(root, "base-surface-file", filename, errTxt) == true)
    inputFiles_->setWaveletEstIntFile(1, filename);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parsePriorModel(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("prior-model");
  if(root == 0)
    return(false);

  parseBackground(root, errTxt);

  Vario* vario = NULL;
  if(parseVariogram(root, "lateral-correlation", vario, errTxt) == true)
    modelSettings_->setLateralCorr(vario);

  std::string filename;
  if(parseFileName(root, "temporal-correlation", filename, errTxt) == true)
    inputFiles_->setTempCorrFile(filename);

  if(parseFileName(root, "parameter-correlation", filename, errTxt) == true)
    inputFiles_->setParamCorrFile(filename);

  if(parseFileName(root, "correlation-direction", filename, errTxt) == true)
    inputFiles_->setCorrDirFile(filename);

  parseFaciesProbabilities(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseBackground(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("background");
  if(root == 0)
    return(false);

  std::string filename;
  bool vp = parseFileName(root, "vp-file", filename, errTxt);
  if(vp == true) {
    inputFiles_->setBackFile(0, filename);
    modelSettings_->setConstBackValue(0, -1);
  }

  bool vs = parseFileName(root, "vs-file", filename, errTxt);
  if(vp == true) {
    inputFiles_->setBackFile(1, filename);
    modelSettings_->setConstBackValue(1, -1);
  }

  bool rho = parseFileName(root, "density-file", filename, errTxt);
  if(vp == true) {
    inputFiles_->setBackFile(2, filename);
    modelSettings_->setConstBackValue(2, -1);
  }

  float value;
  if(parseValue(root, "vp-constant", value, errTxt) == true) {
    if(vp == true)
      errTxt = errTxt +"Error: Both file and constant given for vp in command '"+root->ValueStr()+"'"+
        lineColumnText(root)+".\n";
    else {
      vp = true;
      modelSettings_->setConstBackValue(0, value);
    }
  }

  if(parseValue(root, "vs-constant", value, errTxt) == true) {
    if(vs == true)
      errTxt = errTxt +"Error: Both file and constant given for vs in command '"+root->ValueStr()+"'"+
        lineColumnText(root)+".\n";
    else {
      vs = true;
      modelSettings_->setConstBackValue(1, value);
    }
  }

  if(parseValue(root, "density-constant", value, errTxt) == true) {
    if(rho == true)
      errTxt = errTxt +"Error: Both file and constant given for density in command '"+root->ValueStr()+"'"+
        lineColumnText(root)+".\n";
    else {
      rho = true;
      modelSettings_->setConstBackValue(2, value);
    }
  }

  bool bgGiven = (vp & vs & rho);
  bool estimate = !(vp | vs | rho);
  modelSettings_->setGenerateBackground(estimate);
  if((bgGiven | estimate) == false) {
    errTxt = errTxt + "Error: Either all or no background parameters must be given in command '"+root->ValueStr()+"'"+
        lineColumnText(root)+".\n";
  }

  if(parseFileName(root, "velocity-field", filename, errTxt) == true)
    inputFiles_->setBackVelFile(filename);
  
  Vario * vario = NULL;
  if(parseVariogram(root, "lateral-correlation", vario, errTxt) == true)
    modelSettings_->setBackgroundVario(vario);

  if(parseValue(root, "high-cut-background-modelling", value, errTxt) == true)
    modelSettings_->setMaxHzBackground(value);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseFaciesProbabilities(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("facies-probabilities");
  if(root == 0)
    return(false);

  parseFaciesEstimationInterval(root, errTxt);

  //parsePriorFaciesProbabilities

  float value;
  if(parseValue(root, "facies-probability-undefined-value", value, errTxt) == true)
    modelSettings_->setPundef(value);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseFaciesEstimationInterval(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("facies-estimation-interval");
  if(root == 0)
    return(false);

  std::string filename;
  if(parseFileName(root, "top-file-name", filename, errTxt) == true)
    inputFiles_->setFaciesEstIntFile(0, filename);
  else
    errTxt = errTxt+"Error: Must specify 'top-file-name' in command '"+root->ValueStr()+"'"+
      lineColumnText(root)+".\n";
  if(parseFileName(root, "base-file-name", filename, errTxt) == true)
    inputFiles_->setFaciesEstIntFile(1, filename);
  else
    errTxt = errTxt+"Error: Must specify 'base-file-name' in command '"+root->ValueStr()+"'"+
      lineColumnText(root)+".\n";

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseProjectSettings(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("project-settings");
  if(root == 0)
    return(false);

  if(parseOutputVolume(root, errTxt) == false)
    errTxt = errTxt+"Error: Command 'output-volume' is needed in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  parseIOSettings(root, errTxt);
  parseAdvancedSettings(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseOutputVolume(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("output-volume");
  if(root == 0)
    return(false);

  bool interval = parseIntervalTwoSurfaces(root, errTxt);
  
  if(parseIntervalOneSurface(root, errTxt) == interval) { //Either both or none given
    if(interval == true)
      errTxt = errTxt+"Error: Time interval specified in more than one way in command '"+
        root->ValueStr()+"'"+lineColumnText(root)+".\n";
    else
      errTxt = errTxt+"Error: No time interval specified in command '"+
        root->ValueStr()+"'"+lineColumnText(root)+".\n";
  }

  parseArea(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseIntervalTwoSurfaces(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("interval-two-surfaces");
  if(root == 0)
    return(false);

  modelSettings_->setParallelTimeSurfaces(false);

  if(parseTopSurface(root, errTxt) == false)
    errTxt = errTxt+"Error: Top surface not specified in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  bool topDepthGiven;
  if(inputFiles_->getDepthSurfFile(0) == "")
    topDepthGiven = false;
  else
    topDepthGiven = true;

  if(parseBaseSurface(root, errTxt) == false)
    errTxt = errTxt+"Error: Base surface not specified in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  bool baseDepthGiven;
  if(inputFiles_->getDepthSurfFile(1) == "")
    baseDepthGiven = false;
  else
    baseDepthGiven = true;

  int value = 0;
  if(parseValue(root, "number-of-layers", value, errTxt) == false)
    errTxt = errTxt+"Error: Number of layers not specified in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  else
    modelSettings_->setTimeNz(value);

  std::string filename;
  if(parseFileName(root, "velocity-field", filename, errTxt) == false) {
    if(topDepthGiven != baseDepthGiven)
      errTxt = errTxt+"Error: Only one depth surface given, and no velocity field in command '"+
        root->ValueStr()+"'"+lineColumnText(root)+".\n";
    else if(topDepthGiven == true) // Both top and bottom given.
      modelSettings_->setDepthDataOk(true);
  }
  else {
    inputFiles_->setVelocityField(filename);
    if(topDepthGiven == false && baseDepthGiven == false) {
      errTxt = errTxt+"Error: Velocity field, but no depth surface given in command '"+
        root->ValueStr()+"'"+lineColumnText(root)+".\n";
    }
    else
      modelSettings_->setDepthDataOk(true); //One or two surfaces, and velocity field.
  }

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseTopSurface(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("top-surface");
  if(root == 0)
    return(false);

  std::string filename;
  if(parseFileName(root,"time-file", filename, errTxt) == true)
    inputFiles_->addTimeSurfFile(filename);
  else {
    inputFiles_->addTimeSurfFile("");
    errTxt = errTxt+"Error: No time surface given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  }

  if(parseFileName(root,"depth-file", filename, errTxt) == true)
    inputFiles_->setDepthSurfFile(0, filename);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseBaseSurface(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("base-surface");
  if(root == 0)
    return(false);

  std::string filename;
  if(parseFileName(root,"time-file", filename, errTxt) == true)
    inputFiles_->addTimeSurfFile(filename);
  else {
    inputFiles_->addTimeSurfFile("");
    errTxt = errTxt+"Error: No time surface given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";
  }

  if(parseFileName(root,"depth-file", filename, errTxt) == true)
    inputFiles_->setDepthSurfFile(1, filename);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseIntervalOneSurface(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("interval-one-surface");
  if(root == 0)
    return(false);

  modelSettings_->setParallelTimeSurfaces(true);

  std::string filename;
  if(parseFileName(root, "reference-surface", filename, errTxt) == true)
    inputFiles_->addTimeSurfFile(filename);
  else
    errTxt = errTxt+"Error: No reference surface given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double value;
  if(parseValue(root, "shift-to-interval-top", value, errTxt) == true)
    modelSettings_->setTimeDTop(value);

  if(parseValue(root, "thickness", value, errTxt) == true)
    modelSettings_->setTimeLz(value);
  else
    errTxt = errTxt+"Error: No thickness given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  if(parseValue(root, "sample-density", value, errTxt) == true)
    modelSettings_->setTimeDz(value);
  else
    errTxt = errTxt+"Error: No sample density given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseArea(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("area");
  if(root == 0)
    return(false);

  double x0 = 0;
  if(parseValue(root, "reference-point-x", x0, errTxt) == false)
    errTxt = errTxt+"Error: Reference x-coordinat must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double y0 = 0;
  if(parseValue(root, "reference-point-y", y0, errTxt) == false)
    errTxt = errTxt+"Error: Reference y-coordinat must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double lx = 0;
  if(parseValue(root, "length-x", lx, errTxt) == false)
    errTxt = errTxt+"Error: X-length must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double ly = 0;
  if(parseValue(root, "length-y", ly, errTxt) == false)
    errTxt = errTxt+"Error: Y-length must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double dx = 0;
  if(parseValue(root, "sample-density-x", dx, errTxt) == false)
    errTxt = errTxt+"Error: Sample density for x must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double dy = 0;
  if(parseValue(root, "sample-density-y", dy, errTxt) == false)
    errTxt = errTxt+"Error: Sample density for y must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double angle = 0;
  if(parseValue(root, "angle", angle, errTxt) == false)
    errTxt = errTxt+"Error: Rotation angle must be given in command '"+
      root->ValueStr()+"'"+lineColumnText(root)+".\n";

  double rot = (-1)*angle*(M_PI/180.0);
  int nx = static_cast<int>(lx/dx);
  int ny = static_cast<int>(ly/dy);
  SegyGeometry * geometry = new SegyGeometry(x0, y0, dx, dy, nx, ny, 0, 0, 1, 1, true, rot);
  modelSettings_->setAreaParameters(geometry);
  delete geometry;

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseIOSettings(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("io-settings");
  if(root == 0)
    return(false);

  std::string value;
  if(parseValue(root, "top-directory", value, errTxt) == true); //NBNB Not ready yet.
  if(parseValue(root, "input-directory", value, errTxt) == true); //NBNB Not ready yet.
  if(parseValue(root, "output-directory", value, errTxt) == true); //NBNB Not ready yet.

  parseOutputTypes(root, errTxt);

  if(parseValue(root, "file-output-prefix", value, errTxt) == true)
    ModelSettings::setFilePrefix(value);

  int level;
  if(parseValue(root, "log-level", level, errTxt) == true)
    modelSettings_->setLogLevel(level);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseOutputTypes(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("output-types");
  if(root == 0)
    return(false);

  parseGridOutput(root, errTxt);
  parseWellOutput(root, errTxt);
  parseOtherOutput(root, errTxt);


  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseGridOutput(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("grid-output");
  if(root == 0)
    return(false);

  parseGridFormats(root, errTxt);
  parseGridDomains(root, errTxt);
  parseGridParameters(root, errTxt);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseGridDomains(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("domain");
  if(root == 0)
    return(false);

  bool useDomain = false;
  int domainFlag = modelSettings_->getOutputDomainFlag();
  if(parseBool(root, "time", useDomain, errTxt) == true) {
    if(useDomain == true)
      domainFlag = (domainFlag | ModelSettings::TIMEDOMAIN);
    else if((domainFlag & ModelSettings::TIMEDOMAIN) > 0)
      domainFlag -= ModelSettings::TIMEDOMAIN;
  }
  if(parseBool(root, "depth", useDomain, errTxt) == true) {
    if(useDomain == true) {
      domainFlag = (domainFlag | ModelSettings::DEPTHDOMAIN);
    }
    else if((domainFlag & ModelSettings::DEPTHDOMAIN) > 0)
      domainFlag -= ModelSettings::DEPTHDOMAIN;
  }
  modelSettings_->setOutputDomainFlag(domainFlag);

  if(domainFlag == 0)
    errTxt = errTxt+"Error: Both time and depth domain output turned off after command '"
      +root->ValueStr()+"'"+lineColumnText(root)+".\n";

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseGridFormats(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("format");
  if(root == 0)
    return(false);

  bool useFormat = false;
  int formatFlag = 0;
  bool stormSpecified = false;  //Default format, error if turned off and no others turned on.
  if(parseBool(root, "segy", useFormat, errTxt) == true && useFormat == true)
    formatFlag += ModelSettings::SEGY;
  if(parseBool(root, "storm", useFormat, errTxt) == true && useFormat == true) {
    stormSpecified = true;
    formatFlag += ModelSettings::STORM;
  }
  if(parseBool(root, "ascii", useFormat, errTxt) == true && useFormat == true)
    formatFlag += ModelSettings::ASCII;
  if(parseBool(root, "sgri", useFormat, errTxt) == true && useFormat == true)
    formatFlag += ModelSettings::SGRI;

  if(formatFlag > 0 || stormSpecified == true)
    modelSettings_->setOutputFormatFlag(formatFlag);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseGridParameters(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("parameters");
  if(root == 0)
    return(false);

  bool value = false;
  int paramFlag = 0;
  if(parseBool(root, "vp", value, errTxt) == true)
    paramFlag += ModelSettings::VP;
  if(parseBool(root, "vs", value, errTxt) == true)
    paramFlag += ModelSettings::VS;
  if(parseBool(root, "density", value, errTxt) == true)
    paramFlag += ModelSettings::RHO;
  if(parseBool(root, "lame-lambda", value, errTxt) == true)
    paramFlag += ModelSettings::LAMELAMBDA;
  if(parseBool(root, "lame-mu", value, errTxt) == true)
    paramFlag += ModelSettings::LAMEMU;
  if(parseBool(root, "poisson-ratio", value, errTxt) == true)
    paramFlag += ModelSettings::POISSONRATIO;
  if(parseBool(root, "ai", value, errTxt) == true)
    paramFlag += ModelSettings::AI;
  if(parseBool(root, "si", value, errTxt) == true)
    paramFlag += ModelSettings::SI;
  if(parseBool(root, "vp-vs-ratio", value, errTxt) == true)
    paramFlag += ModelSettings::VPVSRATIO;
  if(parseBool(root, "murho", value, errTxt) == true)
    paramFlag += ModelSettings::MURHO;
  if(parseBool(root, "lambdarho", value, errTxt) == true)
    paramFlag += ModelSettings::LAMBDARHO;
  if(parseBool(root, "correlations", value, errTxt) == true)
    paramFlag += ModelSettings::CORRELATION;
  if(parseBool(root, "residuals", value, errTxt) == true)
    paramFlag += ModelSettings::RESIDUAL;
  if(parseBool(root, "background", value, errTxt) == true)
    paramFlag += ModelSettings::BACKGROUND;
  if(parseBool(root, "extra-grids", value, errTxt) == true)
    paramFlag += ModelSettings::EXTRA_GRIDS;

  if(paramFlag > 0)
    modelSettings_->setGridOutputFlag(paramFlag);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseWellOutput(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("well-output");
  if(root == 0)
    return(false);

  bool value;
  int wellFlag = 0;
  if(parseBool(root, "wells", value, errTxt) == true && value == true)
    wellFlag += ModelSettings::WELLS;
  if(parseBool(root, "blocked-wells", value, errTxt) == true && value == true)
    wellFlag += ModelSettings::BLOCKED_WELLS;
  if(parseBool(root, "blocked-logs", value, errTxt) == true && value == true)
    wellFlag += ModelSettings::BLOCKED_LOGS;

  modelSettings_->setWellOutputFlag(wellFlag);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseOtherOutput(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("other-output");
  if(root == 0)
    return(false);

  bool value;
  int otherFlag = 0;
  if(parseBool(root, "wavelets", value, errTxt) == true && value == true)
    otherFlag += ModelSettings::WAVELETS;
  if(parseBool(root, "extra-surfaces", value, errTxt) == true && value == true)
    otherFlag += ModelSettings::EXTRA_SURFACES;
  if(parseBool(root, "prior-correlations", value, errTxt) == true)
    otherFlag += ModelSettings::PRIORCORRELATIONS;
  if(parseBool(root, "background-trend-1d", value, errTxt) == true)
    otherFlag += ModelSettings::BACKGROUND_TREND_1D;

  modelSettings_->setOtherOutputFlag(otherFlag);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseAdvancedSettings(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("advanced-settings");
  if(root == 0)
    return(false);

  parseFFTGridPadding(root, errTxt);

  int fileGrid;
  if(parseValue(root, "use-intermediate-disk-storage", fileGrid, errTxt) == true)
    modelSettings_->setFileGrid(fileGrid);
  double limit;
  if(parseValue(root,"maximium-relative-thickness-difference", limit, errTxt) == true)
    modelSettings_->setLzLimit(limit);
  
  parseFrequencyBand(root, errTxt);

  float value = 0.0f;
  if(parseValue(root, "energy-threshold", value, errTxt) == true)
    modelSettings_->setEnergyThreshold(value);
  if(parseValue(root, "wavelet-tapering-length", value, errTxt) == true)
    modelSettings_->setWaveletTaperingL(value);
  if(parseValue(root, "minimum-relative-wavelet-amplitude", value, errTxt) == true)
    modelSettings_->setMinRelWaveletAmp(value);
  if(parseValue(root, "maximum-wavelet-shift", value, errTxt) == true)
    modelSettings_->setMaxWaveletShift(value);
  if(parseValue(root, "white-noise-component", value, errTxt) == true)
    modelSettings_->setWNC(value);
  int level = 0;
  if(parseValue(root, "debug-level", level, errTxt) == true)
    modelSettings_->setDebugFlag(level);
  std::string filename;
  if(parseFileName(root, "reflection-matrix", filename, errTxt) == true)
    inputFiles_->setReflMatrFile(filename);
  int kLimit = 0;
  if(parseValue(root, "kriging-data-limit", kLimit, errTxt) == true) {
    if(modelSettings_->getKrigingParameter() >= 0)
      modelSettings_->setKrigingParameter(kLimit);
  }
  if(parseValue(root, "debug-level", level, errTxt) == true)
    modelSettings_->setDebugFlag(level);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseFFTGridPadding(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("fft-grid-padding");
  if(root == 0)
    return(false);

  double padding;
  if(parseValue(root, "x-fraction", padding, errTxt) == true)
    modelSettings_->setXPadFac(padding);
  if(parseValue(root, "y-fraction", padding, errTxt) == true)
    modelSettings_->setYPadFac(padding);
  if(parseValue(root, "z-fraction", padding, errTxt) == true)
    modelSettings_->setZPadFac(padding);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseFrequencyBand(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement("frequency-band");
  if(root == 0)
    return(false);

  float value;
  if(parseValue(root, "low-cut", value, errTxt) == true)
    modelSettings_->setLowCut(value);
  if(parseValue(root, "high-cut", value, errTxt) == true)
    modelSettings_->setHighCut(value);

  checkForJunk(root, errTxt);
  return(true);
}


/*
bool
XmlModelFile::parse(TiXmlNode * node, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement();
  if(root == 0)
    return(false);

  checkForJunk(root, errTxt);
  return(true);
}


*/


bool
XmlModelFile::parseTraceHeaderFormat(TiXmlNode * node, const std::string & keyword, TraceHeaderFormat * thf, std::string & errTxt)
{
  thf = NULL;
  TiXmlNode * root = node->FirstChildElement(keyword);
  if(root == 0)
    return(false);

  std::string stdFormat;
  if(parseValue(root, "standard-format", stdFormat, errTxt) == true) {
    if(stdFormat == "seisworks")
      thf = new TraceHeaderFormat(TraceHeaderFormat::SEISWORKS);
    else if(stdFormat == "iesx")
      thf = new TraceHeaderFormat(TraceHeaderFormat::IESX);
    else {
      errTxt = errTxt+"Error: Unknown segy-format '"+stdFormat+"' found on line"+
        NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
      thf = new TraceHeaderFormat(TraceHeaderFormat::SEISWORKS);
    }
  }
  else
    thf = new TraceHeaderFormat(TraceHeaderFormat::SEISWORKS);

  int value;
  if(parseValue(root,"location-x",value, errTxt) == true)
    thf->SetUtmxLoc(value);
  if(parseValue(root,"location-y",value, errTxt) == true)
    thf->SetUtmyLoc(value);
  if(parseValue(root,"location-il",value, errTxt) == true)
    thf->SetInlineLoc(value);
  if(parseValue(root,"location-xl",value, errTxt) == true)
    thf->SetCrosslineLoc(value);
  if(parseValue(root,"location-scaling-coefficient",value, errTxt) == true)
    thf->SetScaleCoLoc(value);

  bool bypass;
  if(parseBool(root,"bypass-corrdinate-scaling", bypass, errTxt) == true)
    if(bypass == true)
      thf->SetScaleCoLoc(-1);

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseVariogram(TiXmlNode * node, const std::string & keyword, Vario * & vario, std::string & errTxt)
{
  TiXmlNode * root = node->FirstChildElement(keyword);
  if(root == 0)
    return(false);

  float value, range = 1;
  float subrange=1;
  float angle = 0;
  float expo = 1;
  if(parseValue(root,"range", value, errTxt) == true)
    range = value;
  else
    errTxt = errTxt+"Error: Keyword 'range' is lacking for variogram under command '"+keyword+
      "', line "+NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
  if(parseValue(root,"subrange", value, errTxt) == true)
    subrange = value;
  if(parseValue(root,"angle", value, errTxt) == true)
    angle = value;
  bool power = parseValue(root,"power", value, errTxt);
  if(power == true)
    expo = value;

  std::string vType;
  vario = NULL;
  if(parseValue(root,"variogram-type", vType, errTxt) == false)
    errTxt = errTxt+"Error: Keyword 'variogram-type' is lacking for variogram under command '"+keyword+
      "', line "+NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
  else {
    if(vType == "genexp") {
      if(power == false) {
        errTxt = errTxt+"Error: Keyword 'power' is lacking for gen. exp. variogram under command '"+keyword+
          "', line "+NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
      }
      vario = new GenExpVario(expo, range, subrange, angle);
    }
    else if(vType == "spherical") {
      if(power == true) {
        errTxt = errTxt+"Error: Keyword 'power' is given for spherical variogram under command '"+keyword+
          "', line "+NRLib2::ToString(root->Row())+", column "+NRLib2::ToString(root->Column())+".\n";
      }
      vario = new SphericalVario(range, subrange, angle);
    }
  }

  checkForJunk(root, errTxt);
  return(true);
}


bool
XmlModelFile::parseBool(TiXmlNode * node, const std::string & keyword, bool & value, std::string & errTxt, bool allowDuplicates)
{
  std::string tmpVal;
  std::string tmpErr = "";
  if(parseValue(node, keyword, tmpVal, tmpErr, allowDuplicates) == false)
    return(false);

  //Keyword is found.
  if(tmpErr == "") {
    if(tmpVal == "yes")
      value = true;
    else if(tmpVal == "no")
      value = false;
    else {
      tmpErr = "Error: Found '"+tmpVal+"' under keyword '"+keyword+"', expected 'yes' or 'no'. This happened in command '"+
        node->ValueStr()+"' on line "+NRLib2::ToString(node->Row())+", column "+NRLib2::ToString(node->Column())+".\n";
    }
  }

  //No junk-clearing call, done in parseValue.
  errTxt += tmpErr;
  return(true);
}


bool
XmlModelFile::parseFileName(TiXmlNode * node, const std::string & keyword, std::string & filename, std::string & errTxt, bool allowDuplicates)
{
  filename = "";
  std::string value;
  std::string tmpErr;
  if(parseValue(node, keyword, value, tmpErr, allowDuplicates) == false)
    return(false);
  
  filename = value;
  if(tmpErr == "")
    checkFileOpen(value, node, tmpErr);

  //No junk-clearing, done in parseValue
  errTxt += tmpErr;
  return(true);
}


void 
XmlModelFile::checkForJunk(TiXmlNode * root, std::string & errTxt, bool allowDuplicates)
{
  TiXmlNode * child = root->FirstChild();
  while(child != NULL) {
    switch(child->Type()) {
      case TiXmlNode::COMMENT :
      case TiXmlNode::DECLARATION :
        break;
      case TiXmlNode::TEXT :
        errTxt = errTxt + "Error: Unexpected value '"+child->Value()+"' is not part of command '"+root->Value()+
          "' on line "+NRLib2::ToString(child->Row())+", column "+NRLib2::ToString(child->Column())+".\n";
        break;
      case TiXmlNode::ELEMENT :
        errTxt = errTxt + "Error: Unexpected command '"+child->Value()+"' is not part of command '"+root->Value()+
          "' on line "+NRLib2::ToString(child->Row())+", column "+NRLib2::ToString(child->Column())+".\n";
        break;
      default :
        errTxt = errTxt + "Error: Unexpected text '"+child->Value()+"' is not part of command '"+root->Value()+
          "' on line "+NRLib2::ToString(child->Row())+", column "+NRLib2::ToString(child->Column())+".\n";
        break;
    }
    root->RemoveChild(child);
    child = root->FirstChild();
  }

  TiXmlNode * parent = root->Parent();

  if(parent != NULL) {
    std::string cmd = root->ValueStr();
    parent->RemoveChild(root);

    if(allowDuplicates == false) {
      root = parent->FirstChildElement(cmd);
      if(root != NULL) {
        int n = 0;
        while(root != NULL) {
          n++;
          parent->RemoveChild(root);
          root = parent->FirstChildElement(root->Value());
        }
        errTxt = errTxt +"Error: Found "+NRLib2::ToString(n)+" extra occurences of command '"+cmd+"' under command '"+parent->Value()+
          " on line "+NRLib2::ToString(parent->Row())+", column "+NRLib2::ToString(parent->Column())+".\n";
      }
    }
  }
}

bool 
XmlModelFile::checkFileOpen(const std::string & fName, TiXmlNode * node, std::string & errText)
{
  bool error = false;
  if(fName != "*" && fName != "?")
  {
    FILE * file = fopen(fName.c_str(),"r");
    if(file == 0)
    {
      error = true;
      errText = errText +"Error: Failed to open file '"+fName+"' in command '"+node->ValueStr()+
        "' on line "+NRLib2::ToString(node->Row())+", column "+NRLib2::ToString(node->Column())+".\n";
    }
    else
      fclose(file);
  }
  return(error);
}


std::string
XmlModelFile::lineColumnText(TiXmlNode * node)
{
  std::string result = " on line "+NRLib2::ToString(node->Row())+", column "+NRLib2::ToString(node->Column());
  return(result);
}


void
XmlModelFile::checkConsistency(std::string & errTxt) {
  if(modelSettings_->getGenerateSeismic() == true)
    checkForwardConsistency(errTxt);
  else
    checkEstimationInversionConsistency(errTxt);
}


void
XmlModelFile::checkForwardConsistency(std::string & errTxt) {
  //Mostly, we don't care here, but have to straighten some things.
  int i;
  for(i=0;i<modelSettings_->getNumberOfAngles();i++)
    modelSettings_->setSNRatio(i,1.1f);
}


void
XmlModelFile::checkEstimationInversionConsistency(std::string & errTxt) {
  //Quite a few things to check here.
}