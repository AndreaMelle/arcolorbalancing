//
//  mathext.h
//  ar-color-balancing
//
//  Created by Andrea Melle on 18/12/2015.
//
//

#ifndef mathext_h
#define mathext_h

#define  MATHEXT_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))

#define MATHEXT_CLIP(value) \
    value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;

namespace mathext
{
    
    typedef union suf32
    {
        int i;
        unsigned u;
        float f;
    }
    suf32;
    
    float cubeRoot( float value )
    {
        float fr;
        suf32 v, m;
        int ix, s;
        int ex, shx;
        
        v.f = value;
        ix = v.i & 0x7fffffff;
        s = v.i & 0x80000000;
        ex = (ix >> 23) - 127;
        shx = ex % 3;
        shx -= shx >= 0 ? 3 : 0;
        ex = (ex - shx) / 3; /* exponent of cube root */
        v.i = (ix & ((1<<23)-1)) | ((shx + 127)<<23);
        fr = v.f;
        
        /* 0.125 <= fr < 1.0 */
        /* Use quartic rational polynomial with error < 2^(-24) */
        fr = (float)(((((45.2548339756803022511987494 * fr +
                         192.2798368355061050458134625) * fr +
                        119.1654824285581628956914143) * fr +
                       13.43250139086239872172837314) * fr +
                      0.1636161226585754240958355063)/
                     ((((14.80884093219134573786480845 * fr +
                         151.9714051044435648658557668) * fr +
                        168.5254414101568283957668343) * fr +
                       33.9905941350215598754191872) * fr +
                      1.0));
        
        /* fr *= 2^ex * sign */
        m.f = value;
        v.f = fr;
        v.i = (v.i + (ex << 23) + s) & (m.i*2 != 0 ? -1 : 0);
        return v.f;
    }
    
    // computes cubic spline coefficients for a function: (xi=i, yi=f[i]), i=0..n
    template<typename T>
    static void splineBuild(const T* f, int n, T* tab)
    {
        T cn = 0;
        int i;
        tab[0] = tab[1] = (T)0;
        
        for(i = 1; i < n-1; i++)
        {
            T t = 3*(f[i+1] - 2*f[i] + f[i-1]);
            T l = 1/(4 - tab[(i-1)*4]);
            tab[i*4] = l; tab[i*4+1] = (t - tab[(i-1)*4+1])*l;
        }
        
        for(i = n-1; i >= 0; i--)
        {
            T c = tab[i*4+1] - tab[i*4]*cn;
            T b = f[i+1] - f[i] - (cn + c*2)*(T)0.3333333333333333;
            T d = (cn - c)*(T)0.3333333333333333;
            tab[i*4] = f[i]; tab[i*4+1] = b;
            tab[i*4+2] = c; tab[i*4+3] = d;
            cn = c;
        }
    }
    
    // interpolates value of a function at x, 0 <= x <= n using a cubic spline.
    template<typename _Tp> static inline _Tp splineInterpolate(_Tp x, const _Tp* tab, int n)
    {
        // don't touch this function without urgent need - some versions of gcc fail to inline it correctly
        int ix = std::min(std::max(int(x), 0), n-1);
        x -= ix;
        tab += ix*4;
        return ((tab[3]*x + tab[2])*x + tab[1])*x + tab[0];
    }
    
    
    /////////////// saturate_cast (used in image & signal processing) ///////////////////
    
    typedef signed char schar;
    typedef unsigned char uchar;
    typedef unsigned short ushort;
    
