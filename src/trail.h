#pragma once
#include <deque>
#include <cstdint>

struct TrailPoint {
    float x, y;
    uint32_t timestamp_ms;
};

class StickTrail {
public:
    void push(float x, float y, uint32_t now_ms);
    void prune(uint32_t now_ms, float trail_duration_ms, float fade_duration_ms);
    const std::deque<TrailPoint>& points() const { return m_points; }

private:
    std::deque<TrailPoint> m_points;
};