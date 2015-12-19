//
//  colors.h
//  ar-color-balancing
//
//  Created by Andrea Melle on 18/12/2015.
//
//

#ifndef colors_h
#define colors_h

#include <mathext.h>

#define FLOAT_COLOR_CHANNEL_MAX 1.0f

namespace color
{
    typedef enum ConversionMode
    {
        sRGB_to_CIELAB,
        lRGB_to_CIELAB,
        
        CIELAB_sRGB,
        CIELAB_lRGB,
        
    } ConversionMode;
    
    typedef enum ColorMode
    {
        RGB,
        RGBA,
        BGR,
        BGRA,
        
    } ColorMode;
    
    template<typename T>
    class IColorConversion
    {
    public:
        IColorConversion() { }
        virtual ~IColorConversion() { }
        
        // applies color conversion on n samples. dst must be already allocated
        virtual void convert(const T* src, T* dst, const int n) = 0;
        
    };
    
    
    enum
    {
        yuv_shift = 14,
        xyz_shift = 12,
        R2Y = 4899,
        G2Y = 9617,
        B2Y = 1868,
        BLOCK_SIZE = 256
    };
    
    static const float sRGB2XYZ_D65[] =
    {
        0.412453f, 0.357580f, 0.180423f,
        0.212671f, 0.715160f, 0.072169f,
        0.019334f, 0.119193f, 0.950227f
    };
    
    static const float XYZ2sRGB_D65[] =
    {
        3.240479f, -1.53715f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };
    
    ///////////////////////////////////// RGB <-> L*a*b* /////////////////////////////////////
    
    static const float D65[] = { 0.950456f, 1.f, 1.088754f };
    
    enum { LAB_CBRT_TAB_SIZE = 1024, GAMMA_TAB_SIZE = 1024 };
    static float LabCbrtTab[LAB_CBRT_TAB_SIZE*4];
    static const float LabCbrtTabScale = LAB_CBRT_TAB_SIZE/1.5f;
    
    static float sRGBGammaTab[GAMMA_TAB_SIZE*4], sRGBInvGammaTab[GAMMA_TAB_SIZE*4];
    static const float GammaTabScale = (float)GAMMA_TAB_SIZE;
    
    static unsigned short sRGBGammaTab_b[256], linearGammaTab_b[256];
    
    #undef lab_shift
    #define lab_shift xyz_shift
    #define gamma_shift 3
    #define lab_shift2 (lab_shift + gamma_shift)
    #define LAB_CBRT_TAB_SIZE_B (256*3/2*(1<<gamma_shift))
    
    static ushort LabCbrtTab_b[LAB_CBRT_TAB_SIZE_B];
    
    static void initLabTabs()
    {
        static bool initialized = false;
        if(!initialized)
        {
            float f[LAB_CBRT_TAB_SIZE+1], g[GAMMA_TAB_SIZE+1], ig[GAMMA_TAB_SIZE+1], scale = 1.f/LabCbrtTabScale;
            int i;
            for(i = 0; i <= LAB_CBRT_TAB_SIZE; i++)
            {
                float x = i*scale;
                f[i] = x < 0.008856f ? x*7.787f + 0.13793103448275862f : mathext::cubeRoot(x);
            }
            
            mathext::splineBuild(f, LAB_CBRT_TAB_SIZE, LabCbrtTab);
            
            scale = 1.f/GammaTabScale;
            for(i = 0; i <= GAMMA_TAB_SIZE; i++)
            {
                float x = i*scale;
                g[i] = x <= 0.04045f ? x*(1.f/12.92f) : (float)std::pow((double)(x + 0.055)*(1./1.055), 2.4);
                ig[i] = x <= 0.0031308 ? x*12.92f : (float)(1.055*std::pow((double)x, 1./2.4) - 0.055);
            }
            mathext::splineBuild(g, GAMMA_TAB_SIZE, sRGBGammaTab);
            mathext::splineBuild(ig, GAMMA_TAB_SIZE, sRGBInvGammaTab);
            
            for(i = 0; i < 256; i++)
            {
                float x = i*(1.f/255.f);
                sRGBGammaTab_b[i] = mathext::saturate_cast<unsigned short>(255.f*(1 << gamma_shift)*(x <= 0.04045f ? x*(1.f/12.92f) : (float)std::pow((double)(x + 0.055)*(1./1.055), 2.4)));
                linearGammaTab_b[i] = (ushort)(i*(1 << gamma_shift));
            }
            
            for(i = 0; i < LAB_CBRT_TAB_SIZE_B; i++)
            {
                float x = i*(1.f/(255.f*(1 << gamma_shift)));
                LabCbrtTab_b[i] = mathext::saturate_cast<unsigned short>((1 << lab_shift2)*(x < 0.008856f ? x*7.787f + 0.13793103448275862f : mathext::cubeRoot(x)));
            }
            initialized = true;
        }
    }
    
