#include "ghost.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"
#include "shaders.h"

Ghost::Ghost(std::string directory) : Model(directory)
{
    setModel();
}

Ghost::Ghost() : Model("../res/model/ghost/OBJ.obj")
{
    setModel();
}

Ghost::~Ghost()
{
    isSettingAlpha = false;
    isSettingAmbient = false;
    isLooming = false;
    isFloating = false;
    isRunAway = false;
    if (alphaChangeThread.joinable())
        alphaChangeThread.detach();
    if (ambientChangeThread.joinable())
        ambientChangeThread.detach();
    if (loomingThread.joinable())
        loomingThread.detach();
    if (floatingThread.joinable())
        floatingThread.detach();
}

void Ghost::setModel()
{
    centralize(meshes, { 0, 1 });
}

void Ghost::setWindow(GLFWwindow* window)
{
    this->window = window;
}

bool Ghost::getIsCaptured()
{
    return isCaptured;
}

void Ghost::setShaking(bool isShaking)
{
    this->isShaking = isShaking;
}

void Ghost::setView(glm::mat4 view)
{
    this->view = view;
}

void Ghost::setProjection(glm::mat4 projection)
{
    this->projection = projection;
}

glm::vec3 Ghost::getPosition()
{
    return position;
}

void Ghost::setPosition(glm::vec3 position)
{
    this->position = position;
}

void Ghost::drawGhost(Shader shader, Player player)
{
    if (drawEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader.use();

        this->model = glm::mat4(1.0f);
        this->model = glm::translate(this->model, position);
        this->model = glm::scale(this->model, scale);
        this->model = faceToPlayer(this->model, player.getViewPosition());

        shader.setBool("singleColor.isEnable", false);
        shader.setVec3("material.ambient", glm::vec3(ambient));
        shader.setVec3("material.diffuse", glm::vec3(diffuse));
        shader.setVec3("material.specular", glm::vec3(specular));
        shader.setFloat("material.shininess", 32.0f);

        shader.setVec3("torch.position", player.getViewPosition());
        shader.setVec3("torch.direction", player.getFront());
        shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
        shader.setFloat("torch.constant", 1.0f);
        shader.setFloat("torch.linear", 0.09f);
        shader.setFloat("torch.quadratic", 0.032f);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        meshes[0].draw(shader);
    }
}

glm::mat4 Ghost::faceToPlayer(glm::mat4 model, glm::vec3 playerPosition)
{
    glm::vec3 newFront = glm::normalize(playerPosition - position);
    if (glm::distance(newFront, front) > 1e-5)
    {
        glm::vec3 rotateDir = glm::normalize(glm::cross(front, newFront));
        float rotateAngle = glm::acos(glm::dot(front, newFront));
        model = glm::rotate(model, rotateAngle, glm::vec3(rotateDir));
    }
    return model;
}

void Ghost::drawGhostFace(Shader shader, Player player)
{
    if (drawEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader.use();

        shader.setBool("singleColor.isEnable", true);
        shader.setVec3("singleColor.color", glm::vec3(1.0, 0, 0));
        shader.setVec3("material.ambient", glm::vec3(ambient));
        shader.setVec3("material.diffuse", glm::vec3(diffuse));
        shader.setVec3("material.specular", glm::vec3(specular));
        shader.setFloat("material.shininess", 32.0f);

        shader.setVec3("torch.position", player.getViewPosition());
        shader.setVec3("torch.direction", player.getFront());
        shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
        shader.setFloat("torch.constant", 1.0f);
        shader.setFloat("torch.linear", 0.09f);
        shader.setFloat("torch.quadratic", 0.032f);

        this->model = glm::mat4(1.0f);
        this->model = glm::translate(this->model, position);
        this->model = glm::scale(this->model, scale);
        this->model = faceToPlayer(this->model, player.getViewPosition());

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        meshes[1].draw(shader);
    }
}

void Ghost::drawPumpkin(Shader shader, int type)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.setInt("ghostParams.type", 2);
    shader.setVec3("ghostParams.ambient", glm::vec3(ambient));
    shader.setVec3("ghostParams.diffuse", glm::vec3(diffuse));
    shader.setVec3("ghostParams.specular", glm::vec3(specular));
    shader.setFloat("ghostParams.shininess", 32.0f);
    shader.setVec3("ghostParams.faceColor", faceColor);
    shader.setFloat("ghostParams.alpha", faceAlpha);

    switch (type)
    {
    case 0:
        meshes[2].draw(shader);
        meshes[3].draw(shader);
        break;
    case 1:
        for (int i = 4; i != 213; ++i)
        {
            meshes[i].draw(shader);
        }
        break;
    case 2:
        for (int i = 213; i != meshes.size(); ++i)
        {
            meshes[i].draw(shader);
        }
        break;
    default:
        std::cout << "There is no pumpkin type: " << type << std::endl;
        break;
    }
}

