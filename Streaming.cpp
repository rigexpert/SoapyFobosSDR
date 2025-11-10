//==============================================================================
//  SDR plugin wrapper for Fobos SDR API
//  V.T.
//  LGPL-2.1 or above LICENSE
//  05.06.2024
//  10.11.2025 - closeStream()
//==============================================================================

#include "SoapyFobosSDR.hpp"
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>
#include <cstring> 

std::vector<std::string> SoapyFobosSDR::getStreamFormats(const int direction, const size_t channel) const 
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif       
    std::vector<std::string> formats;
    if ((direction != SOAPY_SDR_RX) && (channel == 0))
    {
        throw std::runtime_error("RX only, use SOAPY_SDR_RX");
    }
    formats.push_back(SOAPY_SDR_CF32);
    return formats;
}

std::string SoapyFobosSDR::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const 
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif    
    if ((direction != SOAPY_SDR_RX) && (channel == 0))
    {
        throw std::runtime_error("RX only, use SOAPY_SDR_RX");
    }
    fullScale = 1.0;
    return SOAPY_SDR_CF32;
}

SoapySDR::ArgInfoList SoapyFobosSDR::getStreamArgsInfo(const int direction, const size_t channel) const 
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %d)\n", __CLASS__, __FUNCTION__, direction, (int)channel);
#endif  
    SoapySDR::ArgInfoList result;
    if ((direction == SOAPY_SDR_RX) && (channel == 0))
    {
        {
            SoapySDR::ArgInfo info;
            info.key = "buf_count";
            info.value = std::to_string(DEFAULT_BUFS_COUNT);
            info.name = "Buffers count in queue";
            info.description = "Buffers count in queue";
            info.units = "";
            info.type = SoapySDR::ArgInfo::INT;
            result.push_back(info);
        }
    }
    return result;
}

/*******************************************************************
 * Async thread work
 ******************************************************************/

static void _rx_callback(float* buf, uint32_t buf_length, void* ctx)
{
    SoapyFobosSDR * self = (SoapyFobosSDR *)ctx;
    self->read_samples(buf, buf_length);
}

void SoapyFobosSDR::rx_async_thread_loop(void)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s() started\n", __CLASS__, __FUNCTION__);
#endif      
    int result = fobos_rx_read_async(_dev, &_rx_callback, this, _rx_buffs_count, _rx_buff_len);
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s() done: %d\n", __CLASS__, __FUNCTION__, result);
#endif
    (void)result;
    _running = false;
}

void SoapyFobosSDR::read_samples(float* buf, uint32_t buf_length)
{
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(".");
    fflush(stdout);
#endif      
    if (this->_rx_buff_len != buf_length)
    {
#ifdef SOAPY_FOBOS_PRINT_DEBUG 
        printf("Err: wrong buf_length!!!");
        printf("canceling...");
#endif
        fobos_rx_cancel_async(_dev);
    }
    std::lock_guard<std::mutex> lock(_rx_mutex);
    if (_rx_filled < _rx_buffs_count)
    {
        memcpy(_rx_bufs[_rx_idx_w], buf, _rx_buff_len * 2 * sizeof(float));
        _rx_idx_w = (_rx_idx_w + 1) % _rx_buffs_count;
        _rx_filled++;
    }
    else
    {
        _overruns_count++;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
        printf("#");
        fflush(stdout);
#endif          
    }
    _rx_cond.notify_one();
}

/*******************************************************************
 * Stream API
 ******************************************************************/

//Typically setup/close should handle lengthy allocation and cleanup procedures
SoapySDR::Stream *SoapyFobosSDR::setupStream(
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args)
{
    (void)args;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %s)\n", __CLASS__, __FUNCTION__, direction, format.c_str());
#endif      
    if (direction != SOAPY_SDR_RX)
    {
        throw std::runtime_error("!direction: only SOAPY_SDR_RX");
    }
    if (channels.size() > 1 or (channels.size() > 0 and channels.at(0) != 0))
    {
        throw std::runtime_error("!channels: only one");
    }
    if (format != SOAPY_SDR_CF32)
    {
        throw std::runtime_error("!format: only SOAPY_SDR_CF32");
    }
    _rx_bufs = new float* [_rx_buffs_count];
    for (unsigned int i = 0; i < _rx_buffs_count; i++)
    {
        _rx_bufs[i] = new float [_rx_buff_len * 2];
    }
    return (SoapySDR::Stream *) this;
}

