#pragma once

#include <string>

#include "java/Type.h"

class Minecraft;
class ModelBiped;

// Drawing helpers for the Pocket-Edition style main menu. Everything goes through the
// platform render API / Tessellator, so the same code runs on PC, 3DS, Wii and PS2.
namespace PEDraw
{
struct Tex
{
    int_t id = -1;
    int_t w = 0;
    int_t h = 0;
    bool valid() const { return id >= 0 && w > 0 && h > 0; }
};

// "/pe/<name>" on desktop, "/pe/small/<name>" when a console-sized variant exists and the
// platform has little texture memory (3DS: 1024 max, 256 MB RAM; PS2/Wii similar).
std::string assetPath(Minecraft *mc, const std::string &name, bool hasSmallVariant);
bool hasAsset(Minecraft *mc, const std::string &path);
Tex loadTexture(Minecraft *mc, const std::string &path);

// Crisp UI sampling (nearest, no mipmaps, clamped) or smooth (bilinear, clamped).
void bindUi(const Tex &t, bool smooth = false);

void drawImage(const Tex &t, float x, float y, float w, float h, float z,
    float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f, bool smooth = false);
// slice = width of the fixed border in texels (PE "nineslice_size").
void drawNineSlice(const Tex &t, int_t slice, float x, float y, float w, float h, float z,
    float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

// Six-face cubemap background. yaw/pitch in degrees.
bool drawPanorama(Minecraft *mc, const std::string textures[6], float yawDegrees, float pitchDegrees,
    float fovDegrees, float aspectOverride = 0.0f);

// Mini player ("paper doll"): feet at (centerX, feetY), characterHeight in GUI pixels.
void drawDoll(Minecraft *mc, ModelBiped &model, const std::string &skinTexture,
    float centerX, float feetY, float characterHeight, float yawDegrees);
}
