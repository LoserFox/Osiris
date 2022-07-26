#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <memory>
#include "Aimbot.h"
#include "../Config.h"
#include "../InputUtil.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "Misc.h"
#include "../SDK/Engine.h"
#include "../SDK/EngineTrace.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/UserCmd.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponId.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/PhysicsSurfaceProps.h"
#include "../SDK/WeaponData.h"
#include "../SDK/WeaponSystem.h"
#include <SDK/math.hpp>
#include <SDK/ivmodelrender.hpp>
Vector Aimbot::calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept
{
    return ((destination - source).toAngle() - viewAngles).normalize();
}
void scaleDamage(HitGroups hitgroup, Entity* enemy,float weapon_armor_ratio, float& current_damage) {
    current_damage *= HitGroupsHelper::getDamageMultiplier(hitgroup);
    int ArmorValue = enemy->armor();
    if (ArmorValue > 0) {
        float Damage = current_damage;
        float v47 = 1.f, ArmorBonusRatio = 0.5f,
            ArmorRatio = weapon_armor_ratio * 0.5f;
        auto NewDamage = Damage * ArmorRatio;

        if (((Damage - (Damage * ArmorRatio)) * (v47 * ArmorBonusRatio)) >
            ArmorValue) {
            NewDamage = Damage - (ArmorValue / ArmorBonusRatio);
        }

        current_damage = NewDamage;
        if (hitgroup == HitGroups::Head) {
            if (enemy->hasHelmet())
                current_damage *= weapon_armor_ratio * 0.5f;
        }
        else
            current_damage *= weapon_armor_ratio * 0.5f;
    }
}
static bool traceToExit(Vector& end, Trace* enter_trace, Vector start, Vector dir,
    Trace* exit_trace)
{
    bool result = false;
    float distance = 0.0f;

    while (distance <= 90.0f) {
        distance += 4.0f;
        end = start + dir * distance;

        auto point_contents =
            interfaces->engineTrace->getPointContents(Vector{0,0,0},0);
        auto new_end = end - (dir * 4.0f);

        Ray ray{ end, new_end };
        interfaces->engineTrace->traceRay(ray, 0x46004009, 0, *exit_trace);

        if (exit_trace->startSolid && exit_trace->surface.flags ) {
            Ray ray{ end, start };

            TraceFilter filter{ exit_trace->entity };

            interfaces->engineTrace->traceRay(ray, 0x46004009, filter, *exit_trace);

            if ((exit_trace->fraction < 1.0f || exit_trace->allSolid) &&
                !exit_trace->startSolid) {
                end = exit_trace->endpos;
                return true;
            }

            continue;
        }

        if (!(exit_trace->fraction < 1.0 || exit_trace->allSolid||
            exit_trace->startSolid) ||
            exit_trace->startSolid) {
            if (exit_trace->entity && enter_trace->entity) {
                return true;
            }

            continue;
        }

        if (exit_trace->surface.flags >> 7 & 1 &&
            !(enter_trace->surface.flags >> 7 & 1))
            continue;

        if (exit_trace->plane.normal.Dot(dir) <= 1.0f) {
            auto fraction = exit_trace->fraction * 4.0f;
            end = end - (dir * fraction);

            return true;
        }
    }

    return false;
}

bool handleBulletPenetration(WeaponInfo* weaponInfo, FireBulletData& data) noexcept
{
    SurfaceData* enter_surface_data =
       interfaces->physicsSurfaceProps->getSurfaceData(
            data.enter_trace.surface.surfaceProps);
    int enter_material = enter_surface_data->material;
    float enter_surf_penetration_mod =
        enter_surface_data->penetrationmodifier;

    data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
    data.current_damage *=
        powf(weaponInfo->rangeModifier, data.trace_length * 0.002f);

    if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f)
        data.penetrate_count = 0;

    if (data.penetrate_count <= 0)
        return false;

    Vector dummy;
    Trace trace_exit;

    if (!traceToExit(dummy, &data.enter_trace, data.enter_trace.endpos,
        data.direction, &trace_exit))
        return false;

    SurfaceData* exit_surface_data = interfaces->physicsSurfaceProps->getSurfaceData(trace_exit.surface.surfaceProps);
    int exit_material = exit_surface_data->material;

    float exit_surf_penetration_mod =
        exit_surface_data->penetrationmodifier;

    float final_damage_modifier = 0.16f;
    float combined_penetration_modifier = 0.0f;

    if (enter_material == 89 || enter_material == 71) {
        combined_penetration_modifier = 3.0f;
        final_damage_modifier = 0.05f;
    }
    else
        combined_penetration_modifier =
        (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

    if (enter_material == exit_material) {
        if (exit_material == 87 || exit_material == 85)
            combined_penetration_modifier = 3.0f;
        else if (exit_material == 76)
            combined_penetration_modifier = 2.0f;
    }

    float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
    float v35 =
        (data.current_damage * final_damage_modifier) +
        v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->penetration) * 1.25f);
    float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

    thickness *= thickness;
    thickness *= v34;
    thickness /= 24.0f;

    float lost_damage = fmaxf(0.0f, v35 + thickness);

    if (lost_damage > data.current_damage)
        return false;

    if (lost_damage >= 0.0f)
        data.current_damage -= lost_damage;

    if (data.current_damage < 1.0f)
        return false;

    data.src = trace_exit.endpos;
    data.penetrate_count--;

    return true;
}
void traceLine(Vector vecAbsStart, Vector vecAbsEnd,
    unsigned int mask, Entity * ignore,
    Trace* ptr) {
    Ray ray(vecAbsStart, vecAbsEnd);
    TraceFilter filter { ignore };

    interfaces->engineTrace->traceRay(ray, mask, filter, *ptr);
}

