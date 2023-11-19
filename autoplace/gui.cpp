#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "gui.hpp"
#include "injector.hpp"
#include <dwmapi.h>

#include "mappings.hpp"

using namespace mappings;

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int xSize = 425;
int ySize = 185;

bool toggled = false;

void UpdateWindowLocation(HWND hwnd, RECT rc)
{
    GetWindowRect(hwnd, &rc);

    if (ImGui::GetWindowPos().x != 0 || ImGui::GetWindowPos().y != 0)
    {
        MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, xSize, ySize, TRUE);
        ImGui::SetWindowPos(ImVec2(0.f, 0.f));
    }
}

#define NU(x, a) if (x == NULL) MessageBoxA(NULL, a, "title", NULL);

// Main code
void gui::start_gui()
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Lixoware", NULL };

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = GetModuleHandleA(0); // nullptr
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = NULL;
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

    ::RegisterClassExW(&wc);
    HWND hwnd = CreateWindowEx(NULL, L"Lixoware", L"Lixoware", WS_POPUP | CW_USEDEFAULT, (GetSystemMetrics(SM_CXSCREEN) / 2) - (xSize / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (ySize / 2), xSize, ySize, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;

    // mcp detection
    {
        jclass mc = client->classloader->find_class("net/minecraft/client/Minecraft");

        if(mc)
        {
            mappings::mcp_mappings = true;
        }
    }

    mappings::initialize_mappings();
    c_classmapping* minecraft_mapping = get_classmapping_by_name("net/minecraft/client/Minecraft");
    c_classmapping* movingobjectposition_mapping = get_classmapping_by_name("net/minecraft/util/MovingObjectPosition");
    c_classmapping* movingobjectposition_mot_mapping = get_classmapping_by_name("net/minecraft/util/MovingObjectPosition$MovingObjectType");
    c_classmapping* enumfacing_mapping = get_classmapping_by_name("net/minecraft/util/EnumFacing");

    c_methodmapping* getminecraft_mapping = get_methodmapping_by_name("getMinecraft", "()Lnet/minecraft/client/Minecraft;");
    c_methodmapping* rightclickmouse_mapping = get_methodmapping_by_name("rightClickMouse", "()V");

    c_fieldmapping* theplayer_mapping = get_fieldmapping_by_name("thePlayer", "Lnet/minecraft/client/entity/EntityPlayerSP;");
    c_fieldmapping* rightclickdelaytimer_mapping = get_fieldmapping_by_name("rightClickDelayTimer", "I");
    c_fieldmapping* up_mapping = get_fieldmapping_by_name("UP", "Lnet/minecraft/util/EnumFacing;");
    c_fieldmapping* down_mapping = get_fieldmapping_by_name("DOWN", "Lnet/minecraft/util/EnumFacing;");
    c_fieldmapping* objectmouseover_mapping = get_fieldmapping_by_name("objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;");
    c_fieldmapping* block_mapping = get_fieldmapping_by_name("BLOCK", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");
    c_fieldmapping* sidehit_mapping = get_fieldmapping_by_name("sideHit", "Lnet/minecraft/util/EnumFacing;");
    c_fieldmapping* typeofhit_mapping = get_fieldmapping_by_name("typeOfHit", "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;");

    jclass c_minecraft = client->classloader->find_class(minecraft_mapping->get_class_name());

    jmethodID m_getMinecraft = getminecraft_mapping->get_static_method_id(client->env, c_minecraft);
    jmethodID m_rightClickMouse = rightclickmouse_mapping->get_method_id(client->env, c_minecraft);
    jfieldID f_rightClickDelayTimer = rightclickdelaytimer_mapping->get_field_id(client->env, c_minecraft);
    jobject o_minecraft = client->env->CallStaticObjectMethod(c_minecraft, m_getMinecraft);

    jfieldID f_thePlayer = theplayer_mapping->get_field_id(client->env, c_minecraft);
    jfieldID f_objectMouseOver = objectmouseover_mapping->get_field_id(client->env, c_minecraft);

    jclass c_movingObjectPosition = client->classloader->find_class(movingobjectposition_mapping->get_class_name());
    jfieldID f_mop_enumFacing = sidehit_mapping->get_field_id(client->env, c_movingObjectPosition);
    jfieldID f_mop_typeOfHit = typeofhit_mapping->get_field_id(client->env, c_movingObjectPosition);

    jclass c_movingObjectType = client->classloader->find_class(movingobjectposition_mot_mapping->get_class_name());
    jfieldID f_mot_block = block_mapping->get_static_field_id(client->env, c_movingObjectType);
    jobject o_mot_block = client->env->GetStaticObjectField(c_movingObjectType, f_mot_block);

    jclass c_enumFacing = client->classloader->find_class(enumfacing_mapping->get_class_name());
    jfieldID f_enumFacing_DOWN = down_mapping->get_static_field_id(client->env, c_enumFacing);
    jfieldID f_enumFacing_UP = up_mapping->get_static_field_id(client->env, c_enumFacing);

    jobject o_enumFacing_DOWN = client->env->GetStaticObjectField(c_enumFacing, f_enumFacing_DOWN);
    jobject o_enumFacing_UP = client->env->GetStaticObjectField(c_enumFacing, f_enumFacing_UP);

    static int delay = 10;
    static bool rc = true, fastplace = false;

    // Main loop
    bool done = false;
    while (!done && !GetAsyncKeyState(VK_END))
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

            ImGui::SetNextWindowPos({ 0 , 0 }, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(xSize, ySize));
            ImGui::Begin("autoplace", 0, main_window_flags);

            RECT rect;

            GetWindowRect(hwnd, &rect);
            UpdateWindowLocation(hwnd, rect);

            ImGui::SliderInt("delay", &delay, 0, 100);
            ImGui::Checkbox("hold right click", &rc);
            ImGui::Checkbox("fastplace", &fastplace);

            ImGui::Spacing();

            if (mappings::mcp_mappings)
            {
                ImGui::Text("mode: mcp");
            }
            else 
            {
                ImGui::Text("mode: vanilla");
            }

            ImGui::Text("this autoplace is made by wykt");
            ImGui::Text("please credit me if you use it in any video");

            if (ImGui::Button("yt channel")) {
                ShellExecute(0, 0, L"https://www.youtube.com/@wykt1164", 0, 0, SW_SHOW);
            }

            ImGui::SameLine();

            if (ImGui::Button("exit")) {
                done = true;
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);

        {
            HWND foreground_window = GetForegroundWindow();
            static bool auto_place = false, was_key_pressed = false;

            if (foreground_window != FindWindowA("LWJGL", NULL))
            {
                continue;
            }

            if (fastplace)
            {
                client->env->SetIntField(o_minecraft, f_rightClickDelayTimer, 0);
            }

            if (!GetAsyncKeyState(VK_RBUTTON) && rc) // Continue if user isn't pressing right click
            {
                Sleep(50);
                continue;
            }

            if (!rc)
            {
                bool pressed = GetAsyncKeyState('V') & 0x8000;

                if (was_key_pressed && !pressed)
                {
                    auto_place = !auto_place;
                }

                was_key_pressed = pressed;

                if (!auto_place)
                {
                    Sleep(50);
                    continue;
                }
            }

            jobject o_thePlayer = client->env->GetObjectField(o_minecraft, f_thePlayer);

            if (o_thePlayer) // theplayer null check
            {
                jobject o_objectMouseOver = client->env->GetObjectField(o_minecraft, f_objectMouseOver);

                if (o_objectMouseOver) // objectmouseouver null check
                {
                    jobject o_typeOfHit = client->env->GetObjectField(o_objectMouseOver, f_mop_typeOfHit);

                    if (client->env->IsSameObject(o_mot_block, o_typeOfHit)) // Check if typeOfHit is MovingObjectType.BLOCK
                    {
                        jobject o_enumFacing = client->env->GetObjectField(o_objectMouseOver, f_mop_enumFacing);

                        if (!client->env->IsSameObject(o_enumFacing, o_enumFacing_DOWN) && !client->env->IsSameObject(o_enumFacing, o_enumFacing_UP)) // Check if enum facing is not down/up
                        {
                            client->env->CallVoidMethod(o_minecraft, m_rightClickMouse);
                        }

                        client->env->DeleteLocalRef(o_enumFacing);
                    }

                    client->env->DeleteLocalRef(o_typeOfHit);
                    client->env->DeleteLocalRef(o_objectMouseOver);
                }

                client->env->DeleteLocalRef(o_thePlayer);
            }

            if (delay > 0)
            {
                Sleep(delay);
            }
        }
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    client->env->DeleteLocalRef(c_minecraft);
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}
void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}