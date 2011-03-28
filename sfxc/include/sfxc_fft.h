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
class sfxc_fft_ipp:public sfxc_fft<double>{
public:
  sfxc_fft_ipp();
  virtual ~sfxc_fft_ipp();
  void resize(int size_);
  void fft(const std::complex<double> *in, std::complex<double> *out);
  void ifft(const std::complex<double> *in, std::complex<double> *out);
  void rfft(const double *in, std::complex<double> *out);
  void irfft(const std::complex<double> *in, double *out);
private:
  void alloc();
  void alloc_r2c();
  void free_buffers();
private:
  int order;
  IppsFFTSpec_C_64fc *ippspec;
  IppsFFTSpec_R_64f *ippspec_r2c;
  Ipp8u *buffer, *buffer_r2c;
};
#else // USE FFTW
#include <fftw3.h>
#include <string.h>
class sfxc_fft_fftw : public sfxc_fft<double>{
public:
  sfxc_fft_fftw();
  virtual ~sfxc_fft_fftw();
  void resize(int size_);
  void fft(const std::complex<double> *in, std::complex<double> *out);
  void ifft(const std::complex<double> *in, std::complex<double> *out);
  void rfft(const double *in, std::complex<double> *out);
  void irfft(const std::complex<double> *in, double *out);
private:
  void free_buffers(); 
  fftw_plan alloc(int sign, bool inplace);
  fftw_plan alloc_r2c(int sign);
public:
  int size;
private:
  fftw_plan  plan_forward, plan_backward;
  bool plan_forward_set, plan_backward_set;
  fftw_plan  plan_forward_I, plan_backward_I;
  bool plan_forward_I_set, plan_backward_I_set;
  fftw_plan  plan_forward_r2c, plan_backward_r2c;
  bool plan_forward_r2c_set, plan_backward_r2c_set;
};
#endif // USE_IPP
#endif // SFXC_FFT_H
