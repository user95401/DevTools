#include "../DevTools.hpp"
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>
// #include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <fmod.hpp>
#include <numeric>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

static float RAINBOW_HUE = 0.f;

void DevTools::drawSettings() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 1.f, 1.f });

#ifdef GEODE_IS_MOBILE
    ImGui::Dummy({0.f, 20.f});
#endif

    // TODO: fix this option as it hasnt worked in a while lol
#if 0
    ImGui::Checkbox("GD in Window", &m_settings.GDInWindow);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show GD inside a window when DevTools are open");
    }
#endif
    ImGui::Checkbox("Attributes in Tree", &m_settings.attributesInTree);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Show node attributes in the Tree");
    }
    ImGui::Checkbox("Highlight Nodes", &m_settings.alwaysHighlight);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Always highlight nodes when hovered in the Tree. "
            "When disabled, you can highlight by pressing Shift."
        );
    }
    ImGui::Checkbox("Highlight Layouts", &m_settings.highlightLayouts);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Highlights the borders of all layouts applied to nodes"
        );
    }
    ImGui::Checkbox("Arrow to Expand", &m_settings.arrowExpand);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "If enabled, expanding nodes in the Tree only works with the arrow.\n"
            "Makes selecting nodes less annoying."
        );
    }
    ImGui::Checkbox("Double-Click Expand", &m_settings.doubleClickExpand);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "If enabled, expanding nodes in the Tree only works with double-click instead of simple click.\n"
            "Makes selecting nodes less annoying."
        );
    }
    ImGui::Checkbox("Order Node Children", &m_settings.orderChildren);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "When enabled (default behavior) node children are sorted by Z Order.\n"
            "When disabled, children have the same order they do during init functions (maybe).\n"
            "As a side effect to disabling this, things may render incorrectly."
        );
    }
    //"if advanced ever has more than one option, add it back"? - so i remove it here too huh?
    /*ImGui::Checkbox("Advanced Settings", &m_settings.advancedSettings);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Shows advanced settings. Mostly useful only for development of Geode itself."
        );
    }*/
    ImGui::Checkbox("Show Memory Viewer", &m_settings.showMemoryViewer);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Shows the memory viewer window."
        );
    }
    ImGui::Checkbox("Show Logs Window", &m_settings.showLogsWindow);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Window with current log file contents rendered similar to console."
        );
    }
    ImGui::Checkbox("Show Mod Graph", &m_settings.showModGraph);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Shows the mod graph window."
        );
    }

    ImGui::PopStyleVar();

    ImGui::Separator();

    ImGui::DragFloat("Font Size", &m_settings.fontGlobalScale, 0.01f, 1.0f, 3.0f);
    
#ifdef GEODE_IS_DESKTOP

    ImGui::Separator();

    ImGui::Text("GD Window");

    auto winSize = CCDirector::get()->getWinSize();
    auto frameSize = GameManager::get()->resolutionForKey(GameManager::get()->m_resolution);
    auto fps = roundf(1 / CCDirector::get()->getAnimationInterval());
    auto ratio = std::gcd(static_cast<int>(frameSize.width), static_cast<int>(frameSize.height));
#ifdef GEODE_IS_WINDOWS

    std::string text = "";
    text += "Custom";
    text.push_back('\0');
    for (int i = 1; i < 28; i++) {
        auto size = GameManager::get()->resolutionForKey(i);
        text += fmt::format("{}x{}", size.width, size.height);
        text.push_back('\0');
    }
    int selectedResolution = GameManager::get()->m_resolution;

    static CCSize customResolution = frameSize;

    if (ImGui::Combo("##devtools/resolution", &selectedResolution, text.c_str())) {
        GameManager::get()->m_resolution = selectedResolution;

        // TODO: idk how to do this on macos
        if (selectedResolution != 0) {
            auto size = GameManager::get()->resolutionForKey(selectedResolution);
            CCEGLView::get()->resizeWindow(size.width, size.height);
        }
        else {
            CCEGLView::get()->resizeWindow(customResolution.width, customResolution.height);
        }
        CCEGLView::get()->centerWindow();
    }

    if (selectedResolution == 0) {
        int size[2] = {
            static_cast<int>(customResolution.width),
            static_cast<int>(customResolution.height),
        };
        if (ImGui::DragInt2("Size", size)) {
            size[0] = std::fabs(size[0]);
            size[1] = std::fabs(size[1]);
            customResolution = CCSizeMake(size[0], size[1]);
        }
        if (ImGui::Button("Apply##size-apply")) {
            GameManager::get()->m_resolution = 0;
            CCEGLView::get()->resizeWindow(customResolution.width, customResolution.height);
            CCEGLView::get()->centerWindow();
        }
    }
