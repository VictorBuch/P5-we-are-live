
// Copyright 2020 Varjo Technologies Oy. All rights reserved.

#pragma once

// Compile time flag to disable swapchain creation, rendering, and layer submission.
// In headless mode this application only alters the video-see-through image feed,
// but does not render anything by itself.
#define USE_HEADLESS_MODE 0

#include <glm/glm.hpp>

#include "Globals.hpp"
#include "PostProcess.hpp"
#include "TestTexture.hpp"

//! Application state struct
struct AppState {
    // General params structure
    struct General {
        double frameTime = 0.0f;   //!< Current frame time
        int64_t frameCount = 0;    //!< Current frame count
        bool mrAvailable = false;  //!< Mixed reality available flag
        bool vstEnabled = true;    //!< Render VST image flag
#if (!USE_HEADLESS_MODE)
        bool vrEnabled = false;  //!< Render VR scene flag
#endif
    } general;

    // VST Post process params
    struct PostProcess {
        bool enabled = false;
        VarjoExamples::PostProcess::ShaderSource shaderSource = VarjoExamples::PostProcess::ShaderSource::None;
        VarjoExamples::PostProcess::GraphicsAPI graphicsAPI = VarjoExamples::PostProcess::GraphicsAPI::None;
        TestTexture::Type textureType = TestTexture::Type::Noise;

        // Color clustering params
        bool colorClusteringEnabled = true;
        int clusterSize = 10;

        // Outlines params
        bool outlinesEnabled = true;
        glm::vec4 outlineColor = glm::vec4(0.4f, 0.5f, 0.7f, 1.0f);
        float outlineStrength = 1.0f;

        // Texture params
        bool textureEnabled = true;
        bool textureGeneratedOnGPU = true;
        float textureAmount = 0.1f;
        float textureScale = 1.0f;
#

    } postProcess;
};