#define GLFW_EXPOSE_NATIVE_WIN32

#include <windows.h>

#include "dv_gui_opengl/dv_window.hpp"
#include "dv_gui_opengl/utilities/dv_util_log.hpp"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glfw/glfw3native.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <stdexcept>

using namespace dvsku;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL

static LRESULT CALLBACK redirect_callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    dv_window* dv_wnd = (dv_window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!dv_wnd)
        throw std::runtime_error("");

    return dv_window::wndproc_callback(dv_wnd, hWnd, uMsg, wParam, lParam);
}

static std::string get_imgui_version() {
    std::string str = std::string(ImGui::GetVersion());

#ifdef IMGUI_HAS_DOCK
    str += " docking";
#endif
#ifdef IMGUI_HAS_VIEWPORT
    str += " multi-viewport";
#endif

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC

dv_window::dv_window(uint32_t width, uint32_t height, const std::string& title) {
    glfwSetErrorCallback(NULL);

    if (!glfwInit())
    	throw std::runtime_error("Failed to init glfw.");

    m_native = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!m_native)
    	throw std::runtime_error("Failed to create window.");

    // Get glfw native handle
    auto win32_handle = glfwGetWin32Window(m_native);
    if (!win32_handle)
        throw std::runtime_error("Failed to create window.");

    // Set this as native handle userdata
    SetWindowLongPtr(win32_handle, GWLP_USERDATA, (intptr_t)this);

    glfwMakeContextCurrent(m_native);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(m_native, this);

    glfwSetWindowSizeCallback(m_native, resize_callback);
    glfwSetScrollCallback(m_native, scroll_callback);
    glfwSetMouseButtonCallback(m_native, mouse_button_callback);
    glfwSetCursorPosCallback(m_native, mouse_move_callback);
    glfwSetWindowIconifyCallback(m_native, iconify_callback);
    glfwSetDropCallback(m_native, drop_callback);

    glfwSetWindowMonitor(m_native, NULL, (GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2), width, height, GLFW_DONT_CARE);

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    	throw std::runtime_error("Failed to init glad.");
}

dv_window::~dv_window() {
    glfwDestroyWindow(m_native);
    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void dv_window::run() {
    // Init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_native, true);
    ImGui_ImplOpenGL3_Init();

#ifndef NDEBUG
    DV_LOG("OpenGL version:   {}", (char*)glGetString(GL_VERSION));
    DV_LOG("OpenGL renderer:  {}", (char*)glGetString(GL_RENDERER));
    DV_LOG("ImGUI version:    {}", get_imgui_version());
#endif

    // Cancel running if prepare failed
    if (!prepare()) return;

    loop();
}

intptr_t dv_window::wndproc_callback(dv_window* wnd, handle_t handle, uint32_t msg, uint64_t wparam, int64_t lparam) {
    HWND hwnd = (HWND)handle;

    switch (msg) {
        case WM_NCCALCSIZE: {
            NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);

            WINDOWPLACEMENT wp{};
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(hwnd, &wp);

            if (wparam == TRUE && params) {
                if (wp.showCmd == SW_SHOWMAXIMIZED) {
                    params->rgrc[0].top    += 8;
                    params->rgrc[0].right  -= 8;
                    params->rgrc[0].bottom -= 8;
                    params->rgrc[0].left   += 8;
                    return 0;
                }

                params->rgrc[0].top    += 1;
                params->rgrc[0].right  -= 1;
                params->rgrc[0].bottom -= 1;
                params->rgrc[0].left   += 1;
            }

            return 0;
        }
        case WM_NCPAINT: {
            break;
        }
        case WM_NCHITTEST: {
            POINTS       mouse_pos        = MAKEPOINTS(lparam);
            POINT        client_mouse_pos = { mouse_pos.x, mouse_pos.y };
            ImGuiWindow* modal            = ImGui::GetTopMostAndVisiblePopupModal();
            
            ScreenToClient(hwnd, &client_mouse_pos);

            if (wnd->m_is_maximize)
                return HTMAXBUTTON;

            if (!modal && !ImGui::IsAnyItemHovered() && wnd->m_is_title_bar)
                return HTCAPTION;

            break;
        }
        case WM_NCACTIVATE: {
            return TRUE;
        }
        case WM_GETMINMAXINFO: {
            MINMAXINFO* min_max = (MINMAXINFO*)lparam;

            // Get information about the monitor
            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

            MONITORINFO monitor_info{};
            monitor_info.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &monitor_info);

            // Set the maximum size based on monitor information
            min_max->ptMaxTrackSize.x = monitor_info.rcWork.right - monitor_info.rcWork.left;
            min_max->ptMaxTrackSize.y = monitor_info.rcWork.bottom - monitor_info.rcWork.top;
            min_max->ptMaxPosition.x  = 0;
            min_max->ptMaxPosition.y  = 0;

            break;
        }
        case WM_NCLBUTTONDOWN: {
            if (wparam == HTMAXBUTTON)
                return 0;

            if (wparam == HTCAPTION)
                ImGui::ClosePopupsExceptModals();

            break;
        }
        case WM_NCLBUTTONUP: {
            switch (wparam) {
                case HTMAXBUTTON: {
                    if (glfwGetWindowAttrib(wnd->m_native, GLFW_MAXIMIZED)) {
                        glfwRestoreWindow(wnd->m_native);
                    }
                    else {
                        glfwMaximizeWindow(wnd->m_native);
                    }

                    return 0;
                }
                default: break;
            }

            break;
        }
            
        default: break;
    }

    return CallWindowProc((WNDPROC)wnd->m_default_wndproc, hwnd, msg, wparam, lparam);
}