#endif


    ImGui::TextWrapped(
        "GL Size: %dx%d",
        static_cast<int>(winSize.width),
        static_cast<int>(winSize.height)
    );
    ImGui::TextWrapped(
        "Frame Size: %dx%d",
        static_cast<int>(frameSize.width),
        static_cast<int>(frameSize.height)
    );
    ImGui::TextWrapped("FPS: %d", static_cast<int>(fps));
    ImGui::TextWrapped(
        "Aspect Ratio: %d:%d",
        static_cast<int>(frameSize.width / ratio),
        static_cast<int>(frameSize.height / ratio)
    );
#endif

    ImGui::Separator();

    ImGui::Text("Theme");
    static auto SELECTED = static_cast<int>(getThemeIndex(m_settings.theme));
    if (ImGui::Combo("##devtools/theme", &SELECTED,
        (ranges::join(getThemeOptions(), std::string(1, '\0')) + '\0').c_str()
    )) {
        m_settings.theme = getThemeAtIndex(SELECTED);
        m_reloadTheme = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select Theme");
    }

    if (m_settings.theme == "Dark") {
        auto color = m_settings.themeColor;
        float _color[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
        if (ImGui::ColorEdit4("Primary Color", _color)) {
            color.r = _color[0] * 255;
            color.g = _color[1] * 255;
            color.b = _color[2] * 255;
            color.a = _color[3] * 255;
            m_settings.themeColor = color;
            m_reloadTheme = true;
        }
    }

    ImGui::Separator();

    ImGui::TextWrapped("Developed by ");

    float dt = CCDirector::get()->getDeltaTime();

    RAINBOW_HUE += 0.25f * dt;
    if (RAINBOW_HUE >= 1.f) {
        RAINBOW_HUE = 0.f;
    }

    float hue = RAINBOW_HUE;

    ImVec4 color;
    color.w = 1.f;
    for (auto c : std::string("Geode Team")) {
        hue += 0.04f;
        ImGui::SameLine(0.f, 0.f);
        ImGui::ColorConvertHSVtoRGB(hue, .5f, 1.f, color.x, color.y, color.z);
        ImGui::TextColored(color, "%c", c);
    }

    ImGui::TextWrapped(
        "Running Geode %s, DevTools %s",
        Loader::get()->getVersion().toVString().c_str(),
        Mod::get()->getVersion().toVString().c_str()
    );

    ImGui::BeginChild("Reset Layout Text Box", {}, ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
    ImGui::Text("Reset Layout");
	ImGui::EndChildFrame();
    ImGui::SameLine();
    if (ImGui::Button("Default")) m_shouldRelayout = LayoutPreset::Default;
    ImGui::SameLine();
    if (ImGui::Button("Def. Right")) m_shouldRelayout = LayoutPreset::DefaultRight;
    ImGui::SameLine();
    if (ImGui::Button("Cocos Explorer")) m_shouldRelayout = LayoutPreset::CocosExplorerLike;
}

// TODO: this hook also isnt gd *
/*class $modify(AppDelegate) {
    void applicationWillEnterForeground() override {
        AppDelegate::applicationWillEnterForeground();
        if (DevTools::get()->pausedGame()) {
            // TODO: undo later
            // FMODAudioEngine::sharedEngine()->m_globalChannel->setPaused(true);
        }
    }
};*/
