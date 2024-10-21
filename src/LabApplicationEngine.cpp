
#include "LabApplicationEngine.hpp"
#include "Modes.hpp"

#include "imgui.h"
#include "imgui_internal.h"

namespace lab {


class LabMainWindow {
public:
    explicit LabMainWindow();
    ~LabMainWindow() = default;
    void UpdateMainWindow(float dt, bool viewport_hovering, bool viewport_dragging);
    
    // dimensions of the viewport of the main window ~ it is the area not
    // occupied by docked panels.
    // the value always reflects the most recent invocation of Update
    lab::ViewDimensions GetViewportDimensions() const { return vp; }
    
private:
    lab::ViewDimensions vp;
    int model_generation = 0;
};

namespace {

// Tips: Use with ImGuiDockNodeFlags_PassthruCentralNode!
// The limitation with this call is that your window won't have a menu bar.
// Even though we could pass window flags, it would also require the user to be 
// able to call BeginMenuBar() somehow meaning we can't Begin/End in a single function.
// But you can also use BeginMainMenuBar(). If you really want a menu bar inside the
// same window as the one hosting the dockspace, you will need to copy this code somewhere and tweak it.

ImGuiID LabDockSpaceOverViewport(const ImGuiViewport* viewport, ImGuiDockNodeFlags dockspace_flags, const ImGuiWindowClass* window_class)
{
    using namespace ImGui;
    if (viewport == NULL)
        viewport = GetMainViewport();
    
    SetNextWindowPos(viewport->WorkPos);
    SetNextWindowSize(viewport->WorkSize);
    SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags host_window_flags; /* = 0;
    host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        host_window_flags |= ImGuiWindowFlags_NoBackground;*/
    
    host_window_flags = ImGuiWindowFlags_NoTitleBar | 
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoBackground |
                        ImGuiDockNodeFlags_PassthruCentralNode;
    
    char label[32];
    ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);
    
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    Begin(label, NULL, host_window_flags);
    PopStyleVar(3);
    ImGuiID dockspace_id = GetID("DockSpace");
    DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, window_class);
    End();
    
    return dockspace_id;
}

} // anon

LabMainWindow::LabMainWindow() {
    model_generation = 0;
}

