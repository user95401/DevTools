#include "../fonts/FeatherIcons.hpp"
#include <Geode/utils/cocos.hpp>
#include "../DevTools.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include "../platform/utils.hpp"

using namespace geode::prelude;

#define AXIS_GET(Name_) \
    &AxisLayoutOptions::get##Name_, \
    &AxisLayoutOptions::set##Name_

template <class T, class R>
bool checkbox(const char* text, T* ptr, bool(T::* get)(), R(T::* set)(bool)) {
    bool value = (ptr->*get)();
    bool checkbox_rtn = ImGui::Checkbox(text, &value);
    if (checkbox_rtn) {
        (ptr->*set)(value);
        return true;
    }
    return false;
}

template <class T, class R>
bool checkbox(const char* text, T* ptr, bool(T::* get)() const, R(T::* set)(bool)) {
    bool value = (ptr->*get)();
    bool checkbox_rtn = ImGui::Checkbox(text, &value);
    if (checkbox_rtn) {
        (ptr->*set)(value);
        return true;
    }
    return false;
}

void DevTools::drawNodeAttributes(CCNode* node) {
    if (ImGui::Button("Deselect")) {
        return this->selectNode(nullptr);
    }
    ImGui::Text("Address: %s", fmt::to_string(fmt::ptr(node)).c_str());
    ImGui::SameLine();
    if (ImGui::Button(U8STR(FEATHER_COPY " Copy"))) {
        clipboard::write(
            utils::intToHex(reinterpret_cast<uintptr_t>(node))
        );
    }
    if (node->getUserData()) {
        ImGui::Text("User data: 0x%p", node->getUserData());
    }

    if (!node->getID().empty()) {
        std::string nodeID = node->getID();
        ImGui::Text("Node ID: %s", nodeID.c_str());
        ImGui::SameLine();
        if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copynodeid"))) {
            clipboard::write(nodeID);
        }
    } else {
        ImGui::Text("Node ID: N/A");
    }

    if (auto menuItemNode = typeinfo_cast<CCMenuItem*>(node)) {
        const auto selector = menuItemNode->m_pfnSelector;
        if (!selector) {
            std::string addr = "N/A";
            ImGui::Text("CCMenuItem selector: %s", addr.c_str());
        } else {
            const auto addr = formatAddressIntoOffset(addresser::getNonVirtual(selector), true);
            ImGui::Text("CCMenuItem selector: %s", addr.c_str());
            ImGui::SameLine();
            if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copymenuitem"))) {
                const auto addrNoModule = formatAddressIntoOffset(addresser::getNonVirtual(selector), false);
                clipboard::write(addrNoModule);
            }
        }
    }

    float pos[2] = {
        node->getPositionX(),
        node->getPositionY()
    };
    if (ImGui::Button(U8STR(FEATHER_COPY"##copypos"))) clipboard::write(fmt::format("CCPoint({}, {})", pos[0], pos[1])); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"CCPoint({}, {})\"<={pos[0], pos[1]}");
    ImGui::DragFloat2("Position", pos);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "Gets the position (x,y) of the node in OpenGL coordinates."
    );
    node->setPosition(pos[0], pos[1]);

    float scale[3] = { node->getScale(), node->getScaleX(), node->getScaleY() };
    if (ImGui::Button(U8STR(FEATHER_COPY"##copyscale"))) clipboard::write(fmt::format("({}, {})", scale[1], scale[2])); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"({}, {})\"<={scale[1], scale[2]}");
    ImGui::DragFloat3("Scale", scale, 0.025f);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The scale factor of the node."
    );
    if (node->getScale() != scale[0]) {
        node->setScale(scale[0]);
    } else {
        node->setScale(scale[1], scale[2]);
    }

    float rot[3] = { node->getRotation(), node->getRotationX(), node->getRotationY() };
    if (ImGui::Button(U8STR(FEATHER_COPY"##copyrot"))) clipboard::write(rot[1] != 0.f and rot[2] != 0.f ? fmt::format("{} {} {}", rot[0], rot[1], rot[2]) : fmt::to_string(rot[0])); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"{}\"<={rot[0]} or \"{} {} {}\"<={rot[0], rot[1], rot[2]} if rot[1,2] != 0");
    ImGui::DragFloat3("Rotation", rot);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The rotation of the node in degrees."
    );
    if (node->getRotation() != rot[0]) {
        node->setRotation(rot[0]);
    } else {
        node->setRotationX(rot[1]);
        node->setRotationY(rot[2]);
    }

    float skew[2] = { node->getSkewX(), node->getSkewY() };
    if (ImGui::Button(U8STR(FEATHER_COPY"##copyskew"))) clipboard::write(fmt::format("{} {}", skew[0], skew[1])); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"{} {}\"<={skew[0], skew[1]}");
    ImGui::DragFloat2("Skew", skew);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The skew angle of the node in degrees."
    );
    node->setSkewX(skew[0]);
    node->setSkewY(skew[1]);

    auto anchor = node->getAnchorPoint();
    if (ImGui::Button(U8STR(FEATHER_COPY"##copyanchor"))) clipboard::write(fmt::format("CCPoint({}, {})", anchor.x, anchor.y)); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"CCPoint({}, {})\"<={anchor.x, anchor.y}");
    ImGui::DragFloat2("Anchor Point", &anchor.x, 0.05f, 0.f, 1.f);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The anchor point in percent."
    );
    node->setAnchorPoint(anchor);

    auto contentSize = node->getContentSize();
    if (ImGui::Button(U8STR(FEATHER_COPY"##copycontentsize"))) clipboard::write(fmt::format("CCSize({}, {})", contentSize.width, contentSize.height)); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"CCSize({}, {})\"<={contentSize.width, contentSize.height}");
    ImGui::DragFloat2("Content Size", &contentSize.width);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The untransformed size of the node."
    );
    if (contentSize != node->getContentSize()) {
        node->setContentSize(contentSize);
        node->updateLayout();
    }

    int zOrder = node->getZOrder();
    if (ImGui::Button(U8STR(FEATHER_COPY"##copyzorder"))) clipboard::write(fmt::format("{}", zOrder)); ImGui::SameLine();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"{}\"<={zOrder}");
    ImGui::InputInt("Z Order", &zOrder);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "The z order which stands for the drawing order."
    );
    if (node->getZOrder() != zOrder) node->setZOrder(zOrder);

    checkbox(
        "Ignore Anchor Point for Position",
        node,
        &CCNode::isIgnoreAnchorPointForPosition,
        &CCNode::ignoreAnchorPointForPosition
    );
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "Whether the anchor point will be (0,0) when you position this node."
    );
    
    checkbox("Visible", node, &CCNode::isVisible, &CCNode::setVisible);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
        "Whether the node is visible."
    );

    if (auto spriteNode = typeinfo_cast<CCSprite*>(node)) {
#ifndef GEODE_IS_ARM_MAC //Undefined symbols...?
        checkbox("Flip X", spriteNode, &cocos2d::CCSprite::isFlipX, &cocos2d::CCSprite::setFlipX);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
            "Whether the sprite is flipped horizontally or not." "\n"
            "It only flips the texture of the sprite, and not the texture of the sprite's children."
        );
        checkbox("Flip Y", spriteNode, &cocos2d::CCSprite::isFlipY, &cocos2d::CCSprite::setFlipY);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
            "Whether the sprite is flipped vertically or not." "\n"
            "It only flips the texture of the sprite, and not the texture of the sprite's children."
        );
