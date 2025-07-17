#include <imgui_internal.h>
#include "DevTools.hpp"
#include "fonts/FeatherIcons.hpp"
#include "fonts/OpenSans.hpp"
#include "fonts/GeodeIcons.hpp"
#include "fonts/RobotoMono.hpp"
#include "fonts/SourceCodeProLight.hpp"
#include "platform/platform.hpp"
#include <Geode/loader/Log.hpp>
#include <Geode/loader/Mod.hpp>
#include "ImGui.hpp"

template<>
struct matjson::Serialize<Settings> {
    static Result<Settings> fromJson(const matjson::Value& value) {
        Settings defaults;

        return Ok(Settings {
            .GDInWindow = value["game_in_window"].asBool().unwrapOr(std::move(defaults.GDInWindow))
            ,.attributesInTree = value["attributes_in_tree"].asBool().unwrapOr(std::move(defaults.attributesInTree))
            ,.alwaysHighlight = value["always_highlight"].asBool().unwrapOr(std::move(defaults.alwaysHighlight))
            ,.highlightLayouts = value["highlight_layouts"].asBool().unwrapOr(std::move(defaults.highlightLayouts))
            ,.arrowExpand = value["arrow_expand"].asBool().unwrapOr(std::move(defaults.arrowExpand))
            ,.doubleClickExpand = value["double_click_expand"].asBool().unwrapOr(std::move(defaults.doubleClickExpand))
            ,.orderChildren = value["order_children"].asBool().unwrapOr(std::move(defaults.orderChildren))
            ,.advancedSettings = value["advanced_settings"].asBool().unwrapOr(std::move(defaults.advancedSettings))
            ,.showMemoryViewer = value["show_memory_viewer"].asBool().unwrapOr(std::move(defaults.showMemoryViewer))
            ,.showModGraph = value["show_mod_graph"].asBool().unwrapOr(std::move(defaults.showModGraph))
            ,.theme = value["theme"].asString().unwrapOr(std::move(defaults.theme))
            ,.themeColor = value["theme_color"].as<ccColor4B>().unwrapOr(std::move(defaults.themeColor))
            ,.fontGlobalScale = value["font_global_scale"].as<float>().unwrapOr(std::move(defaults.fontGlobalScale))
            ,.showLogsWindow = value["show_logs_window"].asBool().unwrapOr(std::move(defaults.showLogsWindow))
        });
    }

    static matjson::Value toJson(const Settings& settings) {
        return matjson::makeObject({
            { "game_in_window", settings.GDInWindow },
            { "attributes_in_tree", settings.attributesInTree },
            { "always_highlight", settings.alwaysHighlight },
            { "highlight_layouts", settings.highlightLayouts },
            { "arrow_expand", settings.arrowExpand },
            { "double_click_expand", settings.doubleClickExpand },
            { "order_children", settings.orderChildren },
            { "advanced_settings", settings.advancedSettings },
            { "show_memory_viewer", settings.showMemoryViewer },
            { "show_mod_graph", settings.showModGraph },
            { "theme", settings.theme },
            { "theme_color", settings.themeColor },
            { "font_global_scale", settings.fontGlobalScale },
            { "show_logs_window", settings.showLogsWindow },
        });
    }
};

$on_mod(DataSaved) { DevTools::get()->saveSettings(); }

DevTools* DevTools::get() {
    static auto inst = new DevTools();
    return inst;
}

void DevTools::loadSettings() { m_settings = Mod::get()->getSavedValue<Settings>("settings"); }
void DevTools::saveSettings() { Mod::get()->setSavedValue("settings", m_settings); }
Settings DevTools::getSettings() { return m_settings; }

bool DevTools::shouldPopGame() const {
    return m_visible && m_settings.GDInWindow;
}

bool DevTools::pausedGame() const {
    return m_pauseGame;
}

bool DevTools::isSetup() const {
    return m_setup;
}

bool DevTools::shouldOrderChildren() const {
    return m_settings.orderChildren;
}

CCNode* DevTools::getSelectedNode() const {
    return m_selectedNode;
}

void DevTools::selectNode(CCNode* node) {
    m_selectedNode = node;
}

void DevTools::highlightNode(CCNode* node, HighlightMode mode) {
    m_toHighlight.push_back({ node, mode });
}

void DevTools::drawPage(const char* name, void(DevTools::*pageFun)()) {
    if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x); // Fix wrapping after window resize

        (this->*pageFun)();

        ImGui::PopTextWrapPos();

        // Scroll when dragging (useful for android users)
        auto mouse_dt = ImGui::GetIO().MouseDelta;
        ImVec2 delta = ImGui::GetIO().MouseDownDuration[0] > 0.1 ? ImVec2(mouse_dt.x * -1, mouse_dt.y * -1) : ImVec2(0, 0);
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* window = g.CurrentWindow;
        if (!window) return;
        bool hovered = false;
        bool held = false;
        ImGuiID id = window->GetID("##scrolldraggingoverlay");
        ImGui::KeepAliveID(id);
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_MouseButtonLeft;
        if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
            ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
        if (held && fabs(delta.x) >= 0.1f) ImGui::SetScrollX(window, window->Scroll.x + delta.x);
        if (held && fabs(delta.y) >= 0.1f) ImGui::SetScrollY(window, window->Scroll.y + delta.y);

    }
    ImGui::End();
}

