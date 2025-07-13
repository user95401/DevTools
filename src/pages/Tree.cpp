#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include "../platform/utils.hpp"
#ifndef GEODE_IS_WINDOWS
#include <cxxabi.h>
#endif

using namespace geode::prelude;

void DevTools::drawTreeBranch(CCNode* node, size_t index, bool parentIsVisible) {
    auto selected = DevTools::get()->getSelectedNode() == node;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes;
    if (selected) flags |= ImGuiTreeNodeFlags_Selected;
    if (!node->getChildrenCount()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (m_settings.arrowExpand) flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_settings.doubleClickExpand) flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
    std::stringstream name;
    name << "[" << index << "] " << getNodeName(node) << " ";
    if (node->getTag() != -1) name << "(" << node->getTag() << ") ";
    if (node->getID().size()) name << "\"" << node->getID() << "\" ";
    if (node->getChildrenCount()) name << "<" << node->getChildrenCount() << "> ";

    parentIsVisible = node->isVisible() and parentIsVisible;

    auto alpha = ImGui::GetStyle().DisabledAlpha;
    ImGui::GetStyle().DisabledAlpha = node->isVisible() ? alpha + 0.15f : alpha;

    ImGui::BeginDisabled(!parentIsVisible);
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false); // Bypass iteract blocking in imgui

    // The order here is unusual due to imgui weirdness; see the second-to-last paragraph in https://kahwei.dev/2022/06/20/imgui-tree-node/
    bool expanded = ImGui::TreeNodeEx(node, flags, "%s", name.str().c_str());

    ImGui::GetStyle().DisabledAlpha = alpha;
    ImGui::PopItemFlag();
    ImGui::EndDisabled();

    if (ImGui::IsItemClicked()) { 
        DevTools::get()->selectNode(node);
        selected = true;
    }
    if (ImGui::IsItemHovered() && (m_settings.alwaysHighlight || ImGui::IsKeyDown(ImGuiMod_Shift))) {
        DevTools::get()->highlightNode(node, HighlightMode::Hovered);
    }

    if (expanded) {
        if (m_settings.attributesInTree) {
            this->drawNodeAttributes(node);
        }
        size_t i = 0;
        for (auto& child : CCArrayExt<CCNode*>(node->getChildren())) {
            this->drawTreeBranch(child, i++, parentIsVisible);
        }
        ImGui::TreePop();
    }
}

void DevTools::drawTree() {
#ifdef GEODE_IS_MOBILE
    ImGui::Dummy({0.f, 20.f});
#endif

    this->drawTreeBranch(CCDirector::get()->getRunningScene(), 0);
}
