#include <SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStrip.h>
#include <SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStripFgvbEE.h>
#include <SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStripFormatEB.h>
#include <SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStripFormatEE.h>

#include "Geometry/EcalMapping/interface/EcalElectronicsMapping.h"

#include <DataFormats/EcalDigi/interface/EcalTriggerPrimitiveSample.h>
#include <string>
#include <bitset>

//-------------------------------------------------------------------------------------
EcalFenixStrip::EcalFenixStrip(const edm::EventSetup &setup,
                               const EcalElectronicsMapping *theMapping,
                               bool debug,
                               bool famos,
                               int maxNrSamples,
                               int nbMaxXtals,
                               bool TPinfoPrintout,
                               uint TPmode)
    : theMapping_(theMapping), debug_(debug), famos_(famos), nbMaxXtals_(nbMaxXtals), TPmode_(TPmode) {
  linearizer_.resize(nbMaxXtals_);
  for (int i = 0; i < nbMaxXtals_; i++)
    linearizer_[i] = new EcalFenixLinearizer(famos_);
  adder_ = new EcalFenixEtStrip();
  amplitude_filter_ = new EcalFenixAmplitudeFilter(TPinfoPrintout);
  oddAmplitude_filter_ = new EcalFenixOddAmplitudeFilter(TPinfoPrintout); 
  peak_finder_ = new EcalFenixPeakFinder();
  fenixFormatterEB_ = new EcalFenixStripFormatEB();
  fenixFormatterEE_ = new EcalFenixStripFormatEE();
  fgvbEE_ = new EcalFenixStripFgvbEE();
  
  // prepare data storage for all events
  std::vector<int> v;
  v.resize(maxNrSamples);
  lin_out_.resize(nbMaxXtals_);
  for (int i = 0; i < 5; i++)
    lin_out_[i] = v;
  add_out_.resize(maxNrSamples);

  even_filt_out_.resize(maxNrSamples);
  even_peak_out_.resize(maxNrSamples);
  odd_filt_out_.resize(maxNrSamples); 
  odd_peak_out_.resize(maxNrSamples);

  format_out_.resize(maxNrSamples);
  fgvb_out_.resize(maxNrSamples);
  fgvb_out_temp_.resize(maxNrSamples);

  // Readout TP mode options
  if (TPmode & 0x1) use_odd_filter_ = true;
  if ( (TPmode > 1) & 0x1) use_odd_peak_finder_ = true;
  if ( (TPmode > 2) & 0x1) disable_even_peak_finder_ = true;

}

//-------------------------------------------------------------------------------------
EcalFenixStrip::~EcalFenixStrip() {
  for (int i = 0; i < nbMaxXtals_; i++)
    delete linearizer_[i];
  delete adder_;
  delete amplitude_filter_;
  delete oddAmplitude_filter_; 
  delete peak_finder_;
  delete fenixFormatterEB_;
  delete fenixFormatterEE_;
  delete fgvbEE_;
}

// void process(const edm::EventSetup &setup, std::vector<EBDataFrame> &samples, int nrXtals, std::vector<int> &out, bool OddFilter) {
void EcalFenixStrip::process(const edm::EventSetup &setup, std::vector<EBDataFrame> &samples, int nrXtals, std::vector<int> &out) {
  
  // now call processing
  if (samples.empty()) {
    std::cout << " Warning: 0 size vector found in EcalFenixStripProcess!!!!!" << std::endl;
    return;
  }
  const EcalTriggerElectronicsId elId = theMapping_->getTriggerElectronicsId(samples[0].id());
  uint32_t stripid = elId.rawId() & 0xfffffff8;  // from Pascal

  // by RK 
  if (false){
  const EBDetId & id = samples[0].id();
  const EcalTrigTowerDetId towid = id.tower();
  for (int cryst = 0; cryst < nrXtals; cryst++) {
    //" EcalFenixStrip.h::stripid nsamples, tower eta, phi, xtal eta phi, 10 samples:  "
    std::cout<<stripid<<" "<<samples.size()<<" "<<towid.ieta()<<" "<<towid.iphi()<<"  "<<id.iphi()<<" "<<id.ieta(); // add to TPinfo 
    for (int i = 0; i < samples[cryst].size(); i++) {
      std::cout<< " " << std::dec << samples[cryst][i].adc();
    }
    std::cout<<std::endl;
  }
}

  identif_ = getFGVB()->getMissedStripFlag();

  process_part1(identif_,
                samples,
                nrXtals,
                stripid,
                ecaltpPed_,
                ecaltpLin_,
                ecaltpgWeightMap_,
                ecaltpgWeightGroup_,
                ecaltpgOddWeightMap_,
                ecaltpgOddWeightGroup_,
                ecaltpgBadX_);  // templated part
  process_part2_barrel(stripid, ecaltpgSlidW_,
                        ecaltpgFgStripEE_);  // part different for barrel/endcap
  out = format_out_; 
  
}