bool simulateFireBullet(WeaponInfo* pWeapon, bool allowFriendlyFire,
   FireBulletData& data) {
    WeaponInfo* weaponInfo = weaponInfo;

    data.penetrate_count = 4;
    data.trace_length = 0.0f;
    data.current_damage = (float)weaponInfo->damage;

    while (data.penetrate_count > 0 && data.current_damage >= 1.0f) {
        data.trace_length_remaining = weaponInfo->range - data.trace_length;

        Vector end = data.src + data.direction * data.trace_length_remaining;

        // data.enter_trace
        traceLine(data.src, end, UserCmd::IN_ATTACK, localPlayer.get(),
            &data.enter_trace);

        Ray ray(data.src, end + data.direction * 40.f);


        interfaces->engineTrace->traceRay(ray, UserCmd::IN_ATTACK, data.filter,data.enter_trace);

        traceLine(data.src, end + data.direction * 40.f, UserCmd::IN_ATTACK, localPlayer.get(),
            &data.enter_trace);

        if (data.enter_trace.fraction == 1.0f)
            break;

        if (data.enter_trace.hitgroup <= HitGroups::RightLeg &&
            data.enter_trace.hitgroup > HitGroups::Generic) {
            data.trace_length +=
                data.enter_trace.fraction * data.trace_length_remaining;
            data.current_damage *=
                powf(weaponInfo->rangeModifier, data.trace_length * 0.002f);

            if (!allowFriendlyFire && data.enter_trace.entity &&
                data.enter_trace.entity->isPlayer() && !data.enter_trace.entity->isOtherEnemy(data.enter_trace.entity))
                return false;

            Entity* player = data.enter_trace.entity;
            auto iHitGroups = static_cast<HitGroups>(data.enter_trace.hitgroup);
            scaleDamage(iHitGroups, player,
                weaponInfo->armorRatio, data.current_damage);

            return true;
        }

        if (!handleBulletPenetration(weaponInfo, data))
            break;
    }

    return false;
}



bool canShoot(Entity* weapon, QAngle* angle,
    Entity* enemy, int hitChance) {
    if (hitChance == 0)
        return true;

    Vector src = localPlayer->getEyePosition();
    Vector forward, right, up;
    angleVectors(*angle, &forward, &right, &up);

    int hitCount = 0;

    weapon->UpdateAccuracyPenalty();
    float weap_spread = weapon->GetSpread();
    float weap_inaccuracy = weapon->getInaccuracy();

    for (int i = 0; i < 100; i++) {
        static float val1 = (2.0 * M_PI);

        double b = Helpers::random(0.f, val1);
        double spread = weap_spread * Helpers::random(0.f, 1.0f);
        double d = Helpers::random(0.f, 1.0f);
        double inaccuracy = weap_inaccuracy * Helpers::random(0.f, 1.0f);

        Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread),
            (sin(b) * inaccuracy) + (sin(d) * spread), 0),
            direction;

        direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
        direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
        direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
        direction.normalize();

        QAngle viewAnglesSpread;
        vectorAngles(direction, up, viewAnglesSpread);
        normalizeAngles(viewAnglesSpread);

        Vector viewForward;
        angleVectors(viewAnglesSpread, viewForward);
        viewForward.NormalizeInPlace();

        viewForward =
            src + (viewForward * weapon->getWeaponData()->range);

        Trace tr;
        Ray ray(src, viewForward);
        interfaces->engineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, enemy, &tr);

        // TraceFilter filter;
        // filter.pSkip = Globals::localPlayer;
        // //   hitbox  |  monster  | solid
        // Interfaces::trace->TraceRay(ray, (0x40000000 | 0x40000 | 0x1), &filter, &tr);

        if (tr.entity == enemy) {
            hitCount++;
        }

        if (hitCount >= hitChance) {
            // Notifications::addNotification(ImColor(255, 255, 255), "HITS OK");
            return true;
        }
        else if (100 - i + hitCount < hitChance) {
            return false;
        }
    }

    return false;
}

