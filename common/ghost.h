#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <model/model.h>
#include <shaders.h>
#include <iostream>
#include "tools.h"
#include <thread>
#include <atomic>
#include <random>
#include <player.h>

class Ghost :public Model
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

    //timer
    //Model related
    glm::vec3 scale = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1600 / (float)1000, 0.1f, 200.0f);
    //light settting
    float ambient = 0.01f;
    float diffuse = 0.6f;
    float specular = 0.0f;
    float ghostAlpha = 0.2f;

    float faceAlpha = 0.2f;
    //Position related
    glm::vec3 position = glm::vec3(0.0f, 3.0f, 2.0f);
    glm::vec3 front = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 right = glm::vec3(0.0f, 0.0f, 1.0f);
    //Attributes
    float speed = 5.0f;
    /** Ghosts have 3 status
    * state = 0 -> initial state: alpha=0.01, ambient=random(0~0.1),
    * state = 1 -> been hitten once
    * state = 2 -> been hitten twice
    */
    int state = 0;

    //other attributes -> different ghost using different set
    glm::vec3 ghostColor = glm::vec3(1.0f);
    float ghostLight = 1.0f;
    glm::vec3 faceColor = glm::vec3(0.0f, 0.0f, 0.0f);
    //function threads
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
    std::vector <std::thread> animationThreads;


    bool isShaking = false;
    std::atomic<bool> isTwinkling = false;
    float twinklingTime = 3.0;
    std::atomic<bool> drawEnable = true;
    std::atomic<bool> isRunAway = false;
    //collision detection related
    float innerCollisionOffset = 0.4f;
    float outerCollisionOffset = 1.0f;

    std::vector<Box> innerBoxes;
    std::vector<Box> outerBoxes;

public:
    Ghost(std::string directory) :Model(directory)
    {
        setModel();
    }
    Ghost() :Model("../res/model/ghost/OBJ.obj")
    {
        setModel();
    }
    ~Ghost()
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
    Ghost(const Ghost&) = default;
    void setModel()
    {
        //ghost centralize
        centralize(meshes, { 0,1 });
        //offset = centralize(meshes[0]);

    }

    bool getIsCaptured()
    {
        return isCaptured;
    }
    void setShaking(bool isShaking)
    {
        this->isShaking = isShaking;
    }
    void setView(glm::mat4 view)
    {
        this->view = view;
    }
    void setProjection(glm::mat4 projection)
    {
        this->projection = projection;
    }
    glm::vec3 getPosition()
    {
        return position;
    }
    void setPosition(glm::vec3 position)
    {
        this->position = position;
    }
    void drawGhost(Shader shader, Player player)
    {
        if (drawEnable)
        {
            //glDisable(GL_DEPTH_TEST);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shader.use();


            this->model = glm::mat4(1.0f);


            this->model = glm::translate(this->model, position); // translate the background so the ground is at y = 0 
            this->model = glm::scale(this->model, scale);	// it's a bit too big for our scene, so scale it down
            //this->model = glm::rotate(this->model, glm::radians(rotateAngle), glm::vec3(rotateDir));
            this->model = faceToPlayer(this->model, player.getViewPosition());


            /**
            * light shadering settings
            */
            shader.setBool("singleColor.isEnable", false);
            shader.setVec3("material.ambient", glm::vec3(ambient));
            shader.setVec3("material.diffuse", glm::vec3(diffuse));
            shader.setVec3("material.specular", glm::vec3(specular));
            shader.setFloat("material.shininess", 32.0f);

            //set light
            shader.setVec3("torch.position", player.getViewPosition());
            shader.setVec3("torch.direction", player.getFront());
            shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
            shader.setFloat("torch.constant", 1.0f);
            shader.setFloat("torch.linear", 0.09f);
            shader.setFloat("torch.quadratic", 0.032f);

            //set MVP
            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);

            meshes[0].draw(shader);
        }

    }

    glm::mat4 faceToPlayer(glm::mat4 model, glm::vec3 playerPosition)
    {
        //rotation -> face to player
        glm::vec3 newFront = glm::normalize(playerPosition - position);
        if (glm::distance(newFront, front) > 1e-5)
        {
            glm::vec3 rotateDir = glm::normalize(glm::cross(front, newFront));
            //rotateDir.x = rotateDir.y = 0.0f;
            float rotateAngle = glm::acos(glm::dot(front, newFront));
            model = glm::rotate(model, rotateAngle, glm::vec3(rotateDir));
            //front = newFront;
        }
        return model;

    }

    void drawGhostFace(Shader shader, Player player)
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

            //set light
            shader.setVec3("torch.position", player.getViewPosition());
            shader.setVec3("torch.direction", player.getFront());
            shader.setFloat("torch.cutOff", glm::cos(glm::radians(20.0f)));
            shader.setFloat("torch.constant", 1.0f);
            shader.setFloat("torch.linear", 0.09f);
            shader.setFloat("torch.quadratic", 0.032f);



            this->model = glm::mat4(1.0f);
            this->model = glm::translate(this->model, position); // translate the background so the ground is at y = 0 
            this->model = glm::scale(this->model, scale);	// it's a bit too big for our scene, so scale it down
            //this->model = glm::rotate(this->model, glm::radians(rotateAngle), glm::vec3(rotateDir));
            this->model = faceToPlayer(this->model, player.getViewPosition());

            //set MVP
            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);


            meshes[1].draw(shader);
        }
    }
    void drawPumpkin(Shader shader, int type)
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //set the light
        shader.setInt("ghostParams.type", 2); //set the fragment shader to draw the face
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

    void showUp()
    {
        //the ghost shows up
        alphaChangeThread = std::thread(&Ghost::setGhostAlpha, this, 0.6);
        alphaChangeThread.detach();
    }
    void disappear()
    {
        //the ghost disappears
        alphaChangeThread = std::thread(&Ghost::setGhostAlpha, this, 0.1);
        alphaChangeThread.detach();
    }
    void startLooming(float lowerBound, float upperBound)
    {
        isLooming = true;
        loomingThread = std::thread(&Ghost::looming, this, lowerBound, upperBound);
    }
    void stopLooming()
    {
        isLooming = false;
        loomingThread.detach();
    }
