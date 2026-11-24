#pragma once

#include "java/Type.h"

// Pocket-Edition style main menu layout (MCPE 1.12 "start" screen, $pocket_edition).
//
// Everything here is pure arithmetic on the scaled GUI size: no GL, no assets, so it can
// be unit-tested and shared by every platform. All numbers come from start_screen.json /
// ui_art_assets_common.json of the PE resource pack (see the comments next to each rule).
struct PERect
{
    int_t x = 0;
    int_t y = 0;
    int_t w = 0;
    int_t h = 0;

    bool contains(int_t px, int_t py) const { return px >= x && py >= y && px < x + w && py < y + h; }
    int_t right() const { return x + w; }
    int_t bottom() const { return y + h; }
    int_t centerX() const { return x + w / 2; }
    int_t centerY() const { return y + h / 2; }
};

constexpr int_t PE_MENU_MAX_ROWS = 6;
constexpr int_t PE_ROW_WIDTH = 150;   // main_buttons_panel: [150, 100%c]
constexpr int_t PE_ROW_HEIGHT = 32;   // stacked_row: [100%, 32]

struct PEMenuLayout
{
    int_t screenWidth = 0;
    int_t screenHeight = 0;
    int_t rowCount = 0;

    PERect logo;                      // title image, keep_ratio inside a [100%, 12.5%x] box
    int_t splashX = 0;                // centre of the rotated splash text
    int_t splashY = 0;

    PERect rows[PE_MENU_MAX_ROWS];    // 150x32 cells of the stack panel
    PERect buttons[PE_MENU_MAX_ROWS]; // button inside the cell ("100% - 2px")

    PERect skinPanel;                 // right side, [50% - 75px, 124]
    PERect changeSkin;                // 25x25 hangar button, bottom_middle of the skin panel
    PERect dollBox;                   // rectangle the paper doll is fitted in
    int_t dollFeetY = 0;              // y of the doll's feet
    int_t dollHeight = 0;             // height of the character in GUI pixels
    int_t nameTagY = 0;               // vertical centre of the name tag above the head
    PERect languageButton;            // 36x24 glyph button where PE puts the feedback button
    PERect textBar;                   // copyright (left) / version (right), 10px high

    // Dual screen (3DS): top screen = 400x240 with logo + doll, bottom screen = 320x240 with the
    // buttons. bottom* are in bottom-screen pixels; buttons[]/changeSkin/languageButton hold the
    // same rectangles converted to the top-screen pixels the touch pointer reports (x * 400/320).
    bool dual = false;
    PERect bottomButtons[PE_MENU_MAX_ROWS];
    PERect bottomSkin;
    PERect bottomLanguage;
};

constexpr int_t PE_DS_TOP_WIDTH = 400;
constexpr int_t PE_DS_BOTTOM_WIDTH = 320;
constexpr int_t PE_DS_HEIGHT = 240;

PEMenuLayout peMenuLayoutDual(int_t rowCount, int_t logoTexW, int_t logoTexH);

PEMenuLayout peMenuLayout(int_t screenWidth, int_t screenHeight, int_t rowCount, int_t logoTexW, int_t logoTexH);
