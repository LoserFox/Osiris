#pragma once

struct UserCmd;
struct Vector;
struct FireBulletData {
    Vector src;
    Trace enter_trace;
    Vector direction;
    TraceFilter filter;
    float trace_length;
    float trace_length_remaining;
    float current_damage;
    int penetrate_count;
};
namespace Aimbot
{

    Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept;
    void run(UserCmd*) noexcept;

    void updateInput() noexcept;
}
