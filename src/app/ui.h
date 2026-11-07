#pragma once
#include "app.h"

constexpr float kTitleBarHeight = 52.0f;  // unscaled; the area you can drag the window by
constexpr float kTitleButtonsWidth = 104.0f;

void ApplyTheme(float scale);
void DrawUi(App& app);
