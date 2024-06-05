//==============================================================================
//  SDR plugin wrapper for Fobos SDR API
//  V.T.
//  LGPL-2.1 or above LICENSE
//  05.06.2024
//==============================================================================

#include "SoapyFobosSDR.hpp"
#include <SoapySDR/Time.hpp>
#include <algorithm>
#include <cstring>

SoapyFobosSDR::SoapyFobosSDR(const SoapySDR::Kwargs &args):
    _device_index(0),
    _dev(nullptr),
    _sample_rate(25000000.0),
    _center_frequency(100000000.0),
    _direct_sampling(0),
    _lna_gain(0),
    _lna_gain_scale(1.0 / 16.0),
    _vga_gain(0),
    _vga_gain_scale(1.0 / 2.0),
    _rx_bufs(0),
    _rx_buffs_count(DEFAULT_BUFS_COUNT),
    _rx_buff_len(DEFAULT_BUFF_LEN),
    _rx_filled(0)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif    
    _lna_gain_scale = 1.0 / 16.0;
    _vga_gain_scale = 1.0 / 2.0;

    int result = 0;

    fobos_rx_get_api_info(lib_version, drv_version);
    printf("API Info lib: %s drv: %s\n", lib_version, drv_version); 

    if (args.count("label") != 0)
    {
        SoapySDR_logf(SOAPY_SDR_INFO, "Opening %s...", args.at("label").c_str());
    }

    //if (args.count("serial") == 0) throw std::runtime_error("No RTL-SDR devices found!");

    //const auto serial = args.at("serial");
    //deviceId = rtlsdr_get_index_by_serial(serial.c_str());
    //if (_deviceId < 0) throw std::runtime_error("rtlsdr_get_index_by_serial("+serial+") - " + std::to_string(_deviceId));

    _device_index = 0;
    SoapySDR_logf(SOAPY_SDR_DEBUG, "opening device #%d", _device_index);
    result = fobos_rx_open(&_dev, _device_index);
    if (result != 0) 
    {
        throw std::runtime_error("Unable to open Fobos SDR device");
    }

    result = fobos_rx_get_board_info(_dev, hw_revision, fw_version, manufacturer, product, serial);       
    if (result != 0) 
    {
        throw std::runtime_error("Unable to obtain devoce info");
    }
}

SoapyFobosSDR::~SoapyFobosSDR(void)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif       
    fobos_rx_close(_dev);
}

/*******************************************************************
 * Identification API
 ******************************************************************/

std::string SoapyFobosSDR::getDriverKey(void) const
{
    return "FobosSDR";
}

std::string SoapyFobosSDR::getHardwareKey(void) const
{
    return std::string(hw_revision);
}

SoapySDR::Kwargs SoapyFobosSDR::getHardwareInfo(void) const
{
    SoapySDR::Kwargs args;
    args["lib_version"] = std::string(lib_version);
    args["drv_version"] = std::string(drv_version);
    args["hw_revision"] = std::string(hw_revision);
    args["fw_version"] = std::string(fw_version);
    args["manufacturer"] = std::string(manufacturer);
    args["product"] = std::string(product);
    args["serial"] = std::string(serial);
    args["index"] = std::to_string(_device_index);
    return args;
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyFobosSDR::getNumChannels(const int dir) const
{
    return (dir == SOAPY_SDR_RX) ? 1 : 0;
}

bool SoapyFobosSDR::getFullDuplex(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyFobosSDR::listAntennas(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif      
    std::vector<std::string> antennas;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        antennas.push_back("RX");
    }
    return antennas;
}

void SoapyFobosSDR::setAntenna(const int direction, const size_t channel, const std::string &name)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %s)\n", __CLASS__, __FUNCTION__, direction, (int)channel, name.c_str());
#endif   
    if ((direction == SOAPY_SDR_RX) && (channel == 0) && (name == "RX"))
    {
    }
    else
    {
        throw std::runtime_error("setAntena() not supported");
    }
}

