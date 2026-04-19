#include "trail.h"

void StickTrail::push(float x, float y, uint32_t now_ms) {
    m_points.push_back({x, y, now_ms});
}

void StickTrail::prune(uint32_t now_ms, float trail_ms, float fade_ms) {
    while (!m_points.empty()) {
        float age = float(now_ms - m_points.front().timestamp_ms);
        if (age > trail_ms + fade_ms)
            m_points.pop_front();
        else
            break;
    }
}