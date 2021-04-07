#include "../imgui/imgui.h"

#include "../Config.h"
#include "../Interfaces.h"
#include "../SDK/Entity.h"
#include "../SDK/EntityList.h"
#include "../SDK/LocalPlayer.h"

#include "Sound.h"

static bool soundWindowOpen = false;

void Sound::modulateSound(std::string_view name, int entityIndex, float& volume) noexcept
{
    auto modulateVolume = [&](int Config::Sound::Player::* proj) {
        if (const auto entity = interfaces->entityList->getEntity(entityIndex); localPlayer && entity && entity->isPlayer()) {
            if (entityIndex == localPlayer->index())
                volume *= std::invoke(proj, config->sound.players[0]) / 100.0f;
            else if (!entity->isOtherEnemy(localPlayer.get()))
                volume *= std::invoke(proj, config->sound.players[1]) / 100.0f;
            else
                volume *= std::invoke(proj, config->sound.players[2]) / 100.0f;
        }
    };

    modulateVolume(&Config::Sound::Player::masterVolume);

    using namespace std::literals;

    if (name == "Player.DamageHelmetFeedback"sv)
        modulateVolume(&Config::Sound::Player::headshotVolume);
    else if (name.find("Weapon"sv) != std::string_view::npos && name.find("Single"sv) != std::string_view::npos)
        modulateVolume(&Config::Sound::Player::weaponVolume);
    else if (name.find("Step"sv) != std::string_view::npos)
        modulateVolume(&Config::Sound::Player::footstepVolume);
    else if (name.find("Chicken"sv) != std::string_view::npos)
       volume *= config->sound.chickenVolume / 100.0f;
}

void Sound::menuBarItem() noexcept
{
    if (ImGui::MenuItem("Sound")) {
        soundWindowOpen = true;
        ImGui::SetWindowFocus("Sound");
        ImGui::SetWindowPos("Sound", { 100.0f, 100.0f });
    }
}

void Sound::tabItem() noexcept
{
    if (ImGui::BeginTabItem("Sound")) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Sound::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!soundWindowOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Sound", &soundWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::SliderInt("Chicken volume", &config->sound.chickenVolume, 0, 200, "%d%%");

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("", &currentCategory, "Local player\0Allies\0Enemies\0");
    ImGui::PopItemWidth();
    ImGui::SliderInt("Master volume", &config->sound.players[currentCategory].masterVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Headshot volume", &config->sound.players[currentCategory].headshotVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Weapon volume", &config->sound.players[currentCategory].weaponVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Footstep volume", &config->sound.players[currentCategory].footstepVolume, 0, 200, "%d%%");

    if (!contentOnly)
        ImGui::End();
}

void Sound::resetConfig() noexcept
{
    config->sound = {};
}

static void to_json(json& j, const Config::Sound::Player& o)
{
    const Config::Sound::Player dummy;

    WRITE("Master volume", masterVolume);
    WRITE("Headshot volume", headshotVolume);
    WRITE("Weapon volume", weaponVolume);
    WRITE("Footstep volume", footstepVolume);
}

json Sound::toJson() noexcept
{
    const Config::Sound dummy;

    json j;
    to_json(j["Chicken volume"], config->sound.chickenVolume, dummy.chickenVolume);
    j["Players"] = config->sound.players;
    return j;
}

static void from_json(const json& j, Config::Sound::Player& p)
{
    read(j, "Master volume", p.masterVolume);
    read(j, "Headshot volume", p.headshotVolume);
    read(j, "Weapon volume", p.weaponVolume);
    read(j, "Footstep volume", p.footstepVolume);
}

void Sound::fromJson(const json& j) noexcept
{
    read(j, "Chicken volume", config->sound.chickenVolume);
    read(j, "Players", config->sound.players);
}