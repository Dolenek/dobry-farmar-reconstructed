/* Recovered from Squall 10037840..10037d35. Array stores round to binary32;
   double temporaries retain the native PC53/PC24 x87 arithmetic width.
   The register values carried across the inlined 32/16/8 point stages are
   intentional. Reloading the rounded array elements changes decoded PCM. */
#include "mdct.h"

static float spill32(double value) { volatile float stored=(float)value;return stored; }

static void butterfly8_carried(float *x,double six,float seven) {
    double sum6=spill32(six+x[2]),diff6=six-x[2];
    double sum4=(double)x[0]+x[4],diff4=spill32((double)x[4]-x[0]);
    double a,b,c,d;
    x[6]=spill32(sum4+sum6);x[4]=spill32(sum6-sum4);
    a=(double)x[5]-x[1];b=spill32((double)seven-x[3]);
    x[0]=spill32(diff6+a);x[2]=spill32(diff6-a);
    c=(double)x[5]+x[1];d=(double)seven+x[3];
    x[3]=spill32(diff4+b);x[1]=spill32(b-diff4);
    x[7]=spill32(d+c);x[5]=spill32(d-c);
}

static void butterfly8_stored(float *x) {
    double sum6=(double)x[2]+x[6],diff6=spill32((double)x[6]-x[2]);
    double sum4=(double)x[0]+x[4],diff4=spill32((double)x[4]-x[0]);
    double a,b,c,d;
    x[6]=spill32(sum4+sum6);x[4]=spill32(sum6-sum4);
    a=(double)x[5]-x[1];b=spill32((double)x[7]-x[3]);
    x[0]=spill32(diff6+a);x[2]=spill32(diff6-a);
    c=(double)x[1]+x[5];d=(double)x[3]+x[7];
    x[3]=spill32(diff4+b);x[1]=spill32(b-diff4);
    x[7]=spill32(d+c);x[5]=spill32(d-c);
}

/* Independent native 10037d40 routine, also called at the last 8-point
   stage of 10037840. */
void farmar_native_butterfly8(float *x) { butterfly8_stored(x); }

static void butterfly16(float *x,double one,int carried) {
    double r0=one-x[9],r1=(double)x[0]-x[8],six;
    float seven;
    if(carried) {r0=spill32(r0);r1=spill32(r1);}
    x[8]=spill32((double)x[0]+x[8]);x[9]=spill32(one+x[9]);
    x[0]=spill32((r0+r1)*cPI2_8);x[1]=spill32((r0-r1)*cPI2_8);
    r0=(double)x[3]-x[11];r1=(double)x[10]-x[2];
    x[10]=spill32((double)x[2]+x[10]);x[11]=spill32((double)x[11]+x[3]);
    x[2]=spill32(r0);x[3]=spill32(r1);
    r0=(double)x[12]-x[4];r1=(double)x[13]-x[5];
    x[12]=spill32((double)x[4]+x[12]);x[13]=spill32((double)x[13]+x[5]);
    x[4]=spill32((r0-r1)*cPI2_8);x[5]=spill32((r0+r1)*cPI2_8);
    six=(double)x[14]-x[6];seven=spill32((double)x[15]-x[7]);
    x[14]=spill32((double)x[6]+x[14]);x[15]=spill32((double)x[7]+x[15]);
    butterfly8_carried(x,six,seven);butterfly8_stored(x+8);
}

