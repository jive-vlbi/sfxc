#ifndef SFXC_FFT_H
#define SFXC_FFT_H
#include <complex>
#include "config.h"

// The sfxc fft wrapper class
template <typename float_type>
class sfxc_fft{
public:
  sfxc_fft(){}
  virtual ~sfxc_fft(){}
  virtual void resize(int size_) = 0;
  virtual void fft(const std::complex<float_type> *in, std::complex<float_type> *out) = 0;
  virtual void ifft(const std::complex<float_type> *in, std::complex<float_type> *out) = 0;
  virtual void rfft(const float_type *in, std::complex<float_type> *out) = 0;
  virtual void irfft(const std::complex<float_type> *in, float_type *out) = 0;
public:
  int size;
};

// Define basic math functions
#ifdef USE_IPP
#include <ipps.h>
class sfxc_fft_ipp_float:public sfxc_fft<float>{
public:
  sfxc_fft_ipp_float();
  virtual ~sfxc_fft_ipp_float();
  void resize(int size_);
  void fft(const std::complex<float> *in, std::complex<float> *out);
  void ifft(const std::complex<float> *in, std::complex<float> *out);
  void rfft(const float *in, std::complex<float> *out);
  void irfft(const std::complex<float> *in, float *out);
private:
  void alloc();
  void alloc_r2c();
  void free_buffers();
private:
  int order;
  IppsFFTSpec_C_32fc *ippspec;
  IppsFFTSpec_R_32f *ippspec_r2c;
  Ipp8u *buffer, *buffer_r2c;
};
#else // USE FFTW
#include <fftw3.h>
#include <string.h>
  class sfxc_fft_fftw_float : public sfxc_fft<float>{
  public:
    sfxc_fft_fftw_float();
    virtual ~sfxc_fft_fftw_float();
    void resize(int size_);
    void fft(const std::complex<float> *in, std::complex<float> *out);
    void ifft(const std::complex<float> *in, std::complex<float> *out);
    void rfft(const float *in, std::complex<float> *out);
    void irfft(const std::complex<float> *in, float *out);
  private:
    void free_buffers(); 
    fftwf_plan alloc(int sign, bool inplace);
    fftwf_plan alloc_r2c(int sign);
  public:
    int size;
  private:
    fftwf_plan  plan_forward, plan_backward;
    bool plan_forward_set, plan_backward_set;
    fftwf_plan  plan_forward_I, plan_backward_I;
    bool plan_forward_I_set, plan_backward_I_set;
    fftwf_plan  plan_forward_r2c, plan_backward_r2c;
    bool plan_forward_r2c_set, plan_backward_r2c_set;
  };
#endif // USE_IPP
#endif // SFXC_FFT_H
