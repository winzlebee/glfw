//========================================================================
// Touch input test
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test displays the state of every touch contact point
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct TouchEvent
{
    int id;
    int action;
    double xpos;
    double ypos;
} TouchEvent;

#define MAX_EVENTS 200

static TouchEvent events[MAX_EVENTS];
static int event_count = 0;

static const char* action_label(int action)
{
    switch (action)
    {
        case GLFW_PRESS:   return "Press";
        case GLFW_RELEASE: return "Release";
        case GLFW_MOVE:    return "Move";
        case GLFW_CANCEL:  return "Cancel";
        default:           return "Unknown";
    }
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void touch_callback(GLFWwindow* window, int id, int action,
                            double xpos, double ypos)
{
    if (event_count < MAX_EVENTS)
    {
        events[event_count].id = id;
        events[event_count].action = action;
        events[event_count].xpos = xpos;
        events[event_count].ypos = ypos;
        event_count++;
    }

    printf("Touch %i: %s at %0.1f, %0.1f\n",
           id + 1, action_label(action), xpos, ypos);
}

int main(void)
{
    GLFWwindow* window;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (!glfwTouchInputSupported())
        printf("Touch input is not supported on this platform\n");
    else
        printf("Touch input is supported\n");

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);

    window = glfwCreateWindow(800, 600, "Touch Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    glfwSetInputMode(window, GLFW_TOUCH, GLFW_TRUE);
    glfwSetTouchCallback(window, touch_callback);

    while (!glfwWindowShouldClose(window))
    {
        int i, width, height;

        glfwGetWindowSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_new_frame();

        if (nk_begin(nk,
                     "Touch State",
                     nk_rect(0.f, 0.f, 400.f, (float) height),
                     NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE |
                     NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(nk, 30, 1);
            nk_labelf(nk, NK_TEXT_LEFT, "Touch supported: %s",
                      glfwTouchInputSupported() ? "Yes" : "No");

            nk_layout_row_dynamic(nk, 30, 1);
            nk_label(nk, "Touch points:", NK_TEXT_LEFT);

            for (i = GLFW_TOUCH_1;  i <= GLFW_TOUCH_LAST;  i++)
            {
                int state = glfwGetTouch(window, i);
                double xpos, ypos;

                glfwGetTouchPos(window, i, &xpos, &ypos);

                nk_layout_row_dynamic(nk, 30, 1);

                if (state == GLFW_PRESS || state == GLFW_MOVE)
                {
                    nk_labelf(nk, NK_TEXT_LEFT,
                              "Touch %i: %s (%.1f, %.1f)",
                              i + 1, action_label(state), xpos, ypos);
                }
                else
                {
                    nk_labelf(nk, NK_TEXT_LEFT,
                              "Touch %i: inactive", i + 1);
                }
            }
        }

        nk_end(nk);

        if (nk_begin(nk,
                     "Event Log",
                     nk_rect(400.f, 0.f, 400.f, (float) height),
                     NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE |
                     NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(nk, 30, 1);
            nk_labelf(nk, NK_TEXT_LEFT, "Events: %i", event_count);

            if (nk_button_label(nk, "Clear"))
                event_count = 0;

            for (i = event_count - 1;  i >= 0;  i--)
            {
                nk_layout_row_dynamic(nk, 30, 1);
                nk_labelf(nk, NK_TEXT_LEFT, "Touch %i: %s (%.1f, %.1f)",
                          events[i].id + 1,
                          action_label(events[i].action),
                          events[i].xpos,
                          events[i].ypos);
            }
        }

        nk_end(nk);

        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
