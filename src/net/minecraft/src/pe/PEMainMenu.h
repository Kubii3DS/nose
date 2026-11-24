#pragma once

#include <memory>
#include <string>
#include <vector>

#include "PEAssets.h"
#include "PEMenuLayout.h"

class GuiButton;
class PEGuiButton;
class Minecraft;
class ModelBiped;

// Button ids understood by GuiMainMenu::actionPerformed (same ids the classic menu uses,
// so every button opens the screen it already opened before).
namespace PEMenuId
{
constexpr int_t Options = 0;
constexpr int_t Play = 1;
constexpr int_t Multiplayer = 2;
constexpr int_t Mods = 3;
constexpr int_t Quit = 4;
constexpr int_t Language = 5;
constexpr int_t Skins = 6;
}

// Pocket-Edition (MCPE 1.12) main menu: panorama + overlay, game logo with splash, stacked
// "light" buttons, the mini player on the right and the copyright / version line.
//
// GuiMainMenu owns one of these when the PE assets are installed (assets/pe/) and the user
// is not on the console-style "legacy UI". Buttons live in the screen's control list, so
// keyboard / pad / touch navigation keep working through GuiScreen unchanged.
class PEMainMenu
{
public:
    PEMainMenu();
    ~PEMainMenu();

    // True when the textures needed by the PE menu are present.
    static bool isAvailable(Minecraft *mc);

    // Loads textures and creates the buttons. multiplayerOut receives the Multiplayer button.
    bool init(Minecraft *mc, int_t width, int_t height, std::vector<GuiButton *> &controls,
        GuiButton *&multiplayerOut);

    // Whole scene except the buttons (GuiScreen::drawScreen draws those afterwards).
    void drawScene(Minecraft *mc, int_t mouseX, int_t mouseY, float partialTick, int_t panoramaTimer,
        const std::string &splashText);

    const PEMenuLayout &layout() const { return menuLayout; }

private:
    void drawBackground(Minecraft *mc, int_t panoramaTimer, float partialTick);
    void drawLogoAndSplash(Minecraft *mc, const std::string &splashText);
    void drawSkinPanel(Minecraft *mc, int_t mouseX, int_t mouseY);
    void drawTextBar(Minecraft *mc);
    // 3DS only: panorama + buttons on the bottom LCD.
    void drawBottomScreen(Minecraft *mc, int_t mouseX, int_t mouseY, float partialTick, int_t panoramaTimer);

    PEAssets assets;
    PEMenuLayout menuLayout;
    int_t guiWidth;
    int_t guiHeight;
    bool loaded;

    std::vector<PEGuiButton *> bottomControls; // owned by the screen
    std::unique_ptr<ModelBiped> doll;
    float dollYaw;
    bool dollDragging;
    int_t dragLastX;
    bool mouseWasDown;
};
