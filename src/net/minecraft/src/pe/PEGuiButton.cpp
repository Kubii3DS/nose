#include "PEGuiButton.h"

#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/Tessellator.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/RenderAPI.h"

namespace
{
// $light_button_*_text_color / $light_border_*_color of _global_variables.json
constexpr float TEXT_IDLE = 0.3f;
constexpr float TEXT_ACTIVE = 1.0f;
constexpr float TEXT_LOCKED = 0.7f;
constexpr float BORDER_IDLE = 0.075f;
constexpr float BORDER_ACTIVE = 1.0f;

int_t packRgb(float v)
{
    const int_t c = static_cast<int_t>(v * 255.0f + 0.5f);
    return (c << 16) | (c << 8) | c;
}
}

PEGuiButton::PEGuiButton(int_t id_, const PERect &r, const std::string &text, const PEAssets *assets_)
    : GuiButton(id_, r.x, r.y, r.w, r.h, text)
    , assets(assets_)
    , kind(Kind::Text)
    , glyph(nullptr)
    , glyphOffsetX(0)
    , glyphOffsetY(0)
{
}

PEGuiButton::PEGuiButton(int_t id_, const PERect &r, const PEAssets *assets_, const PEDraw::Tex *glyph_,
    int_t offX, int_t offY)
    : GuiButton(id_, r.x, r.y, r.w, r.h, "")
    , assets(assets_)
    , kind(Kind::Glyph)
    , glyph(glyph_)
    , glyphOffsetX(offX)
    , glyphOffsetY(offY)
{
}

bool PEGuiButton::bottomPassActive = false;

void PEGuiButton::drawButton(Minecraft *mc, int_t mouseX, int_t mouseY)
{
    if (!enabled2 || assets == nullptr)
        return;
    if (bottomOnly != bottomPassActive)
        return;
    const int_t bx = bottomOnly ? drawRect.x : xPosition;
    const int_t by = bottomOnly ? drawRect.y : yPosition;
    const int_t bw = bottomOnly ? drawRect.w : width;
    const int_t bh = bottomOnly ? drawRect.h : height;

    const bool inside = mouseX >= xPosition && mouseY >= yPosition &&
        mouseX < xPosition + width && mouseY < yPosition + height;
    const bool hovered = enabled && (keyboardSelected || inside);
    const bool pressed = hovered && inside && lwjgl::Mouse::isButtonDown(0);

    const PEDraw::Tex *face = &assets->button;
    float textColor = TEXT_IDLE;
    float border = BORDER_IDLE;
    if (!enabled)
    {
        face = &assets->buttonLocked;
        textColor = TEXT_LOCKED;
    }
    else if (pressed)
    {
        face = &assets->buttonPressed;
        textColor = TEXT_ACTIVE;
        border = BORDER_ACTIVE;
    }
    else if (hovered)
    {
        face = &assets->buttonHover;
        textColor = TEXT_ACTIVE;
        border = BORDER_ACTIVE;
    }

    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

    // "button_image" is "100% - 2px" so the border frame sits on the outermost pixel.
    PEDraw::drawNineSlice(*face, 1, static_cast<float>(bx + 1), static_cast<float>(by + 1),
        static_cast<float>(bw - 2), static_cast<float>(bh - 2), zLevel, 1.0f, 1.0f, 1.0f, 1.0f);
    PEDraw::drawNineSlice(assets->border, 1, static_cast<float>(bx), static_cast<float>(by),
        static_cast<float>(bw), static_cast<float>(bh), zLevel, border, border, border, 1.0f);

    const int_t press = pressed ? 1 : 0;
    if (kind == Kind::Glyph)
    {
        if (glyph != nullptr && glyph->valid())
        {
            PEDraw::drawImage(*glyph, static_cast<float>(bx + glyphOffsetX),
                static_cast<float>(by + glyphOffsetY + press), static_cast<float>(glyph->w),
                static_cast<float>(glyph->h), zLevel, textColor, textColor, textColor, 1.0f);
        }
    }
    else if (mc != nullptr && mc->fontRenderer != nullptr)
    {
        FontRenderer *font = mc->fontRenderer;
        const std::string label = font->trimStringToWidth(displayString, bw - 8);
        const int_t textX = bx + (bw - font->getStringWidth(label)) / 2;
        const int_t textY = by + (bh - 8) / 2 + press;
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        font->drawString(label, textX, textY, 0xff000000 | packRgb(textColor));
    }
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
