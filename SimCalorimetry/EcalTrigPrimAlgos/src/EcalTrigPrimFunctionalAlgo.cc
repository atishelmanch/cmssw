/** \class EcalTrigPrimFunctionalAlgo
 *
 * EcalTrigPrimFunctionalAlgo is the main algorithm class for TPG
 * It coordinates all the other algorithms
 * Structure is very close to electronics
 *
 *
 * \author Ursula Berthon, Stephanie Baffioni, LLR Palaiseau
 *
 * \version   1st Version may 2006
 * \version   2nd Version jul 2006

 *
 ************************************************************/
#include <algorithm>
#include <functional>
#include <numeric>
#include <string>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/EcalMapping/interface/EcalElectronicsMapping.h"
#include "Geometry/EcalMapping/interface/EcalMappingRcd.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixLinearizer.h"
#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStrip.h"
#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixTcp.h"
#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalTrigPrimFunctionalAlgo.h"

#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
#include "DataFormats/EcalDetId/interface/EcalTriggerElectronicsId.h"
#include "DataFormats/EcalDigi/interface/EBDataFrame.h"
#include "DataFormats/EcalDigi/interface/EEDataFrame.h"

#include "CondFormats/DataRecord/interface/EcalTPGPedestalsRcd.h"
#include "CondFormats/EcalObjects/interface/EcalTPGPedestals.h"

#include <TMath.h>
#include <TTree.h>
#include <string> 

const unsigned int EcalTrigPrimFunctionalAlgo::nrSamples_ = 5;  // to be written
const unsigned int EcalTrigPrimFunctionalAlgo::maxNrSamplesOut_ = 10;
const unsigned int EcalTrigPrimFunctionalAlgo::maxNrTowers_ = 2448;
const unsigned int EcalTrigPrimFunctionalAlgo::maxNrTPs_ = 2448;  // FIXME??

//----------------------------------------------------------------------

EcalTrigPrimFunctionalAlgo::EcalTrigPrimFunctionalAlgo(
    const edm::EventSetup &setup, int binofmax, bool tcpFormat, bool barrelOnly, bool debug, bool famos, bool TPinfoPrintout)
    : binOfMaximum_(binofmax),
      tcpFormat_(tcpFormat),
      barrelOnly_(barrelOnly),
      debug_(debug),
      famos_(famos),
      TPinfoPrintout_(TPinfoPrintout)

{
  if (famos_)
    maxNrSamples_ = 1;  // get from input??
  else
    maxNrSamples_ = 10;
  this->init(setup);
}

//----------------------------------------------------------------------
void EcalTrigPrimFunctionalAlgo::init(const edm::EventSetup &setup) {
  if (!barrelOnly_) {
    edm::ESHandle<CaloGeometry> theGeometry;
    edm::ESHandle<CaloSubdetectorGeometry> theEndcapGeometry_handle;
    setup.get<CaloGeometryRecord>().get(theGeometry);
    setup.get<EcalEndcapGeometryRecord>().get("EcalEndcap", theEndcapGeometry_handle);
    theEndcapGeometry = &(*theEndcapGeometry_handle);
    setup.get<IdealGeometryRecord>().get(eTTmap_);
  }
  // endcap mapping
  edm::ESHandle<EcalElectronicsMapping> ecalmapping;
  setup.get<EcalMappingRcd>().get(ecalmapping);
  theMapping_ = ecalmapping.product();

  // create main sub algos
  estrip_ = new EcalFenixStrip(setup, theMapping_, debug_, famos_, maxNrSamples_, nbMaxXtals_, TPinfoPrintout_);
  etcp_ = new EcalFenixTcp(setup, tcpFormat_, debug_, famos_, binOfMaximum_, maxNrSamples_, nbMaxStrips_);

  // initialise data structures
  initStructures(towerMapEB_);
  initStructures(towerMapEE_);

  hitTowers_.resize(maxNrTowers_);
  towtp_.resize(maxNrSamplesOut_);
  towtp2_.resize(maxNrSamplesOut_);
}
//----------------------------------------------------------------------

EcalTrigPrimFunctionalAlgo::~EcalTrigPrimFunctionalAlgo() {
  delete estrip_;
  delete etcp_;
}
//----------------------------------------------------------------------
void EcalTrigPrimFunctionalAlgo::run(const edm::EventSetup &setup,
                                     EBDigiCollection const *col,
                                     EcalTrigPrimDigiCollection &result,
                                     EcalTrigPrimDigiCollection &resultTcp) {
  run_part1_EB(col);
  run_part2(setup, col, towerMapEB_, result, resultTcp);
}

