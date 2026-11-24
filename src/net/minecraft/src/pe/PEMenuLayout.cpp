#include "PEMenuLayout.h"

#include <algorithm>
#include <cmath>

namespace
{
int_t roundi(float v)
{
    return static_cast<int_t>(std::floor(v + 0.5f));
}
}

PEMenuLayout peMenuLayout(int_t screenWidth, int_t screenHeight, int_t rowCount, int_t logoTexW, int_t logoTexH)
{
    PEMenuLayout L;
    const float W = static_cast<float>(std::max<int_t>(screenWidth, 1));
    const float H = static_cast<float>(std::max<int_t>(screenHeight, 1));
    L.screenWidth = screenWidth;
    L.screenHeight = screenHeight;
    L.rowCount = std::max<int_t>(0, std::min<int_t>(rowCount, PE_MENU_MAX_ROWS));

    // ---- main_buttons_stack_panel (vertical stack, whole screen) --------------------------
    //   title_offset_padding  [100%, 5%]  (pocket)  -> 0.05 H
    //   title_panel_content   [100%, 100%c]         -> box of 12.5%x = 0.125 W, drawn 5% H lower
    //   padding_fill_1        fill
    //   main_buttons_panel    [150, 100%c]          -> rowCount * 32
    //   padding_fill_2        fill
    //   padding_3             [100%, 10]
    const float titleBoxH = 0.125f * W;
    const float rowsH = static_cast<float>(L.rowCount * PE_ROW_HEIGHT);
    const float titleStackY = 0.05f * H;
    const float fixed = titleStackY + titleBoxH + rowsH + 10.0f;
    const float fill = std::max(0.0f, (H - fixed) * 0.5f);
    const float rowsTop = titleStackY + titleBoxH + fill;

    // Title image: type "image" keeps its aspect ratio inside the box, centred.
    const float texW = static_cast<float>(std::max<int_t>(logoTexW, 1));
    const float texH = static_cast<float>(std::max<int_t>(logoTexH, 1));
    const float scale = std::min(W / texW, titleBoxH / texH);
    const float logoW = texW * scale;
    const float logoH = texH * scale;
    const float boxTop = titleStackY + 0.05f * H;          // title_panel_pocket offset [0, "5%"]
    L.logo.w = roundi(logoW);
    L.logo.h = roundi(logoH);
    L.logo.x = roundi((W - logoW) * 0.5f);
    L.logo.y = roundi(boxTop + (titleBoxH - logoH) * 0.5f);

    // splash_text: anchored right_middle of the title panel, offset [-106, 8] on a phone.
    // Anchoring to the logo's right end keeps the same spot at every resolution.
    L.splashX = L.logo.right() - 34;
    L.splashY = roundi(boxTop + titleBoxH * 0.5f + 8.0f);

    // With many rows on a short screen the stack would run into the logo: never start above
    // the logo's bottom edge (PE only has 2-3 buttons, this menu can have 5).
    const int_t rowsStart = std::max<int_t>(roundi(rowsTop), L.logo.bottom() + 1);

    const int_t rowsX = roundi((W - PE_ROW_WIDTH) * 0.5f);
    for (int_t i = 0; i < L.rowCount; ++i)
    {
        L.rows[i] = PERect{rowsX, rowsStart + i * PE_ROW_HEIGHT, PE_ROW_WIDTH, PE_ROW_HEIGHT};
        L.buttons[i] = PERect{L.rows[i].x + 1, L.rows[i].y + 1, PE_ROW_WIDTH - 2, PE_ROW_HEIGHT - 2};
    }

    // ---- skin_panel: anchor bottom_right, [50% - 75px, 124], offset [0, -10% - 4px] -------
    const float panelW = std::max(0.0f, W * 0.5f - 75.0f);
    const float panelBottom = H - (0.10f * H + 4.0f);
    L.skinPanel = PERect{roundi(W - panelW), roundi(panelBottom - 124.0f), roundi(panelW), 124};
    L.changeSkin = PERect{L.skinPanel.x + (L.skinPanel.w - 25) / 2, L.skinPanel.bottom() - 25, 25, 25};

    // viewer panel [100%, 100% - 24px] offset -16 -> paper_doll_panel [100%, 100% - 10px]
    //   -> paper_doll [100%, 100%] offset [0, -22]  =>  90px tall box, bottom at panel - 43
    const int_t dollBottom = L.skinPanel.bottom() - 16 - 5 - 22;
    L.dollBox = PERect{L.skinPanel.x, dollBottom - 90, L.skinPanel.w, 90};
    L.dollFeetY = dollBottom;
    // The renderer fits the character inside the box with a little air around it.
    L.dollHeight = roundi(90.0f * 0.78f);
    L.nameTagY = L.dollFeetY - L.dollHeight - 8;

    // ---- online_buttons_panel: bottom_left, [50% - 75px, 32], offset y -10% ----------------
    // stacked_column offset x 5%, feedback column 28x32 holding a 24x24 button.
    const float onlineTop = H - 0.10f * H - 32.0f;
    L.languageButton = PERect{roundi(0.05f * panelW) + 2, roundi(onlineTop) + 4, 36, 24};

    // ---- text_panel: [100% - 2px, 10], bottom_middle, offset y -1 --------------------------
    L.textBar = PERect{1, screenHeight - 10 - 1, screenWidth - 2, 10};
    return L;
}

