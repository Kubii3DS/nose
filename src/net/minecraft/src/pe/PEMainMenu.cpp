#include "PEMainMenu.h"

#include <algorithm>
#include <cmath>

#include "PEGuiButton.h"
#include "PEMenuDraw.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/Gui.h"
#include "net/minecraft/src/MathHelper.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/ModelBiped.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/Session.h"
#include "net/minecraft/src/StringTranslate.h"
#include "net/minecraft/src/UiStrings.h"
#include "net/minecraft/src/skin/SkinManager.h"
#include "java/System.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/PlatformConfig.h"
#include "platform/RenderAPI.h"

namespace
{
// ---- Tunables. PE draws the panorama from native code, so these cannot be read from the
// ---- resource pack JSON; they are calibrated by eye against the real menu.
constexpr float PANORAMA_FOV = 80.0f;          // vertical field of view, degrees
constexpr float PANORAMA_SPIN_PER_TICK = 0.10f;  // degrees per game tick (20 Hz)
constexpr float PANORAMA_PITCH = 0.0f;         // degrees, + looks down
constexpr float DOLL_DRAG_DEGREES_PER_PIXEL = 1.5f;

constexpr const char *COPYRIGHT_TEXT = "\xC2\xA9" "OptiJuegos"; // start.copyright in the APK
constexpr const char *VERSION_TEXT = "Minecraft 1.2.5";           // #version

// PE wording for the two buttons whose label differs from the Java menu.
std::string peLabel(const char *key, const char *english, const char *spanish)
{
    StringTranslate *tr = StringTranslate::getInstance();
    const std::string translated = tr->translateKey(key);
    if (translated != key)
        return translated;
    if (tr->getCurrentLanguage().rfind("es_", 0) == 0)
        return spanish;
    return english;
}
}

PEMainMenu::PEMainMenu()
    : guiWidth(0)
    , guiHeight(0)
    , loaded(false)
    , dollYaw(0.0f)
    , dollDragging(false)
    , dragLastX(0)
    , mouseWasDown(false)
{
}

PEMainMenu::~PEMainMenu() = default;

bool PEMainMenu::isAvailable(Minecraft *mc)
{
    return PEAssets::isAvailable(mc);
}

bool PEMainMenu::init(Minecraft *mc, int_t width, int_t height, std::vector<GuiButton *> &controls,
    GuiButton *&multiplayerOut)
{
    guiWidth = width;
    guiHeight = height;
    multiplayerOut = nullptr;
    loaded = assets.load(mc);
    if (!loaded)
        return false;
    if (!doll)
        doll.reset(new ModelBiped());

    StringTranslate *tr = StringTranslate::getInstance();
    struct Row
    {
        int_t id;
        std::string text;
    };
    std::vector<Row> rows;
    rows.push_back({PEMenuId::Play, peLabel("menu.play", "Play", "Jugar")});
    rows.push_back({PEMenuId::Multiplayer, tr->translateKey("menu.multiplayer")});
    rows.push_back({PEMenuId::Mods, uiText("Mods")});
    rows.push_back({PEMenuId::Options, peLabel("menu.settings", "Settings", "Ajustes")});
    if (!mc->hideQuitButton)
        rows.push_back({PEMenuId::Quit, tr->translateKey("menu.quit")});

#if PLATFORM_3DS
    menuLayout = peMenuLayoutDual(static_cast<int_t>(rows.size()), assets.title.w, assets.title.h);
#else
    menuLayout = peMenuLayout(width, height, static_cast<int_t>(rows.size()), assets.title.w, assets.title.h);
#endif
    bottomControls.clear();

    for (std::size_t i = 0; i < rows.size(); ++i)
    {
        PEGuiButton *button = new PEGuiButton(rows[i].id, menuLayout.buttons[i], rows[i].text, &assets);
        controls.push_back(button);
        if (menuLayout.dual)
        {
            button->setBottomScreen(menuLayout.bottomButtons[i]);
            bottomControls.push_back(button);
        }
        if (rows[i].id == PEMenuId::Multiplayer)
        {
            multiplayerOut = button;
#if !PLATFORM_PS2
            if (mc->session == nullptr)
                button->enabled = false;
#endif
        }
    }

    // Change-skin button (hanger glyph): the glyph is 17x12 inside a 32x16 canvas.
    PEGuiButton *skinsButton = new PEGuiButton(PEMenuId::Skins, menuLayout.changeSkin, &assets, &assets.hangar, 4, 6);
    controls.push_back(skinsButton);
    if (menuLayout.dual)
    {
        skinsButton->setBottomScreen(menuLayout.bottomSkin);
        bottomControls.push_back(skinsButton);
    }
    // Language button (the base game's way into the language list).
    if (assets.language.valid())
    {
        const PERect &lb = menuLayout.dual ? menuLayout.bottomLanguage : menuLayout.languageButton;
        PEGuiButton *languageButton = new PEGuiButton(PEMenuId::Language, menuLayout.languageButton, &assets,
            &assets.language, (lb.w - assets.language.w) / 2, (lb.h - assets.language.h) / 2);
        controls.push_back(languageButton);
        if (menuLayout.dual)
        {
            languageButton->setBottomScreen(menuLayout.bottomLanguage);
            bottomControls.push_back(languageButton);
        }
    }
    return true;
}

