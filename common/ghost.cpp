#include "ghost.h"

#include <algorithm>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"
#include "shaders.h"

Ghost::Ghost(std::string directory) : Model(directory)
{
    setModel();
    rng.seed(std::random_device{}());
}

Ghost::Ghost() : Model("../res/model/ghost/OBJ.obj")
{
    setModel();
    rng.seed(std::random_device{}());
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

void Ghost::capture()
{
    if (isCaptured)
    {
        return;
    }
    captureRequestedThisFrame = true;
    if (!captureActive)
    {
        captureActive = true;
        captureElapsed = 0.0f;
        captureOriginScale = scale;
        faceColor = glm::vec3(0.8f, 0.0f, 0.0f);
    }
}

void Ghost::update(float dt, const Player& player)
{
    // 1. Capture pressure (Phase A: drain health while vacuum aimed)
    if (captureActive && health > 0.0f)
    {
        if (captureRequestedThisFrame)
        {
            captureElapsed += dt;
            health -= dt;
            float effect = 0.1f * (std::sin(8.0f * captureElapsed * glm::radians(45.0f)) + 1.0f);
            scale = captureOriginScale * (1.0f - effect);
        }
        else
        {
            // Vacuum released before depletion: apply partial recovery
            // factor (matches the trailing line of the legacy worker)
            scale = captureOriginScale * std::min(health / fullHealth + 0.2f, 1.0f);
            captureActive = false;
        }
    }

    // 2. Capture finalisation (Phase B: shrink to zero, then mark captured)
    if (captureActive && health <= 0.0f)
    {
        scale -= glm::vec3(dt * 0.5f);
        if (scale.x <= 0.0f)
        {
            scale = glm::vec3(0.0f);
            isCaptured = true;
            captureActive = false;
            runAwayActive = false;
            twinklingActive = true;
            twinklingElapsed = 0.0f;
            drawEnable = true;
        }
    }
    captureRequestedThisFrame = false;

    // 3. Free-roam runaway (default state while alive)
    if (!isCaptured)
    {
        if (!runAwayActive)
        {
            runAwayActive = true;
        }

        glm::vec3 runDir = position - player.getViewPosition();
        float ang = glm::radians(runAwayCorrectAngle);
        runDir = glm::vec3(
            runDir.x * glm::cos(ang) - runDir.z * glm::sin(ang),
            0.0f,
            runDir.x * glm::sin(ang) + runDir.z * glm::cos(ang)
        );
        if (glm::length(runDir) > 1e-6f)
        {
            runDir = glm::normalize(runDir);
            position += runDir * speed * dt;
        }

        floatPhase += dt;
        float currentFloatIncre = 0.4f * std::sin(3.0f * floatPhase * glm::radians(45.0f));
        position.y += currentFloatIncre - lastFloatIncre;
        lastFloatIncre = currentFloatIncre;

        bool collided = outerCollisionCheck() || innerCollisionCheck();
        if (collided)
        {
            runAwayCorrectAngle = angleDist(rng);
        }
    }

    // 4. Post-capture twinkle
    if (twinklingActive)
    {
        twinklingElapsed += dt;
        int idx = static_cast<int>(twinklingElapsed / twinklingBlinkPeriod);
        drawEnable = (idx % 2) == 0;
        if (twinklingElapsed >= twinklingTotal)
        {
            twinklingActive = false;
            drawEnable = false;
        }
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
