#pragma once

#include "net/minecraft/src/GuiButton.h"
#include "PEAssets.h"
#include "PEMenuLayout.h"

// Button drawn like the MCPE "light" buttons: a nine-slice face (grey -> green on hover ->
// dark green when pressed -> flat grey when locked) inside a 1px focus border that is dark
// when idle and white when hovered/pressed. Text is not shadowed; idle text is dark grey,
// hovered/pressed text white, locked text light grey. Pressing moves the content 1px down.
class PEGuiButton : public GuiButton
{
public:
    enum class Kind
    {
        Text,
        Glyph
    };

    PEGuiButton(int_t id, const PERect &rect, const std::string &text, const PEAssets *assets);
    // Glyph button (PE "light_glyph_button"): the glyph is a texture drawn at its native size.
    PEGuiButton(int_t id, const PERect &rect, const PEAssets *assets, const PEDraw::Tex *glyph,
        int_t glyphOffsetX, int_t glyphOffsetY);

    void drawButton(Minecraft *mc, int_t mouseX, int_t mouseY) override;

    // 3DS: the button is hit-tested at its top-screen (touch) position but drawn on the bottom
    // screen at `rect`, and only while the bottom pass is running.
    void setBottomScreen(const PERect &rect)
    {
        drawRect = rect;
        bottomOnly = true;
    }
    static bool bottomPassActive;

private:
    const PEAssets *assets;
    Kind kind;
    const PEDraw::Tex *glyph;
    int_t glyphOffsetX;
    int_t glyphOffsetY;
    PERect drawRect;
    bool bottomOnly = false;
};