// void process(const edm::EventSetup &setup, std::vector<EEDataFrame> &samples, int nrXtals, std::vector<int> &out, bool OddFilter) {
void EcalFenixStrip::process(const edm::EventSetup &setup, std::vector<EEDataFrame> &samples, int nrXtals, std::vector<int> &out) { 
  // now call processing
  if (samples.empty()) {
    std::cout << " Warning: 0 size vector found in EcalFenixStripProcess!!!!!" << std::endl;
    return;
  }
  const EcalTriggerElectronicsId elId = theMapping_->getTriggerElectronicsId(samples[0].id());
  uint32_t stripid = elId.rawId() & 0xfffffff8;  // from Pascal

  identif_ = getFGVB()->getMissedStripFlag();

  process_part1(identif_,
                samples,
                nrXtals,
                stripid,
                ecaltpPed_,
                ecaltpLin_,
                ecaltpgWeightMap_,
                ecaltpgWeightGroup_,
                ecaltpgOddWeightMap_,
                ecaltpgOddWeightGroup_,
                ecaltpgBadX_);  // templated part
  process_part2_endcap(stripid, ecaltpgSlidW_, ecaltpgFgStripEE_, ecaltpgStripStatus_);
  out = format_out_; // FIXME: timing
  // if(OddFilter) out = odd_format_out_;
  // else out = format_out_; // FIXME: timing
  return;
}

