#include "CondFormats/EcalObjects/interface/EcalTPGTPMode.h"
#include <iostream>

EcalTPGTPMode::EcalTPGTPMode() {}

EcalTPGTPMode::~EcalTPGTPMode() {}

void EcalTPGTPMode::Print() const{
    std::cout << ">>> Trigger primitive mode:  " << std::endl;
    std::cout << "    enable EE odd filter  "<< EnableEEOddFilter << std::endl;
    std::cout << "    enable EB odd filter  "<< EnableEBOddFilter << std::endl;
    std::cout << "    enable EE odd peak finder  "<< EnableEEOddPeakFinder << std::endl;
    std::cout << "    enable EB odd peak finder  "<< EnableEBOddPeakFinder << std::endl;
    std::cout << "    disable EE even peak finder  "<< DisableEEEvenPeakFinder << std::endl;
    std::cout << "    disable EB even peak finder  "<< DisableEBEvenPeakFinder << std::endl;
    if (FenixEEStripOutput == 0) std::cout << "    EE strip formatter output: even filter " << std::endl;
    if (FenixEEStripOutput == 1) std::cout << "    EE strip formatter output: odd filter " << std::endl;
    if (FenixEEStripOutput == 2) std::cout << "    EE strip formatter output: larger of odd and even " << std::endl;
    if (FenixEEStripOutput == 3) std::cout << "    EE strip formatter output: odd + even " << std::endl;
    if (FenixEBStripOutput == 0) std::cout << "    EB strip formatter output: even filter " << std::endl;
    if (FenixEBStripOutput == 1) std::cout << "    EB strip formatter output: odd filter " << std::endl;
    if (FenixEBStripOutput == 2) std::cout << "    EB strip formatter output: larger of odd and even " << std::endl;
    if (FenixEBStripOutput == 3) std::cout << "    EB strip formatter output: odd + even " << std::endl;
    std::cout << "    Flag EE odd>even strip  "<< FenixEEStripInfobit2 << std::endl;
    std::cout << "    Flag EB odd>even strip  "<< FenixEBStripInfobit2 << std::endl;
    if (EBFenixTcpOutput == 0) std::cout << "    EB tcp formatter output: even filter " << std::endl;
    if (EBFenixTcpOutput == 1) std::cout << "    EB tcp formatter output: larger of odd and even " << std::endl;
    if (EBFenixTcpOutput == 2) std::cout << "    EB tcp formatter output: even + odd " << std::endl;
    std::cout << "    Flag EB odd>even TCP  "<< EBFenixTcpInfobit1 << std::endl;
}