void Ghost::showUp()
{
    alphaChangeThread = std::thread(&Ghost::setGhostAlpha, this, 0.6);
    alphaChangeThread.detach();
}

void Ghost::disappear()
{
    alphaChangeThread = std::thread(&Ghost::setGhostAlpha, this, 0.1);
    alphaChangeThread.detach();
}

void Ghost::startLooming(float lowerBound, float upperBound)
{
    isLooming = true;
    loomingThread = std::thread(&Ghost::looming, this, lowerBound, upperBound);
}

void Ghost::stopLooming()
{
    isLooming = false;
    loomingThread.detach();
}

void Ghost::setGhostAlpha(float alpha)
{
    if (!isSettingAlpha)
    {
        isSettingAlpha = true;
        Timer timer;
        float diff = alpha - ghostAlpha;
        while ((alpha - ghostAlpha > 0 && diff > 0)
            || (ghostAlpha - alpha > 0 && diff < 0))
        {
            timer.tictok();
            ghostAlpha += diff * timer.getDeltaTime();
        }
        ghostAlpha = alpha;
        isSettingAlpha = false;
    }
    else
    {
        std::cout << "Alpha setting process is running!" << std::endl;
    }
}

void Ghost::setAmbient(float ambient)
{
    if (!isSettingAmbient)
    {
        isSettingAmbient = true;
        Timer timer;
        float sign = std::abs(ambient - ambient) / (ambient - ambient);
        while ((ambient - ambient > 0 && sign > 0)
            || (ambient - ambient > 0 && sign < 0))
        {
            timer.tictok();
            ambient += sign * timer.getDeltaTime();
        }
        ambient = ambient;
        isSettingAmbient = false;
    }
    else
    {
        std::cout << "Ambient setting process is running!" << std::endl;
    }
}

void Ghost::looming(float lowerBound, float upperBound)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> realDist(lowerBound, upperBound);
    while (isLooming)
    {
        double randomReal = realDist(gen);
        setAmbient(randomReal);
    }
}

void Ghost::captureTFunc()
{
    glm::vec3 originScale = scale;
    float currentTime;
    if (health > 0.0)
    {
        currentTime = glfwGetTime();
        while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && health > 0)
        {
            float effect = 0.1f * (std::sin(8.0 * glfwGetTime() * glm::radians(45.0f)) + 1);

            health -= glfwGetTime() - currentTime;
            currentTime = glfwGetTime();

            scale = originScale * (1 - effect);
        }
    }
    if (health <= 0.0)
    {
        currentTime = glfwGetTime();
        std::cout << "Captured!" << std::endl;
        while (scale.x > 0)
        {
            float temp = (glfwGetTime() - currentTime);
            scale = scale - temp * glm::vec3(1e-2);
        }
        isCaptured = true;
    }

    scale *= std::min(health / fullHealth + 0.2, 1.0);
    isCapturing = false;
}

void Ghost::capture()
{
    if (!isCapturing)
    {
        isCapturing = true;
        faceColor = glm::vec3(0.8, 0.0, 0.0);
        animationThreads.push_back(std::thread(&Ghost::captureTFunc, this));
        animationThreads[animationThreads.size() - 1].detach();
    }
}

void Ghost::twinkling()
{
    animationThreads.push_back(std::thread(&Ghost::twinklingTFunc, this));
    animationThreads[animationThreads.size() - 1].detach();
}

void Ghost::twinklingTFunc()
{
    if (!isTwinkling)
    {
        isTwinkling = true;
        float startTime = glfwGetTime();
        int times = twinklingTime * 4;
        float interval = twinklingTime / times;
        int lastCnt = -1;
        while (glfwGetTime() - startTime < twinklingTime)
        {
            if (int((glfwGetTime() - startTime) / interval) != lastCnt)
            {
                drawEnable = !drawEnable;
                lastCnt = int((glfwGetTime() - startTime) / interval);
            }
        }
        drawEnable = true;
        isTwinkling = false;
    }
    else
    {
        std::cout << "Is twinkling!" << std::endl;
    }
}