std::vector<Vector> getPoints(Entity* player, int iHitbox,
    float headScale,
    float bodyScale) {
    std::vector<Vector> points;

    Model* pModel = player->getModel();
    if (!pModel)
        return points;
    
    studiohdr_t* hdr = interfaces->modelInfo->getStudioModel(pModel);
    if (!hdr)
        return points;
    mstudiobbox_t* bbox = hdr->pHitbox(iHitbox, 0);
    if (!bbox)
        return points;

    matrix3x4_t matrix[128];

    Vector mins, maxs;
    vectorTransform(bbox->bbmin, matrix[bbox->bone], mins);
    vectorTransform(bbox->bbmax, matrix[bbox->bone], maxs);
    Vector center = (mins + maxs) * 0.5f;

    if (bbox->radius <= 0.f) {
        matrix3x4_t rot;
        angleMatrix(bbox->rotation, rot);

        auto origin = rot.GetOrigin();

        Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

        points.emplace_back(Vector(center.Dot(rot[0]), center.Dot(rot[1]), center.Dot(rot[2])) + origin);
    }
    else {
        points.emplace_back(center);
        auto final_radius = bbox->radius * bodyScale;

        if (iHitbox == (int)HitboxModel::HITBOX_HEAD) {
            auto pitch_down = normalizePitch(player->eyeAngles().x) > 85.0f;
            auto originY = localPlayer.get()->origin().y;
            float stuff =
                calcAngle(player->getEyePosition(), Vector{ originY, originY, originY }).y;
            auto backward = fabs(player->eyeAngles().y - stuff) > 120.0f;

            points.emplace_back(Vector(bbox->bbmax.x + 0.70710678f * final_radius,
                bbox->bbmax.y - 0.70710678f * final_radius,
                bbox->bbmax.z));
            points.emplace_back(
                Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius));
            points.emplace_back(
                Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius));
            points.emplace_back(
                Vector(bbox->bbmax.x, bbox->bbmax.y - final_radius, bbox->bbmax.z));

            if (pitch_down && backward)
                points.emplace_back(
                    Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z));
        }
        else {
            points.emplace_back(
                Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius));
            points.emplace_back(
                Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius));
            points.emplace_back(
                Vector(center.x, bbox->bbmax.y - final_radius, center.z));
        }
    }

    return points;
}

void bestMultiPoint(Player* player, int& BoneIndex,
    int& Damage, Vector& Spot, float headScale, float bodyScale,
    Weapon* weapon, bool friendlyFire) {
    for (const auto& point : getPoints(player, BoneIndex, headScale, bodyScale)) {
        int bestDamage = getDamageDeal(player, point, weapon, friendlyFire);
        if (bestDamage > Damage) {
            Damage = bestDamage;
            Spot = point;
        }
    }
}

void applyAutoSlow(UserCmd* cmd, WeaponInfo* weapon) {
    if (memory->globalVars->intervalPerTick == FP_NAN ||
        memory->globalVars->intervalPerTick == 0 ||
        memory->globalVars->intervalPerTick == FP_INFINITE)
        return;

    const float maxSpeed =
        (localPlayer.get()->isScoped()
            ? weapon->maxSpeed
            : weapon->maxSpeedAlt);

    if (localPlayer->velocity().Length() > maxSpeed / 3) {
        cmd->forwardmove = -cmd->forwardmove;
        cmd->sidemove = -cmd->sidemove;
        cmd->upmove = 0;
        cmd->buttons |= UserCmd::IN_WALK;
    }
    else {
        float sped = 0.1f;
        float ratio = maxSpeed / 255.0f;
        sped *= ratio;

        cmd->forwardmove *= sped;
        cmd->sidemove *= sped;
    }
}

static bool keyPressed = false;

void Aimbot::updateInput() noexcept
{
    if (config->aimbotKeyMode == 0)
        keyPressed = config->aimbotKey.isDown();
    if (config->aimbotKeyMode == 1 && config->aimbotKey.isPressed())
        keyPressed = !keyPressed;
}

