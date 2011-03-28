#ifndef SFXC_MATH_H
#define SFXC_MATH_H
#include <complex>
#include "config.h"

// Define basic math functions
#ifdef USE_IPP
  #include <ipps.h>
  extern inline void sfxc_zero(double *p, size_t len){
    ippsZero_64f((Ipp64f *)p, len);
  }

  extern inline void sfxc_zero_f(float *p, size_t len){
    ippsZero_32f((Ipp32f *)p, len);
  }

  extern inline void sfxc_zero_c(std::complex<double> *p, size_t len){
    ippsZero_64fc((Ipp64fc *)p, len);
  }

  extern inline void sfxc_zero_fc(std::complex<float> *p, size_t len){
    ippsZero_32fc((Ipp32fc *)p, len);
  }

  extern inline void sfxc_mul_c(const std::complex<double> *s1, const std::complex<double> *s2, std::complex<double>  *dest, int len){
    ippsMul_64fc((const Ipp64fc*)s1, (const Ipp64fc*) s2, (Ipp64fc *) dest, len);
  }

  extern inline void sfxc_mul_fc(const std::complex<float> *s1, const std::complex<float> *s2, std::complex<float> *dest, int len){
    ippsMul_32fc((const Ipp32fc*)s1, (const Ipp32fc*) s2, (Ipp32fc *) dest, len);
  }

  extern inline void sfxc_mul_c_I(const std::complex<double> *s1, std::complex<double>  *s2dest, int len){
    ippsMul_64fc_I((const Ipp64fc*)s1, (Ipp64fc *) s2dest, len);
  }

  extern inline void sfxc_mul_fc_I(const std::complex<float> *s1, std::complex<float> *s2dest, int len){
    ippsMul_32fc_I((const Ipp32fc*)s1, (Ipp32fc *) s2dest, len);
  }

  extern inline void sfxc_conj_fc(const std::complex<float> *s1, std::complex<float> *dest, int len){
    ippsConj_32fc((const Ipp32fc*) s1, (Ipp32fc*) dest, len);
  }

  extern inline void sfxc_conj_c(const std::complex<double> *s1, std::complex<double> *dest, int len){
    ippsConj_64fc((const Ipp64fc*) s1, (Ipp64fc*) dest, len);
  }

  extern inline void sfxc_add_product_fc(const std::complex<float> *s1, const std::complex<float> *s2, std::complex<float> *dest, int len){
    ippsAddProduct_32fc((const Ipp32fc*) s1, (const Ipp32fc*) s2, (Ipp32fc*) dest, len);
  }

  extern inline void sfxc_add_product_c(const std::complex<double> *s1, const std::complex<double> *s2, std::complex<double> *dest, int len){
    ippsAddProduct_64fc((const Ipp64fc*) s1, (const Ipp64fc*) s2, (Ipp64fc*) dest, len);
  }
#else // USE FFTW
  #include <string.h>
  extern inline void sfxc_zero(double *p, size_t len){
    memset(p, 0, len * sizeof(double));
  }

  extern inline void sfxc_zero_f(float *p, size_t len){
    memset(p, 0, len * sizeof(float));
  }

  extern inline void sfxc_zero_c(std::complex<double> *p, size_t len){
    memset(p, 0, len * sizeof(std::complex<double>));
  }

  extern inline void sfxc_zero_fc(std::complex<float> *p, size_t len){
    memset(p, 0, len * sizeof(std::complex<float>));
  }

  extern inline void sfxc_mul_fc(const std::complex<float> *s1, const std::complex<float> *s2, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = s1[i] * s2[i];
    }
  }

  extern inline void sfxc_mul_c_I(const std::complex<double> *s1, std::complex<double>  *s2dest, int len){
    for(int i = 0; i < len; i++){
      s2dest[i] = s1[i] * s2dest[i];
    }
   }

  extern inline void sfxc_mul_fc_I(const std::complex<float> *s1, std::complex<float> *s2dest, int len){
    for(int i = 0; i < len; i++){
      s2dest[i] = s1[i] * s2dest[i];
    }
  } 
  extern inline void sfxc_conj_fc(const std::complex<float> *s1, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = conj(s1[i]);
    }
  }
  extern inline void sfxc_conj_c(const std::complex<double> *s1, std::complex<double> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = conj(s1[i]);
    }
  }
  extern inline void sfxc_add_product_fc(const std::complex<float> *s1, const std::complex<float> *s2, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += s1[i] * s2[i];
    }
  }
  extern inline void sfxc_add_product_c(const std::complex<double> *s1, const std::complex<double> *s2, std::complex<double> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += s1[i] * s2[i];
    }
  }
#endif
#endif // SFXC_MATH_H