    template<typename T>
    class RGB2Lab : public IColorConversion<T>
    {
    public:
        RGB2Lab() { }
        virtual ~RGB2Lab() { }
    };
    
    template<typename T>
    class Lab2RGB : public IColorConversion<T>
    {
    public:
        Lab2RGB() { }
        virtual ~Lab2RGB() { }
    };
    
    // Converts [0.0, 1.0] RGB, RGBA, BGR, BGRA colors to Lab
    
    template<>
    class RGB2Lab<float>  : public IColorConversion<float>
    {
    public:
        RGB2Lab(int in_num_channels,
                int in_blue_index,
                const float* in_coeffs,
                const float* in_whitept,
                bool in_srgb) : num_channels(in_num_channels) , srgb(in_srgb)
        {
            volatile int _3 = 3;
            initLabTabs();
            
            if (!in_coeffs)
                in_coeffs = sRGB2XYZ_D65;
            if (!in_whitept)
                in_whitept = D65;
            
            float scale[] = { 1.0f / in_whitept[0], 1.0f, 1.0f / in_whitept[2] };
            
            for( int i = 0; i < _3; i++ )
            {
                int j = i * 3;
                coeffs[j + (in_blue_index ^ 2)] = in_coeffs[j] * scale[i];
                coeffs[j + 1]                   = in_coeffs[j + 1] * scale[i];
                coeffs[j + in_blue_index]       = in_coeffs[j + 2] * scale[i];
                
                assert(coeffs[j] >= 0
                       && coeffs[j + 1] >= 0
                       && coeffs[j + 2] >= 0
                       && coeffs[j] + coeffs[j + 1] + coeffs[j + 2] < 1.5f*LabCbrtTabScale );
            }
        }
        
        virtual ~RGB2Lab() { }
        
        virtual void convert(const float* src, float* dst, const int n)
        {
            int i, scn = num_channels;
            
            float gscale = GammaTabScale;
            const float* gammaTab = srgb ? sRGBGammaTab : 0;
            float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
            C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
            C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];

            static const float _1_3 = 1.0f / 3.0f;
            static const float _a = 16.0f / 116.0f;
            
            for (i = 0; i < n*3; i += 3, src += scn )
            {
                float R = MATHEXT_CLIP(src[0]);
                float G = MATHEXT_CLIP(src[1]);
                float B = MATHEXT_CLIP(src[2]);
                
                if (gammaTab)
                {
                    R = mathext::splineInterpolate(R * gscale, gammaTab, GAMMA_TAB_SIZE);
                    G = mathext::splineInterpolate(G * gscale, gammaTab, GAMMA_TAB_SIZE);
                    B = mathext::splineInterpolate(B * gscale, gammaTab, GAMMA_TAB_SIZE);
                }
                float X = R*C0 + G*C1 + B*C2;
                float Y = R*C3 + G*C4 + B*C5;
                float Z = R*C6 + G*C7 + B*C8;
                
                float FX = X > 0.008856f ? std::pow(X, _1_3) : (7.787f * X + _a);
                float FY = Y > 0.008856f ? std::pow(Y, _1_3) : (7.787f * Y + _a);
                float FZ = Z > 0.008856f ? std::pow(Z, _1_3) : (7.787f * Z + _a);
                
                float L = Y > 0.008856f ? (116.f * FY - 16.f) : (903.3f * Y);
                float a = 500.f * (FX - FY);
                float b = 200.f * (FY - FZ);
                
                dst[i] = L;
                dst[i + 1] = a;
                dst[i + 2] = b;
            }
        }
        
