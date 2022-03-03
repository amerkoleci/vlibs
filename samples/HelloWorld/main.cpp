// Copyright © Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "GLFW/glfw3.h"
#if defined(__linux__)
#   define GLFW_EXPOSE_NATIVE_X11
#elif defined(_WIN32)
#   define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"
#include <vgfx.h>

gfxDevice device = nullptr;

void init_gfx(GLFWwindow* window)
{
    device = vgfxCreateDevice();
}

void draw_frame()
{
}

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    init_gfx(window);

    while (!glfwWindowShouldClose(window))
    {
        draw_frame();
        glfwPollEvents();
    }

    vgfxDestroyDevice(device);

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}