void Aimbot::run(UserCmd* cmd) noexcept
{
    if (!localPlayer || localPlayer->nextAttack() > memory->globalVars->serverTime() || localPlayer->isDefusing() || localPlayer->waitForNoAttack())
        return;

    auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    if (localPlayer->shotsFired() > 0 && !activeWeapon->isFullAuto())
        return;

    auto weaponIndex = getWeaponIndex(activeWeapon->itemDefinitionIndex());
    if (!weaponIndex)
        return;

    auto weaponClass = getWeaponClass(activeWeapon->itemDefinitionIndex());
    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = weaponClass;

    if (!config->aimbot[weaponIndex].enabled)
        weaponIndex = 0;

    if (!config->aimbot[weaponIndex].betweenShots && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime())
        return;

    if (!config->aimbot[weaponIndex].ignoreFlash && localPlayer->isFlashed())
        return;

    if (config->aimbotOnKey && !keyPressed)
        return;

    if (config->aimbot[weaponIndex].enabled && (cmd->buttons & UserCmd::IN_ATTACK || config->aimbot[weaponIndex].autoShot || config->aimbot[weaponIndex].aimlock) && activeWeapon->getInaccuracy() <= config->aimbot[weaponIndex].maxAimInaccuracy) {

        if (config->aimbot[weaponIndex].scopedOnly && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
            return;

        auto bestFov = config->aimbot[weaponIndex].fov;
        Vector bestTarget{ };
        const auto localPlayerEyePosition = localPlayer->getEyePosition();

        const auto aimPunch = activeWeapon->requiresRecoilControl() ? localPlayer->getAimPunch() : Vector{ };

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
                || !entity->isOtherEnemy(localPlayer.get()) && !config->aimbot[weaponIndex].friendlyFire || entity->gunGameImmunity())
                continue;

            for (auto bone : { 8, 4, 3, 7, 6, 5 }) {
                const auto bonePosition = entity->getBonePosition(config->aimbot[weaponIndex].bone > 1 ? 10 - config->aimbot[weaponIndex].bone : bone);
                const auto angle = calculateRelativeAngle(localPlayerEyePosition, bonePosition, cmd->viewangles + aimPunch);
                
                const auto fov = std::hypot(angle.x, angle.y);
                if (fov > bestFov)
                    continue;

                if (!config->aimbot[weaponIndex].ignoreSmoke && memory->lineGoesThroughSmoke(localPlayerEyePosition, bonePosition, 1))
                    continue;

                if (!entity->isVisible(bonePosition) && (config->aimbot[weaponIndex].visibleOnly || !simulateFireBullet(activeWeapon->getWeaponData(), false)))
                    continue;

                if (fov < bestFov) {
                    bestFov = fov;
                    bestTarget = bonePosition;
                }
                if (config->aimbot[weaponIndex].bone)
                    break;
            }
        }

        if (bestTarget.notNull()) {
            static Vector lastAngles{ cmd->viewangles };
            static int lastCommand{ };

            if (lastCommand == cmd->commandNumber - 1 && lastAngles.notNull() && config->aimbot[weaponIndex].silent)
                cmd->viewangles = lastAngles;

            auto angle = calculateRelativeAngle(localPlayerEyePosition, bestTarget, cmd->viewangles + aimPunch);
            bool clamped{ false };

            if (std::abs(angle.x) > Misc::maxAngleDelta() || std::abs(angle.y) > Misc::maxAngleDelta()) {
                    angle.x = std::clamp(angle.x, -Misc::maxAngleDelta(), Misc::maxAngleDelta());
                    angle.y = std::clamp(angle.y, -Misc::maxAngleDelta(), Misc::maxAngleDelta());
                    clamped = true;
            }
            
            angle /= config->aimbot[weaponIndex].smooth;
            cmd->viewangles += angle;
            if (!config->aimbot[weaponIndex].silent)
                interfaces->engine->setViewAngles(cmd->viewangles);

            if (config->aimbot[weaponIndex].autoScope && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && activeWeapon->isSniperRifle() && !localPlayer->isScoped())
                cmd->buttons |= UserCmd::IN_ATTACK2;

            if (config->aimbot[weaponIndex].autoShot && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime() && !clamped && activeWeapon->getInaccuracy() <= config->aimbot[weaponIndex].maxShotInaccuracy)
                cmd->buttons |= UserCmd::IN_ATTACK;

            if (clamped)
                cmd->buttons &= ~UserCmd::IN_ATTACK;

            if (clamped || config->aimbot[weaponIndex].smooth > 1.0f) lastAngles = cmd->viewangles;
            else lastAngles = Vector{ };

            lastCommand = cmd->commandNumber;
        }
    }
}

