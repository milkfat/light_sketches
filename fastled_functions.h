#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef _max
#define _max(a,b) ((a)>(b)?(a):(b))
#endif

uint8_t sqrt16 (uint16_t x) {
    return sqrt(x);
}

uint8_t qadd8( uint8_t i, uint8_t j)
    {
    
        unsigned int t = i + j;
        if( t > 255) t = 255;
        return t;
   
    
    }

uint8_t ease8InOutApprox( uint8_t i)
{
    if( i < 64) {
        // start with slope 0.5
        i /= 2;
    } else if( i > (255 - 64)) {
        // end with slope 0.5
        i = 255 - i;
        i /= 2;
        i = 255 - i;
    } else {
        // in the middle, use slope 192/128 = 1.5
        i -= 64;
        i += (i / 2);
        i += 32;
    }

    return i;
}




uint8_t scale8( uint8_t i, uint8_t scale)
{
#if (FASTLED_SCALE8_FIXED == 1)
    return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
#else
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
#endif
}




/// ease8InOutQuad: 8-bit quadratic ease-in / ease-out function
///                Takes around 13 cycles on AVR

uint8_t ease8InOutQuad( uint8_t i)
{
    uint8_t j = i;
    if( j & 0x80 ) {
        j = 255 - j;
    }
    uint8_t jj  = scale8(  j, j);
    uint8_t jj2 = jj << 1;
    if( i & 0x80 ) {
        jj2 = 255 - jj2;
    }
    return jj2;
}

const uint8_t b_m16_interleave[] = { 0, 49, 49, 41, 90, 27, 117, 10 };

uint8_t sin8( uint8_t theta)
{
    uint8_t offset = theta;
    if( theta & 0x40 ) {
        offset = (uint8_t)255 - offset;
    }
    offset &= 0x3F; // 0..63

    uint8_t secoffset  = offset & 0x0F; // 0..15
    if( theta & 0x40) secoffset++;

    uint8_t section = offset >> 4; // 0..3
    uint8_t s2 = section * 2;
    const uint8_t* p = b_m16_interleave;
    p += s2;
    uint8_t b   =  *p;
    p++;
    uint8_t m16 =  *p;

    uint8_t mx = (m16 * secoffset) >> 4;

    int8_t y = mx + b;
    if( theta & 0x80 ) y = -y;

    y += 128;

    return y;
}

uint8_t cos8( uint8_t theta)
{
    return sin8( theta + 64);
}


int16_t sin16( uint16_t theta )
{
    static const uint16_t base[] =
    { 0, 6393, 12539, 18204, 23170, 27245, 30273, 32137 };
    static const uint8_t slope[] =
    { 49, 48, 44, 38, 31, 23, 14, 4 };

    uint16_t offset = (theta & 0x3FFF) >> 3; // 0..2047
    if( theta & 0x4000 ) offset = 2047 - offset;

    uint8_t section = offset / 256; // 0..7
    uint16_t b   = base[section];
    uint8_t  m   = slope[section];

    uint8_t secoffset8 = (uint8_t)(offset) / 2;

    uint16_t mx = m * secoffset8;
    int16_t  y  = mx + b;

    if( theta & 0x8000 ) y = -y;

    return y;
}

int16_t cos16( uint16_t theta)
{
    return sin16( theta + 16384);
}


//create a datatype that is clamped to 0-255 with no overflow
typedef class clampedByte
{
    private:
        uint8_t val;

    public:
        //conversion to Uint8
        //operator Uint8() {
        //    return val;
        //} 

        //conversion to int, allow pass by reference (I think)
        operator uint8_t&() {
            return val;
        }
        


        //default uninitialized
        clampedByte() {};

        //conversion from int
        clampedByte(int n) {
            val = _max(_min(n,255),0);
        }

        //overload +=
         clampedByte& operator+= (const int& rhs) {
            if (this->val + rhs > 255) {
                this->val = 255;
            } else {
                this->val += rhs;
            }
            return *this;
         }

        //overload -=
        clampedByte& operator-= (const int& rhs) {
           if (this->val - rhs < 0) {
               this->val = 0;
           } else {
               this->val -= rhs;
           }
           return *this;
        }
 
        //overload =
        clampedByte& operator= (const int& rhs) {
           if (rhs > 255) {
               this->val = 255;
           } else {
               this->val = rhs;
           }
           return *this;
        }

} clampedByte;