void PEMainMenu::drawScene(Minecraft *mc, int_t mouseX, int_t mouseY, float partialTick, int_t panoramaTimer,
    const std::string &splashText)
{
    if (!loaded)
        return;
    drawBackground(mc, panoramaTimer, partialTick);
    drawLogoAndSplash(mc, splashText);
    drawSkinPanel(mc, mouseX, mouseY);
    drawTextBar(mc);
#if PLATFORM_3DS
    drawBottomScreen(mc, mouseX, mouseY, partialTick, panoramaTimer);
#endif
}

#if PLATFORM_3DS
void PEMainMenu::drawBottomScreen(Minecraft *mc, int_t mouseX, int_t mouseY, float partialTick, int_t panoramaTimer)
{
    if (!renderKeyboardBottomBegin())
        return;
    const float t = static_cast<float>(panoramaTimer) + partialTick;
    bool drawn = false;
    if (assets.hasPanorama)
        drawn = PEDraw::drawPanorama(mc, assets.panorama, t * PANORAMA_SPIN_PER_TICK, PANORAMA_PITCH, PANORAMA_FOV,
            static_cast<float>(PE_DS_BOTTOM_WIDTH) / static_cast<float>(PE_DS_HEIGHT));
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    if (!drawn)
        Gui::drawRect(0, 0, PE_DS_BOTTOM_WIDTH, PE_DS_HEIGHT, static_cast<int_t>(0xff202830u));
    if (assets.overlay.valid())
        PEDraw::drawImage(assets.overlay, 0.0f, 0.0f, static_cast<float>(PE_DS_BOTTOM_WIDTH),
            static_cast<float>(PE_DS_HEIGHT), 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, true);

    PEGuiButton::bottomPassActive = true;
    for (PEGuiButton *button : bottomControls)
        button->drawButton(mc, mouseX, mouseY);
    PEGuiButton::bottomPassActive = false;
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderKeyboardBottomEnd();
}
#endif

void PEMainMenu::drawBackground(Minecraft *mc, int_t panoramaTimer, float partialTick)
{
    const float t = static_cast<float>(panoramaTimer) + partialTick;
    bool drawn = false;
    if (assets.hasPanorama)
    {
        drawn = PEDraw::drawPanorama(mc, assets.panorama, t * PANORAMA_SPIN_PER_TICK, PANORAMA_PITCH, PANORAMA_FOV);
    }
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    if (!drawn)
    {
        Gui::drawRect(0, 0, guiWidth, guiHeight, static_cast<int_t>(0xff202830u));
    }
    if (assets.overlay.valid())
        PEDraw::drawImage(assets.overlay, 0.0f, 0.0f, static_cast<float>(guiWidth), static_cast<float>(guiHeight),
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, true);
}