//----------------------------------------------------------------------
void EcalTrigPrimFunctionalAlgo::run(const edm::EventSetup &setup,
                                     EEDigiCollection const *col,
                                     EcalTrigPrimDigiCollection &result,
                                     EcalTrigPrimDigiCollection &resultTcp) {
  run_part1_EE(col);
  run_part2(setup, col, towerMapEE_, result, resultTcp);
}
//----------------------------------------------------------------------
int EcalTrigPrimFunctionalAlgo::findStripNr(const EBDetId &id) {
  int stripnr;
  int n = ((id.ic() - 1) % 100) / 20;  // 20 corresponds to 4 * ecal_barrel_crystals_per_strip FIXME!!
  if (id.ieta() < 0)
    stripnr = n + 1;
  else
    stripnr = nbMaxStrips_ - n;
  return stripnr;
}
//----------------------------------------------------------------------
int EcalTrigPrimFunctionalAlgo::findStripNr(const EEDetId &id) {
  int stripnr;
  const EcalTriggerElectronicsId elId = theMapping_->getTriggerElectronicsId(id);
  stripnr = elId.pseudoStripId();
  return stripnr;
}
//----------------------------------------------------------------------

void EcalTrigPrimFunctionalAlgo::run_part1_EB(EBDigiCollection const *col) {
  clean(towerMapEB_);
  // loop over dataframes and fill map
  fillMap(col, towerMapEB_);
}
//----------------------------------------------------------------------
void EcalTrigPrimFunctionalAlgo::run_part1_EE(EEDigiCollection const *col) {
  clean(towerMapEE_);
  // loop over dataframes and fill map
  fillMap(col, towerMapEE_);
}

template <class Coll>
void EcalTrigPrimFunctionalAlgo::run_part2(
    const edm::EventSetup &setup,
    Coll const *col,
    std::vector<std::vector<std::pair<int, std::vector<typename Coll::Digi>>>> &towerMap,
    EcalTrigPrimDigiCollection &result,
    EcalTrigPrimDigiCollection &resultTcp) {
  typedef typename Coll::Digi Digi;

  // prepare writing of TP-s

  int firstSample = binOfMaximum_ - 1 - nrSamples_ / 2;
  int lastSample = binOfMaximum_ - 1 + nrSamples_ / 2;
  int nrTP = 0;
  std::vector<typename Coll::Digi> dummy;
  EcalTriggerPrimitiveDigi tptow[2];
  EcalTriggerPrimitiveDigi tptowTcp[2];

  estrip_->getFGVB()->setbadStripMissing(false);

  for (int itow = 0; itow < nrTowers_; ++itow) {
    if(debug_){
      std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl; 
      std::cout << "on Tower " << itow << " of " << nrTowers_ << std::endl;
    } 
    int index = hitTowers_[itow].first;
    const EcalTrigTowerDetId &thisTower = hitTowers_[itow].second;

    // loop over all strips assigned to this trigger tower
    int nstr = 0;
    for (unsigned int i = 0; i < towerMap[itow].size(); ++i) {
      std::vector<Digi> &df = (towerMap[index])[i].second;  // vector of dataframes for this strip,
                                                            // size; nr of crystals/strip

      if ((towerMap[index])[i].first > 0) {
        if(debug_){
          std::cout << "-------------------------------------------------" << std::endl; 
          std::cout << "on Strip index " << i << std::endl;
        }         
        estrip_->process(setup, df, (towerMap[index])[i].first, striptp_[nstr++]); 
      }
    }  // loop over strips in one tower

    bool isInInnerRings = false;
    if (thisTower.subDet() == EcalEndcap && (thisTower.ietaAbs() == 27 || thisTower.ietaAbs() == 28))
      isInInnerRings = true;
    etcp_->process(setup, dummy, striptp_, nstr, towtp_, towtp2_, isInInnerRings, thisTower);

    // prepare TP-s
    // special treatment for 2 inner endcap rings
    int nrTowers;
    if (isInInnerRings) {
      nrTowers = 2;
      int phi = 2 * ((thisTower.iphi() - 1) / 2);
      tptow[0] = EcalTriggerPrimitiveDigi(
          EcalTrigTowerDetId(thisTower.zside(), thisTower.subDet(), thisTower.ietaAbs(), phi + 1));
      tptow[1] = EcalTriggerPrimitiveDigi(
          EcalTrigTowerDetId(thisTower.zside(), thisTower.subDet(), thisTower.ietaAbs(), phi + 2));

      if (tcpFormat_) {
        tptowTcp[0] = EcalTriggerPrimitiveDigi(
            EcalTrigTowerDetId(thisTower.zside(), thisTower.subDet(), thisTower.ietaAbs(), phi + 1));
        tptowTcp[1] = EcalTriggerPrimitiveDigi(
            EcalTrigTowerDetId(thisTower.zside(), thisTower.subDet(), thisTower.ietaAbs(), phi + 2));
      }
    } else {
      nrTowers = 1;
      tptow[0] = EcalTriggerPrimitiveDigi(thisTower);
      if (tcpFormat_)
        tptowTcp[0] = EcalTriggerPrimitiveDigi(thisTower);
    }

    // now fill in
    for (int nrt = 0; nrt < nrTowers; nrt++) {
      (tptow[nrt]).setSize(nrSamples_);
      if (towtp_.size() < nrSamples_) {  // FIXME: only once
        edm::LogWarning("") << "Too few samples produced, nr is " << towtp_.size();
        break;
      }
      int isam = 0;
      for (int i = firstSample; i <= lastSample; ++i) {
        tptow[nrt].setSample(isam++, EcalTriggerPrimitiveSample(towtp_[i]));
      }
      nrTP++;
      LogDebug("EcalTPG") << " For tower " << itow << " created TP nr " << nrTP << " with Et "
                          << tptow[nrt].compressedEt();
      result.push_back(tptow[nrt]);
    }

    if (tcpFormat_) {
      for (int nrt = 0; nrt < nrTowers; nrt++) {
        tptowTcp[nrt].setSize(nrSamples_);
        if (towtp2_.size() < nrSamples_) {  // FIXME: only once
          edm::LogWarning("") << "Too few samples produced, nr is " << towtp2_.size();
          break;
        }
        int isam = 0;
        for (int i = firstSample; i <= lastSample; ++i) {
          if (nrTowers <= 1)
            tptowTcp[nrt].setSample(isam++, EcalTriggerPrimitiveSample(towtp2_[i]));
          else {
            int et = towtp2_[i].compressedEt() / 2;
            tptowTcp[nrt].setSample(isam++,
                                    EcalTriggerPrimitiveSample(et, towtp2_[i].fineGrain(), towtp2_[i].ttFlag()));
          }
        }
        resultTcp.push_back(tptowTcp[nrt]);
      }
    }
  }
  return;
}

