#include "RGFW-GL.h"

#define RGFW_IMGUI_IMPLEMENTATION
#include "imgui_impl_rgfw.h"

#include <stdio.h>

void drawLoop(RGFW_window* w); /* I separate the draw loop only because it's run twice */

#ifdef RGFW_WINDOWS
DWORD loop2(void* args);
#else
void* loop2(void* args);
#endif


unsigned char icon[4 * 3 * 3] = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};
unsigned char running = 1, running2 = 1;

/* callbacks are another way you can handle events in RGFW */
void refreshCallback(RGFW_window* win) {
    drawLoop(win);
}


RGFW_window* win2;

int main_basic(void) {
	RGFW_setClassName("RGFW Basic");
    RGFW_window* win = RGFW_createWindow("RGFW Example Window", RGFW_RECT(500, 500, 500, 500), RGFW_ALLOW_DND | RGFW_CENTER);
    RGFW_window_makeCurrent(win);
    
    RGFW_window_setIcon(win, icon, RGFW_AREA(3, 3), 4);

    RGFW_setWindowRefreshCallback(refreshCallback);

    #ifdef RGFW_MACOS
    win2 = RGFW_createWindow("subwindow", RGFW_RECT(200, 200, 200, 200), 0);
    #endif
    RGFW_createThread((RGFW_threadFunc_ptr)loop2, NULL); /* the function must be run after the window of this thread is made for some reason (using X11) */

    unsigned char i;

    glEnable(GL_BLEND);             
    glClearColor(0, 0, 0, 0);

    RGFW_window_setMouseStandard(win, RGFW_MOUSE_RESIZE_NESW);
    
    u32 fps = 0;

    while (running && !RGFW_isPressed(win, RGFW_Escape)) {   
        #ifdef __APPLE__
        RGFW_window_checkEvent(win2);
        #endif

        RGFW_window_eventWait(win, RGFW_NEXT);
        
        while (RGFW_window_checkEvent(win) != NULL) {
            if (win->event.type == RGFW_windowMoved) {
                printf("window moved\n");
            }
            else if (win->event.type == RGFW_windowResized) {
                printf("window resized\n");
            }
            if (win->event.type == RGFW_quit) {
                running = 0;  
                break;
            }
            if (RGFW_isPressed(win, RGFW_Up)) {
                char* str = RGFW_readClipboard(NULL);
                printf("Pasted : %s\n", str);
                free(str);
            }
            else if (RGFW_isPressed(win, RGFW_Down))
                RGFW_writeClipboard("DOWN", 4);
            else if (RGFW_isPressed(win, RGFW_Space))
                printf("fps : %i\n", fps);
            else if (RGFW_isPressed(win, RGFW_w))
                RGFW_window_setMouseDefault(win);
            else if (RGFW_isPressed(win, RGFW_q))
                RGFW_window_showMouse(win, 0);
            else if (RGFW_isPressed(win, RGFW_t)) {
                RGFW_window_setMouse(win, icon, RGFW_AREA(3, 3), 4);
            }

            if (win->event.type == RGFW_dnd) {
                for (i = 0; i < win->event.droppedFilesCount; i++)
                    printf("dropped : %s\n", win->event.droppedFiles[i]);
            }

            else if (win->event.type == RGFW_jsButtonPressed)
                printf("pressed %i\n", win->event.button);

            else if (win->event.type == RGFW_jsAxisMove && !win->event.button)
                printf("{%i, %i}\n", win->event.axis[0].x, win->event.axis[0].y);
        }

        drawLoop(win);
        fps = RGFW_window_checkFPS(win, 0);
    }

    running2 = 0;
    RGFW_window_close(win);
    return EXIT_SUCCESS;
}

void drawLoop(RGFW_window *w) {
    RGFW_window_makeCurrent(w);

    #ifndef RGFW_VULKAN
    glClearColor(255, 255, 255, 255);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glBegin(GL_TRIANGLES);
        glColor3f(1, 0, 0); glVertex2f(-0.6, -0.75);
        glColor3f(0, 1, 0); glVertex2f(0.6, -0.75);
        glColor3f(0, 0, 1); glVertex2f(0, 0.75);
    glEnd();
    
    #else

    #endif

    
    RGFW_window_swapBuffers(w); /* NOTE(EimaMei): Rendering should always go: 1. Clear everything 2. Render 3. Swap buffers. Based on https://www.khronos.org/opengl/wiki/Common_Mistakes#Swap_Buffers */
}


#ifdef RGFW_WINDOWS
DWORD loop2(void* args) {
#else
void* loop2(void* args) {
#endif
    RGFW_UNUSED(args);

    #ifndef __APPLE__
    RGFW_window* win = RGFW_createWindow("subwindow", RGFW_RECT(200, 200, 200, 200), 0);
    #else
    RGFW_window* win = win2;
    #endif

    while (running2) {
//printf("hello\n");
		/* 
            not using a while loop here because there is only one event I care about 
        */
        #ifndef __APPLE__
        RGFW_window_checkEvent(win);
        #endif

        /* 
            I could've also done

            if (RGFW_checkEvents(win).type == RGFW_quit)
        */

        if (win->event.type == RGFW_quit)
            break;

        if (win->event.type == RGFW_mouseButtonPressed) {
            RGFW_stopCheckEvents();
        }

        drawLoop(win);
    }

    running = 0;
    RGFW_window_close(win);

    #ifdef RGFW_WINDOWS
    return 0;
    #else
    return NULL;
    #endif
}

///-----------------------------------------------------------------------------
/// ImGui demo

/* I'm using opengl 2 here because it requires less setup and I'm lazy */
#include "backends/imgui_impl_opengl2.h"


/* handle these functions across all apis */
void imgui_newFrame(void);
void imgui_render(void);
void imgui_shutdown(void);

int main_imgui() {
    RGFW_window* win = RGFW_createWindow("imgui", RGFW_RECT(0, 0, 700, 600), RGFW_CENTER);
    RGFW_window_makeCurrent(win);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    // Build atlas
    unsigned char* tex_pixels = nullptr;
    int tex_w, tex_h;
    io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

    ImGui_ImplRgfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL2_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        RGFW_window_checkEvents(win, RGFW_NO_WAIT);
        io.DisplaySize = ImVec2(win->r.w, win->r.h);

        imgui_newFrame();

        static float f = 0.0f;
        static int counter = 0;
        
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            char buffer[200];
            ImGui::InputText("label", buffer, 200);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
        ImGui::End();

        glViewport(0, 0, win->r.w, win->r.h);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

        imgui_render();
        RGFW_window_swapBuffers(win);
    }

    imgui_shutdown();
    RGFW_window_close(win);
    return EXIT_SUCCESS;
}

void imgui_newFrame() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(2,2); // this should be a platform appropriate scaling factor

    ImGui::NewFrame();
    ImGui_ImplRgfw_NewFrame();
    ImGui_ImplOpenGL2_NewFrame();
}

void imgui_render(void) {
    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();

    ImGui_ImplOpenGL2_RenderDrawData(draw_data);
}

void imgui_shutdown(void) {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplRgfw_Shutdown();
    ImGui::DestroyContext();
}

int main(void) {
    printf("LabTemplate RGFW OpenGL2 demo\n");
    //return main_basic();
    return main_imgui();
}
