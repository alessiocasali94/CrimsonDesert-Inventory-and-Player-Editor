#include "TrainerPanel.h"

#include "imgui.h"

#include <array>
#include <string>

namespace trainer
{
    namespace
    {
        enum class TrainerTab
        {
            Functions,
            Options
        };

        struct FeatureEntry
        {
            const char* Label;
            bool Enabled;
        };

        struct TrainerUiState
        {
            TrainerTab ActiveTab = TrainerTab::Functions;
            bool CloseRequested = false;
            std::array<FeatureEntry, 10> PlayerFeatures = {
                FeatureEntry{ "God Mode", false },
                FeatureEntry{ "Infinite Health", false },
                FeatureEntry{ "Infinite Mana", false },
                FeatureEntry{ "Infinite Breath", false },
                FeatureEntry{ "Infinite Flight", false },
                FeatureEntry{ "No Knockback", false },
                FeatureEntry{ "Super Speed", false },
                FeatureEntry{ "Infinite Jumps", false },
                FeatureEntry{ "Noclip", false },
                FeatureEntry{ "No Enemy Aggro", false },
            };
            std::array<FeatureEntry, 8> CombatFeatures = {
                FeatureEntry{ "One Hit Kill", false },
                FeatureEntry{ "Damage Multiplier", false },
                FeatureEntry{ "Infinite Ammo", false },
                FeatureEntry{ "No Mana Cost", false },
                FeatureEntry{ "Max Luck", false },
                FeatureEntry{ "Instant Mine", false },
                FeatureEntry{ "Enemy Freeze", false },
                FeatureEntry{ "Spawn Rate Multiplier", false },
            };
            std::array<FeatureEntry, 10> InventoryFeatures = {
                FeatureEntry{ "Item Spawner", false },
                FeatureEntry{ "Infinite Stack", false },
                FeatureEntry{ "Item Duplicator", false },
                FeatureEntry{ "Edit Prefix", false },
                FeatureEntry{ "Infinite Potion Time", false },
                FeatureEntry{ "Easy Craft", false },
                FeatureEntry{ "Reveal Full Map", false },
                FeatureEntry{ "Edit Item Quantity", false },
                FeatureEntry{ "Debug Items", false },
                FeatureEntry{ "Trash Dupe", false },
            };
            std::array<FeatureEntry, 12> WorldFeatures = {
                FeatureEntry{ "Set Time of Day", false },
                FeatureEntry{ "Freeze Time", false },
                FeatureEntry{ "Fast Time", false },
                FeatureEntry{ "Max Brightness", false },
                FeatureEntry{ "Change Gravity", false },
                FeatureEntry{ "Weather Control", false },
                FeatureEntry{ "Teleport to Cursor", false },
                FeatureEntry{ "Instant Build", false },
                FeatureEntry{ "Unlimited Build Range", false },
                FeatureEntry{ "Item ESP", false },
                FeatureEntry{ "Auto Collect", false },
                FeatureEntry{ "Save/Load Character State", false },
            };
            bool AlwaysOnTop = false;
            bool CompactMode = false;
            float UiOpacity = 1.0f;
            int AccentColor = 0;
            float DamageMultiplierValue = 2.0f;
            float SpawnRateValue = 1.0f;
            int ItemQuantityValue = 999;
            int PrefixValue = 1;
            int TimeOfDayValue = 12;
            float GravityValue = 1.0f;
            float BuildRangeValue = 6.0f;
            int BrightnessValue = 100;
            char ItemName[64] = "Terra Blade";
            char WeatherName[32] = "Clear";
            const char* LastAction = "Idle";
        };

        struct BackgroundTextureState
        {
            ImTextureID Texture = nullptr;
            ImVec2 Size = ImVec2(0.0f, 0.0f);
        };

        TrainerUiState& GetState()
        {
            static TrainerUiState state;
            return state;
        }

        BackgroundTextureState& GetBackgroundTextureState()
        {
            static BackgroundTextureState state;
            return state;
        }

