#include "PEMenuDraw.h"

#include <algorithm>
#include <cmath>

#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/ModelBiped.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/RenderHelper.h"
#include "net/minecraft/src/Tessellator.h"
#include "platform/PlatformConfig.h"
#include "platform/RenderAPI.h"

namespace PEDraw
{
namespace
{
void setPerspective(float fovY, float aspect, float nearPlane, float farPlane)
{
    const double radians = static_cast<double>(fovY) * 3.14159265358979323846 / 360.0;
    const double top = static_cast<double>(nearPlane) * std::tan(radians);
    renderFrustum(-top * aspect, top * aspect, -top, top, nearPlane, farPlane);
}

void quad(Tessellator *t, float x0, float y0, float x1, float y1, float z, float u0, float v0, float u1, float v1)
{
    t->addVertexWithUV(x0, y1, z, u0, v1);
    t->addVertexWithUV(x1, y1, z, u1, v1);
    t->addVertexWithUV(x1, y0, z, u1, v0);
    t->addVertexWithUV(x0, y0, z, u0, v0);
}
}

std::string assetPath(Minecraft *, const std::string &name, bool hasSmallVariant)
{
#if PLATFORM_3DS || PLATFORM_PS2 || PLATFORM_WII
    if (hasSmallVariant)
        return "/pe/small/" + name;
#else
    (void)hasSmallVariant;
#endif
    return "/pe/" + name;
}

bool hasAsset(Minecraft *mc, const std::string &path)
{
    return mc != nullptr && mc->renderEngine != nullptr && mc->renderEngine->hasResource(path);
}

Tex loadTexture(Minecraft *mc, const std::string &path)
{
    Tex t;
    if (!hasAsset(mc, path))
        return t;
    const int_t id = mc->renderEngine->getTexture(path);
    int_t w = 0, h = 0;
    if (id < 0 || !mc->renderEngine->getTextureDimensions(id, &w, &h) || w <= 0 || h <= 0)
        return t;
    t.id = id;
    t.w = w;
    t.h = h;
    return t;
}

void bindUi(const Tex &t, bool smooth)
{
    renderBindTexture(t.id);
    renderTextureParameters(smooth, false, true);
}

void drawImage(const Tex &t, float x, float y, float w, float h, float z, float r, float g, float b, float a,
    bool smooth)
{
    if (!t.valid() || w <= 0.0f || h <= 0.0f)
        return;
    bindUi(t, smooth);
    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorRGBA_F(r, g, b, a);
    quad(tess, x, y, x + w, y + h, z, 0.0f, 0.0f, 1.0f, 1.0f);
    tess->draw();
}

void drawNineSlice(const Tex &t, int_t slice, float x, float y, float w, float h, float z,
    float r, float g, float b, float a)
{
    if (!t.valid() || w <= 0.0f || h <= 0.0f)
        return;
    bindUi(t, false);

    const float geoX = std::min(static_cast<float>(slice), w * 0.5f);
    const float geoY = std::min(static_cast<float>(slice), h * 0.5f);
    const float xs[4] = {x, x + geoX, x + w - geoX, x + w};
    const float ys[4] = {y, y + geoY, y + h - geoY, y + h};
    const float su = static_cast<float>(slice) / static_cast<float>(t.w);
    const float sv = static_cast<float>(slice) / static_cast<float>(t.h);
    const float us[4] = {0.0f, su, 1.0f - su, 1.0f};
    const float vs[4] = {0.0f, sv, 1.0f - sv, 1.0f};

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorRGBA_F(r, g, b, a);
    for (int_t j = 0; j < 3; ++j)
        for (int_t i = 0; i < 3; ++i)
            quad(tess, xs[i], ys[j], xs[i + 1], ys[j + 1], z, us[i], vs[j], us[i + 1], vs[j + 1]);
    tess->draw();
}

bool drawPanorama(Minecraft *mc, const std::string textures[6], float yawDegrees, float pitchDegrees,
    float fovDegrees, float aspectOverride)
{
    if (mc == nullptr || mc->renderEngine == nullptr)
        return false;
    Tex faces[6];
    for (int_t i = 0; i < 6; ++i)
    {
        faces[i] = loadTexture(mc, textures[i]);
        if (!faces[i].valid())
            return false;
    }

    const float aspect = aspectOverride > 0.0f ? aspectOverride : (mc->displayHeight > 0
        ? static_cast<float>(mc->displayWidth) / static_cast<float>(mc->displayHeight) : 1.0f);

    renderMatrixMode(RenderMatrixMode::Projection);
    renderPushMatrix();
    renderLoadIdentity();
    setPerspective(fovDegrees, aspect, 0.05f, 10.0f);
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderPushMatrix();
    renderLoadIdentity();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderRotate(180.0f, 1.0f, 0.0f, 0.0f);
    renderDisable(RenderCapability::Blend);
    renderDisable(RenderCapability::AlphaTest);
    renderDisable(RenderCapability::CullFace);
    renderDisable(RenderCapability::DepthTest);
    renderDepthMask(false);

    renderRotate(pitchDegrees, 1.0f, 0.0f, 0.0f);
    renderRotate(-yawDegrees, 0.0f, 1.0f, 0.0f);

    Tessellator *tess = &Tessellator::instance;
    for (int_t face = 0; face < 6; ++face)
    {
        renderPushMatrix();
        if (face == 1) renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
        if (face == 2) renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
        if (face == 3) renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
        if (face == 4) renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
        if (face == 5) renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);

        bindUi(faces[face], true);
        // One texel of overlap hides the seam between neighbouring faces.
        const float e = 1.0f / static_cast<float>(std::max<int_t>(faces[face].w, 2));
        tess->startDrawingQuads();
        tess->setColorRGBA_F(1.0f, 1.0f, 1.0f, 1.0f);
        tess->addVertexWithUV(-1.0, -1.0, 1.0, e, e);
        tess->addVertexWithUV(1.0, -1.0, 1.0, 1.0f - e, e);
        tess->addVertexWithUV(1.0, 1.0, 1.0, 1.0f - e, 1.0f - e);
        tess->addVertexWithUV(-1.0, 1.0, 1.0, e, 1.0f - e);
        tess->draw();
        renderPopMatrix();
    }

