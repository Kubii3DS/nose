#include "PEAssets.h"

#include "net/minecraft/src/Minecraft.h"

bool PEAssets::isAvailable(Minecraft *mc)
{
    return PEDraw::hasAsset(mc, PEDraw::assetPath(mc, "title.png", true)) &&
        PEDraw::hasAsset(mc, "/pe/button_light.png") &&
        PEDraw::hasAsset(mc, "/pe/button_lighthover.png");
}

bool PEAssets::load(Minecraft *mc)
{
    title = PEDraw::loadTexture(mc, PEDraw::assetPath(mc, "title.png", true));
    button = PEDraw::loadTexture(mc, "/pe/button_light.png");
    buttonHover = PEDraw::loadTexture(mc, "/pe/button_lighthover.png");
    buttonPressed = PEDraw::loadTexture(mc, "/pe/button_lightpressed.png");
    buttonLocked = PEDraw::loadTexture(mc, "/pe/button_locked.png");
    border = PEDraw::loadTexture(mc, "/pe/focus_border_white.png");
    hangar = PEDraw::loadTexture(mc, "/pe/hangar.png");
    language = PEDraw::loadTexture(mc, "/pe/language.png");
    overlay = PEDraw::loadTexture(mc, "/pe/panorama_overlay.png");

    hasPanorama = true;
    for (int_t i = 0; i < 6; ++i)
    {
        panorama[i] = PEDraw::assetPath(mc, "panorama_" + std::to_string(i) + ".png", true);
        if (!PEDraw::hasAsset(mc, panorama[i]))
            hasPanorama = false;
    }
    return title.valid() && button.valid() && buttonHover.valid();
}
