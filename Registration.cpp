//==============================================================================
//  SDR plugin wrapper for Fobos SDR API
//  V.T.
//  LGPL-2.1 or above LICENSE
//  05.06.2024
//==============================================================================

#include "SoapyFobosSDR.hpp"
#include <SoapySDR/Registry.hpp>
#include <string.h>
#include <mutex>
#include <map>


static std::vector<SoapySDR::Kwargs> findSDR(const SoapySDR::Kwargs &args)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif
    std::vector<SoapySDR::Kwargs> results;
    char serials[256] = {0};
    int count = fobos_rx_list_devices(serials);
#ifdef SOAPY_FOBOS_PRINT_DEBUG
    printf("[%d], %s\n", count, serials);
#endif
    char* pserial = strtok(serials, " ");
    SoapySDR_logf(SOAPY_SDR_DEBUG, "found devices: %d\n", count);
    for (int index = 0; index < count; index++)
    {
        SoapySDR_logf(SOAPY_SDR_DEBUG, "  dev# %i  %s\n", index, pserial);
        SoapySDR::Kwargs devInfo;
        devInfo["label"] = "Fobos SDR";
        devInfo["serial"] = pserial;
        devInfo["manufacturer"] = "RigExpert";
        pserial = strtok(0, " ");
        if (args.count("serial") != 0) 
        {
            if (args.at("serial") != devInfo.at("serial")) 
            {
                continue;
            }
            SoapySDR_logf(SOAPY_SDR_DEBUG, "Found device by serial %s", devInfo.at("serial").c_str());
        }
        
        results.push_back(devInfo);
    }
    return results;
}

static SoapySDR::Device *makeSDR(const SoapySDR::Kwargs &args)
{
    return new SoapyFobosSDR(args);
}

static SoapySDR::Registry registerFobosSDR("fobos", &findSDR, &makeSDR, SOAPY_SDR_ABI_VERSION);
