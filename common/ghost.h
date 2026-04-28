#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <glm/glm.hpp>

#include <model/model.h>
#include "tools.h"

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

    std::atomic<bool> isSettingAlpha = false;
    std::thread alphaChangeThread;
    std::atomic<bool> isSettingAmbient = false;
    std::thread ambientChangeThread;
    std::atomic<bool> isLooming = false;
    std::thread loomingThread;
    std::atomic<bool> isFloating = false;
    std::thread floatingThread;
    std::atomic<bool> isCapturing = false;
    std::atomic<bool> isCaptured = false;
    std::vector<std::thread> animationThreads;

    bool isShaking = false;
    std::atomic<bool> isTwinkling = false;
    float twinklingTime = 3.0;
    std::atomic<bool> drawEnable = true;
    std::atomic<bool> isRunAway = false;

    float innerCollisionOffset = 0.4f;
    float outerCollisionOffset = 1.0f;

    std::vector<Box> innerBoxes;
    std::vector<Box> outerBoxes;

public:
    explicit Ghost(std::string directory);
    Ghost();
    ~Ghost();
    Ghost(const Ghost&) = default;

    void setModel();

    bool getIsCaptured();
    void setShaking(bool isShaking);
    void setView(glm::mat4 view);
    void setProjection(glm::mat4 projection);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 position);

    void drawGhost(Shader shader, Player player);
    glm::mat4 faceToPlayer(glm::mat4 model, glm::vec3 playerPosition);
    void drawGhostFace(Shader shader, Player player);
    void drawPumpkin(Shader shader, int type);

    void showUp();
    void disappear();
    void startLooming(float lowerBound, float upperBound);
    void stopLooming();

protected:
    void setGhostAlpha(float alpha);
    void setAmbient(float ambient);
    void looming(float lowerBound, float upperBound);

public:
    void captureTFunc();
    void capture();
    void twinkling();
    void twinklingTFunc();

    void runAwayTFun(Player& player);
    void runAway(Player& player);

    bool outerCollisionCheck();
    void setInnerBoxes(std::vector<Box> innerBoxes);
    void setOuterBoxes(std::vector<Box> outerBoxes);
    bool innerCollisionCheck();
};