void LabMainWindow::UpdateMainWindow(float dt, bool viewport_hovering, bool viewport_dragging)
{
    ApplicationEngine::app()->UpdateApplicationEngine();
    ImGuiIO& io = ImGui::GetIO();

    auto& mm = *ApplicationEngine::app()->mm();
    mm.UpdateTransactionQueueAndModes();
        
    if (ImGui::BeginMainMenuBar()) {
        auto currMajMode = mm.CurrentMajorMode();
        std::string menuName;
        if (currMajMode) {
            menuName = mm.CurrentMajorMode()->Name();
        }
        if (menuName == "" || menuName == "Empty")
            menuName = "Welcome";

        menuName += "###MM";
        if (ImGui::BeginMenu(menuName.c_str())) {
            for (auto m : mm.MajorModeNames()) {
                auto menuName = m + "###MjM";
                auto mode = mm.FindMode(m);
                if (mode) {
                    bool active = mm.CurrentMajorMode()->Name() == m;
                    if (ImGui::MenuItem(menuName.c_str(), nullptr, active, true)) {
                        if (!active)
                            mm.ActivateMajorMode(m);
                    }
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Modes##mmenu")) {
            for (auto m : mm.MinorModeNames()) {
                auto menuName = m + "###mM";
                auto mode = mm.FindMode(m);
                if (mode) {
                    bool active = mode->IsActive();
                    if (ImGui::MenuItem(menuName.c_str(), nullptr, active, true)) {
                        if (active)
                            mode->Deactivate();
                        else
                            mode->Activate();
                    }
                }
            }
            ImGui::EndMenu();
        }

        mm.RunMainMenu();
        ImGui::EndMainMenuBar();
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspace_id = LabDockSpaceOverViewport(viewport, 
                                    ImGuiDockNodeFlags_NoDockingInCentralNode |
                                    ImGuiDockNodeFlags_PassthruCentralNode,
                                    NULL);

    // outline the remaining viewport region
    // https://github.com/ocornut/imgui/issues/5921
    ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockspace_id);

    float view_pos_x, view_pos_y, view_sz_x, view_sz_y;

    if (centralNode) {
        bool centralNodeHovered = false;//centralNode && centralNode->IsHovered;

        // Draw a red rectangle in the central node just for demonstration purposes
        ImGui::GetBackgroundDrawList()->AddRect(   centralNode->Pos,
                                                { centralNode->Pos.x + centralNode->Size.x,
            centralNode->Pos.y + centralNode->Size.y },
                                                IM_COL32(0, centralNodeHovered? 100 : 0, 0, 255),
                                                0.f,
                                                ImDrawFlags_None,
                                                3.f);

        view_pos_x = centralNode->Pos.x  * io.DisplayFramebufferScale.x;
        view_pos_y = centralNode->Pos.y  * io.DisplayFramebufferScale.y;
        view_sz_x =  centralNode->Size.x * io.DisplayFramebufferScale.x;
        view_sz_y =  centralNode->Size.y * io.DisplayFramebufferScale.y;
    }
    else {
        view_pos_x = 0;
        view_pos_y = 0;
        view_sz_x = (float) viewport->Size.x;
        view_sz_y = (float) viewport->Size.y;
    }

    /*   _   _ ___
        | | | |_ _|
        | | | || |
        | |_| || |
         \___/|___|  */

    ViewInteraction vi;
    vi.view = {
        (float) viewport->Size.x, (float) viewport->Size.y,
        view_pos_x, view_pos_y, view_sz_x, view_sz_y };
    vi.dt = dt;
    mm.RunModeUIs(vi);

    static bool was_dragging = false;
    
    ImVec2 mousePos = io.MousePos;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 mousePosInWindow = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);

    if (centralNode) {
        vi.x = io.DisplayFramebufferScale.x * (mousePosInWindow.x - centralNode->Pos.x);
        vi.y = io.DisplayFramebufferScale.y * (mousePosInWindow.y - centralNode->Pos.y);
    }
    else {
        vi.x = io.DisplayFramebufferScale.x * (mousePosInWindow.x);
        vi.y = io.DisplayFramebufferScale.y * (mousePosInWindow.y);
    }

    if (viewport_dragging) {
        vi.start = !was_dragging;
        vi.end = false;
        mm.RunViewportDragging(vi);
    }
    else {
        if (was_dragging) {
            vi.start = false;
            vi.end = true;
            mm.RunViewportDragging(vi);
        }
        else {
            mm.RunViewportHovering(vi);
        }
    }
    was_dragging = viewport_dragging;
}



struct ApplicationEngine::Data {
    Data() {
    }
    ~Data() {
    }
    ModeManager mm;
    LabMainWindow mainWindow;
    float dt;
};

ApplicationEngine::ApplicationEngine() {
}

ApplicationEngine::~ApplicationEngine() {
    delete _self;
}

void ApplicationEngine::UpdateApplicationEngine() {
    static auto prev= std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev);
    _self->dt = dt.count() / 1000.0f;
}

ModeManager* ApplicationEngine::mm() {
    return &_self->mm;
}

namespace {
ApplicationEngine gApp;
}

//static
ApplicationEngine* ApplicationEngine::app() {
    if (!gApp._self) {
        gApp._self = new Data();
        gApp.RegisterAllModes();
    }
    return &gApp;
}

void ApplicationEngine::RegisterAllModes() {
}

void ApplicationEngine::RunUI(bool viewport_hovering, bool viewport_dragging) {
    _self->mainWindow.UpdateMainWindow(_self->dt, viewport_hovering, viewport_dragging);
}

} // namespace lab
