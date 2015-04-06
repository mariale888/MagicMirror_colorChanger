// fragment shader

#version 150

// this is how we receive the texture thanks to bind()
uniform sampler2DRect inputTexture;
uniform sampler2DRect mask;
in vec2 texCoordVarying;
uniform float hueAdjust;


out vec4 outputColor; // anything marked "out" foes to next step in pipeline

void main()
{
    
    // as reference: http://en.wikipedia.org/wiki/YCbCr
    const vec4  kRGBToYPrime = vec4 (0.299, 0.587, 0.114, 0.0);
    const vec4  kRGBToI      = vec4 (0.596, -0.275, -0.321, 0.0);
    const vec4  kRGBToQ      = vec4 (0.212, -0.523, 0.311, 0.0);
    
    const vec4  kYIQToR  = vec4 (1.0, 0.956, 0.621, 0.0);
    const vec4  kYIQToG  = vec4 (1.0, -0.272, -0.647, 0.0);
    const vec4  kYIQToB  = vec4 (1.0, -1.107, 1.704, 0.0);
    
    // Sample the input pixel

    vec4 color = texture(inputTexture, texCoordVarying);
    vec4 color1 = texture(mask, texCoordVarying);
    
    // Convert to YIQ
    float YPrime  = dot (color, kRGBToYPrime);
    float I      = dot (color, kRGBToI);
    float Q      = dot (color, kRGBToQ);
    
    // Calculate the hue and chroma
    float hue     = atan (Q, I);
    float chroma  = sqrt (I * I + Q * Q);
    
    // Make the user's adjustments
    hue += hueAdjust;
    
    // Convert back to YIQ
    Q = chroma * sin (hue);
    I = chroma * cos (hue);
    
    // Convert back to RGB
    vec4 yIQ = vec4 (YPrime, I, Q, 0.0);
    if( !(color.r == 1 && color.g == 1 && color.b ==1) ) {
        color.r = dot (yIQ, kYIQToR);
        color.g = dot (yIQ, kYIQToG);
        color.b = dot (yIQ, kYIQToB);
       
    }
    else color =vec4(color1.r,color1.g,color1.b,1);
   
    // Save the result
    outputColor = color;
  
}