///////////////////////////////////////////////////////////////////////////////
// PROTECTED

bool dv_window::prepare() {
    return true;
}

void dv_window::release() {}

void dv_window::on_before_update() {}

void dv_window::on_update() {}

void dv_window::on_after_update() {}

void dv_window::on_gui_before_update() {}

void dv_window::on_gui_update() {}

void dv_window::on_gui_after_update() {}

void dv_window::on_resize(int width, int height) {}

void dv_window::on_scroll(double dx, double dy) {}

void dv_window::on_mouse_button(int btn, int action, int modifier) {}

void dv_window::on_mouse_move(double dx, double dy) {}

void dv_window::on_drop(int count, const char* paths[]) {}

void dv_window::set_borderless() {
    auto win32_wnd = glfwGetWin32Window(m_native);
    if (!win32_wnd)
        return;

    dv_window* dv_wnd = (dv_window*)GetWindowLongPtr(win32_wnd, GWLP_USERDATA);
    if (!dv_wnd)
        return;

    // Save default callback
    dv_wnd->m_default_wndproc = GetWindowLongPtr(win32_wnd, GWLP_WNDPROC);
    if (!dv_wnd->m_default_wndproc)
        return;

    // Get win32 window style
    auto wnd_style = GetWindowLongPtr(win32_wnd, GWL_STYLE);

    // Set window style
    wnd_style |= WS_OVERLAPPEDWINDOW;
    wnd_style |= WS_CAPTION;
    wnd_style |= WS_MAXIMIZEBOX;
    wnd_style |= WS_THICKFRAME;
    wnd_style |= CS_VREDRAW;
    wnd_style |= CS_HREDRAW;

    // Set win32 window style
    SetWindowLongPtr(win32_wnd, GWL_STYLE, wnd_style);

    // Set new callback
    SetWindowLongPtr(win32_wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(redirect_callback));

    // Force window redraw
    SetWindowPos(win32_wnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void dv_window::close() {
    glfwSetWindowShouldClose(m_native, 1);
}

void dv_window::minimize() {
    glfwIconifyWindow(m_native);
}

bool dv_window::is_minimized() {
    return m_minimized;
}

///////////////////////////////////////////////////////////////////////////////
// PRIVATE

void dv_window::loop() {
    while (!glfwWindowShouldClose(m_native)) {
    	if (m_minimized) {
    		glfwWaitEvents();
    		continue;
    	}

    	// OpenGL render pass
    	on_before_update();
    	on_update();
    	on_after_update();

    	// GUI render pass
    	ImGui_ImplOpenGL3_NewFrame();
    	ImGui_ImplGlfw_NewFrame();
    	ImGui::NewFrame();

    	on_gui_before_update();
    	on_gui_update();

    	ImGui::Render();
    	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    	on_gui_after_update();

    	glfwSwapBuffers(m_native);
    	glfwPollEvents();
    }

    release();
}

void dv_window::resize_callback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return;

    glViewport(0, 0, width, height);

    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));
    instance->on_resize(width, height);
}

void dv_window::scroll_callback(GLFWwindow* window, double delta_x, double delta_y) {
    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));
    instance->on_scroll(delta_x, delta_y);
}

void dv_window::mouse_button_callback(GLFWwindow* window, int button, int action, int modifier) {
    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));
    instance->on_mouse_button(button, action, modifier);
}

void dv_window::mouse_move_callback(GLFWwindow* window, double x, double y) {
    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));

    instance->m_mouse_pos.dx = x - instance->m_mouse_pos.x;
    instance->m_mouse_pos.dy = instance->m_mouse_pos.y - y;
    instance->m_mouse_pos.x  = x;
    instance->m_mouse_pos.y  = y;

    instance->on_mouse_move(instance->m_mouse_pos.dx, instance->m_mouse_pos.dy);
}

void dv_window::iconify_callback(GLFWwindow* window, int iconified) {
    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));
    instance->m_minimized = (bool)iconified;
}

void dv_window::drop_callback(GLFWwindow* window, int count, const char* paths[]) {
    dv_window* instance = static_cast<dv_window*>(glfwGetWindowUserPointer(window));
    instance->on_drop(count, paths);
}