//Typically setup/close should handle lengthy allocation and cleanup procedures
void SoapyFobosSDR::closeStream(SoapySDR::Stream *stream)
{
    (void)stream;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif 
    if (_rx_async_thread.joinable())
    {
        fobos_rx_cancel_async(_dev);
        _rx_async_thread.join();
    }
    if (_rx_bufs)
    {
        for (unsigned int i = 0; i < _rx_buffs_count; i++)
        {
            if (_rx_bufs[i])
            {
                delete _rx_bufs[i];
            }
        }
        delete _rx_bufs;
    }
    _rx_bufs = nullptr;
}

size_t SoapyFobosSDR::getStreamMTU(SoapySDR::Stream *stream) const
{
    (void)stream;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s()\n", __CLASS__, __FUNCTION__);
#endif     
    return _rx_buff_len;
}

// activate/deactivate may be called multiple times and should be lightweight on and off switches
int SoapyFobosSDR::activateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems)
{
    (void)stream;
    (void)timeNs;
    (void)numElems;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %lld, %d)\n", __CLASS__, __FUNCTION__, flags, timeNs, (int)numElems);
#endif     
    if (flags != 0)
    {
        return SOAPY_SDR_NOT_SUPPORTED;
    }
    _rx_idx_w = 0;
    _rx_pos_r = 0;
    _rx_idx_r = 0;
    _rx_pos_w = 0;
    _rx_filled = 0;
    _running = true;
    _buff_counter = 0;
    _overruns_count = 0;
    if (not _rx_async_thread.joinable())
    {
        _rx_async_thread = std::thread(&SoapyFobosSDR::rx_async_thread_loop, this);
    }
    return 0;
}

// activate / deactivate may be called multiple times and should be lightweight on and off switches
int SoapyFobosSDR::deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs)
{
    (void)stream;
    (void)timeNs;
#ifdef SOAPY_FOBOS_PRINT_DEBUG  
    printf(">>> %s::%s(%d, %lld)\n", __CLASS__, __FUNCTION__, flags, timeNs);
#endif     
    if (flags != 0)
    {
        return SOAPY_SDR_NOT_SUPPORTED;
    }
    if (_rx_async_thread.joinable())
    {
        fobos_rx_cancel_async(_dev);
        _rx_async_thread.join();
    }
    return 0;
}

int SoapyFobosSDR::readStream(
        SoapySDR::Stream *stream,
        void * const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs)
{
    (void)stream;
    (void)timeNs;
    (void)timeoutUs;

    if (flags != 0)
    {
        return SOAPY_SDR_NOT_SUPPORTED;
    }
    if (!_running)
    {
#ifdef SOAPY_FOBOS_PRINT_DEBUG        
        printf("^%d ", (int)numElems);
#endif
        return 0;
    }
    {
        std::unique_lock<std::mutex> lock(_rx_mutex);
        if (_rx_filled == 0)
        {
#ifdef SOAPY_FOBOS_PRINT_DEBUG        
            printf("w");
#endif
            _rx_cond.wait(lock);
        }
    }
    size_t samples_count = 0;
    if (_rx_filled > 0)
    {
        float* src_buff = _rx_bufs[_rx_idx_r] + _rx_pos_r * 2;
        samples_count = (_rx_buff_len - _rx_pos_r);
        if (samples_count > numElems)
        {
            samples_count = numElems;
        }
        void* dst_buf = buffs[0]; //this is the user's buffer for channel 0
        memcpy(dst_buf, src_buff, samples_count * 2 * sizeof(float));
        _rx_pos_r += samples_count;
        if (_rx_pos_r >= _rx_buff_len)
        {
            std::lock_guard<std::mutex> lock(_rx_mutex);
            _rx_pos_r = 0;
            _rx_idx_r = (_rx_idx_r + 1) % _rx_buffs_count;
            _rx_filled--;
        }
    }
    else
    {
#ifdef SOAPY_FOBOS_PRINT_DEBUG        
        printf("u");
#endif        
    }
    return samples_count;
}

