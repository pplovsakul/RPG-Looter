#pragma once

#include "UIContext.h"

namespace EngineUI {

// ============================================================================
// Basic Widgets
// ============================================================================

void Text(const char* text);
void TextColored(const Color& color, const char* text);

bool Button(const char* label, float width = 0.0f, float height = 0.0f);
bool Checkbox(const char* label, bool* v);
bool RadioButton(const char* label, bool active);

bool SliderFloat(const char* label, float* v, float v_min, float v_max);
bool SliderInt(const char* label, int* v, int v_min, int v_max);

bool DragFloat(const char* label, float* v, float speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
bool DragInt(const char* label, int* v, float speed = 1.0f, int v_min = 0, int v_max = 0);

bool InputText(const char* label, char* buf, size_t buf_size);

bool ColorEdit3(const char* label, float col[3]);
bool ColorEdit4(const char* label, float col[4]);

// Layout
void Separator();
void SameLine(float offset = 0.0f);
void Spacing();
void Indent(float amount = 0.0f);
void Unindent(float amount = 0.0f);

// Containers
bool CollapsingHeader(const char* label);
bool TreeNode(const char* label);
void TreePop();

bool BeginChild(const char* str_id, float width, float height);
void EndChild();

// Progress
void ProgressBar(float fraction, float width = -1.0f, const char* overlay = nullptr);

// Combo
bool BeginCombo(const char* label, const char* preview);
void EndCombo();
bool Combo(const char* label, int* current, const char* const* items, int count);

// Helpers
void HelpMarker(const char* desc);

} // namespace EngineUI