template <class Coll>
void EcalTrigPrimFunctionalAlgo::fillMap(
    Coll const *col, std::vector<std::vector<std::pair<int, std::vector<typename Coll::Digi>>>> &towerMap) {
  typedef typename Coll::Digi Digi;

  // implementation for Barrel and Endcap

  if (col) {
    nrTowers_ = 0;
    LogDebug("EcalTPG") << "Fill mapping, Collection size = " << col->size();
    for (unsigned int i = 0; i < col->size(); ++i) {
      Digi samples((*col)[i]);
      EcalTrigTowerDetId coarser = (*eTTmap_).towerOf(samples.id());
      int index = getIndex(col, coarser);
      int stripnr = findStripNr(samples.id());

      int filled = 0;
      for (unsigned int ij = 0; ij < towerMap[index].size(); ++ij)
        filled += towerMap[index][ij].first;
      if (!filled) {
        hitTowers_[nrTowers_++] = std::pair<int, EcalTrigTowerDetId>(index, coarser);
      }

      // FIXME: temporary protection
      int ncryst = towerMap[index][stripnr - 1].first;
      if (ncryst >= nbMaxXtals_) {
        edm::LogError("EcalTrigPrimFunctionAlgo")
            << "! Too many xtals for TT " << coarser << " stripnr " << stripnr << " xtalid " << samples.id();
        continue;
      }
      ((towerMap[index])[stripnr - 1].second)[ncryst] = samples;
      (towerMap[index])[stripnr - 1].first++;
    }

    LogDebug("EcalTPG") << "fillMap"
                        << "[EcalTrigPrimFunctionalAlgo] (found " << col->size() << " frames in " << towerMap.size()
                        << " towers) ";
  } else {
    LogDebug("EcalTPG") << "FillMap - FillMap Collection size=0 !!!!";
  }
}

template <class T>
void EcalTrigPrimFunctionalAlgo::clean(std::vector<std::vector<std::pair<int, std::vector<T>>>> &towMap) {
  // clean internal data structures
  for (unsigned int i = 0; i < maxNrTowers_; ++i)
    for (int j = 0; j < nbMaxStrips_; ++j)
      (towMap[i])[j].first = 0;
  return;
}

template <class T>
void EcalTrigPrimFunctionalAlgo::initStructures(std::vector<std::vector<std::pair<int, std::vector<T>>>> &towMap) {
  // initialise internal data structures

  std::vector<T> vec0(nbMaxXtals_);
  std::vector<std::pair<int, std::vector<T>>> vec1(nbMaxStrips_);
  for (int i = 0; i < nbMaxStrips_; ++i)
    vec1[i] = std::pair<int, std::vector<T>>(0, vec0);
  towMap.resize(maxNrTowers_);
  for (unsigned int i = 0; i < maxNrTowers_; ++i)
    towMap[i] = vec1;

  std::vector<int> vecint(maxNrSamples_);
  striptp_.resize(nbMaxStrips_);
  for (int i = 0; i < nbMaxStrips_; ++i)
    striptp_[i] = vecint;

  // // duplicate for odd filter 
  // std::vector<int> vecint(maxNrSamples_);
  // odd_striptp_.resize(nbMaxStrips_);
  // for (int i = 0; i < nbMaxStrips_; ++i)
  //   odd_striptp_[i] = vecint;  
  
}