    /**
     Template function for accurate conversion from one primitive type to another.
     
     The functions saturate_cast resemble the standard C++ cast operations, such as static_cast\<T\>()
     and others. They perform an efficient and accurate conversion from one primitive type to another
     (see the introduction chapter). saturate in the name means that when the input value v is out of the
     range of the target type, the result is not formed just by taking low bits of the input, but instead
     the value is clipped. For example:
     @code
     uchar a = saturate_cast<uchar>(-100); // a = 0 (UCHAR_MIN)
     short b = saturate_cast<short>(33333.33333); // b = 32767 (SHRT_MAX)
     @endcode
     Such clipping is done when the target type is unsigned char , signed char , unsigned short or
     signed short . For 32-bit integers, no clipping is done.
     
     When the parameter is a floating-point value and the target type is an integer (8-, 16- or 32-bit),
     the floating-point value is first rounded to the nearest integer and then clipped if needed (when
     the target type is 8- or 16-bit).
     
     This operation is used in the simplest or most complex image processing functions in OpenCV.
     
     @param v Function parameter.
     @sa add, subtract, multiply, divide, Mat::convertTo
     */
    template<typename _Tp> static inline _Tp saturate_cast(uchar v)    { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(schar v)    { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(ushort v)   { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(short v)    { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(int v)      { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(float v)    { return _Tp(v); }
    /** @overload */
    template<typename _Tp> static inline _Tp saturate_cast(double v)   { return _Tp(v); }
    /** @overload */
    //template<typename _Tp> static inline _Tp saturate_cast(int64 v)    { return _Tp(v); }
    /** @overload */
    //template<typename _Tp> static inline _Tp saturate_cast(uint64 v)   { return _Tp(v); }
    
    //! @cond IGNORED
    
//    template<> inline uchar saturate_cast<uchar>(schar v)        { return (uchar)std::max((int)v, 0); }
//    template<> inline uchar saturate_cast<uchar>(ushort v)       { return (uchar)std::min((unsigned)v, (unsigned)UCHAR_MAX); }
    template<> inline unsigned char saturate_cast<unsigned char>(int v)
    {
        return (unsigned char)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0);
    }
    
//    template<> inline uchar saturate_cast<uchar>(short v)        { return saturate_cast<uchar>((int)v); }
//    template<> inline uchar saturate_cast<uchar>(unsigned v)     { return (uchar)std::min(v, (unsigned)UCHAR_MAX); }
//    template<> inline uchar saturate_cast<uchar>(float v)        { int iv = cvRound(v); return saturate_cast<uchar>(iv); }
//    template<> inline uchar saturate_cast<uchar>(double v)       { int iv = cvRound(v); return saturate_cast<uchar>(iv); }
//    //template<> inline uchar saturate_cast<uchar>(int64 v)        { return (uchar)((uint64)v <= (uint64)UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
//    //template<> inline uchar saturate_cast<uchar>(uint64 v)       { return (uchar)std::min(v, (uint64)UCHAR_MAX); }
//    
//    template<> inline schar saturate_cast<schar>(uchar v)        { return (schar)std::min((int)v, SCHAR_MAX); }
//    template<> inline schar saturate_cast<schar>(ushort v)       { return (schar)std::min((unsigned)v, (unsigned)SCHAR_MAX); }
//    template<> inline schar saturate_cast<schar>(int v)          { return (schar)((unsigned)(v-SCHAR_MIN) <= (unsigned)UCHAR_MAX ? v : v > 0 ? SCHAR_MAX : SCHAR_MIN); }
//    template<> inline schar saturate_cast<schar>(short v)        { return saturate_cast<schar>((int)v); }
//    template<> inline schar saturate_cast<schar>(unsigned v)     { return (schar)std::min(v, (unsigned)SCHAR_MAX); }
//    template<> inline schar saturate_cast<schar>(float v)        { int iv = cvRound(v); return saturate_cast<schar>(iv); }
//    template<> inline schar saturate_cast<schar>(double v)       { int iv = cvRound(v); return saturate_cast<schar>(iv); }
//    template<> inline schar saturate_cast<schar>(int64 v)        { return (schar)((uint64)((int64)v-SCHAR_MIN) <= (uint64)UCHAR_MAX ? v : v > 0 ? SCHAR_MAX : SCHAR_MIN); }
//    template<> inline schar saturate_cast<schar>(uint64 v)       { return (schar)std::min(v, (uint64)SCHAR_MAX); }
//    
//    template<> inline ushort saturate_cast<ushort>(schar v)      { return (ushort)std::max((int)v, 0); }
//    template<> inline ushort saturate_cast<ushort>(short v)      { return (ushort)std::max((int)v, 0); }

    template<> inline unsigned short saturate_cast<unsigned short>(int v)
    {
        return (unsigned short)((unsigned)v <= (unsigned)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0);
    }
//    template<> inline ushort saturate_cast<ushort>(unsigned v)   { return (ushort)std::min(v, (unsigned)USHRT_MAX); }
    
    
    template<> inline unsigned short saturate_cast<unsigned short>(float v)
    {
        int iv = std::round(v);// cvRound(v);
        return saturate_cast<unsigned short>(iv);
    }

    
//    template<> inline ushort saturate_cast<ushort>(double v)     { int iv = cvRound(v); return saturate_cast<ushort>(iv); }
//    template<> inline ushort saturate_cast<ushort>(int64 v)      { return (ushort)((uint64)v <= (uint64)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0); }
//    template<> inline ushort saturate_cast<ushort>(uint64 v)     { return (ushort)std::min(v, (uint64)USHRT_MAX); }
//    
//    template<> inline short saturate_cast<short>(ushort v)       { return (short)std::min((int)v, SHRT_MAX); }
//    template<> inline short saturate_cast<short>(int v)          { return (short)((unsigned)(v - SHRT_MIN) <= (unsigned)USHRT_MAX ? v : v > 0 ? SHRT_MAX : SHRT_MIN); }
//    template<> inline short saturate_cast<short>(unsigned v)     { return (short)std::min(v, (unsigned)SHRT_MAX); }
//    template<> inline short saturate_cast<short>(float v)        { int iv = cvRound(v); return saturate_cast<short>(iv); }
//    template<> inline short saturate_cast<short>(double v)       { int iv = cvRound(v); return saturate_cast<short>(iv); }
//    template<> inline short saturate_cast<short>(int64 v)        { return (short)((uint64)((int64)v - SHRT_MIN) <= (uint64)USHRT_MAX ? v : v > 0 ? SHRT_MAX : SHRT_MIN); }
//    template<> inline short saturate_cast<short>(uint64 v)       { return (short)std::min(v, (uint64)SHRT_MAX); }
//    
//    template<> inline int saturate_cast<int>(float v)            { return cvRound(v); }
//    template<> inline int saturate_cast<int>(double v)           { return cvRound(v); }
//    
//    // we intentionally do not clip negative numbers, to make -1 become 0xffffffff etc.
//    template<> inline unsigned saturate_cast<unsigned>(float v)  { return cvRound(v); }
//    template<> inline unsigned saturate_cast<unsigned>(double v) { return cvRound(v); }
    
    //! @endcond
};

#endif /* mathext_h */
