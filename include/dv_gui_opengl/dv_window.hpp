#pragma once

#include <string>

struct GLFWwindow;

namespace dvsku {
    class dv_window {
    public:
        using handle_t = void*;

        struct mouse_pos {
            double x  = 0.0;
            double y  = 0.0;
            double dx = 0.0;
            double dy = 0.0;
        };

    public:
        dv_window(uint32_t width, uint32_t height, const std::string& title);
    	virtual ~dv_window();

    public:
    	void run();

        static intptr_t wndproc_callback(dv_window* wnd, handle_t handle, uint32_t msg, uint64_t wparam, int64_t lparam);

    protected:
    	GLFWwindow* m_native    = nullptr;
    	bool        m_minimized = false;

        mouse_pos m_mouse_pos{};

    protected:
    	virtual bool prepare();
        virtual void release();

    	virtual void on_before_update();
    	virtual void on_update();
    	virtual void on_after_update();

    	virtual void on_gui_before_update();
    	virtual void on_gui_update();
    	virtual void on_gui_after_update();

    	virtual void on_resize(int width, int height);
    	virtual void on_scroll(double dx, double dy);
    	virtual void on_mouse_button(int btn, int action, int modifier);
    	virtual void on_mouse_move(double dx, double dy);
        virtual void on_drop(int count, const char* paths[]);

        /// <summary>
        /// Title bar hit testing when borderless.
        /// Enables window drag and snapping.
        /// </summary>
        virtual bool is_title_bar(int32_t x, int32_t y);

        /// <summary>
        /// Maximize/restore button hit testing when borderless
        /// Enables snap.
        /// </summary>
        virtual bool is_maximize_button(int32_t x, int32_t y);

    protected:
        void set_borderless();

    private:
        intptr_t m_default_wndproc = 0;

    private:
    	void loop();

    	static void resize_callback(GLFWwindow* window, int width, int height);
    	static void scroll_callback(GLFWwindow* window, double dx, double dy);
    	static void mouse_button_callback(GLFWwindow* window, int button, int action, int modifier);
    	static void mouse_move_callback(GLFWwindow* window, double x, double y);
    	static void iconify_callback(GLFWwindow* window, int iconified);
        static void drop_callback(GLFWwindow* window, int count, const char* paths[]);
    };
}