void farmar_native_butterfly32(float *x) {
    double r0,r1,one;
    r0=(double)x[30]-x[14];r1=(double)x[31]-x[15];
    x[30]=spill32((double)x[14]+x[30]);x[31]=spill32((double)x[31]+x[15]);
    x[14]=spill32(r0);x[15]=spill32(r1);
    r0=(double)x[28]-x[12];r1=(double)x[29]-x[13];
    x[28]=spill32((double)x[12]+x[28]);x[29]=spill32((double)x[29]+x[13]);
    x[12]=spill32(r0*cPI1_8-r1*cPI3_8);x[13]=spill32(r1*cPI1_8+r0*cPI3_8);
    r0=(double)x[26]-x[10];r1=(double)x[27]-x[11];
    x[26]=spill32((double)x[10]+x[26]);x[27]=spill32((double)x[27]+x[11]);
    x[10]=spill32((r0-r1)*cPI2_8);x[11]=spill32((r1+r0)*cPI2_8);
    r0=(double)x[24]-x[8];r1=(double)x[25]-x[9];
    x[24]=spill32((double)x[8]+x[24]);x[25]=spill32((double)x[25]+x[9]);
    x[8]=spill32(r0*cPI3_8-r1*cPI1_8);x[9]=spill32(r1*cPI3_8+r0*cPI1_8);
    r0=(double)x[22]-x[6];r1=(double)x[7]-x[23];
    x[22]=spill32((double)x[6]+x[22]);x[23]=spill32((double)x[7]+x[23]);
    x[6]=spill32(r1);x[7]=spill32(r0);
    r0=(double)x[4]-x[20];r1=(double)x[5]-x[21];
    x[20]=spill32((double)x[20]+x[4]);x[21]=spill32((double)x[5]+x[21]);
    x[4]=spill32(r1*cPI1_8+r0*cPI3_8);x[5]=spill32(r1*cPI3_8-r0*cPI1_8);
    r0=(double)x[2]-x[18];r1=(double)x[3]-x[19];
    x[18]=spill32((double)x[18]+x[2]);x[19]=spill32((double)x[3]+x[19]);
    x[2]=spill32((r1+r0)*cPI2_8);x[3]=spill32((r1-r0)*cPI2_8);
    r0=(double)x[0]-x[16];r1=(double)x[1]-x[17];
    x[16]=spill32((double)x[16]+x[0]);x[17]=spill32((double)x[17]+x[1]);
    x[0]=spill32(r1*cPI3_8+r0*cPI1_8);one=r1*cPI1_8-r0*cPI3_8;
    butterfly16(x,one,1);butterfly16(x+16,x[17],0);
}

/* Native 100375f8..100376e7: r2 and the second half-sum spill to
   binary32; the r3 and first half-sum registers do not. */
void farmar_native_bitreverse(mdct_lookup *init,float *out) {
    int *bit=init->bitrev;
    float *base=out+(init->n>>1),*w0=out,*w1=base,*t=init->trig+init->n;
    do {
        int pair;
        w1-=4;
        for(pair=0;pair<2;++pair) {
            float *a=base+bit[pair*2],*b=base+bit[pair*2+1];
            double r0=(double)a[1]-b[1],r1=(double)b[0]+a[0];
            float r2=spill32(r1*t[pair*2]+r0*t[pair*2+1]);
            double r3=r1*t[pair*2+1]-r0*t[pair*2];
            double half0=((double)b[1]+a[1])*0.5f;
            float half1=spill32(((double)a[0]-b[0])*0.5f);
            w0[pair*2]=spill32(r2+half0);w1[2-pair*2]=spill32(half0-r2);
            w0[pair*2+1]=spill32(r3+half1);w1[3-pair*2]=spill32(r3-half1);
        }
        w0+=4;t+=4;bit+=4;
    } while(w0<w1);
}

/* Native first stage in 10037330 and generic stage 10037dd0. */
void farmar_native_butterfly_stage(float *t,float *x,int points,int step) {
    float *a=x+points-8,*b=x+(points>>1)-8;
    do {
        int i;
        for(i=6;i>=0;i-=2) {
            double r0=(double)a[i]-b[i],r1=(double)a[i+1]-b[i+1];
            a[i]=spill32((double)b[i]+a[i]);a[i+1]=spill32((double)a[i+1]+b[i+1]);
            b[i]=spill32(r1*t[1]+r0*t[0]);b[i+1]=spill32(r1*t[0]-r0*t[1]);
            t+=step;
        }
        a-=8;b-=8;
    } while(b>=x);
}