protected:
    void setGhostAlpha(float alpha)
    {
        if (!isSettingAlpha)
        {
            isSettingAlpha = true;
            Timer timer;
            float diff = alpha - ghostAlpha;
            while ((alpha - ghostAlpha > 0 && diff > 0) // 0 -> 1
                || (ghostAlpha - alpha > 0 && diff < 0))// 1 -> 0
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
    void setAmbient(float ambient)
    {
        if (!isSettingAmbient)
        {
            isSettingAmbient = true;
            Timer timer;
            float sign = std::abs(ambient - ambient) / (ambient - ambient);
            while ((ambient - ambient > 0 && sign > 0) // 0 -> 1
                || (ambient - ambient > 0 && sign < 0))// 1 -> 0
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
    };
    void looming(float lowerBound, float upperBound)
    {
        // use random device as seed
        std::random_device rd;
        std::mt19937 gen(rd());
        //randomly change ghost ambient
        std::uniform_real_distribution<float> realDist(lowerBound, upperBound);
        while (isLooming)
        {
            double randomReal = realDist(gen);
            setAmbient(randomReal);
        }
    }

    //void floating(float hight)
    //{
    //    std::random_device rd;
    //    std::mt19937 gen(rd());
    //    //randomly change ghost ambient
    //    std::uniform_real_distribution<float> realDist(0.0f, hight);
    //    while (isFloating)
    //    {
    //        double randomHeight = realDist(gen);
    //        setGhostAmbient(randomHeight);
    //    }
    //}

public:
    //animation
    void captureTFunc()
    {
        glm::vec3 originScale = scale;
        float currentTime;
        if (health > 0.0)
        {
            //scale change
            currentTime = glfwGetTime();
            while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && health > 0)
            {
                float effect = 0.1 * (std::sinf(8 * glfwGetTime() * glm::radians(45.0f)) + 1);

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
    void capture()
    {
        if (!isCapturing)
        {
            isCapturing = true;
            faceColor = glm::vec3(0.8, 0.0, 0.0);
            animationThreads.push_back(std::thread(&Ghost::captureTFunc, this));
            animationThreads[animationThreads.size() - 1].detach();
        }
    }
    void twinkling()
    {
        animationThreads.push_back(std::thread(&Ghost::twinklingTFunc, this));
        animationThreads[animationThreads.size() - 1].detach();
    }
    void twinklingTFunc()
    {
        if (!isTwinkling)
        {
            isTwinkling = true;
            //void twinkling() will create a thread to excute the twinklingTFunc()
            float startTime = glfwGetTime();
            int times = twinklingTime * 4; // object will twinkling times/2 times in a twinklingTime
            float interval = twinklingTime / times;
            int lastCnt = -1;
            while (glfwGetTime() - startTime < twinklingTime)
            {
                //how many times it twinkling
                if (int((glfwGetTime() - startTime) / interval) != lastCnt)
                {
                    //do not draw any thing
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

    //void runAway() will create a thread to excute the runAwayTFun()
    void runAwayTFun(Player& player)
    {
        Timer timer;
        //runing on the line of player-ghost line
        glm::vec3 runDir;
        float lastFloatIncre = 0;
        float currentFloatIncre = 0;
        bool isCollide = false;
        float correctAngle = 0.0f;
        // use random device as seed
        std::random_device rd;
        std::mt19937 gen(rd());
        //randomly change ghost ambient
        std::uniform_real_distribution<float> realDist(0.0f, 360.0f);

        while (isRunAway)
        {
            if (isCollide)
            {
                //correctAngle += (float)(((int)correctAngle + 10) % 360);
                correctAngle = realDist(gen);
            }
            else if (!(isCollide) && correctAngle > 0)
            {
                //correctAngle -= 10.0f;
            }
            timer.tictok();
            runDir = position - player.getViewPosition();

            runDir = glm::vec3(
                runDir.x * glm::cos(glm::radians(correctAngle)) - runDir.z * glm::sin(glm::radians(correctAngle)),
                0.0,
                runDir.x * glm::sin(glm::radians(correctAngle)) + runDir.z * glm::cos(glm::radians(correctAngle))
            ); // mov horizontally
            runDir = glm::normalize(runDir);


            position += runDir * speed * timer.getDeltaTime();
            //floating
            currentFloatIncre = 0.4 * std::sinf(3 * glfwGetTime() * glm::radians(45.0f));
            position.y += currentFloatIncre - lastFloatIncre;
            lastFloatIncre = currentFloatIncre;
            //collision dectect
            isCollide = outerCollisionCheck() || innerCollisionCheck();
        }
        isRunAway = false;
    }
    void runAway(Player& player)
    {
        if (!isRunAway)
        {
            isRunAway = true;
            animationThreads.push_back(std::thread(&Ghost::runAwayTFun, this, std::ref(player)));
            animationThreads[animationThreads.size() - 1].detach();
        }
    }




    bool outerCollisionCheck()
    {
        bool isCollide = false;
        //3 dimmensional check, only check the xz plane
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
    void setInnerBoxes(std::vector<Box> innerBoxes)
    {
        this->innerBoxes = innerBoxes;
    }
    void setOuterBoxes(std::vector<Box> outerBoxes)
    {
        this->outerBoxes = outerBoxes;
    }

    bool innerCollisionCheck()
    {
        bool isCollide = false;
        for (const Box& box : innerBoxes)
        {

            glm::vec3 maxBorder = box.maxVec + innerCollisionOffset;
            glm::vec3 minBorder = box.minVec - innerCollisionOffset;
            //see if the player already loacted inside of the inner box
            if (position.x < maxBorder.x && position.x > minBorder.x &&
                position.y < maxBorder.y && position.y > minBorder.y &&
                position.z < maxBorder.z && position.z > minBorder.z)
                //check the xz plane
            {
                //player is in inner box
                //1. which line is the closest
                //2. project the xz position on that line 
                //3. update the projected point as new position
                //collide with the inner object
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


};