    private:
        int     num_channels;
        float   coeffs[9];
        bool    srgb;
    };
    
    
    // Converts [0, 255] RGB, RGBA, BGR, BGRA colors to Lab
    
    template<>
    class RGB2Lab<unsigned char>  : public IColorConversion<unsigned char>
    {
    public:
        RGB2Lab(int in_num_channels,
                int in_blue_index,
                const float* in_coeffs,
                const float* in_whitept,
                bool in_srgb) : num_channels(in_num_channels) , srgb(in_srgb)
        {
            static volatile int _3 = 3;
            initLabTabs();
            
            if (!in_coeffs)
                in_coeffs = sRGB2XYZ_D65;
            if (!in_whitept)
                in_whitept = D65;
            
            float scale[] =
            {
                (1 << lab_shift)/in_whitept[0],
                (float)(1 << lab_shift),
                (1 << lab_shift)/in_whitept[2]
            };
            
            for( int i = 0; i < _3; i++ )
            {
                coeffs[i*3+(in_blue_index^2)]   = std::round(in_coeffs[i*3]*scale[i]);
                coeffs[i*3+1]                   = std::round(in_coeffs[i*3+1]*scale[i]);
                coeffs[i*3+in_blue_index]       = std::round(in_coeffs[i*3+2]*scale[i]);
                
                assert(coeffs[i] >= 0
                       && coeffs[i*3+1] >= 0
                       && coeffs[i*3+2] >= 0
                       && coeffs[i*3] + coeffs[i*3+1] + coeffs[i*3+2] < 2*(1 << lab_shift) );
            }
        }
        
        virtual ~RGB2Lab() { }
        
        virtual void convert(const unsigned char* src, unsigned char* dst, const int n)
        {
            const int Lscale = (116*255+50)/100;
            const int Lshift = -((16*255*(1 << lab_shift2) + 50)/100);
            
            const unsigned short* tab = srgb ? sRGBGammaTab_b : linearGammaTab_b;
            int i, scn = num_channels;
            
            int C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
            C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
            C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
            
            for( i = 0; i < n * 3; i += 3, src += scn )
            {
                int R = tab[src[0]], G = tab[src[1]], B = tab[src[2]];
                int fX = LabCbrtTab_b[MATHEXT_DESCALE(R*C0 + G*C1 + B*C2, lab_shift)];
                int fY = LabCbrtTab_b[MATHEXT_DESCALE(R*C3 + G*C4 + B*C5, lab_shift)];
                int fZ = LabCbrtTab_b[MATHEXT_DESCALE(R*C6 + G*C7 + B*C8, lab_shift)];
                
                int L = MATHEXT_DESCALE( Lscale*fY + Lshift, lab_shift2 );
                int a = MATHEXT_DESCALE( 500*(fX - fY) + 128*(1 << lab_shift2), lab_shift2 );
                int b = MATHEXT_DESCALE( 200*(fY - fZ) + 128*(1 << lab_shift2), lab_shift2 );
                
                dst[i] = mathext::saturate_cast<unsigned char>(L);
                dst[i+1] = mathext::saturate_cast<unsigned char>(a);
                dst[i+2] = mathext::saturate_cast<unsigned char>(b);
            }
        }
        
    private:
        int     num_channels;
        int     coeffs[9];
        bool    srgb;
        
    };
    
    // Converts Lab to [0.0, 1.0] RGB, RGBA, BGR, BGRA colors
    
    template <>
    class Lab2RGB<float> : public IColorConversion<float>
    {
    public:
        Lab2RGB(int in_num_channels,
                int in_blue_index,
                const float* in_coeffs,
                const float* in_whitept,
                bool in_srgb) : num_channels(in_num_channels) , srgb(in_srgb)
        {
            initLabTabs();
            
            if(!in_coeffs)
                in_coeffs = XYZ2sRGB_D65;
            if(!in_whitept)
                in_whitept = D65;
            
            for( int i = 0; i < 3; i++ )
            {
                coeffs[i+(in_blue_index^2)*3] =   in_coeffs[i]*in_whitept[i];
                coeffs[i+3] =               in_coeffs[i+3]*in_whitept[i];
                coeffs[i+in_blue_index*3] =       in_coeffs[i+6]*in_whitept[i];
            }
        }
        
        virtual void convert(const float* src, float* dst, const int n)
        {
            int i, dcn = num_channels;
            const float* gammaTab = srgb ? sRGBInvGammaTab : 0;
            float gscale = GammaTabScale;
            float C0 = coeffs[0], C1 = coeffs[1], C2 = coeffs[2],
            C3 = coeffs[3], C4 = coeffs[4], C5 = coeffs[5],
            C6 = coeffs[6], C7 = coeffs[7], C8 = coeffs[8];
            
            float alpha = FLOAT_COLOR_CHANNEL_MAX;
            
            static const float lThresh = 0.008856f * 903.3f;
            static const float fThresh = 7.787f * 0.008856f + 16.0f / 116.0f;
            
            for (i = 0; i < n*3; i += 3, dst += dcn)
            {
                float li = src[i];
                float ai = src[i + 1];
                float bi = src[i + 2];
                
                float y, fy;
                if (li <= lThresh)
                {
                    y = li / 903.3f;
                    fy = 7.787f * y + 16.0f / 116.0f;
                }
                else
                {
                    fy = (li + 16.0f) / 116.0f;
                    y = fy * fy * fy;
                }
                
                float fxz[] = { ai / 500.0f + fy, fy - bi / 200.0f };
                
                for (int j = 0; j < 2; j++)
                    if (fxz[j] <= fThresh)
                        fxz[j] = (fxz[j] - 16.0f / 116.0f) / 7.787f;
                    else
                        fxz[j] = fxz[j] * fxz[j] * fxz[j];
                
                
                float x = fxz[0], z = fxz[1];
                float ro = C0 * x + C1 * y + C2 * z;
                float go = C3 * x + C4 * y + C5 * z;
                float bo = C6 * x + C7 * y + C8 * z;
                
                ro = MATHEXT_CLIP(ro);
                go = MATHEXT_CLIP(go);
                bo = MATHEXT_CLIP(bo);
                
                if (gammaTab)
                {
                    ro = mathext::splineInterpolate(ro * gscale, gammaTab, GAMMA_TAB_SIZE);
                    go = mathext::splineInterpolate(go * gscale, gammaTab, GAMMA_TAB_SIZE);
                    bo = mathext::splineInterpolate(bo * gscale, gammaTab, GAMMA_TAB_SIZE);
                }
                
                dst[0] = ro, dst[1] = go, dst[2] = bo;
                if( dcn == 4 )
                    dst[3] = alpha;
            }
        }
        
    private:
        int num_channels;
        float coeffs[9];
        bool srgb;
    };
    
    // Converts Lab to [0, 255] RGB, RGBA, BGR, BGRA colors
    
    template <>
    class Lab2RGB<unsigned char> : public IColorConversion<unsigned char>
    {
    public:
        Lab2RGB(int in_num_channels,
                int in_blue_index,
                const float* in_coeffs,
                const float* in_whitept,
                bool in_srgb)// : num_channels(in_num_channels) , srgb(in_srgb)
        {
            //TODO
        }
        
        virtual void convert(const unsigned char* src, unsigned char* dst, const int n)
        {
            //TODO
        }
        
    private:
//        int num_channels;
//        float coeffs[9];
//        bool srgb;
    };
    
    template<typename T>
    IColorConversion<T>* CreateColorConversion(ConversionMode cvtmode, ColorMode colormode)
    {
        int num_channels = 3;
        int blue_index = 2;
        
        if(RGBA == colormode || BGRA == colormode)
        {
            num_channels = 4;
        }
        
        if(BGR == colormode || BGRA == colormode)
        {
            blue_index = 0;
        }
        
        if(sRGB_to_CIELAB == cvtmode || lRGB_to_CIELAB == cvtmode)
        {
            bool srgb = (sRGB_to_CIELAB == cvtmode) ? true : false;
            IColorConversion<T>* cvt = new RGB2Lab<T>(num_channels, blue_index, nullptr, nullptr, srgb);
            
            return cvt;
        }
        else if(CIELAB_sRGB == cvtmode || CIELAB_lRGB == cvtmode)
        {
            bool srgb = (CIELAB_sRGB == cvtmode) ? true : false;
            IColorConversion<T>* cvt = new Lab2RGB<T>(num_channels, blue_index, nullptr, nullptr, srgb);
            
            return cvt;
        }
        
        return nullptr;
    }
    
    static void RGB255_to_RGB01(const unsigned char* src, float* dst, const int n = 1)
    {
        for(int i = 0; i < n*3; i+=3)
        {
            dst[i + 0] = (float)src[i + 0] / 255.0f;
            dst[i + 1] = (float)src[i + 1] / 255.0f;
            dst[i + 2] = (float)src[i + 2] / 255.0f;
        }
    }
    
    static void RGB01_to_RGB255(const float* src, unsigned char* dst, const int n = 1)
    {
        for(int i = 0; i < n*3; i+=3)
        {
            dst[i + 0] = (unsigned char)(src[i + 0] * 255.0f);
            dst[i + 1] = (unsigned char)(src[i + 1] * 255.0f);
            dst[i + 2] = (unsigned char)(src[i + 2] * 255.0f);
        }
    }
    
    inline void print_Lab(const float* labcolor)
    {
        std::cout << "L: " << labcolor[0] << " a: " << labcolor[1] << " b: " << labcolor[2] << std::endl;
    }
    
    inline void print_RGB(const unsigned char* rgbcolor)
    {
        std::cout << "R: " << (int)rgbcolor[0] << " G: " << (int)rgbcolor[1] << " B: " << (int)rgbcolor[2] << std::endl;
    }
    
    inline void print_RGB(const float* rgbcolor)
    {
        std::cout << "R: " << rgbcolor[0] << " G: " << rgbcolor[1] << " B: " << rgbcolor[2] << std::endl;
    }
    
    
}


#endif /* colors_h */