/*
* strip level processing - part1. 
* The linearizer and adder are run only once. 
* Then the even and odd filters and peak finder are run looking at the TPmode fla
*/
template <class T>
void EcalFenixStrip::process_part1(int identif,
                    std::vector<T> &df,
                    int nrXtals,
                    uint32_t stripid,
                    const EcalTPGPedestals *ecaltpPed,
                    const EcalTPGLinearizationConst *ecaltpLin,
                    const EcalTPGWeightIdMap *ecaltpgWeightMap,
                    const EcalTPGWeightGroup *ecaltpgWeightGroup,
                    const EcalTPGOddWeightIdMap *ecaltpgOddWeightMap,
                    const EcalTPGOddWeightGroup *ecaltpgOddWeightGroup,
                    const EcalTPGCrystalStatus *ecaltpBadX) {
  if (debug_)  {
     std::cout << "\n\nEcalFenixStrip input is a vector of size: " << nrXtals << std::endl;
     std::cout << "Trigger Primitive mode: " << std::bitset<9>(TPmode_).to_string() << std::endl;
  } 

  // loop over crystals
  for (int cryst = 0; cryst < nrXtals; cryst++) {
    
    if (debug_) {
      std::cout << std::endl;
      std::cout << "cryst= " << cryst << " EBDataFrame/EEDataFrame is: " << std::endl;
      for (int i = 0; i < df[cryst].size(); i++) {
        std::cout << " " << std::dec << df[cryst][i].adc();
      }
      std::cout << std::endl;
    }
    // call linearizer
    this->getLinearizer(cryst)->setParameters(df[cryst].id().rawId(), ecaltpPed, ecaltpLin, ecaltpBadX);
    this->getLinearizer(cryst)->process(df[cryst], lin_out_[cryst]);
    
  }
  
  if (debug_) {
    std::cout << "output of linearizer is a vector of size: " << std::dec << lin_out_.size() << " of which used "
              << nrXtals << std::endl;
    for (int ix = 0; ix < nrXtals; ix++) {
      std::cout << "cryst: " << ix << "  value : " << std::dec << std::endl;
      std::cout << " lin_out[ix].size()= " << std::dec << lin_out_[ix].size() << std::endl;
      for (unsigned int i = 0; i < lin_out_[ix].size(); i++) {
        std::cout << " " << std::dec << (lin_out_[ix])[i];
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  // Now call the sFGVB - this is common between EB and EE!
  getFGVB()->setParameters(identif, stripid, ecaltpgFgStripEE_);
  getFGVB()->process(lin_out_, fgvb_out_temp_);

  if (debug_) {
    std::cout << "output of strip fgvb is a vector of size: " << std::dec << fgvb_out_temp_.size() << std::endl;
    for (unsigned int i = 0; i < fgvb_out_temp_.size(); i++) {
      std::cout << " " << std::dec << (fgvb_out_temp_[i]);
    }
    std::cout << std::endl;
  }
  // call adder
  this->getAdder()->process(lin_out_, nrXtals, add_out_);  // add_out is of size SIZEMAX=maxNrSamples

  if (debug_) {
    std::cout << "output of adder is a vector of size: " << std::dec << add_out_.size() << std::endl;
    for (unsigned int ix = 0; ix < add_out_.size(); ix++) {
      std::cout << "Clock: " << ix << "  value : " << std::dec << add_out_[ix] << std::endl;
    }
    std::cout << std::endl;
  }
  
  if (famos_) {
    even_filt_out_[0] = add_out_[0];
    even_peak_out_[0] = add_out_[0];
    return;
  } else {

    // This is where the amplitude filters are called 
    // the TPmode flag will determine which are called and if the peak finder is called.
    // Call even amplitude filter 
    this->getEvenFilter()->setParameters(stripid, ecaltpgWeightMap, ecaltpgWeightGroup);
    this->getEvenFilter()->process(add_out_, even_filt_out_, fgvb_out_temp_, fgvb_out_);

    // Print out even filter ET and sfgvb values 
    if(debug_){
      std::cout << "output of EVEN filter is a vector of size: " << std::dec << even_filt_out_.size() << std::endl;
      for (unsigned int ix = 0; ix < even_filt_out_.size(); ix++) {
        std::cout << "Clock: " << ix << "  value : " << std::dec << even_filt_out_[ix] << std::endl;
      }
      std::cout << std::endl;    
      std::cout << "output of EVEN sfgvb after filter is a vector of size: " << std::dec << fgvb_out_.size() << std::endl;
      for (unsigned int ix = 0; ix < fgvb_out_.size(); ix++) {
        std::cout << "Clock: " << ix << "  value : " << std::dec << fgvb_out_[ix] << std::endl;
      }
      std::cout << std::endl;            
    }


    // Call peak finder on even filter output 
    this->getPeakFinder()->process(even_filt_out_, even_peak_out_);   

    // Print out even filter peak finder values 
    if(debug_){
      std::cout << "output of EVEN peakfinder is a vector of size: " << even_peak_out_.size() << std::endl;
      for (unsigned int ix = 0; ix < even_peak_out_.size(); ix++) {
        std::cout << "Clock: " << ix << "  value : " << even_peak_out_[ix] << std::endl;
      }
      std::cout << std::endl;               
    }     

    if (use_odd_filter_){
      this->getOddFilter()->setParameters(stripid, ecaltpgOddWeightMap, ecaltpgOddWeightGroup);
      this->getOddFilter()->process(add_out_, odd_filt_out_);

      // Print out odd filter ET 
      if(debug_){
        std::cout << "output of ODD filter is a vector of size: " << std::dec << odd_filt_out_.size() << std::endl;
        for (unsigned int ix = 0; ix < odd_filt_out_.size(); ix++) {
          std::cout << "Clock: " << ix << "  value : " << std::dec << odd_filt_out_[ix] << std::endl;
        }
        std::cout << std::endl;            
      }

    }
      
    // else if(TPmode_ == "Odd"){

    //   // loop over crystals
    //   for (int cryst = 0; cryst < nrXtals; cryst++) {
        
    //     // if (debug_) {
    //     //   std::cout << std::endl;
    //     //   std::cout << "cryst= " << cryst << " EBDataFrame/EEDataFrame is: " << std::endl;
    //     //   for (int i = 0; i < df[cryst].size(); i++) {
    //     //     std::cout << " " << std::dec << df[cryst][i].adc();
    //     //   }
    //     //   std::cout << std::endl;
    //     // }
    //     // call linearizer
    //     this->getLinearizer(cryst)->setParameters(df[cryst].id().rawId(), ecaltpPed, ecaltpLin, ecaltpBadX);
    //     this->getLinearizer(cryst)->process(df[cryst], odd_lin_out_[cryst]);
        
    //   }

    //   getFGVB()->process(odd_lin_out_, odd_fgvb_out_temp_);
    //   this->getAdder()->process(odd_lin_out_, nrXtals, odd_add_out_);  // for odd filter input 

    //   // Call odd amplitude filter 
    //   this->getOddFilter()->setParameters(stripid, ecaltpgOddWeightMap, ecaltpgOddWeightGroup);
    //   this->getOddFilter()->process(add_out_, odd_filt_out_, odd_fgvb_out_temp_, odd_fgvb_out_);      
    //   // this->getOddFilter()->process(odd_add_out_, odd_filt_out_, odd_fgvb_out_temp_, odd_fgvb_out_);    

    //   // Print out odd filter ET and sfgvb values 
    //   if(debug_){
    //     std::cout << "output of ODD filter is a vector of size: " << std::dec << odd_filt_out_.size() << std::endl;
    //     for (unsigned int ix = 0; ix < odd_filt_out_.size(); ix++) {
    //       std::cout << "Clock: " << ix << "  value : " << std::dec << odd_filt_out_[ix] << std::endl;
    //     }
    //     std::cout << std::endl; 

    //     std::cout << "output of ODD sfgvb after filter is a vector of size: " << std::dec << odd_fgvb_out_.size() << std::endl;
    //     for (unsigned int ix = 0; ix < odd_fgvb_out_.size(); ix++) {
    //       std::cout << "Clock: " << ix << "  value : " << std::dec << odd_fgvb_out_[ix] << std::endl;
    //     }
    //     std::cout << std::endl;
    //   }        

    //   // Call peak finder on odd filter output 
    //   this->getPeakFinder()->process(odd_filt_out_, odd_event_peak_out_);        
        
    //   // Print out odd filter peak finder values 
    //   if (debug_) {
    //     std::cout << "output of ODD peakfinder is a vector of size: " << odd_event_peak_out_.size() << std::endl;
    //     for (unsigned int ix = 0; ix < odd_event_peak_out_.size(); ix++) {
    //       std::cout << "Clock: " << ix << "  value : " << odd_event_peak_out_[ix] << std::endl;
    //     }
    //     std::cout << std::endl;        
    //   }

    // }
    return;
  }
}

//----------------------------------------------------------------------------------
void EcalFenixStrip::process_part2_barrel(uint32_t stripid,
                                          const EcalTPGSlidingWindow *ecaltpgSlidW,
                                          const EcalTPGFineGrainStripEE *ecaltpgFgStripEE) {
  // call  Fgvb
  // this->getFGVB()->setParameters(stripid,ecaltpgFgStripEE);
  // this->getFGVB()->process(lin_out_,fgvb_out_);

  // call formatter
  this->getFormatterEB()->setParameters(stripid, ecaltpgSlidW);

  this->getFormatterEB()->process(fgvb_out_, even_peak_out_, even_filt_out_, format_out_);

  // Duplicate for Odd filter 
  // this->getFormatterEB()->process(odd_fgvb_out_, odd_event_peak_out_, odd_filt_out_, odd_format_out_);

  // this is a test:
  // if (debug_) {
  //   std::cout << "output of formatter is a vector of size: " << format_out_.size() << std::endl;
  //   std::cout << "value : " << std::endl;
  //   for (unsigned int i = 0; i < format_out_.size(); i++) {
  //     std::cout << " " << format_out_[i];
  //   }
  //   std::cout << std::endl;
  // }
  return;
}
//-------------------------------------------------------------------------------------
void EcalFenixStrip::process_part2_endcap(uint32_t stripid,
                                          const EcalTPGSlidingWindow *ecaltpgSlidW,
                                          const EcalTPGFineGrainStripEE *ecaltpgFgStripEE,
                                          const EcalTPGStripStatus *ecaltpgStripStatus) {
  // call  Fgvb
  // this->getFGVB()->setParameters(stripid,ecaltpgFgStripEE);
  // this->getFGVB()->process(lin_out_,fgvb_out_);

  // call formatter
  this->getFormatterEE()->setParameters(stripid, ecaltpgSlidW, ecaltpgStripStatus);
  this->getFormatterEE()->process(fgvb_out_, even_peak_out_, even_filt_out_, format_out_);

  // Duplicate for odd filter 
  // this->getFormatterEE()->process(odd_fgvb_out_, odd_event_peak_out_, odd_filt_out_, odd_format_out_);

  // this is a test:
  // if (debug_) {
  //   std::cout << "output of formatter is a vector of size: " << format_out_.size() << std::endl;
  //   std::cout << "value = " << std::endl;  // DP FORMATTED
  //   for (unsigned int i = 0; i < format_out_.size(); i++) {
  //     std::cout << " " << std::dec << format_out_[i] << std::endl;   // DP FORMATTED
  //     if (format_out_[i]>100) mydebug_=true; // DP ADDED
  //   }
  //   std::cout << std::endl;
  //   if (mydebug_) std::cout << "MYDEBUG=TRUE " <<a<< std::endl;  // DP ADDED
  // }

  return;
}
