#include "TrainerPanel.h"
#include "resource.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3_loader.h"
#include "backends/imgui_impl_opengl3.h"

#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#endif

#include <GLFW/glfw3.h>

#if defined(_WIN32)
static bool LoadTextureFromBitmap(Gdiplus::Bitmap& bitmap, GLuint* outTexture, int* outWidth, int* outHeight)
{
    if (bitmap.GetLastStatus() != Gdiplus::Ok)
    {
        return false;
    }

    const UINT width = bitmap.GetWidth();
    const UINT height = bitmap.GetHeight();
    Gdiplus::Rect rect(0, 0, static_cast<INT>(width), static_cast<INT>(height));

    Gdiplus::Bitmap converted(width, height, PixelFormat32bppARGB);
    {
        Gdiplus::Graphics graphics(&converted);
        graphics.DrawImage(&bitmap, 0, 0, width, height);
    }

    Gdiplus::BitmapData data{};
    if (converted.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
    {
        return false;
    }

    std::vector<unsigned char> pixels(width * height * 4);
    for (UINT y = 0; y < height; ++y)
    {
        const auto* sourceRow = static_cast<const unsigned char*>(data.Scan0) + (y * data.Stride);
        auto* destinationRow = pixels.data() + (y * width * 4);
        for (UINT x = 0; x < width; ++x)
        {
            destinationRow[x * 4 + 0] = sourceRow[x * 4 + 2];
            destinationRow[x * 4 + 1] = sourceRow[x * 4 + 1];
            destinationRow[x * 4 + 2] = sourceRow[x * 4 + 0];
            destinationRow[x * 4 + 3] = sourceRow[x * 4 + 3];
        }
    }

    converted.UnlockBits(&data);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    *outTexture = texture;
    *outWidth = static_cast<int>(width);
    *outHeight = static_cast<int>(height);
    return true;
}

static bool LoadTextureFromFile(const wchar_t* filename, GLuint* outTexture, int* outWidth, int* outHeight)
{
    Gdiplus::Bitmap bitmap(filename);
    return LoadTextureFromBitmap(bitmap, outTexture, outWidth, outHeight);
}

static bool LoadTextureFromResource(int resourceId, GLuint* outTexture, int* outWidth, int* outHeight)
{
    HMODULE moduleHandle = GetModuleHandleW(nullptr);
    HRSRC resourceInfo = FindResourceW(moduleHandle, MAKEINTRESOURCEW(resourceId), MAKEINTRESOURCEW(10));
    if (resourceInfo == nullptr)
    {
        return false;
    }

    HGLOBAL loadedResource = LoadResource(moduleHandle, resourceInfo);
    if (loadedResource == nullptr)
    {
        return false;
    }

    const DWORD resourceSize = SizeofResource(moduleHandle, resourceInfo);
    const void* resourceBytes = LockResource(loadedResource);
    if (resourceBytes == nullptr || resourceSize == 0)
    {
        return false;
    }

    HGLOBAL resourceCopy = GlobalAlloc(GMEM_MOVEABLE, resourceSize);
    if (resourceCopy == nullptr)
    {
        return false;
    }

    void* resourceCopyBytes = GlobalLock(resourceCopy);
    if (resourceCopyBytes == nullptr)
    {
        GlobalFree(resourceCopy);
        return false;
    }

    std::memcpy(resourceCopyBytes, resourceBytes, resourceSize);
    GlobalUnlock(resourceCopy);

    IStream* stream = nullptr;
    if (CreateStreamOnHGlobal(resourceCopy, TRUE, &stream) != S_OK)
    {
        GlobalFree(resourceCopy);
        return false;
    }

    Gdiplus::Bitmap bitmap(stream);
    const bool loaded = LoadTextureFromBitmap(bitmap, outTexture, outWidth, outHeight);
    stream->Release();
    return loaded;
}
#endif

static void GlfwErrorCallback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void CenterWindow(GLFWwindow* window)
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr)
    {
        return;
    }

    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    if (videoMode == nullptr)
    {
        return;
    }

    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwSetWindowPos(window, (videoMode->width - windowWidth) / 2, (videoMode->height - windowHeight) / 2);
}

int main()
{
    constexpr int splashWindowWidth = 760;
    constexpr int splashWindowHeight = 360;
    constexpr int mainWindowWidth = 1280;
    constexpr int mainWindowHeight = 720;

#if defined(_WIN32)
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
#endif

    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit())
    {
#if defined(_WIN32)
        Gdiplus::GdiplusShutdown(gdiplusToken);
#endif
        return 1;
    }

#if defined(__APPLE__)
    const char* glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(splashWindowWidth, splashWindowHeight, "Crimson desert trainer", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return 1;
    }

    CenterWindow(window);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    GLuint backgroundTexture = 0;
    int backgroundWidth = 0;
    int backgroundHeight = 0;
    GLuint posterTexture = 0;
    int posterWidth = 0;
    int posterHeight = 0;
#if defined(_WIN32)
    if (LoadTextureFromResource(IDR_BACKGROUND_IMAGE, &backgroundTexture, &backgroundWidth, &backgroundHeight))
    {
        trainer::SetBackgroundTexture(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(backgroundTexture)), ImVec2(static_cast<float>(backgroundWidth), static_cast<float>(backgroundHeight)));
    }
    if (LoadTextureFromResource(IDR_POSTER_IMAGE, &posterTexture, &posterWidth, &posterHeight))
    {
        trainer::SetPosterTexture(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(posterTexture)), ImVec2(static_cast<float>(posterWidth), static_cast<float>(posterHeight)));
    }
#endif

    const double splashStartTime = glfwGetTime();
    constexpr double splashDurationSeconds = 4.0;
    bool mainWindowActivated = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const double splashElapsed = glfwGetTime() - splashStartTime;
        if (splashElapsed < splashDurationSeconds)
        {
            const float progress = static_cast<float>(splashElapsed / splashDurationSeconds);
            trainer::RenderSplashScreen(progress);
        }
        else
        {
            if (!mainWindowActivated)
            {
                glfwSetWindowSize(window, mainWindowWidth, mainWindowHeight);
                CenterWindow(window);
                mainWindowActivated = true;
            }

            trainer::RenderTrainerPanel();
        }

        if (trainer::ConsumeCloseRequested())
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.00f, 0.00f, 0.00f, 0.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (backgroundTexture != 0)
    {
        glDeleteTextures(1, &backgroundTexture);
    }

    if (posterTexture != 0)
    {
        glDeleteTextures(1, &posterTexture);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

#if defined(_WIN32)
    Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

    return 0;
}