std::string SoapyFobosSDR::getAntenna(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif   
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        return "RX";
    }    
    return "";
}

/*******************************************************************
 * Frontend corrections API
 ******************************************************************/

bool SoapyFobosSDR::hasDCOffsetMode(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

bool SoapyFobosSDR::hasFrequencyCorrection(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyFobosSDR::listGains(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif  
    std::vector<std::string> results;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {    
        results.push_back("LNA");
        results.push_back("VGA");
    }
    return results;
}

bool SoapyFobosSDR::hasGainMode(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

void SoapyFobosSDR::setGain(const int direction, const size_t channel, const double value)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %f)\n", __CLASS__, __FUNCTION__, direction, (int)channel, value);
#endif  
    //set the overall gain by distributing it across available gain elements
    //OR delete this function to use SoapySDR's default gain distribution algorithm...
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        SoapySDR::Device::setGain(direction, channel, value);
    }
}

void SoapyFobosSDR::setGain(const int direction, const size_t channel, const std::string &name, const double value)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %s, %f)\n", __CLASS__, __FUNCTION__, direction, (int)channel, name.c_str(),  value);
#endif    
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        if (name == "LNA")
        {
            this->_lna_gain = value;
            unsigned int idx = (round(value * _lna_gain_scale)) + 1;
            fobos_rx_set_lna_gain(_dev, idx);
        }
        else if (name == "VGA")
        {
            this->_vga_gain = value;
            unsigned int idx = uint8_t(round(value * _vga_gain_scale));
            fobos_rx_set_vga_gain(_dev, idx);
        }
    }
}

double SoapyFobosSDR::getGain(const int direction, const size_t channel, const std::string &name) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);

#endif  
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        if (name == "LNA")
        {
            return this->_lna_gain;
        }
        if (name == "VGA")
        {
            return this->_vga_gain;
        }
    }
    return 0.0;
}

SoapySDR::Range SoapyFobosSDR::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %s)\n", __CLASS__, __FUNCTION__, direction, (int)channel, name.c_str());
#endif  
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        if (name == "LNA")
        {
            return SoapySDR::Range(0.0, 33.0);
        }
        if (name == "VGA")
        {
            return SoapySDR::Range(0.0, 30.0);
        }
    }
    return SoapySDR::Range(0, 0);
}
/******************************************************************************/
/******************************************************************************
 * Frequency API
 ******************************************************************************/
/******************************************************************************/
void SoapyFobosSDR::setFrequency(
        const int direction,
        const size_t channel,
        const std::string &name,
        const double frequency,
        const SoapySDR::Kwargs &args)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %s, %f)\n", __CLASS__, __FUNCTION__, direction, (int)channel, name.c_str(),  frequency);
#endif  
    (void)args;
    double actual;
    if ((direction == SOAPY_SDR_RX) && (channel == 0) && (name == "RF"))
    {
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting center freq: %f", frequency);
        int r = fobos_rx_set_frequency(_dev, frequency, &actual);
        if (r != 0)
        {
            throw std::runtime_error("setFrequency failed");
        }
        _center_frequency = actual;
    }
}
/******************************************************************************/
double SoapyFobosSDR::getFrequency(const int direction, const size_t channel, const std::string &name) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %s)\n", __CLASS__, __FUNCTION__, direction, (int)channel, name.c_str());
#endif     
    if ((direction == SOAPY_SDR_RX) && (channel == 0) && (name == "RF"))
    {
        return _center_frequency;
    }
    return 0.0;
}
/******************************************************************************/
std::vector<std::string> SoapyFobosSDR::listFrequencies(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif     
    std::vector<std::string> names;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        names.push_back("RF");
    }
    return names;
}
/******************************************************************************/
SoapySDR::RangeList SoapyFobosSDR::getFrequencyRange(
        const int direction,
        const size_t channel,
        const std::string &name) const
{
    SoapySDR::RangeList results;
    if ((direction == SOAPY_SDR_RX) && (channel == 0) && (name == "RF"))
    {
        results.push_back(SoapySDR::Range(50E6, 6000E6));
    }
    return results;
}
/******************************************************************************/
SoapySDR::ArgInfoList SoapyFobosSDR::getFrequencyArgsInfo(const int direction, const size_t channel) const
{
    SoapySDR::ArgInfoList freqArgs;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        // TODO: frequency arguments
        return freqArgs;
    }
    return freqArgs;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