void DevTools::drawPages() {
    const auto size = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize();

    if ((!Mod::get()->setSavedValue("layout-loaded-1.92.1", true) || m_shouldRelayout > LayoutPreset::DontReset)) {

        auto id = m_dockspaceID;
        ImGui::DockBuilderRemoveNode(id);
        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_PassthruCentralNode);

        if (m_shouldRelayout == LayoutPreset::CocosExplorerLike) { //"cocos-explorer"-like layout
            ImGui::DockBuilderDockWindow("###devtools/geometry-dash", id);

            auto window = ImGui::DockBuilderAddNode(0, ImGuiDockNodeFlags_PassthruCentralNode);

            ImGui::DockBuilderSetNodeSize(window, ImGui::GetMainViewport()->Size / 1.9f);
            ImGui::DockBuilderSetNodePos(window, { 50, 60 });

            auto windowLeft = ImGui::DockBuilderSplitNode(window, ImGuiDir_Left, 0.5f, nullptr, &window);

			ImGui::DockBuilderDockWindow("###devtools/tree", windowLeft);
            ImGui::DockBuilderDockWindow("###devtools/settings", windowLeft);
            ImGui::DockBuilderDockWindow("###devtools/advanced/settings", windowLeft);

			ImGui::DockBuilderDockWindow("###devtools/attributes", window);
            ImGui::DockBuilderDockWindow("###devtools/advanced/mod-graph", window);
            ImGui::DockBuilderDockWindow("###devtools/advanced/mod-index", window);

            ImGui::DockBuilderDockWindow("###devtools/preview", window);
            ImGui::DockBuilderDockWindow("###devtools/memory-viewer", window);
            ImGui::DockBuilderDockWindow("###devtools/logs", window);

            ImGui::DockBuilderFinish(window);
        }
        else {
            //m_shouldRelayout  = 1 -> left
            //m_shouldRelayout  = 2 -> right 
            //m_shouldRelayout >= 3 -> others non-default.
            auto sideDock = ImGui::DockBuilderSplitNode(m_dockspaceID, m_shouldRelayout == LayoutPreset::DefaultRight ? ImGuiDir_Right : ImGuiDir_Left, 0.3f, nullptr, &id);
            auto topSideDock = ImGui::DockBuilderSplitNode(sideDock, ImGuiDir_Up, 0.45f, nullptr, &sideDock);
            auto bottomLeftTopHalfDock = ImGui::DockBuilderSplitNode(sideDock, ImGuiDir_Up, 0.55f, nullptr, &sideDock);

            ImGui::DockBuilderDockWindow("###devtools/tree", topSideDock);
            ImGui::DockBuilderDockWindow("###devtools/settings", topSideDock);
            ImGui::DockBuilderDockWindow("###devtools/advanced/settings", topSideDock);
            ImGui::DockBuilderDockWindow("###devtools/attributes", bottomLeftTopHalfDock);
            ImGui::DockBuilderDockWindow("###devtools/memory-viewer", bottomLeftTopHalfDock);
            ImGui::DockBuilderDockWindow("###devtools/preview", sideDock);
            ImGui::DockBuilderDockWindow("###devtools/geometry-dash", id);
            ImGui::DockBuilderDockWindow("###devtools/advanced/mod-graph", topSideDock);
            ImGui::DockBuilderDockWindow("###devtools/advanced/mod-index", topSideDock);

            auto bottom = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 0.228f, nullptr, &id);
            ImGui::DockBuilderDockWindow("###devtools/logs", bottom);
        };
        ImGui::DockBuilderFinish(id);

        m_shouldRelayout = LayoutPreset::DontReset;
    }

    this->drawPage(
        U8STR(FEATHER_GIT_MERGE " Tree###devtools/tree"),
        &DevTools::drawTree
    );

    this->drawPage(
        U8STR(FEATHER_SETTINGS " Settings###devtools/settings"),
        &DevTools::drawSettings
    );

    // if advanced ever has more than one option, add it back
#if 0
    if (m_settings.advancedSettings) {
        this->drawPage(
                U8STR(FEATHER_SETTINGS " Advanced Settings###devtools/advanced/settings"),
                &DevTools::drawAdvancedSettings
        );
    }
#endif

    this->drawPage(
        U8STR(FEATHER_TOOL " Attributes###devtools/attributes"),
        &DevTools::drawAttributes
    );

    // TODO: fix preview tab
#if 0
    this->drawPage(
        U8STR(FEATHER_DATABASE " Preview###devtools/preview"),
        &DevTools::drawPreview
    );
