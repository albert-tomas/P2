#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    float pot = 1e-12;
    for(unsigned int n=0; n<N; n++ ){
        pot += x[n]*x[n];
    }
    return 10*log10(pot/N);
}

float compute_am(const float *x, unsigned int N) {
    float am = 0;
    for(unsigned n=0; n<N; n++){
        am = am + fabs(x[n]);
    }
    return am/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float zcr = 0;
    
    for(unsigned n=1; n<N; n++){
        if((x[n]>0 && x[n-1]<0) || (x[n]<0 && x[n-1]>0) || (x[n]==0 && x[n-1] != 0) || (x[n-1]==0 && x[n]!=0))
            zcr++;
        
    }
    float res = ((fm/2)*(1.0/(N-1))*zcr);
    return res;
}
