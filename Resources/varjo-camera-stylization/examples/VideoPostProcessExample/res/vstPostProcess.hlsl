// Copyright 2020 Varjo Technologies Oy. All rights reserved.

// This is example shader for showcasing how to use video post process filters from
// your own application. In your application, implement your own shader that suits
// your needs.

// Compute shader thread block size
#define BLOCK_SIZE (8)

// Debug texture output modes
#define DEBUG_TEXTURE_OUT_NONE (0)
#define DEBUG_TEXTURE_OUT_RGB (1)
#define DEBUG_TEXTURE_OUT_RG_A (2)

// TODO: We could get this mode in constant buffer to be able to switch runtime
#define DEBUG_TEXTURE_OUT (DEBUG_TEXTURE_OUT_NONE)
//#define DEBUG_TEXTURE_OUT (DEBUG_TEXTURE_OUT_RG_A)

// -------------------------------------------------------------------------

// Shader input layout V1 built-in bindings

// Input buffers: t0 = Camera image input
Texture2D<float4> inputTex : register(t0);  // NOTICE! This is sRGBA data bound as RGBA so fetched values are in screen gamma.

// Output buffers: u0 = Camera image output
RWTexture2D<float4> outputTex : register(u0);

// Texture samplers
SamplerState SamplerLinearClamp : register(s0);
SamplerState SamplerLinearWrap : register(s1);

// Varjo generic constants
cbuffer ConstantBuffer : register(b0)
{
    int2 sourceSize;   // Source texture dimensions
    float sourceTime;  // Source texture timestamp
    int viewIndex;     // View to be rendered: 0=LC, 1=RC, 2=LF, 3=RF
    int4 destRect;     // Destination rectangle: x, y, w, h
}

// -------------------------------------------------------------------------

// Shader specific constants
cbuffer ConstantBuffer : register(b1)
{

    // Noise texture
    float noiseAmount;  // Noise amount: 0=off, 1=full
    float noiseScale;   // Noise scale

    int clusterSize;
    float4 outlineColor;
    float outlineStrength;

    float blah;

}

// Shader specific textures
Texture2D<float4> noiseTexture : register(t1);

// -------------------------------------------------------------------------

static const float Epsilon = 1e-10;

float3 convertRGBtoHCV(in float3 RGB)
{
    float4 P = (RGB.g < RGB.b) ? float4(RGB.bg, -1.0, 2.0 / 3.0) : float4(RGB.gb, 0.0, -1.0 / 3.0);
    float4 Q = (RGB.r < P.x) ? float4(P.xyw, RGB.r) : float4(RGB.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    float H = abs((Q.w - Q.y) / (6 * C + Epsilon) + Q.z);
    return float3(H, C, Q.x);
}

// Converts given RGB color to HSV color. RGB = [0..1], HSV = [0..1].
float3 convertRGBtoHSV(in float3 RGB)
{
    float3 HCV = convertRGBtoHCV(RGB);
    float S = HCV.y / (HCV.z + Epsilon);
    return float3(HCV.x, S, HCV.z);
}

// Converts HSV hue channel value to RGB color. H = [0..1].
float3 hueToRGB(in float H)
{
    float R = abs(H * 6.0f - 3.0f) - 1.0f;
    float G = 2.0f - abs(H * 6.0f - 2.0f);
    float B = 2.0f - abs(H * 6.0f - 4.0f);
    return saturate(float3(R, G, B));
}

// Converts given HSV color to RGB color. HSV = [0..1], RGB = [0..1].
float3 hsvToRGB(in float3 HSV) {
    float3 RGB = hueToRGB(HSV.x);
    return ((RGB - 1.0f) * HSV.y + 1.0f) * HSV.z;
}

float sobelEdgeDetection(in float2 uv) {

    float3x3 gx = float3x3(-1, 0, 1, -2, 0, 2, -1, 0, 1);
    float3x3 gy = float3x3(-1, -2, -1, 0, 0, 0, 1, 2, 1);

    float pixelSumX = 0.0;
    float pixelSumY = 0.0;

    for (int y = -1; y < 2; y++) {
        for (int x = -1; x < 2; x++) {
        
            float4 pixel = inputTex.SampleLevel(SamplerLinearClamp, uv + float2((float) y / sourceSize[0], (float) x / sourceSize[1]), 0.0, 0.0);

            float grayscaleValue = (pixel.r + pixel.g + pixel.b) / 3;
        
            pixelSumX += grayscaleValue * gx[y + 1][x + 1];
            pixelSumY += grayscaleValue * gy[y + 1][x + 1];
        }
    }

    return abs(pixelSumX / 3) + abs(pixelSumY / 3);
}

// -------------------------------------------------------------------------

// Shader entrypoint
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)] void main(uint3 dispatchThreadID
                                                  : SV_DispatchThreadID) {
    // Calculate thread coordinates
    const int2 thisThread = dispatchThreadID.xy + int2(destRect.xy);
    const float2 uv = float2(thisThread) / sourceSize;

    // Load source sample
    float4 origColor = inputTex.Load(int3(thisThread.xy, 0)).rgba;
    float4 color = origColor;


    // // Grayscale clustering
    // float grayscaleValue = round((color.r + color.g + color.b) / 3 * clusterSize) / clusterSize;
    // float3 clusteredColor = float3(grayscaleValue, grayscaleValue, grayscaleValue);

    //Color clustering


    if (clusterSize > 0) {
        color.rgb = round(color.rgb * clusterSize) / clusterSize;
    }


    // if (outlineStrength > 0.0f) {
    //     float edgeValue = sobelEdgeDetection(uv);

    //     if (edgeValue > 0.1) {
    //         color.rgb = outlineColor.rgb;
    //         //color.rgb = hsvToRGB(outlineColor.rgb);
    //     } 
    // }

    outputTex[thisThread.xy] = float4(blah, blah, blah, origColor.a);
}