void nscale8x3( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t scale)
{
    r = ((int)r * (int)(scale) ) >> 8;
    g = ((int)g * (int)(scale) ) >> 8;
    b = ((int)b * (int)(scale) ) >> 8;
}


typedef struct CRGB
{

    enum  	HTMLColorCode { 
        AliceBlue =0xF0F8FF, Amethyst =0x9966CC, AntiqueWhite =0xFAEBD7, Aqua =0x00FFFF, 
        Aquamarine =0x7FFFD4, Azure =0xF0FFFF, Beige =0xF5F5DC, Bisque =0xFFE4C4, 
        Black =0x000000, BlanchedAlmond =0xFFEBCD, Blue =0x0000FF, BlueViolet =0x8A2BE2, 
        Brown =0xA52A2A, BurlyWood =0xDEB887, CadetBlue =0x5F9EA0, Chartreuse =0x7FFF00, 
        Chocolate =0xD2691E, Coral =0xFF7F50, CornflowerBlue =0x6495ED, Cornsilk =0xFFF8DC, 
        Crimson =0xDC143C, Cyan =0x00FFFF, DarkBlue =0x00008B, DarkCyan =0x008B8B, 
        DarkGoldenrod =0xB8860B, DarkGray =0xA9A9A9, DarkGrey =0xA9A9A9, DarkGreen =0x006400, 
        DarkKhaki =0xBDB76B, DarkMagenta =0x8B008B, DarkOliveGreen =0x556B2F, DarkOrange =0xFF8C00, 
        DarkOrchid =0x9932CC, DarkRed =0x8B0000, DarkSalmon =0xE9967A, DarkSeaGreen =0x8FBC8F, 
        DarkSlateBlue =0x483D8B, DarkSlateGray =0x2F4F4F, DarkSlateGrey =0x2F4F4F, DarkTurquoise =0x00CED1, 
        DarkViolet =0x9400D3, DeepPink =0xFF1493, DeepSkyBlue =0x00BFFF, DimGray =0x696969, 
        DimGrey =0x696969, DodgerBlue =0x1E90FF, FireBrick =0xB22222, FloralWhite =0xFFFAF0, 
        ForestGreen =0x228B22, Fuchsia =0xFF00FF, Gainsboro =0xDCDCDC, GhostWhite =0xF8F8FF, 
        Gold =0xFFD700, Goldenrod =0xDAA520, Gray =0x808080, Grey =0x808080, 
        Green =0x008000, GreenYellow =0xADFF2F, Honeydew =0xF0FFF0, HotPink =0xFF69B4, 
        IndianRed =0xCD5C5C, Indigo =0x4B0082, Ivory =0xFFFFF0, Khaki =0xF0E68C, 
        Lavender =0xE6E6FA, LavenderBlush =0xFFF0F5, LawnGreen =0x7CFC00, LemonChiffon =0xFFFACD, 
        LightBlue =0xADD8E6, LightCoral =0xF08080, LightCyan =0xE0FFFF, LightGoldenrodYellow =0xFAFAD2, 
        LightGreen =0x90EE90, LightGrey =0xD3D3D3, LightPink =0xFFB6C1, LightSalmon =0xFFA07A, 
        LightSeaGreen =0x20B2AA, LightSkyBlue =0x87CEFA, LightSlateGray =0x778899, LightSlateGrey =0x778899, 
        LightSteelBlue =0xB0C4DE, LightYellow =0xFFFFE0, Lime =0x00FF00, LimeGreen =0x32CD32, 
        Linen =0xFAF0E6, Magenta =0xFF00FF, Maroon =0x800000, MediumAquamarine =0x66CDAA, 
        MediumBlue =0x0000CD, MediumOrchid =0xBA55D3, MediumPurple =0x9370DB, MediumSeaGreen =0x3CB371, 
        MediumSlateBlue =0x7B68EE, MediumSpringGreen =0x00FA9A, MediumTurquoise =0x48D1CC, MediumVioletRed =0xC71585, 
        MidnightBlue =0x191970, MintCream =0xF5FFFA, MistyRose =0xFFE4E1, Moccasin =0xFFE4B5, 
        NavajoWhite =0xFFDEAD, Navy =0x000080, OldLace =0xFDF5E6, Olive =0x808000, 
        OliveDrab =0x6B8E23, Orange =0xFFA500, OrangeRed =0xFF4500, Orchid =0xDA70D6, 
        PaleGoldenrod =0xEEE8AA, PaleGreen =0x98FB98, PaleTurquoise =0xAFEEEE, PaleVioletRed =0xDB7093, 
        PapayaWhip =0xFFEFD5, PeachPuff =0xFFDAB9, Peru =0xCD853F, Pink =0xFFC0CB, 
        Plaid =0xCC5533, Plum =0xDDA0DD, PowderBlue =0xB0E0E6, Purple =0x800080, 
        Red =0xFF0000, RosyBrown =0xBC8F8F, RoyalBlue =0x4169E1, SaddleBrown =0x8B4513, 
        Salmon =0xFA8072, SandyBrown =0xF4A460, SeaGreen =0x2E8B57, Seashell =0xFFF5EE, 
        Sienna =0xA0522D, Silver =0xC0C0C0, SkyBlue =0x87CEEB, SlateBlue =0x6A5ACD, 
        SlateGray =0x708090, SlateGrey =0x708090, Snow =0xFFFAFA, SpringGreen =0x00FF7F, 
        SteelBlue =0x4682B4, Tan =0xD2B48C, Teal =0x008080, Thistle =0xD8BFD8, 
        Tomato =0xFF6347, Turquoise =0x40E0D0, Violet =0xEE82EE, Wheat =0xF5DEB3, 
        White =0xFFFFFF, WhiteSmoke =0xF5F5F5, Yellow =0xFFFF00, YellowGreen =0x9ACD32, 
        FairyLight =0xFFE42D, FairyLightNCC =0xFF9D2A 
    };

    clampedByte r; //red
    clampedByte g; //green
    clampedByte b; //blue

    //overload for adding two CRGB objects together
    //CRGB& operator+= (CRGB& rhs) { <--- would this be more efficient? doesn't work with the current CHSV to CRGB conversion
    CRGB& operator+= (CRGB rhs) {
        this->r += rhs.r;
        this->g += rhs.g;
        this->b += rhs.b;
        return *this;
    }
    CRGB& operator+= (uint8_t rhs) {
        this->r += rhs;
        this->g += rhs;
        this->b += rhs;
        return *this;
    }
    CRGB& operator-= (CRGB rhs) {
        this->r -= rhs.r;
        this->g -= rhs.g;
        this->b -= rhs.b;
        return *this;
    }
    CRGB& operator*= (float rhs) {
        this->r *= rhs;
        this->g *= rhs;
        this->b *= rhs;
        return *this;
    }
    // default values are UNINITIALIZED
	inline CRGB() __attribute__((always_inline))
    {
    }

    /// allow construction from 32-bit (really 24-bit) bit 0xRRGGBB color code
    inline CRGB( uint32_t colorcode)  __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }

    /// allow construction from R, G, B
    inline CRGB( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline))
        : r(ir), g(ig), b(ib)
    {
    }


    /// scale down a RGB to N 256ths of it's current brightness, using
    /// 'plain math' dimming rules, which means that if the low light levels
    /// may dim all the way to 100% black.
    inline CRGB& nscale8 (uint8_t scaledown )
    {
        nscale8x3( r, g, b, scaledown);
        return *this;
    }

    /// fadeToBlackBy is a synonym for nscale8( ..., 255-fadefactor)
    inline CRGB& fadeToBlackBy (uint8_t fadefactor )
    {
        nscale8x3( r, g, b, 255 - fadefactor);
        return *this;
    }

    /// divide each of the channels by a constant
    inline CRGB& operator/= (uint8_t d )
    {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }
    
    
} CRGB;


