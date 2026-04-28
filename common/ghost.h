#pragma once
#include <random>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <model/model.h>
#include "tools.h"

struct GLFWwindow;
class Player;
class Shader;

class Ghost : public Model
{
    /**
    * Meshes[0]: Ghost
    * Meshes[1]: Ghost face
    * Meshes[2-3]: Big stand pumpkin
    * Meshes[4-212]: Big lie pumpkin
    * Meshes[213-437]: smaller pumpkin
    */
protected:

    float fullHealth = 3.0;
    float health = fullHealth;

    glm::vec3 scale = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.1f, 200.0f);

    float ambient = 0.01f;
    float diffuse = 0.6f;
    float specular = 0.0f;
    float ghostAlpha = 0.2f;

    float faceAlpha = 0.2f;

    glm::vec3 position = glm::vec3(0.0f, 3.0f, 2.0f);
    glm::vec3 front = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 right = glm::vec3(0.0f, 0.0f, 1.0f);

    float speed = 5.0f;
    int state = 0;

    glm::vec3 ghostColor = glm::vec3(1.0f);
    float ghostLight = 1.0f;
    glm::vec3 faceColor = glm::vec3(0.0f, 0.0f, 0.0f);

    // Capture animation state (driven by update(dt, player))
    bool captureActive = false;
    bool captureRequestedThisFrame = false;
    float captureElapsed = 0.0f;
    glm::vec3 captureOriginScale = glm::vec3(0.1f);
    bool isCaptured = false;

    // Twinkling state (post-capture blink, currently unused but reachable
    // via update once captureActive transitions to depleted)
    bool twinklingActive = false;
    float twinklingElapsed = 0.0f;
    float twinklingTotal = 3.0f;
    float twinklingBlinkPeriod = 0.25f;
    bool drawEnable = true;

    // Free-roam runaway state (player exists -> ghost flees)
    bool runAwayActive = false;
    float runAwayCorrectAngle = 0.0f;
    float floatPhase = 0.0f;
    float lastFloatIncre = 0.0f;
    std::mt19937 rng;
    std::uniform_real_distribution<float> angleDist =
        std::uniform_real_distribution<float>(0.0f, 360.0f);

    float innerCollisionOffset = 0.4f;
    float outerCollisionOffset = 1.0f;

    std::vector<Box> innerBoxes;
    std::vector<Box> outerBoxes;

    GLFWwindow* window = nullptr;

public:
    void setWindow(GLFWwindow* window);

    explicit Ghost(std::string directory);
    Ghost();
    ~Ghost() = default;
    Ghost(const Ghost&) = default;

    void setModel();

    bool getIsCaptured();
    void setView(glm::mat4 view);
    void setProjection(glm::mat4 projection);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 position);

    void drawGhost(Shader shader, Player player);
    glm::mat4 faceToPlayer(glm::mat4 model, glm::vec3 playerPosition);
    void drawGhostFace(Shader shader, Player player);

    // Single per-frame entry point. Replaces the runAway / capture /
    // twinkling / looming worker threads with explicit dt-driven state.
    void update(float dt, const Player& player);

    // Called by the main loop on every frame the player's vacuum is
    // currently aiming at this ghost within range. Marks the ghost as
    // being drained; the next update() consumes the request.
    void capture();

    bool outerCollisionCheck();
    void setInnerBoxes(std::vector<Box> innerBoxes);
    void setOuterBoxes(std::vector<Box> outerBoxes);
    bool innerCollisionCheck();
};
