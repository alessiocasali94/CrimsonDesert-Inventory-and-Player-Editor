#pragma once

#include "imgui.h"

namespace trainer
{
    void SetBackgroundTexture(ImTextureID texture, const ImVec2& size);
    void SetPosterTexture(ImTextureID texture, const ImVec2& size);
    bool ConsumeCloseRequested();
    void RenderSplashScreen(float progress);
    void RenderTrainerPanel();
}