namespace
{
PERect touchRect(const PERect &b)
{
    // DsInput: touchX * 400 / 320, touchY unchanged.
    const int_t x0 = b.x * PE_DS_TOP_WIDTH / PE_DS_BOTTOM_WIDTH;
    const int_t x1 = (b.x + b.w) * PE_DS_TOP_WIDTH / PE_DS_BOTTOM_WIDTH;
    return PERect{x0, b.y, x1 - x0, b.h};
}
}

PEMenuLayout peMenuLayoutDual(int_t rowCount, int_t logoTexW, int_t logoTexH)
{
    PEMenuLayout L;
    L.dual = true;
    L.screenWidth = PE_DS_TOP_WIDTH;
    L.screenHeight = PE_DS_HEIGHT;
    L.rowCount = std::max<int_t>(0, std::min<int_t>(rowCount, PE_MENU_MAX_ROWS));

    // ---- top screen: logo on the left, mini player on the right ---------------------------
    const int_t logoW = 236;
    const int_t logoH = roundi(static_cast<float>(logoW) * static_cast<float>(std::max<int_t>(logoTexH, 1)) /
        static_cast<float>(std::max<int_t>(logoTexW, 1)));
    L.logo = PERect{14, (PE_DS_HEIGHT - logoH) / 2 - 12, logoW, logoH};
    L.splashX = L.logo.right() - 30;
    L.splashY = L.logo.centerY() + 8;

    L.dollFeetY = 206;
    L.dollHeight = 128;
    L.dollBox = PERect{282, L.dollFeetY - 100, 116, 100};
    L.skinPanel = PERect{282, L.dollFeetY - L.dollHeight, 116, L.dollHeight};
    L.nameTagY = L.dollFeetY - L.dollHeight - 8;
    L.textBar = PERect{1, PE_DS_HEIGHT - 11, PE_DS_TOP_WIDTH - 2, 10};

    // ---- bottom screen: one column of smaller buttons + the square skins button -----------
    const int_t bw = 132;
    const int_t bh = 26;
    const int_t pitch = 30;
    const int_t total = L.rowCount * pitch - (pitch - bh);
    const int_t squareH = 25;
    const int_t block = total + 6 + squareH;
    const int_t top = std::max<int_t>(8, (PE_DS_HEIGHT - block) / 2);
    const int_t bx = (PE_DS_BOTTOM_WIDTH - bw) / 2;
    for (int_t i = 0; i < L.rowCount; ++i)
    {
        L.bottomButtons[i] = PERect{bx, top + i * pitch, bw, bh};
        L.buttons[i] = touchRect(L.bottomButtons[i]);
        L.rows[i] = L.buttons[i];
    }
    L.bottomSkin = PERect{(PE_DS_BOTTOM_WIDTH - 25) / 2, top + total + 6, 25, squareH};
    L.changeSkin = touchRect(L.bottomSkin);
    L.bottomLanguage = PERect{8, PE_DS_HEIGHT - 24 - 8, 36, 24};
    L.languageButton = touchRect(L.bottomLanguage);
    return L;
}
