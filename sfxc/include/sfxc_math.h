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

  extern inline void sfxc_magnitude(const std::complex<double> *s1, double *dest, size_t len){
    ippsMagnitude_64fc((const Ipp64fc*) s1, (Ipp64f*) dest, len);
  }

  extern inline void sfxc_magnitude_f(const std::complex<float> *s1, float *dest, size_t len){
    ippsMagnitude_32fc((const Ipp32fc*) s1, (Ipp32f*) dest, len);
  }

  extern inline void sfxc_mulc_I(const double s1, double  *srcdest, int len){
    ippsMulC_64f_I((const Ipp64f)s1, (Ipp64f*) srcdest, len);
  }

  extern inline void sfxc_mulc_f_I(const float s1, float *srcdest, int len){
    ippsMulC_32f_I((const Ipp32f)s1, (Ipp32f *) srcdest, len);
  }

   extern inline void sfxc_mul(const double *s1, const double *s2, double  *dest, int len){
    ippsMul_64f((const Ipp64f*)s1, (const Ipp64f*) s2, (Ipp64f *) dest, len);
  }

  extern inline void sfxc_mul_f(const float *s1, const float *s2, float *dest, int len){
    ippsMul_32f((const Ipp32f*)s1, (const Ipp32f*) s2, (Ipp32f *) dest, len);
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

  extern inline void sfxc_mul_f_fc_I(const float *s1, std::complex<float> *s2dest, int len){
    ippsMul_32f32fc_I((const Ipp32f *)s1, (Ipp32fc *) s2dest, len);
  }

  extern inline void sfxc_conj_fc(const std::complex<float> *s1, std::complex<float> *dest, int len){
    ippsConj_32fc((const Ipp32fc*) s1, (Ipp32fc*) dest, len);
  }

  extern inline void sfxc_conj_c(const std::complex<double> *s1, std::complex<double> *dest, int len){
    ippsConj_64fc((const Ipp64fc*) s1, (Ipp64fc*) dest, len);
  }

  extern inline void sfxc_add_f(const float *src1, const float *src2, float *dest, int len){
    ippsAdd_32f((const Ipp32f*) src1, (const Ipp32f*) src2, (Ipp32f*) dest, len);
  }

  extern inline void sfxc_add(const double *src1, const double *src2, double *dest, int len){
    ippsAdd_64f((const Ipp64f*) src1, (const Ipp64f*) src2, (Ipp64f*) dest, len);
  }

  extern inline void sfxc_add_fc(const std::complex<float> *src, std::complex<float> *dest, int len){
    ippsAdd_32fc_I((const Ipp32fc*) src, (Ipp32fc*) dest, len);
  }

  extern inline void sfxc_sub_fc(const std::complex<float> *src, std::complex<float> *dest, int len){
    ippsSub_32fc_I((const Ipp32fc*) src, (Ipp32fc*) dest, len);
  }

  extern inline void sfxc_add_c(const std::complex<double> *src, std::complex<double> *dest, int len){
    ippsAdd_64fc_I((const Ipp64fc*) src, (Ipp64fc*) dest, len);
  }

  extern inline void sfxc_sub_c(const std::complex<double> *src, std::complex<double> *dest, int len){
    ippsSub_64fc_I((const Ipp64fc*) src, (Ipp64fc*) dest, len);
  }

  extern inline void sfxc_add_f(const float *src, float *dest, int len){
    ippsAdd_32f_I((const Ipp32f*) src, (Ipp32f*) dest, len);
  }

  extern inline void sfxc_sub_f(const float *src, float *dest, int len){
    ippsSub_32f_I((const Ipp32f*) src, (Ipp32f*) dest, len);
  }

  extern inline void sfxc_add(const double *src, double *dest, int len){
    ippsAdd_64f_I((const Ipp64f*) src, (Ipp64f*) dest, len);
  }

  extern inline void sfxc_sub(const double *src, double *dest, int len){
    ippsSub_64f_I((const Ipp64f*) src, (Ipp64f*) dest, len);
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

  extern inline void sfxc_magnitude(const std::complex<double> *s1, double *dest, size_t len){
    for(int i = 0; i < len; i++){
      dest[i] = real(s1[1]*conj(s1[i]));
    }
  }

  extern inline void sfxc_magnitude_f(const std::complex<float> *s1, float *dest, size_t len){
    for(int i = 0; i < len; i++){
      dest[i] = real(s1[1]*conj(s1[i]));
    }
  }

  extern inline void sfxc_mul_c(const std::complex<double> *s1, const std::complex<double> *s2, std::complex<double> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = s1[i] * s2[i];
    }
  }

  extern inline void sfxc_mul_fc(const std::complex<float> *s1, const std::complex<float> *s2, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = s1[i] * s2[i];
    }
  }

  extern inline void sfxc_mulc_I(const double s1, double *srcdest, int len){
    for(int i = 0; i < len; i++){
      srcdest[i] *= s1;
    }
  }

   extern inline void sfxc_mulc_f_I(const float s1, float *srcdest, int len){
    for(int i = 0; i < len; i++){
      srcdest[i] *= s1;
    }
  }

   extern inline void sfxc_mul(const double *s1, const double *s2, double *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = s1[i] * s2[i];
    }
  }

   extern inline void sfxc_mul_f(const float *s1, const float *s2, float *dest, int len){
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

  extern inline void sfxc_mul_f_fc_I(const float *s1, std::complex<float> *s2dest, int len){
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

  extern inline void sfxc_add_f(const float *src1, const float *src2, float *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = src1[i] + src2[i];
    }
  }

  extern inline void sfxc_add(const double *src1, const double *src2, double *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] = src1[i] + src2[i];
    }
  }

  extern inline void sfxc_add_fc(const std::complex<float> *src, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += src[i];
    }
  }

  extern inline void sfxc_sub_fc(const std::complex<float> *src, std::complex<float> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] -= src[i];
    }
  }

  extern inline void sfxc_add_c(const std::complex<double> *src, std::complex<double> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += src[i];
    }
  }
  extern inline void sfxc_sub_c(const std::complex<double> *src, std::complex<double> *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] -= src[i];
    }
  }

  extern inline void sfxc_add_f(const float *src, float *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += src[i];
    }
  }
  extern inline void sfxc_sub_f(const float *src, float *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] -= src[i];
    }
  }

  extern inline void sfxc_add_c(const double *src, double *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] += src[i];
    }
  }
  extern inline void sfxc_sub_c(const double *src, double *dest, int len){
    for(int i = 0; i < len; i++){
      dest[i] -= src[i];
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