void nscale8( CRGB* leds, uint16_t num_leds, uint8_t scale)
{
    for( uint16_t i = 0; i < num_leds; i++) {
        leds[i].nscale8( scale);
    }
}

void fadeToBlackBy( CRGB* leds, uint16_t num_leds, uint8_t fadeBy)
{
    nscale8( leds, num_leds, 255 - fadeBy);
}

typedef struct CHSV
{
    clampedByte h; //hue
    clampedByte s; //saturation
    clampedByte v; //value

    //constructor
    CHSV(uint8_t ih = 0, uint8_t is = 0, uint8_t iv = 0) {
        h = ih;
        s = is;
        v = iv;
    }

    //header for CHSV to CRGB conversion
    operator CRGB();

} CHSV;


CRGB HsvToRgb(CHSV hsv)
{
    CRGB rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}
void hsv2rgb_rainbow(CHSV hsv, CRGB & rgb) {
    rgb = HsvToRgb(hsv);
}

void hsv2rgb_spectrum(CHSV hsv, CRGB & rgb) {
    rgb = HsvToRgb(hsv);
}

CHSV RgbToHsv(CRGB rgb)
{
    CHSV hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

//conversion from CHSV to CRGB
CHSV::operator CRGB() {
    CRGB rgb = HsvToRgb(*this);
    return rgb;
} 




#define P(x) p[x]

static uint8_t const p[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,151
   };


static int8_t inline __attribute__((always_inline)) lerp7by8( int8_t a, int8_t b, uint8_t frac)
{
    // int8_t delta = b - a;
    // int16_t prod = (uint16_t)delta * (uint16_t)frac;
    // int8_t scaled = prod >> 8;
    // int8_t result = a + scaled;
    // return result;
    int8_t result;
    if( b > a) {
        uint8_t delta = b - a;
        uint8_t scaled = scale8( delta, frac);
        result = a + scaled;
    } else {
        uint8_t delta = a - b;
        uint8_t scaled = scale8( delta, frac);
        result = a - scaled;
    }
    return result;
}

static int8_t  inline __attribute__((always_inline)) grad8(uint8_t hash, int8_t x, int8_t y, int8_t z) {

  switch(hash & 0xF) {
    case  0: return (( x) + ( y))>>1;
    case  1: return ((-x) + ( y))>>1;
    case  2: return (( x) + (-y))>>1;
    case  3: return ((-x) + (-y))>>1;
    case  4: return (( x) + ( z))>>1;
    case  5: return ((-x) + ( z))>>1;
    case  6: return (( x) + (-z))>>1;
    case  7: return ((-x) + (-z))>>1;
    case  8: return (( y) + ( z))>>1;
    case  9: return ((-y) + ( z))>>1;
    case 10: return (( y) + (-z))>>1;
    case 11: return ((-y) + (-z))>>1;
    case 12: return (( y) + ( x))>>1;
    case 13: return ((-y) + ( z))>>1;
    case 14: return (( y) + (-x))>>1;
    case 15: return ((-y) + (-z))>>1;
  }
  return 0;
}

#define EASE8(x)  (ease8InOutQuad(x) )

int8_t inoise8_raw(uint16_t x, uint16_t y, uint16_t z)
{
  // Find the unit cube containing the point
  uint8_t X = x>>8;
  uint8_t Y = y>>8;
  uint8_t Z = z>>8;

  // Hash cube corner coordinates
  uint8_t A = P(X)+Y;
  uint8_t AA = P(A)+Z;
  uint8_t AB = P(A+1)+Z;
  uint8_t B = P(X+1)+Y;
  uint8_t BA = P(B) + Z;
  uint8_t BB = P(B+1)+Z;

  // Get the relative position of the point in the cube
  uint8_t u = x;
  uint8_t v = y;
  uint8_t w = z;

  // Get a signed version of the above for the grad function
  int8_t xx = ((uint8_t)(x)>>1) & 0x7F;
  int8_t yy = ((uint8_t)(y)>>1) & 0x7F;
  int8_t zz = ((uint8_t)(z)>>1) & 0x7F;
  uint8_t N = 0x80;

  u = EASE8(u); v = EASE8(v); w = EASE8(w);
    
  int8_t X1 = lerp7by8(grad8(P(AA), xx, yy, zz), grad8(P(BA), xx - N, yy, zz), u);
  int8_t X2 = lerp7by8(grad8(P(AB), xx, yy-N, zz), grad8(P(BB), xx - N, yy - N, zz), u);
  int8_t X3 = lerp7by8(grad8(P(AA+1), xx, yy, zz-N), grad8(P(BA+1), xx - N, yy, zz-N), u);
  int8_t X4 = lerp7by8(grad8(P(AB+1), xx, yy-N, zz-N), grad8(P(BB+1), xx - N, yy - N, zz - N), u);

  int8_t Y1 = lerp7by8(X1,X2,v);
  int8_t Y2 = lerp7by8(X3,X4,v);

  int8_t ans = lerp7by8(Y1,Y2,w);

  return ans;
}

uint8_t inoise8(uint16_t x, uint16_t y, uint16_t z) {
//  return scale8(76+(inoise8_raw(x,y,z)),215)<<1;
    int8_t n = inoise8_raw( x, y, z);  // -64..+64
    n+= 64;                            //   0..128
    uint8_t ans = qadd8( n, n);        //   0..255
    return ans;
}





///  ANSI: unsigned _Fract.  range is 0 to 0.99998474121
///                 in steps of 0.00001525878
typedef uint16_t  fract16;  ///< ANSI: unsigned       _Fract

typedef uint8_t  fract8;  ///< ANSI: unsigned       _Fract


#define FADE(x) scale16(x,x)
#define LERP(a,b,u) lerp15by16(a,b,u)



/// scale a 16-bit unsigned value by a 16-bit value,
///         considered as numerator of a fraction whose denominator
///         is 65536. In other words, it computes i * (scale / 65536)

uint16_t scale16( uint16_t i, fract16 scale )
{
    uint16_t result;
//    result = ((uint32_t)(i) * (1+(uint32_t)(scale))) / 65536;
    result = ((uint32_t)(i) * (uint32_t)(scale)) / 65536;
    return result;
}



/// linear interpolation between two signed 15-bit values,
/// with 8-bit fraction
int16_t lerp15by16( int16_t a, int16_t b, fract16 frac)
{
    int16_t result;
    if( b > a) {
        uint16_t delta = b - a;
        uint16_t scaled = scale16( delta, frac);
        result = a + scaled;
    } else {
        uint16_t delta = a - b;
        uint16_t scaled = scale16( delta, frac);
        result = a - scaled;
    }
    return result;
}


/// ease16InOutQuad: 16-bit quadratic ease-in / ease-out function
// C implementation at this point
uint16_t ease16InOutQuad( uint16_t i)
{
    uint16_t j = i;
    if( j & 0x8000 ) {
        j = 65535 - j;
    }
    uint16_t jj  = scale16( j, j);
    uint16_t jj2 = jj << 1;
    if( i & 0x8000 ) {
        jj2 = 65535 - jj2;
    }
    return jj2;
}

#define AVG15(U,V) (((U)+(V)) >> 1)

#define EASE16(x) (ease16InOutQuad(x))

static int16_t inline __attribute__((always_inline))  grad16(uint8_t hash, int16_t x, int16_t y, int16_t z) {
#if 0
  switch(hash & 0xF) {
    case  0: return (( x) + ( y))>>1;
    case  1: return ((-x) + ( y))>>1;
    case  2: return (( x) + (-y))>>1;
    case  3: return ((-x) + (-y))>>1;
    case  4: return (( x) + ( z))>>1;
    case  5: return ((-x) + ( z))>>1;
    case  6: return (( x) + (-z))>>1;
    case  7: return ((-x) + (-z))>>1;
    case  8: return (( y) + ( z))>>1;
    case  9: return ((-y) + ( z))>>1;
    case 10: return (( y) + (-z))>>1;
    case 11: return ((-y) + (-z))>>1;
    case 12: return (( y) + ( x))>>1;
    case 13: return ((-y) + ( z))>>1;
    case 14: return (( y) + (-x))>>1;
    case 15: return ((-y) + (-z))>>1;
  }
#else
  hash = hash&15;
  int16_t u = hash<8?x:y;
  int16_t v = hash<4?y:hash==12||hash==14?x:z;
  if(hash&1) { u = -u; }
  if(hash&2) { v = -v; }

  return AVG15(u,v);
#endif
}

int16_t inoise16_raw(uint32_t x, uint32_t y, uint32_t z)
{
  // Find the unit cube containing the point
  uint8_t X = (x>>16)&0xFF;
  uint8_t Y = (y>>16)&0xFF;
  uint8_t Z = (z>>16)&0xFF;

  // Hash cube corner coordinates
  uint8_t A = P(X)+Y;
  uint8_t AA = P(A)+Z;
  uint8_t AB = P(A+1)+Z;
  uint8_t B = P(X+1)+Y;
  uint8_t BA = P(B) + Z;
  uint8_t BB = P(B+1)+Z;

  // Get the relative position of the point in the cube
  uint16_t u = x & 0xFFFF;
  uint16_t v = y & 0xFFFF;
  uint16_t w = z & 0xFFFF;

  // Get a signed version of the above for the grad function
  int16_t xx = (u >> 1) & 0x7FFF;
  int16_t yy = (v >> 1) & 0x7FFF;
  int16_t zz = (w >> 1) & 0x7FFF;
  uint16_t N = 0x8000L;

  u = EASE16(u); v = EASE16(v); w = EASE16(w);

  // skip the log fade adjustment for the moment, otherwise here we would
  // adjust fade values for u,v,w
  int16_t X1 = LERP(grad16(P(AA), xx, yy, zz), grad16(P(BA), xx - N, yy, zz), u);
  int16_t X2 = LERP(grad16(P(AB), xx, yy-N, zz), grad16(P(BB), xx - N, yy - N, zz), u);
  int16_t X3 = LERP(grad16(P(AA+1), xx, yy, zz-N), grad16(P(BA+1), xx - N, yy, zz-N), u);
  int16_t X4 = LERP(grad16(P(AB+1), xx, yy-N, zz-N), grad16(P(BB+1), xx - N, yy - N, zz - N), u);

  int16_t Y1 = LERP(X1,X2,v);
  int16_t Y2 = LERP(X3,X4,v);

  int16_t ans = LERP(Y1,Y2,w);

  return ans;
}

uint16_t inoise16(uint32_t x, uint32_t y, uint32_t z) {
  int32_t ans = inoise16_raw(x,y,z);
  ans = ans + 19052L;
  uint32_t pan = ans;
  // pan = (ans * 220L) >> 7.  That's the same as:
  // pan = (ans * 440L) >> 8.  And this way avoids a 7X four-byte shift-loop on AVR.
  // Identical math, except for the highest bit, which we don't care about anyway,
  // since we're returning the 'middle' 16 out of a 32-bit value anyway.
  pan *= 440L;
  return (pan>>8);

  // // return scale16by8(pan,220)<<1;
  // return ((inoise16_raw(x,y,z)+19052)*220)>>7;
  // return scale16by8(inoise16_raw(x,y,z)+19052,220)<<1;
}


/// blend a variable proproportion(0-255) of one byte to another
/// @param a - the starting byte value
/// @param b - the byte value to blend toward
/// @param amountOfB - the proportion (0-255) of b to blend
/// @returns a byte value between a and b, inclusive

uint8_t blend8( uint8_t a, uint8_t b, uint8_t amountOfB)
{
    uint16_t partial;
    uint8_t result;
    
    uint8_t amountOfA = 255 - amountOfB;
    
    partial = (a * amountOfA);

    partial += (b * amountOfB);

    result = partial >> 8;
    
    return result;

}


CRGB& nblend( CRGB& existing, CRGB& overlay, fract8 amountOfOverlay )
{
    if( amountOfOverlay == 0) {
        return existing;
    }

    if( amountOfOverlay == 255) {
        existing = overlay;
        return existing;
    }

    // Corrected blend method, with no loss-of-precision rounding errors
    existing.r   = blend8( existing.r,   overlay.r,   amountOfOverlay);
    existing.g = blend8( existing.g, overlay.g, amountOfOverlay);
    existing.b  = blend8( existing.b,  overlay.b,  amountOfOverlay);
    
    return existing;
}

void memset8(void * ptr, uint8_t value, uint16_t num) {
    memset(ptr, value, num);
}