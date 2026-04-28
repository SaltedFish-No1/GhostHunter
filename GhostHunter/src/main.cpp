#include <iostream>
#include <app.h>
#include <tools.h>
#include <player.h>
#include <model/model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //load texture
#endif // !STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <memory>
#include <stdexcept>
#include <World.h>
#include <ghost.h>
#include <random>


bool ghostKillDetect(Ghost* ghosts, int ghostNum, Player player, float threshold = 0.5);
bool winDetect(Ghost* ghosts, int ghostNum);

enum class GamePhase
{
    Countdown,
    Playing,
    Won,
    Lost,
    Closing
};

struct GameClock
{
    GamePhase phase = GamePhase::Countdown;
    double phaseStart = 0.0;
    double elapsed(double now) const { return now - phaseStart; }
};

int main()
{
    // Make "../res/..." relative paths resolve against the staged tree
    // regardless of the cwd the user launched from. Must happen before
    // any model/shader loads.
    GLTools::anchorWorkingDirectoryToExecutable();

    App app;
    try
    {
        app.create();
    }
    catch (const std::exception& e)
    {
        std::cerr << "App init failed: " << e.what() << std::endl;
        return 1;
    }

    std::unique_ptr<Shader> shaderPtr;
    try
    {
        shaderPtr = std::make_unique<Shader>(
            "../res/shaders/simpleShader.vertexshader",
            "../res/shaders/simpleShader.fragmentshader");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shader load failed: " << e.what() << std::endl;
        app.shutdown();
        return 1;
    }
    Shader& shader = *shaderPtr;

    glEnable(GL_DEPTH_TEST);

    World garage("../res/model/garage/obj/old room 1.obj");

    Model emoji("../res/model/emoji/WinEmoji.obj");

    Player player(app.window, glm::vec3(0.0f, 0.0f, 0.0f));

    glm::mat4 projection = glm::perspective(glm::radians(player.getFOV()), (float)app.windowWidth / (float)app.windowHeight, 0.01f, 200.0f);
    garage.setModel(glm::mat4(1.0f));
    garage.setView(player.getView());
    garage.setProjection(projection);
    player.setInnerBoxes(garage.getInnerBoxes());
    player.setOuterBoxes(garage.getOuterBoxes());

    //can not using dynamic vector or malloc stack memory to store ghost->thread variables inside
    const int ghostNum = 4;
    Ghost ghosts[ghostNum];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xRandom(
        garage.getOuterBoxes()[0].minVec.x,
        garage.getOuterBoxes()[0].maxVec.x);
    std::uniform_real_distribution<float> zRandom(
        garage.getOuterBoxes()[0].minVec.z,
        garage.getOuterBoxes()[0].maxVec.z);

    for (int i = 0; i != ghostNum; ++i)
    {
        ghosts[i].setWindow(app.window);
        ghosts[i].setInnerBoxes(garage.getInnerBoxes());
        ghosts[i].setOuterBoxes(garage.getOuterBoxes());
        ghosts[i].setPosition(glm::vec3(xRandom(gen), 2.5f, zRandom(gen)));
    }

    constexpr double countdownSeconds = 5.0;
    constexpr double exitDelaySeconds = 5.0;

    GameClock clock;
    clock.phaseStart = glfwGetTime();
    double lastFrameTime = clock.phaseStart;
    glm::vec3 lastPos = player.getViewPosition();

    while (!glfwWindowShouldClose(app.window))
    {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastFrameTime);
        lastFrameTime = now;

        if (clock.phase == GamePhase::Countdown && clock.elapsed(now) >= countdownSeconds)
        {
            clock.phase = GamePhase::Playing;
            clock.phaseStart = now;
        }

        if (clock.phase == GamePhase::Playing)
        {
            if (ghostKillDetect(ghosts, ghostNum, player))
            {
                clock.phase = GamePhase::Lost;
                clock.phaseStart = now;
            }
            else if (winDetect(ghosts, ghostNum))
            {
                clock.phase = GamePhase::Won;
                clock.phaseStart = now;
            }
        }

        const bool playing = (clock.phase == GamePhase::Playing);

        if (!playing)
        {
            player.setViewPosition(lastPos);
        }
        lastPos = player.getViewPosition();

        player.processInput();

        if (playing && glfwGetMouseButton(app.window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        {
            for (int i = 0; i != ghostNum; ++i)
            {
                float cosTheta = glm::dot(glm::normalize(player.getFront()), glm::normalize(ghosts[i].getPosition() - player.getViewPosition()));
                if (cosTheta > glm::cos(glm::radians(10.0f)))
                {
                    float dist = glm::distance(player.getViewPosition(), ghosts[i].getPosition());
                    if (dist < 5)
                    {
                        ghosts[i].capture();
                    }
                }
            }
        }

        float ghostDt = playing ? dt : 0.0f;
        for (int i = 0; i != ghostNum; ++i)
        {
            ghosts[i].update(ghostDt, player);
        }

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        garage.setView(player.getView());
        garage.draw(shader, player);
        player.drawContainer(shader);
        player.drawVacuum(shader);
        player.drawCrosshair();

        for (int i = 0; i != ghostNum; ++i)
        {
            ghosts[i].setProjection(projection);
            ghosts[i].setView(player.getView());
            ghosts[i].drawGhost(shader, player);
            ghosts[i].drawGhostFace(shader, player);
        }

        if (clock.phase == GamePhase::Won || clock.phase == GamePhase::Lost)
        {
            player.drawEmoji(shader, clock.phase == GamePhase::Won);
            if (clock.elapsed(now) >= exitDelaySeconds)
            {
                clock.phase = GamePhase::Closing;
            }
        }

        glfwSwapBuffers(app.window);
        glfwPollEvents();

        if (clock.phase == GamePhase::Closing)
        {
            break;
        }
    }

    if (clock.phase == GamePhase::Won)
    {
        std::cout << "YOU WIN!" << std::endl;
    }
    else if (clock.phase == GamePhase::Lost)
    {
        std::cout << "YOU LOSE!" << std::endl;
    }

    app.shutdown();
    return 0;
}

/**
* ghosts can kill the player if they touch the player
* This function keep dectecting if there is any ghost touch the player
* If it is -> close the window and tell the player they die
* return:
*	true  -> detected ghost touch the player
*	false -> not detected yet
*/
bool ghostKillDetect(Ghost* ghosts, int ghostNum, Player player, float threshold)
{
    for (int i = 0; i < ghostNum; i++)
    {
        if (!ghosts[i].getIsCaptured() && glm::distance(ghosts[i].getPosition(), player.getViewPosition()) < threshold)
        {
            return true;
        }
    }
    return false;
}

bool winDetect(Ghost* ghosts, int ghostNum)
{
    for (int i = 0; i != ghostNum; ++i)
    {
        if (!ghosts[i].getIsCaptured())
        {
            return false;
        }
    }
    return true;
}