#endif
    }
    
    if (auto rgbaNode = typeinfo_cast<CCRGBAProtocol*>(node)) {
        checkbox("Cascade Color", rgbaNode, &CCRGBAProtocol::isCascadeColorEnabled, &CCRGBAProtocol::setCascadeColorEnabled);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip(
                "Whether or not color should be propagated to its children."
            );
        auto color = rgbaNode->getColor();
        float _color[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, rgbaNode->getOpacity() / 255.f };
        if (ImGui::Button(U8STR(FEATHER_COPY"##copycolor"))) clipboard::write(fmt::format("ccColor3B({}, {}, {})", color.r, color.g, color.b)); ImGui::SameLine();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("\"ccColor3B({}, {}, {})\"<={color.r, color.g, color.b}");
        if (ImGui::Button(U8STR(FEATHER_EYE"##copyop"))) clipboard::write(fmt::format("{}", rgbaNode->getOpacity())); ImGui::SameLine();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"{}, {}, {}\"<={rgbaNode->getOpacity()}");
        if (ImGui::ColorEdit4("Color", _color)) {
            rgbaNode->setColor({
                static_cast<GLubyte>(_color[0] * 255),
                static_cast<GLubyte>(_color[1] * 255),
                static_cast<GLubyte>(_color[2] * 255)
            });
            rgbaNode->setOpacity(static_cast<GLubyte>(_color[3] * 255));
        }
    }
    
    if (auto labelNode = typeinfo_cast<CCLabelProtocol*>(node)) {
        std::string str = labelNode->getString();
        if (ImGui::Button(U8STR(FEATHER_COPY"##copylabeltext"))) clipboard::write(fmt::format("{}", str)); ImGui::SameLine();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("\"{}\"<={str}");
        if (ImGui::InputText("Text", &str, 256)) {
            labelNode->setString(str.c_str());
        }
    }

    if (auto textureProtocol = typeinfo_cast<CCTextureProtocol*>(node)) {
        if (auto texture = textureProtocol->getTexture()) {
            auto* cachedTextures = CCTextureCache::sharedTextureCache()->m_pTextures;
            for (auto [key, obj] : CCDictionaryExt<std::string, CCTexture2D*>(cachedTextures)) {
                if (obj == texture) {
                    ImGui::TextWrapped("Texture name: %s", key.c_str());
                    break;
                }
            }

            if (auto spriteNode = typeinfo_cast<CCSprite*>(node)) {
                auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
                const auto rect = spriteNode->getTextureRect();
                for (auto [key, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
                    if (frame->getTexture() == texture && frame->getRect() == rect) {
                        ImGui::Text("Frame name: %s", key.c_str());
                        ImGui::SameLine();
                        if (ImGui::Button(U8STR(FEATHER_COPY " Copy##copysprframename"))) {
                            clipboard::write(key);
                        }
                        break;
                    }
                }
            }

        }
    }

    if (auto rawOpts = node->getLayoutOptions()) {
        ImGui::Text("Layout options: %s", typeid(*rawOpts).name());

        if (ImGui::Button(U8STR(FEATHER_REFRESH_CW " Update Parent Layout"))) {
            if (auto parent = node->getParent()) {
                parent->updateLayout();
            }
        }
        if (auto opts = typeinfo_cast<AxisLayoutOptions*>(rawOpts)) {
            bool updateLayout = false;

            ImGui::Text("Auto Scale");
            auto updateAxis = false;
            int autoScale = opts->getAutoScale() ? opts->getAutoScale().value() + 1 : 0;
            updateAxis |= ImGui::RadioButton("Default", &autoScale, 0);
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Enable", &autoScale, 1);
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Disable", &autoScale, 2);
            if (updateAxis) {
                switch (autoScale) {
                    case 0: opts->setAutoScale(std::nullopt); break;
                    case 1: opts->setAutoScale(true); break;
                    case 2: opts->setAutoScale(false); break;
                }
                updateLayout = true;
            }

            if (checkbox("Break Line", opts, AXIS_GET(BreakLine))) {
                updateLayout = true;
            }
            if (checkbox("Same Line", opts, AXIS_GET(SameLine))) {
                updateLayout = true;
            }

            auto prio = opts->getScalePriority();
            if (ImGui::DragInt("Scale Priority", &prio, .03f)) {
                opts->setScalePriority(prio);
                updateLayout = true;
            }

            if (updateLayout && node->getParent()) {
                node->getParent()->updateLayout();
            }
        }
        else if (auto opts = typeinfo_cast<AnchorLayoutOptions*>(rawOpts)) {
            bool updateLayout = false;

            auto offset = opts->getOffset();
            ImGui::DragFloat2("Offset", &offset.x);
            if (opts->getOffset() != offset) {
                opts->setOffset(offset);
                updateLayout = true;
            }

            auto anchor = static_cast<int>(opts->getAnchor());
            auto updateAnchor = false;
            ImGui::BeginTable("anchor-table", 3);
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top Left", &anchor, static_cast<int>(Anchor::TopLeft));
            updateAnchor |= ImGui::RadioButton("Left", &anchor, static_cast<int>(Anchor::Left));
            updateAnchor |= ImGui::RadioButton("Bottom Left", &anchor, static_cast<int>(Anchor::BottomLeft));
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top", &anchor, static_cast<int>(Anchor::Top));
            updateAnchor |= ImGui::RadioButton("Center", &anchor, static_cast<int>(Anchor::Center));
            updateAnchor |= ImGui::RadioButton("Bottom", &anchor, static_cast<int>(Anchor::Bottom));
            ImGui::TableNextColumn();
            updateAnchor |= ImGui::RadioButton("Top Right", &anchor, static_cast<int>(Anchor::TopRight));
            updateAnchor |= ImGui::RadioButton("Right", &anchor, static_cast<int>(Anchor::Right));
            updateAnchor |= ImGui::RadioButton("Bottom Right", &anchor, static_cast<int>(Anchor::BottomRight));
            ImGui::EndTable();

            if (updateAnchor) {
                if (opts->getAnchor() != static_cast<Anchor>(anchor)) {
                    opts->setAnchor(static_cast<Anchor>(anchor));
                    updateLayout = true;
                }
            }

            if (updateLayout && node->getParent()) {
                node->getParent()->updateLayout();
            }
        }
    }
    else {
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AxisLayoutOptions"))) {
            node->setLayoutOptions(AxisLayoutOptions::create());
        }
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AnchorLayoutOptions"))) {
            node->setLayoutOptions(AnchorLayoutOptions::create());
        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    if (auto delegate = typeinfo_cast<CCTouchDelegate*>(node)) {
        if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
            auto priority = handler->getPriority();

            if (ImGui::DragInt("Touch Priority", &priority, .03f)) {
                CCTouchDispatcher::get()->setPriority(priority, handler->getDelegate());
            }
        }
    }
    

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    if (auto rawLayout = node->getLayout()) {
        ImGui::Text("Layout: %s", typeid(*rawLayout).name());
        
        if (ImGui::Button(U8STR(FEATHER_REFRESH_CW " Update Layout"))) {
            node->updateLayout();
        }
        ImGui::SameLine();
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add Test Child"))) {
            auto spr = CCSprite::create("GJ_button_01.png");
            auto btn = CCMenuItemSpriteExtra::create(spr, node, nullptr);
            node->addChild(btn);
            node->updateLayout();
        }
        if (auto layout = typeinfo_cast<AxisLayout*>(rawLayout)) {
            bool updateLayout = false;

            auto axis = static_cast<int>(layout->getAxis());
            ImGui::Text("Axis");
            auto updateAxis = false;
            updateAxis |= ImGui::RadioButton("Row",    &axis, static_cast<int>(Axis::Row));
            ImGui::SameLine();
            updateAxis |= ImGui::RadioButton("Column", &axis, static_cast<int>(Axis::Column));
            if (updateAxis) {
                if (layout->getAxis() != static_cast<Axis>(axis)) {
                    node->setContentSize({
                        node->getContentSize().height,
                        node->getContentSize().width
                    });
                }
                layout->setAxis(static_cast<Axis>(axis));
                updateLayout = true;
            }

            auto axisReverse = layout->getAxisReverse();
            if (ImGui::Checkbox("Flip Axis Direction", &axisReverse)) {
                layout->setAxisReverse(axisReverse);
                updateLayout = true;
            }
            axisReverse = layout->getCrossAxisReverse();
            if (ImGui::Checkbox("Flip Cross Axis Direction", &axisReverse)) {
                layout->setCrossAxisReverse(axisReverse);
                updateLayout = true;
            }

            {
                auto align = static_cast<int>(layout->getAxisAlignment());
                ImGui::Text("Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setAxisAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            {
                auto align = static_cast<int>(layout->getCrossAxisAlignment());
                ImGui::Text("Cross Axis Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start##cross0", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center##cross1", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End##cross2", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even##cross3", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between##cross4", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setCrossAxisAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            {
                auto align = static_cast<int>(layout->getCrossAxisLineAlignment());
                ImGui::Text("Cross Axis Line Alignment");
                bool updateAlign = false;
                updateAlign |= ImGui::RadioButton(
                    "Start##crossline0", &align, static_cast<int>(AxisAlignment::Start)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Center##crossline1", &align, static_cast<int>(AxisAlignment::Center)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "End##crossline2", &align, static_cast<int>(AxisAlignment::End)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Even##crossline3", &align, static_cast<int>(AxisAlignment::Even)
                );
                ImGui::SameLine();
                updateAlign |= ImGui::RadioButton(
                    "Between##crossline4", &align, static_cast<int>(AxisAlignment::Between)
                );
                if (updateAlign) {
                    layout->setCrossAxisLineAlignment(static_cast<AxisAlignment>(align));
                    updateLayout = true;
                }
            }

            auto gap = layout->getGap();
            if (ImGui::DragFloat("Gap", &gap)) {
                layout->setGap(gap);
                updateLayout = true;
            }

            auto autoScale = layout->getAutoScale();
            if (ImGui::Checkbox("Auto Scale", &autoScale)) {
                layout->setAutoScale(autoScale);
                updateLayout = true;
            }

            auto grow = layout->getGrowCrossAxis();
            if (ImGui::Checkbox("Grow Cross Axis", &grow)) {
                layout->setGrowCrossAxis(grow);
                updateLayout = true;
            }

            auto overflow = layout->getCrossAxisOverflow();
            if (ImGui::Checkbox("Allow Cross Axis Overflow", &overflow)) {
                layout->setCrossAxisOverflow(overflow);
                updateLayout = true;
            }

            if (updateLayout) {
                node->updateLayout();
            }
        }
    }
    else {
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AxisLayout"))) {
            node->setLayout(AxisLayout::create());
        }
        if (ImGui::Button(U8STR(FEATHER_PLUS " Add AnchorLayout"))) {
            node->setLayout(AnchorLayout::create());
        }
    }
}

void DevTools::drawAttributes() {
    if (!m_selectedNode) {
        ImGui::TextWrapped("Select a Node to Edit in the Scene or Tree");
    } else {
        this->drawNodeAttributes(m_selectedNode);
    }
}