        BackgroundTextureState& GetPosterTextureState()
        {
            static BackgroundTextureState state;
            return state;
        }

        void DrawCroppedImage(ImDrawList* drawList, ImTextureID texture, const ImVec2& size, const ImVec2& min, const ImVec2& max, ImU32 tint, float rounding)
        {
            if (texture == nullptr || size.x <= 0.0f || size.y <= 0.0f)
            {
                return;
            }

            const ImVec2 areaSize(max.x - min.x, max.y - min.y);
            const float imageAspect = size.x / size.y;
            const float areaAspect = areaSize.x / areaSize.y;

            ImVec2 uv0(0.0f, 0.0f);
            ImVec2 uv1(1.0f, 1.0f);

            if (imageAspect > areaAspect)
            {
                const float visibleWidth = areaAspect / imageAspect;
                const float crop = (1.0f - visibleWidth) * 0.5f;
                uv0.x = crop;
                uv1.x = 1.0f - crop;
            }
            else
            {
                const float visibleHeight = imageAspect / areaAspect;
                const float crop = (1.0f - visibleHeight) * 0.5f;
                uv0.y = crop;
                uv1.y = 1.0f - crop;
            }

            drawList->AddImageRounded(texture, min, max, uv0, uv1, tint, rounding);
        }

        bool DrawTabButton(const char* label, bool selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, selected ? ImVec4(0.18f, 0.18f, 0.18f, 1.0f) : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.23f, 0.23f, 0.23f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, selected ? 1.0f : 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

            const bool pressed = ImGui::Button(label, ImVec2(120.0f, 32.0f));

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
            return pressed;
        }

        bool DrawCloseButton()
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.10f, 0.14f, 0.70f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.72f, 0.16f, 0.22f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.82f, 0.20f, 0.26f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            const bool pressed = ImGui::Button("X", ImVec2(32.0f, 32.0f));

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
            return pressed;
        }

        void DrawPoster()
        {
            const ImVec2 posterSize(188.0f, 268.0f);
            ImGui::InvisibleButton("poster", posterSize);

            const ImVec2 topLeft = ImGui::GetItemRectMin();
            const ImVec2 bottomRight = ImGui::GetItemRectMax();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const BackgroundTextureState& poster = GetPosterTextureState();

            if (poster.Texture != nullptr)
            {
                DrawCroppedImage(drawList, poster.Texture, poster.Size, topLeft, bottomRight, IM_COL32(255, 255, 255, 255), 6.0f);
                drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(20, 12, 14, 70), 6.0f);
                drawList->AddRect(topLeft, bottomRight, IM_COL32(255, 255, 255, 55), 6.0f, 0, 1.0f);
                return;
            }

            drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(197, 20, 52, 255), 6.0f);
            drawList->AddRectFilled(
                ImVec2(topLeft.x, topLeft.y + 138.0f),
                bottomRight,
                IM_COL32(35, 18, 18, 180),
                6.0f,
                ImDrawFlags_RoundCornersBottom);
            drawList->AddLine(
                ImVec2(topLeft.x + 18.0f, topLeft.y + 192.0f),
                ImVec2(bottomRight.x - 18.0f, topLeft.y + 176.0f),
                IM_COL32(240, 240, 240, 200),
                2.0f);
            drawList->AddRect(
                ImVec2(topLeft.x + 22.0f, topLeft.y + 210.0f),
                ImVec2(bottomRight.x - 22.0f, bottomRight.y - 18.0f),
                IM_COL32(255, 255, 255, 30),
                0.0f,
                0,
                1.0f);
            drawList->AddText(
                ImVec2(topLeft.x + 14.0f, topLeft.y + 14.0f),
                IM_COL32(255, 255, 255, 255),
                "GHOSTRUNNER");
            drawList->AddText(
                ImVec2(topLeft.x + 54.0f, topLeft.y + 92.0f),
                IM_COL32(255, 255, 255, 215),
                "UI");
            drawList->AddText(
                ImVec2(topLeft.x + 48.0f, topLeft.y + 112.0f),
                IM_COL32(255, 255, 255, 215),
                "PREVIEW");
        }

        template <std::size_t N>
        void DrawFeatureSection(const char* title, std::array<FeatureEntry, N>& entries)
        {
            if (!ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen))
            {
                return;
            }

            const float baseX = ImGui::GetCursorStartPos().x;
            const float checkboxColumnX = baseX + 12.0f;
            const float labelColumnX = baseX + 50.0f;

            for (std::size_t index = 0; index < entries.size(); ++index)
            {
                FeatureEntry& entry = entries[index];
                std::string checkboxId = std::string("##") + title + std::to_string(index);

                ImGui::PushID(static_cast<int>(index));
                ImGui::SetCursorPosX(checkboxColumnX);
                ImGui::Checkbox(checkboxId.c_str(), &entry.Enabled);

                ImGui::SameLine(labelColumnX);
                ImGui::AlignTextToFramePadding();
                ImGui::GetWindowDrawList()->AddText(ImVec2(ImGui::GetCursorScreenPos().x + 1.0f, ImGui::GetCursorScreenPos().y + 1.0f), IM_COL32(0, 0, 0, 220), entry.Label);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.98f, 0.99f, 1.0f));
                ImGui::TextUnformatted(entry.Label);
                ImGui::PopStyleColor();
                ImGui::PopID();
            }

            ImGui::Dummy(ImVec2(0.0f, 6.0f));
        }

        void DrawSidebar(const TrainerUiState& state)
        {
            DrawPoster();
            ImGui::Spacing();
            ImGui::TextUnformatted("Profile:");
            ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), "Standalone trainer UI");
            ImGui::Spacing();
            ImGui::TextUnformatted("Functions:");
            ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), "40 UI toggles");
            ImGui::Spacing();
            ImGui::TextUnformatted("Trainer Version:");
            ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), "Build 2026.04.03");
        }

        void DrawFunctionsTab(TrainerUiState& state)
        {
            DrawFeatureSection("Player", state.PlayerFeatures);
            DrawFeatureSection("Combat", state.CombatFeatures);
            DrawFeatureSection("Inventory", state.InventoryFeatures);
            DrawFeatureSection("World", state.WorldFeatures);

        }

        void DrawOptionsTab(TrainerUiState& state)
        {
            static const char* accentItems[] = { "Red", "Blue", "Purple", "Green" };

            ImGui::Checkbox("Always on top", &state.AlwaysOnTop);
            ImGui::Checkbox("Compact mode", &state.CompactMode);
            ImGui::SliderFloat("UI opacity", &state.UiOpacity, 0.65f, 1.00f, "%.2f");
            ImGui::Combo("Accent color", &state.AccentColor, accentItems, IM_ARRAYSIZE(accentItems));

            ImGui::Dummy(ImVec2(0.0f, 12.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::TextUnformatted("Function parameters");
            ImGui::SliderFloat("Damage multiplier", &state.DamageMultiplierValue, 1.0f, 25.0f, "x%.1f");
            ImGui::SliderFloat("Spawn rate multiplier", &state.SpawnRateValue, 0.0f, 10.0f, "x%.1f");
            ImGui::SliderInt("Item quantity", &state.ItemQuantityValue, 1, 9999);
            ImGui::SliderInt("Prefix value", &state.PrefixValue, 0, 100);
            ImGui::SliderInt("Time of day", &state.TimeOfDayValue, 0, 24, "%02d:00");
            ImGui::SliderFloat("Gravity scale", &state.GravityValue, 0.1f, 3.0f, "%.2f");
            ImGui::SliderFloat("Build range", &state.BuildRangeValue, 1.0f, 30.0f, "%.1f tiles");
            ImGui::SliderInt("Brightness", &state.BrightnessValue, 0, 200, "%d%%");
            ImGui::InputText("Item name", state.ItemName, IM_ARRAYSIZE(state.ItemName));
            ImGui::InputText("Weather preset", state.WeatherName, IM_ARRAYSIZE(state.WeatherName));

            if (ImGui::Button("Spawn Preview Item", ImVec2(160.0f, 0.0f)))
            {
                state.LastAction = "Spawn Preview Item";
            }

            ImGui::SameLine();

            if (ImGui::Button("Duplicate Preview Item", ImVec2(170.0f, 0.0f)))
            {
                state.LastAction = "Duplicate Preview Item";
            }

            if (ImGui::Button("Save Character State", ImVec2(160.0f, 0.0f)))
            {
                state.LastAction = "Save Character State";
            }

            ImGui::SameLine();

            if (ImGui::Button("Load Character State", ImVec2(160.0f, 0.0f)))
            {
                state.LastAction = "Load Character State";
            }

            ImGui::Dummy(ImVec2(0.0f, 12.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::TextUnformatted("Preview behavior:");
            ImGui::BulletText("No process lookup");
            ImGui::BulletText("No memory access");
            ImGui::BulletText("No input injection");
            ImGui::BulletText("Only local Dear ImGui state");
        }

        void ApplyTheme(float opacity)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 8.0f;
            style.ChildRounding = 6.0f;
            style.FrameRounding = 4.0f;
            style.GrabRounding = 4.0f;
            style.WindowBorderSize = 0.0f;
            style.ChildBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.WindowPadding = ImVec2(10.0f, 10.0f);
            style.ItemSpacing = ImVec2(10.0f, 10.0f);

            ImVec4* colors = style.Colors;
            colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.98f, 1.0f);
            colors[ImGuiCol_TextDisabled] = ImVec4(0.82f, 0.82f, 0.86f, 1.0f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.11f, 0.10f * opacity);
            colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.13f, 0.06f * opacity);
            colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.42f * opacity);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.20f, 0.62f * opacity);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.27f, 0.78f * opacity);
            colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.13f, 0.08f * opacity);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.13f, 0.08f * opacity);
            colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.20f, 0.10f * opacity);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.27f, 0.22f * opacity);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.28f, 0.31f, 0.30f * opacity);
            colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 0.10f * opacity);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.27f, 0.20f * opacity);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.28f, 0.31f, 0.28f * opacity);
            colors[ImGuiCol_Separator] = ImVec4(0.26f, 0.26f, 0.28f, opacity);
        }

        void DrawWindowBackgroundImage()
        {
            const BackgroundTextureState& background = GetBackgroundTextureState();
            if (background.Texture == nullptr || background.Size.x <= 0.0f || background.Size.y <= 0.0f)
            {
                return;
            }

            const ImVec2 windowPos = ImGui::GetWindowPos();
            const ImVec2 windowSize = ImGui::GetWindowSize();
            const ImVec2 imageMin = windowPos;
            const ImVec2 imageMax(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            DrawCroppedImage(drawList, background.Texture, background.Size, imageMin, imageMax, IM_COL32(255, 255, 255, 255), 8.0f);
            drawList->AddRectFilled(imageMin, imageMax, IM_COL32(10, 10, 12, 18), 8.0f);
        }

        void BeginFullscreenWindow(const char* id, float backgroundAlpha)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowBgAlpha(backgroundAlpha);
            ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
            ImGui::Begin(
                id,
                nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize);
        }
    }

    void SetBackgroundTexture(ImTextureID texture, const ImVec2& size)
    {
        BackgroundTextureState& background = GetBackgroundTextureState();
        background.Texture = texture;
        background.Size = size;
    }

    void SetPosterTexture(ImTextureID texture, const ImVec2& size)
    {
        BackgroundTextureState& poster = GetPosterTextureState();
        poster.Texture = texture;
        poster.Size = size;
    }

    bool ConsumeCloseRequested()
    {
        TrainerUiState& state = GetState();
        const bool closeRequested = state.CloseRequested;
        state.CloseRequested = false;
        return closeRequested;
    }

    void RenderSplashScreen(float progress)
    {
        TrainerUiState& state = GetState();
        ApplyTheme(state.UiOpacity);

        BeginFullscreenWindow("SplashScreen", 1.0f);

        ImDrawList* backgroundDrawList = ImGui::GetWindowDrawList();
        const ImVec2 splashWindowMin = ImGui::GetWindowPos();
        const ImVec2 splashWindowMax(
            splashWindowMin.x + ImGui::GetWindowSize().x,
            splashWindowMin.y + ImGui::GetWindowSize().y);
        backgroundDrawList->AddRectFilledMultiColor(
            splashWindowMin,
            splashWindowMax,
            IM_COL32(18, 20, 28, 255),
            IM_COL32(12, 14, 20, 255),
            IM_COL32(8, 10, 16, 255),
            IM_COL32(14, 16, 24, 255));
        backgroundDrawList->AddCircleFilled(
            ImVec2(splashWindowMin.x + 120.0f, splashWindowMin.y + 90.0f),
            120.0f,
            IM_COL32(170, 34, 54, 42),
            48);
        backgroundDrawList->AddCircleFilled(
            ImVec2(splashWindowMax.x - 120.0f, splashWindowMax.y - 80.0f),
            150.0f,
            IM_COL32(70, 90, 180, 24),
            48);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(
            ImVec2(splashWindowMin.x + 22.0f, splashWindowMin.y + 22.0f),
            ImVec2(splashWindowMax.x - 22.0f, splashWindowMin.y + 22.0f),
            IM_COL32(180, 48, 66, 90),
            1.0f);

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 54.0f, 22.0f));
        if (DrawCloseButton())
        {
            state.CloseRequested = true;
        }

        ImGui::BeginChild("splash_left", ImVec2(200.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
        DrawPoster();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("splash_right", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::Dummy(ImVec2(0.0f, 30.0f));
        ImGui::SetWindowFontScale(1.6f);
        ImGui::TextUnformatted("CRIMSON DESERT");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::TextColored(ImVec4(0.86f, 0.25f, 0.34f, 1.0f), "TRAINER");
        ImGui::Dummy(ImVec2(0.0f, 12.0f));
        ImGui::TextColored(ImVec4(0.86f, 0.86f, 0.90f, 1.0f), "Build 2026.04.03");
        ImGui::TextColored(ImVec4(0.72f, 0.72f, 0.76f, 1.0f), "Initializing interface...");
        ImGui::Dummy(ImVec2(0.0f, 28.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.82f, 0.16f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.55f));
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 18.0f), "");
        ImGui::PopStyleColor(2);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::TextColored(ImVec4(0.78f, 0.78f, 0.82f, 1.0f), "%d%%", static_cast<int>(progress * 100.0f));
        ImGui::EndChild();

        ImGui::End();
    }

    void RenderTrainerPanel()
    {
        TrainerUiState& state = GetState();
        ApplyTheme(state.UiOpacity);

        BeginFullscreenWindow("Standalone Trainer Preview", 0.02f * state.UiOpacity);
        if (!ImGui::BeginChild("main_root", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        DrawWindowBackgroundImage();

        ImGui::BeginChild("sidebar", ImVec2(210.0f, 0.0f), true);
        DrawSidebar(state);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("content", ImVec2(0.0f, 0.0f), true);
        if (DrawTabButton("Functions", state.ActiveTab == TrainerTab::Functions))
        {
            state.ActiveTab = TrainerTab::Functions;
        }

        ImGui::SameLine();

        if (DrawTabButton("Options", state.ActiveTab == TrainerTab::Options))
        {
            state.ActiveTab = TrainerTab::Options;
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 32.0f);
        if (DrawCloseButton())
        {
            state.CloseRequested = true;
        }

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 14.0f));

        if (state.ActiveTab == TrainerTab::Functions)
        {
            DrawFunctionsTab(state);
        }
        else
        {
            DrawOptionsTab(state);
        }

        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::End();
    }
}