void PEMainMenu::drawLogoAndSplash(Minecraft *mc, const std::string &splashText)
{
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    const PERect &logo = menuLayout.logo;
    PEDraw::drawImage(assets.title, static_cast<float>(logo.x), static_cast<float>(logo.y),
        static_cast<float>(logo.w), static_cast<float>(logo.h), 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, true);

    FontRenderer *font = mc->fontRenderer;
    if (font == nullptr || splashText.empty())
        return;

    // Same pulse as the Java splash; PE keeps it yellow and rotated -20 degrees.
    const float pulse = 1.8f - MathHelper::abs(MathHelper::sin(
        (static_cast<float>(System::currentTimeMillis() % 1000LL) / 1000.0f) * 3.1415927f * 2.0f) * 0.1f);
    const float textWidth = static_cast<float>(font->getStringWidth(splashText));
    float scale = pulse * 100.0f / (textWidth + 32.0f);
    // The vanilla number is tuned for a 274 px wide logo: follow the logo when it differs.
    scale *= std::max(0.5f, static_cast<float>(logo.w) / 274.0f);

    float cx = static_cast<float>(menuLayout.splashX);
    const float half = textWidth * scale * 0.5f;
    cx = std::min(cx, static_cast<float>(guiWidth - 4) - half);
    cx = std::max(cx, 4.0f + half);

    renderPushMatrix();
    renderTranslate(cx, static_cast<float>(menuLayout.splashY), 0.0f);
    renderRotate(-20.0f, 0.0f, 0.0f, 1.0f);
    renderScale(scale, scale, scale);
    font->drawStringWithShadow(splashText, -static_cast<int_t>(textWidth) / 2, -8, 0xffff00);
    renderPopMatrix();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void PEMainMenu::drawSkinPanel(Minecraft *mc, int_t mouseX, int_t mouseY)
{
    if (!doll || menuLayout.skinPanel.w < 24)
        return;

    // Drag to turn the doll (PE: "gesture_x" rotation).
    const bool down = lwjgl::Mouse::isButtonDown(0);
    if (down && !mouseWasDown && menuLayout.dollBox.contains(mouseX, mouseY))
    {
        dollDragging = true;
        dragLastX = mouseX;
    }
    if (!down)
        dollDragging = false;
    if (dollDragging)
    {
        dollYaw += static_cast<float>(mouseX - dragLastX) * DOLL_DRAG_DEGREES_PER_PIXEL;
        dragLastX = mouseX;
    }
    mouseWasDown = down;

    // Same texture the in-game player uses, looked up every frame so a skin picked in the skin
    // menu shows up as soon as the menu is back. Steve is only the fallback for "no skin at all".
    std::string skin = SkinManager::getActiveSkinTexture();
    if (skin.empty() || SkinManager::getSkinById(SkinManager::getSelectedSkinId()) == nullptr)
        skin = PEDraw::hasAsset(mc, skin) ? skin : "/pe/steve_32.png";

    const float centerX = static_cast<float>(menuLayout.dollBox.centerX());
    PEDraw::drawDoll(mc, *doll, skin, centerX, static_cast<float>(menuLayout.dollFeetY),
        static_cast<float>(menuLayout.dollHeight), dollYaw);

    // Name tag above the head: white text on the translucent plate used for in-world tags.
    FontRenderer *font = mc->fontRenderer;
    if (font != nullptr && mc->session != nullptr && !mc->session->username.empty())
    {
        const std::string &name = mc->session->username;
        const int_t w = font->getStringWidth(name);
        const int_t x = static_cast<int_t>(centerX) - w / 2;
        const int_t y = menuLayout.nameTagY - 4;
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
        Gui::drawRect(x - 2, y - 2, x + w + 2, y + 9, static_cast<int_t>(0x70000000u));
        font->drawString(name, x, y, 0xffffffff);
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void PEMainMenu::drawTextBar(Minecraft *mc)
{
    FontRenderer *font = mc->fontRenderer;
    if (font == nullptr)
        return;
    renderEnable(RenderCapability::Blend);
    const PERect &bar = menuLayout.textBar;
    const int_t y = bar.y + 1;
    font->drawString(COPYRIGHT_TEXT, bar.x + 1, y, 0xffffffff);
    const std::string version = VERSION_TEXT;
    font->drawString(version, bar.right() - 1 - font->getStringWidth(version), y, 0xffffffff);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