    renderMatrixMode(RenderMatrixMode::Projection);
    renderPopMatrix();
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderPopMatrix();
    renderDepthMask(true);
    renderEnable(RenderCapability::CullFace);
    renderEnable(RenderCapability::AlphaTest);
    return true;
}

void drawDoll(Minecraft *mc, ModelBiped &model, const std::string &skinTexture,
    float centerX, float feetY, float characterHeight, float yawDegrees)
{
    if (mc == nullptr || mc->renderEngine == nullptr || characterHeight <= 0.0f)
        return;

    // The model is 32 texels tall = 2.0 world units (24 * 0.0625 below the pivot + 8 above).
    const float s = characterHeight / 2.0f;

    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::RescaleNormal);
    renderEnable(RenderCapability::ColorMaterial);
    // The 2D GUI pass runs with the depth test off. The doll needs it to sort its own
    // limbs, so clear the depth buffer for it and switch it back off afterwards.
    renderClear(RenderClearMask::Depth);
    renderEnable(RenderCapability::DepthTest);
    renderEnable(RenderCapability::CullFace);

    renderPushMatrix();
    renderTranslate(centerX, feetY, 50.0f);
    renderScale(-s, s, s);
    renderRotate(180.0f, 0.0f, 0.0f, 1.0f);
    renderRotate(135.0f, 0.0f, 1.0f, 0.0f);
    RenderHelper::enableStandardItemLighting();
    renderRotate(-135.0f, 0.0f, 1.0f, 0.0f);

    // Same chain RenderLiving uses for an entity facing the camera.
    renderRotate(180.0f - yawDegrees, 0.0f, 1.0f, 0.0f);
    renderScale(-1.0f, -1.0f, 1.0f);
    renderTranslate(0.0f, -24.0f * 0.0625f - 0.0078125f, 0.0f);

    mc->renderEngine->bindTexture(mc->renderEngine->getTexture(skinTexture));
    model.render(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);

    renderPopMatrix();
    RenderHelper::disableStandardItemLighting();
    renderDisable(RenderCapability::RescaleNormal);
    renderDisable(RenderCapability::ColorMaterial);
    renderDisable(RenderCapability::DepthTest);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
}
