#pragma once

#include <string>

#include "PEMenuDraw.h"

class Minecraft;

// Textures of the Pocket-Edition main menu, resolved once per screen init.
// Files live in assets/pe/ (and assets/pe/small/ for consoles).
struct PEAssets
{
    PEDraw::Tex button;        // button_borderless_light      (4x4, nine-slice 1)
    PEDraw::Tex buttonHover;   // button_borderless_lighthover (green)
    PEDraw::Tex buttonPressed; // button_borderless_lightpressed
    PEDraw::Tex buttonLocked;  // disabledButtonNoBorder       (2x2, nine-slice 1)
    PEDraw::Tex border;        // focus_border_white           (5x5, nine-slice 1)
    PEDraw::Tex hangar;        // change-skin glyph (32x16)
    PEDraw::Tex language;      // speech bubble + globe glyph (25x13) for the language button
    PEDraw::Tex title;         // game logo
    PEDraw::Tex overlay;       // panorama_overlay (16x128 vertical gradient)
    std::string panorama[6];   // cubemap faces
    bool hasPanorama = false;

    // Returns true when the minimum set (logo + button textures) is present.
    bool load(Minecraft *mc);
    static bool isAvailable(Minecraft *mc);
};
