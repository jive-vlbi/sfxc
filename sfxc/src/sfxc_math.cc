#include "sfxc_math.h"

/*************************************************************************
Bessel function of order one

Returns Bessel function of order one of the argument.

The domain is divided into the intervals [0, 8] and
(8, infinity). In the first interval a 24 term Chebyshev
expansion is used. In the second, the asymptotic
trigonometric representation is employed using two
rational functions of degree 5/5.

ACCURACY:

                     Absolute error:
arithmetic   domain      # trials      peak         rms
   IEEE      0, 30       30000       2.6e-16     1.1e-16

Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1989, 2000 by Stephen L. Moshier

(2016) Aard Keimpema, modified to code to be stand-alone for use in SFXC
*************************************************************************/
static void bessel_besselasympt1(double x,
     double* pzero,
     double* qzero)
{
    double xsq;
    double p2;
    double q2;
    double p3;
    double q3;

    *pzero = 0;
    *qzero = 0;

    xsq = 64.0/(x*x);
    p2 = -1611.616644324610116477412898;
    p2 = -109824.0554345934672737413139+xsq*p2;
    p2 = -1523529.351181137383255105722+xsq*p2;
    p2 = -6603373.248364939109255245434+xsq*p2;
    p2 = -9942246.505077641195658377899+xsq*p2;
    p2 = -4435757.816794127857114720794+xsq*p2;
    q2 = 1.0;
    q2 = -1455.009440190496182453565068+xsq*q2;
    q2 = -107263.8599110382011903063867+xsq*q2;
    q2 = -1511809.506634160881644546358+xsq*q2;
    q2 = -6585339.479723087072826915069+xsq*q2;
    q2 = -9934124.389934585658967556309+xsq*q2;
    q2 = -4435757.816794127856828016962+xsq*q2;
    p3 = 35.26513384663603218592175580;
    p3 = 1706.375429020768002061283546+xsq*p3;
    p3 = 18494.26287322386679652009819+xsq*p3;
    p3 = 66178.83658127083517939992166+xsq*p3;
    p3 = 85145.16067533570196555001171+xsq*p3;
    p3 = 33220.91340985722351859704442+xsq*p3;
    q3 = 1.0;
    q3 = 863.8367769604990967475517183+xsq*q3;
    q3 = 37890.22974577220264142952256+xsq*q3;
    q3 = 400294.4358226697511708610813+xsq*q3;
    q3 = 1419460.669603720892855755253+xsq*q3;
    q3 = 1819458.042243997298924553839+xsq*q3;
    q3 = 708712.8194102874357377502472+xsq*q3;
    *pzero = p2/q2;
    *qzero = 8*p3/q3/x;
}

double besselj1(double x)
{
    double s;
    double xsq;
    double nn;
    double pzero;
    double qzero;
    double p1;
    double q1;
    double result;


    s = (double)(std::signbit(x)) ? -1. : 1.;
    if( std::isless(x,(double)(0)) )
    {
        x = -x;
    }
    if( std::isgreater(x,8.0) )
    {
        bessel_besselasympt1(x, &pzero, &qzero);
        nn = x-3*M_PI/4;
        result = sqrt(2/M_PI/x)*(pzero*cos(nn)-qzero*sin(nn));
        if( std::isless(s,(double)(0)) )
        {
            result = -result;
        }
        return result;
    }
    xsq =x*x;
    p1 = 2701.122710892323414856790990;
    p1 = -4695753.530642995859767162166+xsq*p1;
    p1 = 3413234182.301700539091292655+xsq*p1;
    p1 = -1322983480332.126453125473247+xsq*p1;
    p1 = 290879526383477.5409737601689+xsq*p1;
    p1 = -35888175699101060.50743641413+xsq*p1;
    p1 = 2316433580634002297.931815435+xsq*p1;
    p1 = -66721065689249162980.20941484+xsq*p1;
    p1 = 581199354001606143928.050809+xsq*p1;
    q1 = 1.0;
    q1 = 1606.931573481487801970916749+xsq*q1;
    q1 = 1501793.594998585505921097578+xsq*q1;
    q1 = 1013863514.358673989967045588+xsq*q1;
    q1 = 524371026216.7649715406728642+xsq*q1;
    q1 = 208166122130760.7351240184229+xsq*q1;
    q1 = 60920613989175217.46105196863+xsq*q1;
    q1 = 11857707121903209998.37113348+xsq*q1;
    q1 = 1162398708003212287858.529400+xsq*q1;
    result = s*x*p1/q1;
    return result;
}

