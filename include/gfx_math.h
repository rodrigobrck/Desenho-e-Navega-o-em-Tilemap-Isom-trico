#pragma once

#include <algorithm>

inline void orthoMat4(float left, float right, float bottom, float top, float out[16]) {
    std::fill(out, out + 16, 0.0f);
    out[0]  = 2.0f / (right - left);
    out[5]  = 2.0f / (top - bottom);
    out[10] = -1.0f;
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[15] = 1.0f;
}
