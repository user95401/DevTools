
#include "platform/platform.hpp"
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/AchievementNotifier.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCNode.hpp>
#include "DevTools.hpp"
#include <imgui.h>
#include "ImGui.hpp"

using namespace geode::prelude;

class $modify(CCNode) {
    void sortAllChildren() override {
        if (DevTools::get()->shouldOrderChildren()) {
            CCNode::sortAllChildren();
        }
    }
};

// todo: use shortcuts api once Geode has those
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr) {
        if (down && (key == KEY_F11 GEODE_MACOS(|| key == KEY_F10))) {
            DevTools::get()->toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
    }
};

#ifdef GEODE_IS_MOBILE
// lol
#include <Geode/modify/MenuLayer.hpp>
class $modify(MenuLayer) {
    void onMoreGames(CCObject*) {
        DevTools::get()->toggle();
    }
};

#endif

class $modify(CCDirector) {
    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        DevTools::get()->sceneChanged();
    }

    void drawScene() {
        if (!DevTools::get()->shouldUseGDWindow()) {
            return CCDirector::drawScene();
        }
        
        DevTools::get()->setup();

        static GLRenderCtx* gdTexture = nullptr;

        if (!DevTools::get()->shouldPopGame()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldPassEventsToGDButTransformed() = false;
            CCDirector::drawScene();
            return;
        }

        if (shouldUpdateGDRenderBuffer()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldUpdateGDRenderBuffer() = false;
        }

        auto winSize = this->getOpenGLView()->getViewPortRect() * geode::utils::getDisplayFactor();
        if (!gdTexture) {
            gdTexture = new GLRenderCtx({ winSize.size.width, winSize.size.height });
        }

        if (!gdTexture->begin()) {
            delete gdTexture;
            gdTexture = nullptr;
            CCDirector::drawScene();
            DevTools::get()->render(nullptr);
            return;
        }
        CCDirector::drawScene();
        gdTexture->end();

        DevTools::get()->render(gdTexture);

        // if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     auto backup_current_context = this->getOpenGLView()->getWindow();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }
    }
};

class $modify(CCEGLView) {
    // this is needed for popout mode because we need to render after gd has rendered,
    // but before the buffers have been swapped, which is not possible with just a
    // CCDirector::drawScene hook.
    void swapBuffers() {
        if (!DevTools::get()->shouldUseGDWindow() || !DevTools::get()->shouldPopGame()) {
            DevTools::get()->setup();
            DevTools::get()->render(nullptr);
        }
        CCEGLView::swapBuffers();
    }
};


// Add toggle button to mod info popup of DevTools
#include <Geode/ui/GeodeUI.hpp>
static auto g_ModPopupEventCallback = +[](ModPopupUIEvent* event)
    {
        if (event->getMod() == getMod()) { //Shared ptr compare if popup created for DevTools (this mod)

            if (auto item = typeinfo_cast<CCMenuItemSpriteExtra*>(event->getPopup()->querySelector("links-container > support"))) {

                // Move support button to line start
                item->setZOrder(-1);
                if (auto menu = item->getParent()) menu->updateLayout();

                // Assingn custom button callback with toggle call
                CCMenuItemExt::assignCallback<CCMenuItem>(
                    item, [](auto) {
                        DevTools::get()->toggle();
                    }
                );
                item->setEnabled(true); // Enable button anyways.

                // Update button icon
                if (auto image = item->getChildByType<CCSprite>(-1)) {
                    // If button added by loader as disabled
                    image->setOpacity(255);
                    image->setColor(ccWHITE);

                    // Frame updater up to actual visibility state of DevTools
                    image->runAction(CCRepeatForever::create(CCSequence::create(CallFuncExt::create(
                        [image = Ref(image), item = Ref(item)] {
                            auto GJ_checkOn_001 = CCSpriteFrameCache::get()->spriteFrameByName("GJ_checkOn_001.png");
                            auto GJ_checkOff_001 = CCSpriteFrameCache::get()->spriteFrameByName("GJ_checkOff_001.png");
                            image->setDisplayFrame(DevTools::get()->isVisible() ? GJ_checkOn_001 : GJ_checkOff_001);
                            cocos::limitNodeSize(image, item->getContentSize(), 123.f, 0.f);
                        }
                    ), CCDelayTime::create(0.01f), nullptr)));
                };

            }

        }
        return ListenerResult::Propagate;
    };
$execute{ new EventListener<EventFilter<ModPopupUIEvent>>(g_ModPopupEventCallback); }