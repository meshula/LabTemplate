
// ApplicationEngine.h

#ifndef LABAPPLICATION_APPLICATIONENGINE_H
#define LABAPPLICATION_APPLICATIONENGINE_H

namespace lab {

class ModeManager;

class ApplicationEngine {
    struct Data;
    Data* _self = nullptr;

    void RegisterAllModes();

public:
    ApplicationEngine();
    ~ApplicationEngine();
    void UpdateApplicationEngine();
    void RunUI(bool viewport_hovering, bool viewport_dragging);
    ModeManager* mm();
    static ApplicationEngine* app();
};

} // namespace lab

#endif
