#include "esphome.h"

using namespace std;

// Declare functions before calling them.
int  convert_float_to_color_temp(float);
int  convert_float_to_color_rain(float);
int  convert_float_to_color_temp(float color_in_float) {
                    
              if (color_in_float<= -23 && color_in_float >= -34) return 0x1f4799;
              if (color_in_float<= -16 && color_in_float >= -22) return 0x0262a9;
              if (color_in_float<= -11 && color_in_float >= -15) return 0x2374b6;
              if (color_in_float<= -6 && color_in_float >= -10) return 0x3789c6;
              if (color_in_float<= -3 && color_in_float >= -5) return 0x43a3d9;
              if (color_in_float<= -0 && color_in_float >= -2) return 0x51bfed;
              if (color_in_float>= 1 && color_in_float <= 2) return 0xaedcd8;
              if (color_in_float>= 3 && color_in_float <= 4) return 0xaad6ae;               
              if (color_in_float>= 5 && color_in_float <= 6) return 0x9fcd80;  
              if (color_in_float>= 7 && color_in_float <= 8) return 0xafd251;                        
              if (color_in_float>= 9 && color_in_float <= 10) return 0xd0d73e;
              if (color_in_float>= 11 && color_in_float <= 12) return 0xfedb00; 
              if (color_in_float>= 13 && color_in_float <= 14) return 0xfcc90d;  
              if (color_in_float>= 15 && color_in_float <= 16) return 0xfbb616;
              if (color_in_float>= 17 && color_in_float <= 18) return 0xfaa31a;
              if (color_in_float>= 19 && color_in_float <= 20) return 0xf68a1f;
              if (color_in_float>= 21 && color_in_float <= 24) return 0xf26a30;
              if (color_in_float>= 25 && color_in_float <= 29) return 0xeb5038;
              if (color_in_float>= 30 && color_in_float <= 35) return 0xee2d29;
              if (color_in_float>= 36 && color_in_float <= 40) {
                    return 0xc12026;
               } else 
                   {
                        return 0x9a1b1e;
                   }
}

int  convert_float_to_color_rain(float color_in_float) {                    
              if (color_in_float<=  5 ) return 0xc9c9c9;
              if (color_in_float>= 6 && color_in_float <= 15) return 0xac1e8f9;
              if (color_in_float>= 16 && color_in_float <= 30) return 0xa69c9f1;               
              if (color_in_float>= 31 && color_in_float <= 50) return 0x29b0eb;  
              if (color_in_float>= 51 && color_in_float <= 75)  {
                return 0x1286b8;        
              }
              else
                  {
                      return 0xc1a56b0;
                  }                

}

int  convert_float_to_color_uv(float color_in_float) {                    
              if (color_in_float<=  0 ) return 0x757575;
              if (color_in_float>= 1 && color_in_float <= 2) return 0x41AA48;
              if (color_in_float>= 3 && color_in_float <= 5) return 0xFAE300;               
              if (color_in_float>= 6 && color_in_float <= 7) return 0xF37721;  
              if (color_in_float>= 8 && color_in_float <= 10)  {
                return 0xE22426;               
              }
              else
                  {
                    return 0xA289BF;
                  }          
}