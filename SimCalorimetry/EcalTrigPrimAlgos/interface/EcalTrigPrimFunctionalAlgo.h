#ifndef EcalTrigPrimFunctionalAlgo_h
#define EcalTrigPrimFunctionalAlgo_h
/** \class EcalTrigPrimFunctionalAlgo
 *
 * EcalTrigPrimFunctionalAlgo is the main algorithm class for TPG
 * It coordinates all the aother algorithms
 * Structure is as close as possible to electronics
 *
 *
 * \author Ursula Berthon, Stephanie Baffioni, LLR Palaiseau
 *
 * \version   1st Version may 2006
 * \version   2nd Version jul 2006
 * \version   3rd Version sep 2007  introducing new Records closer to the db

 *
 ************************************************************/
#include <iostream>
#include <sys/time.h>
#include <vector>

#include "Geometry/CaloTopology/interface/EcalTrigTowerConstituentsMap.h"

#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStrip.h"
#include "SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixTcp.h"

#include "DataFormats/Common/interface/SortedCollection.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <map>
#include <utility>
#include <string>

/** Main Algo for Ecal trigger primitives. */

class EcalTrigTowerDetId;
class ETPCoherenceTest;
class EcalTriggerPrimitiveSample;
class CaloSubdetectorGeometry;
class EBDataFrame;
class EEDataFrame;
class EcalElectronicsMapping;

class EcalTrigPrimFunctionalAlgo {
public:
  explicit EcalTrigPrimFunctionalAlgo(
      const edm::EventSetup &setup, int binofmax, bool tcpFormat, bool barrelOnly, bool debug, bool famos, bool TPinfoPrintout);

  virtual ~EcalTrigPrimFunctionalAlgo();

  void run(const edm::EventSetup &,
           const EBDigiCollection *col,
           EcalTrigPrimDigiCollection &result,
           EcalTrigPrimDigiCollection &resultTcp);
  void run(const edm::EventSetup &,
           const EEDigiCollection *col,
           EcalTrigPrimDigiCollection &result,
           EcalTrigPrimDigiCollection &resultTcp);
  void run_part1_EB(EBDigiCollection const *col);
  void run_part1_EE(EEDigiCollection const *col);
  template <class Coll>
  void run_part2(const edm::EventSetup &,
                 Coll const *col,
                 std::vector<std::vector<std::pair<int, std::vector<typename Coll::Digi>>>> &towerMap,
                 EcalTrigPrimDigiCollection &result,
                 EcalTrigPrimDigiCollection &resultTcp);

  void setPointers(const EcalTPGLinearizationConst *ecaltpLin,
                   const EcalTPGPedestals *ecaltpPed,
                   const EcalTPGSlidingWindow *ecaltpgSlidW,
                   const EcalTPGWeightIdMap *ecaltpgWeightMap,
                   const EcalTPGWeightGroup *ecaltpgWeightGroup,
                   const EcalTPGOddWeightIdMap *ecaltpgOddWeightMap,
                   const EcalTPGOddWeightGroup *ecaltpgOddWeightGroup,
                   const EcalTPGFineGrainStripEE *ecaltpgFgStripEE,
                   const EcalTPGCrystalStatus *ecaltpgBadX,
                   const EcalTPGStripStatus *ecaltpgStripStatus,
                   const EcalTPGTPMode * ecaltpgTPMode) {
    estrip_->setPointers(ecaltpPed,
                         ecaltpLin,
                         ecaltpgWeightMap,
                         ecaltpgWeightGroup,
                         ecaltpgOddWeightMap,
                         ecaltpgOddWeightGroup,
                         ecaltpgSlidW,
                         ecaltpgFgStripEE,
                         ecaltpgBadX,
                         ecaltpgStripStatus,
                         ecaltpgTPMode);
  }
  void setPointers2(const EcalTPGFineGrainEBGroup *ecaltpgFgEBGroup,
                    const EcalTPGLutGroup *ecaltpgLutGroup,
                    const EcalTPGLutIdMap *ecaltpgLut,
                    const EcalTPGFineGrainEBIdMap *ecaltpgFineGrainEB,
                    const EcalTPGFineGrainTowerEE *ecaltpgFineGrainTowerEE,
                    const EcalTPGTowerStatus *ecaltpgBadTT,
                    const EcalTPGSpike *ecaltpgSpike,
                    const EcalTPGTPMode * ecaltpgTPMode) {
    etcp_->setPointers(ecaltpgFgEBGroup,
                       ecaltpgLutGroup,
                       ecaltpgLut,
                       ecaltpgFineGrainEB,
                       ecaltpgFineGrainTowerEE,
                       ecaltpgBadTT,
                       ecaltpgSpike,
                       ecaltpgTPMode);
  }

private:
  void init(const edm::EventSetup &);
  template <class T>
  void initStructures(std::vector<std::vector<std::pair<int, std::vector<T>>>> &towMap);
  template <class T>
  void clean(std::vector<std::vector<std::pair<int, std::vector<T>>>> &towerMap);
  template <class Coll>
  void fillMap(Coll const *col, std::vector<std::vector<std::pair<int, std::vector<typename Coll::Digi>>>> &towerMap);
  int findStripNr(const EBDetId &id);
  int findStripNr(const EEDetId &id);

  // FIXME: temporary until hashedIndex works alsom for endcap
  int getIndex(const EBDigiCollection *, EcalTrigTowerDetId &id) { return id.hashedIndex(); }
  // mind that eta is continuous between barrel+endcap
  int getIndex(const EEDigiCollection *, EcalTrigTowerDetId &id) {
    int ind = (id.ietaAbs() - 18) * 72 + id.iphi();
    if (id.zside() < 0)
      ind += 792;
    return ind;
  }

  EcalFenixStrip *estrip_; 
  EcalFenixTcp *etcp_;

  edm::ESHandle<EcalTrigTowerConstituentsMap> eTTmap_;
  const CaloSubdetectorGeometry *theEndcapGeometry;
  const EcalElectronicsMapping *theMapping_;

  float threshold;

  int binOfMaximum_;
  int maxNrSamples_;

  bool tcpFormat_;
  bool barrelOnly_;
  bool debug_;
  bool famos_; 
  bool TPinfoPrintout_; 

  static const unsigned int nrSamples_;        // nr samples to write, should not be changed since by
                                               // convention the size means that it is coming from simulation
  static const unsigned int maxNrSamplesOut_;  // to be placed in the intermediate samples
  static const unsigned int maxNrTowers_;      // would be better to get from somewhere..
  static const unsigned int maxNrTPs_;         // would be better to get from
                                               // somewhere..

  int nrTowers_;  // nr of towers found by fillmap method

  // data structures kept during the whole run
  std::vector<std::vector<int>> striptp_;
  // std::vector<std::vector<int>> odd_striptp_; // duplicate data path for odd filter 
  std::vector<std::vector<std::pair<int, std::vector<EBDataFrame>>>> towerMapEB_;
  std::vector<std::vector<std::pair<int, std::vector<EEDataFrame>>>> towerMapEE_;
  std::vector<std::pair<int, EcalTrigTowerDetId>> hitTowers_;
  std::vector<EcalTriggerPrimitiveSample> towtp_;
  std::vector<EcalTriggerPrimitiveSample> towtp2_;

  enum { nbMaxStrips_ = 5 };
  enum { nbMaxXtals_ = 5 };
};



#endif