void SoapyFobosSDR::setSampleRate(const int direction, const size_t channel, const double rate)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d, %f)\n", __CLASS__, __FUNCTION__, direction, (int)channel, rate);
#endif  
    SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting sample rate: %f", rate);
    double actual;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        int r = fobos_rx_set_samplerate(_dev, rate, &actual);
        if (r == 0)
        {
            _sample_rate = actual;
            SoapySDR_logf(SOAPY_SDR_DEBUG, "actual: %f", actual);
        }
        else
        {
            SoapySDR_logf(SOAPY_SDR_DEBUG, "falied, err#: %d", r);
        }
    }
}

double SoapyFobosSDR::getSampleRate(const int direction, const size_t channel) const
{
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        return _sample_rate;
    }
    return 0.0;
}

std::vector<double> SoapyFobosSDR::listSampleRates(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif      
    unsigned int count;
    std::vector<double> rates;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        int r = fobos_rx_get_samplerates(_dev, 0, &count);
        if ((r == 0) && (count > 0))
        {
            rates.resize(count);
            fobos_rx_get_samplerates(_dev, rates.data(), &count);
            if (rates[0]>rates[count-1])
            {
                std::reverse(rates.begin(), rates.end());
            }
        }
    }
    return rates;
}

SoapySDR::RangeList SoapyFobosSDR::getSampleRateRange(const int direction, const size_t channel) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif      
    SoapySDR::RangeList results;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        std::vector<double> rates;
        unsigned int count;
        int r = fobos_rx_get_samplerates(_dev, 0, &count);
        if ((r == 0) && (count > 0))
        {
            rates.resize(count);
            fobos_rx_get_samplerates(_dev, rates.data(), &count);
            if (rates[0] > rates[count - 1])
            {
                results.push_back(SoapySDR::Range(rates[count - 1], rates[0]));    
            }
            else
            {
                results.push_back(SoapySDR::Range(rates[0], rates[count - 1]));
            }
        }
    }
    return results;
}

/*******************************************************************
 * Settings API
 ******************************************************************/

SoapySDR::ArgInfoList SoapyFobosSDR::getSettingInfo(void) const
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif      
    SoapySDR::ArgInfoList args;
    {
        SoapySDR::ArgInfo info;
        info.key = "direct_samp";
        info.value = "0";
        info.name = "Direct Sampling";
        info.description = " HF1/HF2 Direct Sampling Mode";
        info.type = SoapySDR::ArgInfo::STRING;
        info.options.push_back("0");
        info.optionNames.push_back("Off");
        info.options.push_back("1");
        info.optionNames.push_back("On");
        args.push_back(info);
    }
    return args;
}

void SoapyFobosSDR::writeSetting(const std::string &key, const std::string &value)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif      
    if (key == "direct_samp")
    {
        try
        {
            _direct_sampling = std::stoi(value);
        }
        catch (const std::invalid_argument &) 
        {
            SoapySDR_logf(SOAPY_SDR_ERROR, "Invalid direct sampling mode '%s', [0:Off, 1:On]", value.c_str());
            _direct_sampling = 0;
        }
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Direct sampling mode: %d", _direct_sampling);
        fobos_rx_set_direct_sampling(_dev, _direct_sampling);
    }
}

std::string SoapyFobosSDR::readSetting(const std::string &key) const
{
    if (key == "direct_samp") 
    {
        return std::to_string(_direct_sampling);
    }
    return "";
}