#endif

    if (m_settings.showModGraph) {
        this->drawPage(
            U8STR(FEATHER_SHARE_2 " Mod Graph###devtools/advanced/mod-graph"),
            &DevTools::drawModGraph
        );
    }

    if (m_settings.showMemoryViewer) {
        this->drawPage(
            U8STR(FEATHER_TERMINAL " Memory viewer###devtools/memory-viewer"), 
            &DevTools::drawMemory
        );
    }

    if (m_settings.showLogsWindow) {
        this->drawPage(
            U8STR(FEATHER_FILE " Logs###devtools/logs"),
            &DevTools::drawLogs
        );
    }

    ImGui::ShowMetricsWindow();

}

void DevTools::draw(GLRenderCtx* ctx) {
    if (m_visible) {
        if (m_reloadTheme) {
            m_reloadTheme = false;
            applyTheme(m_settings.theme);
        }
        
        ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        bgDrawList->AddRectFilled(ImVec2(0, 0), viewportSize, IM_COL32(255, 255, 255, 255));

        m_dockspaceID = ImGui::DockSpaceOverViewport(
            0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode
        );

        ImGui::GetIO().FontGlobalScale = m_settings.fontGlobalScale;

        ImGui::PushFont(m_defaultFont);
        this->drawPages();
        if (m_selectedNode) this->highlightNode(m_selectedNode, HighlightMode::Selected);
        if (this->shouldUseGDWindow()) this->drawGD(ctx);
        ImGui::PopFont();
    }

    // Shows hidden cursor out of GD Window
    auto cursorHidden = CCEGLView::get()->m_bCursorHidden;
    auto drawCursor = m_visible and cursorHidden and !shouldPassEventsToGDButTransformed();
    GEODE_DESKTOP(ImGui::GetIO().MouseDrawCursor = drawCursor);
}

void DevTools::setupFonts() {
    static const ImWchar icon_ranges[] = { FEATHER_MIN_FA, FEATHER_MAX_FA, 0 };
    static const ImWchar box_ranges[]  = { BOX_DRAWING_MIN_FA, BOX_DRAWING_MAX_FA, 0 };
    static const ImWchar* def_ranges   = ImGui::GetIO().Fonts->GetGlyphRangesDefault();

    static constexpr auto add_font = [](
        void* font, size_t realSize, float size, const ImWchar* range
    ) {
        auto& io = ImGui::GetIO();
        // AddFontFromMemoryTTF assumes ownership of the passed data unless you configure it not to.
        // Our font data has static lifetime, so we're handling the ownership.

        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        auto* result = io.Fonts->AddFontFromMemoryTTF(
            font, realSize, size, &config, range
        );
        config.MergeMode = true;
        io.Fonts->AddFontFromMemoryTTF(
            Font_FeatherIcons, sizeof(Font_FeatherIcons), size - 4.f, &config, icon_ranges
        );
        io.Fonts->Build();
        return result;
    };

    m_defaultFont = add_font(Font_OpenSans, sizeof(Font_OpenSans), 18.f, def_ranges);
    m_smallFont = add_font(Font_OpenSans, sizeof(Font_OpenSans), 10.f, def_ranges);
    m_monoFont = add_font(Font_RobotoMono, sizeof(Font_RobotoMono), 18.f, def_ranges);
    m_boxFont = add_font(Font_SourceCodeProLight, sizeof(Font_SourceCodeProLight), 23.f, box_ranges);
}

void DevTools::setup() {
    if (m_setup) return;
    m_setup = true;

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // if this is true then it just doesnt work :( why
    io.ConfigDockingWithShift = false;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigWindowsResizeFromEdges = true;
    io.FontAllowUserScaling = true;
    io.MouseDoubleClickTime = 1.0f;

    this->setupFonts();
    this->setupPlatform();

#ifdef GEODE_IS_MOBILE
    ImGui::GetStyle().ScrollbarSize = 20.f;
    // ImGui::GetStyle().TabBarBorderSize = 60.f;
#endif
}

void DevTools::destroy() {
    if (!m_setup) return;
    this->show(false);
    auto& io = ImGui::GetIO();
    io.BackendPlatformUserData = nullptr;
    m_fontTexture->release();
    m_fontTexture = nullptr;

    ImGui::DestroyContext();
    m_setup = false;
    m_reloadTheme = true;
}

bool DevTools::isVisible() {
    return m_visible;
}

void DevTools::show(bool visible) {
    m_visible = visible;

    auto& io = ImGui::GetIO();
    io.WantCaptureMouse = visible;
    io.WantCaptureKeyboard = visible;

    getMod()->setSavedValue<bool>("visible", m_visible);
}

void DevTools::toggle() {
    this->show(!m_visible);
}

void DevTools::sceneChanged() {
    m_selectedNode = nullptr;
}

bool DevTools::shouldUseGDWindow() const {
    return Mod::get()->getSettingValue<bool>("should-use-gd-window");
}