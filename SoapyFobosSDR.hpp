//==============================================================================
//  SDR plugin wrapper for Fobos SDR API
//  V.T.
//  LGPL-2.1 or above LICENSE
//  05.06.2024
//==============================================================================

#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>
#include <fobos.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
// uncomment to bisplay debug info
//#define SOAPY_FOBOS_PRINT_DEBUG
#define DEFAULT_BUFF_LEN (128 * 1024)
#define DEFAULT_BUFS_COUNT 16
//==============================================================================
class SoapyFobosSDR: public SoapySDR::Device
{
public:
    SoapyFobosSDR(const SoapySDR::Kwargs &args);

    ~SoapyFobosSDR(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

    /*******************************************************************
     * Channels API
     ******************************************************************/

    size_t getNumChannels(const int) const;

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/

    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels =
            std::vector<size_t>(), const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream *stream);

    size_t getStreamMTU(SoapySDR::Stream *stream) const;

    int activateStream(
            SoapySDR::Stream *stream,
            const int flags = 0,
            const long long timeNs = 0,
            const size_t numElems = 0);

    int deactivateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0);

    int readStream(
            SoapySDR::Stream *stream,
            void * const *buffs,
            const size_t numElems,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000);

    /*******************************************************************
     * Antenna API
     ******************************************************************/

    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    void setAntenna(const int direction, const size_t channel, const std::string &name);

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/

    bool hasDCOffsetMode(const int direction, const size_t channel) const;

    bool hasFrequencyCorrection(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/

    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    bool hasGainMode(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const double value);

    void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    double getGain(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/

    void setFrequency(
            const int direction,
            const size_t channel,
            const std::string &name,
            const double frequency,
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/

    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Settings API
     ******************************************************************/

    SoapySDR::ArgInfoList getSettingInfo(void) const;

    void writeSetting(const std::string &key, const std::string &value);

    std::string readSetting(const std::string &key) const;

private:
    const char * __CLASS__ = "SoapyFobosSDR"; 
    //device handle
    int _device_index;
    fobos_dev_t *_dev;

    // device info
    char lib_version[32];
    char drv_version[32];  
    char hw_revision[32];
    char fw_version[32];
    char manufacturer[32];
    char product[32];
    char serial[32]; 

    //cached settings
    double _sample_rate;
    double _center_frequency;
    int _direct_sampling;
    double _lna_gain;
    double _lna_gain_scale;
    double _vga_gain;
    double _vga_gain_scale;

    //async api usage
    std::thread _rx_async_thread;
    void rx_async_thread_loop(void);

    uint32_t _buff_counter;
    bool _running;
    std::mutex _rx_mutex;
    std::condition_variable _rx_cond;
    float** _rx_bufs;
    size_t _rx_buffs_count;
    size_t _rx_buff_len;
    size_t _rx_filled;
    size_t _rx_idx_w;
    size_t _rx_pos_w;
    size_t _rx_idx_r;
    size_t _rx_pos_r;
    uint32_t _overruns_count;

public:
    void read_samples(float* buf, uint32_t buf_length);

};
//==============================================================================