void Ghost::runAwayTFun(Player& player)
{
    Timer timer;
    glm::vec3 runDir;
    float lastFloatIncre = 0;
    float currentFloatIncre = 0;
    bool isCollide = false;
    float correctAngle = 0.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> realDist(0.0f, 360.0f);

    while (isRunAway)
    {
        if (isCollide)
        {
            correctAngle = realDist(gen);
        }
        else if (!(isCollide) && correctAngle > 0)
        {
        }
        timer.tictok();
        runDir = position - player.getViewPosition();

        runDir = glm::vec3(
            runDir.x * glm::cos(glm::radians(correctAngle)) - runDir.z * glm::sin(glm::radians(correctAngle)),
            0.0,
            runDir.x * glm::sin(glm::radians(correctAngle)) + runDir.z * glm::cos(glm::radians(correctAngle))
        );
        runDir = glm::normalize(runDir);

        position += runDir * speed * timer.getDeltaTime();

        currentFloatIncre = 0.4f * std::sin(3.0 * glfwGetTime() * glm::radians(45.0f));
        position.y += currentFloatIncre - lastFloatIncre;
        lastFloatIncre = currentFloatIncre;

        isCollide = outerCollisionCheck() || innerCollisionCheck();
    }
    isRunAway = false;
}

void Ghost::runAway(Player& player)
{
    if (!isRunAway)
    {
        isRunAway = true;
        animationThreads.push_back(std::thread(&Ghost::runAwayTFun, this, std::ref(player)));
        animationThreads[animationThreads.size() - 1].detach();
    }
}

bool Ghost::outerCollisionCheck()
{
    bool isCollide = false;
    glm::vec3 maxBorder = outerBoxes[0].maxVec - outerCollisionOffset;
    glm::vec3 minBorder = outerBoxes[0].minVec + outerCollisionOffset;
    if (position.x > maxBorder.x)
    {
        position.x = maxBorder.x;
        isCollide = true;
    }
    if (position.x < minBorder.x)
    {
        position.x = minBorder.x;
        isCollide = true;
    }
    if (position.y > maxBorder.y)
    {
        position.y = maxBorder.y;
    }
    if (position.y < minBorder.y)
    {
        position.y = minBorder.y;
    }
    if (position.z > maxBorder.z)
    {
        position.z = maxBorder.z;
        isCollide = true;
    }
    if (position.z < minBorder.z)
    {
        position.z = minBorder.z;
        isCollide = true;
    }
    return isCollide;
}

void Ghost::setInnerBoxes(std::vector<Box> innerBoxes)
{
    this->innerBoxes = innerBoxes;
}

void Ghost::setOuterBoxes(std::vector<Box> outerBoxes)
{
    this->outerBoxes = outerBoxes;
}

bool Ghost::innerCollisionCheck()
{
    bool isCollide = false;
    for (const Box& box : innerBoxes)
    {
        glm::vec3 maxBorder = box.maxVec + innerCollisionOffset;
        glm::vec3 minBorder = box.minVec - innerCollisionOffset;
        if (position.x < maxBorder.x && position.x > minBorder.x &&
            position.y < maxBorder.y && position.y > minBorder.y &&
            position.z < maxBorder.z && position.z > minBorder.z)
        {
            float dist_x_min = position.x - minBorder.x;
            float dist_y_min = position.y - minBorder.y;
            float dist_z_min = position.z - minBorder.z;

            float dist_x_max = maxBorder.x - position.x;
            float dist_y_max = maxBorder.y - position.y;
            float dist_z_max = maxBorder.z - position.z;

            if (dist_x_min < dist_z_min &&
                dist_x_min < dist_x_max &&
                dist_x_min < dist_z_max)
            {
                position.x = minBorder.x;
                isCollide = true;
            }
            else if (dist_z_min < dist_x_min &&
                dist_z_min < dist_x_max &&
                dist_z_min < dist_z_max)
            {
                position.z = minBorder.z;
                isCollide = true;
            }
            else if (dist_x_max < dist_x_min &&
                dist_x_max < dist_z_min &&
                dist_x_max < dist_z_max)
            {
                position.x = maxBorder.x;
                isCollide = true;
            }
            else
            {
                position.z = maxBorder.z;
                isCollide = true;
            }
        }
    }
    return isCollide;
}
