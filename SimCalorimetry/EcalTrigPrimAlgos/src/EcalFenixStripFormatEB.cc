#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <CondFormats/EcalObjects/interface/EcalTPGSlidingWindow.h>
#include <SimCalorimetry/EcalTrigPrimAlgos/interface/EcalFenixStripFormatEB.h>


EcalFenixStripFormatEB::EcalFenixStripFormatEB() : shift_(0) {}

EcalFenixStripFormatEB::~EcalFenixStripFormatEB() {}

int EcalFenixStripFormatEB::setInput(int input_even, int inputEvenPeak, int input_odd, int inputOddPeak, int inputsFGVB) {
  inputsFGVB_ = inputsFGVB;
  inputEvenPeak_ = inputEvenPeak;
  input_even_ = input_even;
  inputOddPeak_ = inputOddPeak;
  input_odd_ = input_odd;
  return 0;
}

int EcalFenixStripFormatEB::process() {
  int even_output = 0;
  int odd_output = 0;

  if (TPmode_.disable_EB_even_peak_finder){
    even_output = input_even_ >> shift_;
  }else{
    if (inputEvenPeak_ == 1) even_output = input_even_ >> shift_;
  }

  if (TPmode_.enable_EB_odd_peak_finder){
    if(inputOddPeak_ == 1) odd_output = input_odd_ >> shift_;
  }else{
    odd_output = input_odd_ >> shift_;
  }

  // Prepare the amplitude output for the strip looking at the TPmode options
  int output = 0;
  bool is_odd_larger = false;
  switch(TPmode_.EB_strip_formatter_output){
    case 0: // even filter out
      output = even_output;  
      break;
    case 1: // odd filter out
      if (TPmode_.enable_EB_odd_filter)  output = odd_output;
      else output = even_output;
      break;
    case 2: // larger between odd and even
      if (TPmode_.enable_EB_odd_filter && odd_output > even_output) {
        output = odd_output;
        is_odd_larger = true;
      }
      else output = even_output;
      break;
    case 3: // even + odd
      if (TPmode_.enable_EB_odd_filter) output = even_output + odd_output;
      else output = even_output;
      break;
  }

  if (output > 0XFFF)
    output = 0XFFF;  // ok: barrel saturates at 12 bits

  // Info bits
  // bit12 is sFGVB, bit13 is for odd>even flagging
  output |= ((inputsFGVB_ & 0x1) << 12);

  // if the flagging mode is OFF the bit stays 0, since it is not used for other things
  if (TPmode_.enable_EB_odd_filter && TPmode_.flag_EB_odd_even_strip ) {
    output |= ((is_odd_larger & 0x1) << 13);
  }

  return output;
}

void EcalFenixStripFormatEB::process(std::vector<int> &sFGVBout,
                                    std::vector<int> &peakout_even,
                                    std::vector<int> &filtout_even,
                                    std::vector<int> &peakout_odd,
                                    std::vector<int> &filtout_odd,
                                    std::vector<int> &output) {
  if (peakout_even.size() != filtout_even.size() || sFGVBout.size() != filtout_even.size() ||
      peakout_odd.size() != filtout_odd.size() || filtout_odd.size() != filtout_even.size()) {
    edm::LogWarning("EcalTPG") << " problem in EcalFenixStripFormatEB: sfgvb_out, peak_out and "
                                  "filt_out don't have the same size";
  }
  for (unsigned int i = 0; i < filtout_even.size(); i++) {
    setInput(filtout_even[i], peakout_even[i],filtout_odd[i], peakout_odd[i], sFGVBout[i]);
    output[i] = process();
  }
  return;
}

void EcalFenixStripFormatEB::setParameters(uint32_t &id, const EcalTPGSlidingWindow *&slWin,  EcalFenixTPMode TPmode) {
  // TP mode contains options for the formatter (odd/even filters config)
  TPmode_ = TPmode;
  const EcalTPGSlidingWindowMap &slwinmap = slWin->getMap();
  EcalTPGSlidingWindowMapIterator it = slwinmap.find(id);
  if (it != slwinmap.end())
    shift_ = (*it).second;
  else
    edm::LogWarning("EcalTPG") << " could not find EcalTPGSlidingWindowMap entry for " << id;
}
