#include <iostream>
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
#include <world.h>
#include <ghost.h>
#include <random>


GLsizei window_width = 1600, window_height = 1000;
//glm::vec3 lightPos = glm::vec3(2.0f, 2.0f, 2.0f);
extern MouseInfo mouseInfo;
GLFWwindow* window; //used by ./Tools::framebuffer_size_callback

bool ghostKillDetect(Ghost* ghosts, int ghostNum, Player player, float threshold = 0.5);
bool winDetect(Ghost* ghosts, int ghostNum);

int main()
{
    window = GLTools::gltCreateContext();
    Shader shader = Shader("../res/shaders/simpleShader.vertexshader", "../res/shaders/simpleShader.fragmentshader");
    



    //stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    // load models
    // -----------
    World garage("../res/model/garage/obj/old room 1.obj");
    
    Model emoji("../res/model/emoji/WinEmoji.obj");

    Player player(window, glm::vec3(0.0f, 0.0f, 0.0f));

    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //bind the callback function: resize the windowsize->callback the function
    //mouse control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //hidden and disabled the cursor
    glfwSetCursorPosCallback(window, mouse_callback); 



    //setting world params
    glm::mat4 projection = glm::perspective(glm::radians(player.getFOV()), (float)window_width / (float)window_height, 0.01f, 200.0f);
    garage.setModel(glm::mat4(1.0f));
    garage.setView(player.getView());
    garage.setProjection(projection);
    //getInnerBoxes() and getOuterBo-xes() must after calling setModel() 
    player.setInnerBoxes(garage.getInnerBoxes());
    player.setOuterBoxes(garage.getOuterBoxes());


    //can not using dynamic vector or malloc stack memory to store ghost->thread variables inside
    const int ghostNum = 4;
    Ghost ghosts[ghostNum];

    // use random number as ghosts initial position
    std::random_device rd;
    std::mt19937 gen(rd());
    //randomly change ghost ambient
    std::uniform_real_distribution<float> xRandom(
        garage.getOuterBoxes()[0].minVec.x,
        garage.getOuterBoxes()[0].maxVec.x);
    std::uniform_real_distribution<float> zRandom(
        garage.getOuterBoxes()[0].minVec.z,
        garage.getOuterBoxes()[0].maxVec.z);

    for (int i = 0; i != ghostNum; ++i)
    {
        ghosts[i].setInnerBoxes(garage.getInnerBoxes());
        ghosts[i].setOuterBoxes(garage.getOuterBoxes());
        ghosts[i].setPosition(glm::vec3(xRandom(gen), 2.5f, zRandom(gen)));
        
    }

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // render loop
    // -----------
    bool isLose = false;
    bool isWin = false;
    bool isFrozen = true;
    static float currentTime = glfwGetTime();
    bool gameOver = false;
    bool closeWindow = false;
    while (!glfwWindowShouldClose(window) && !closeWindow)
    {
        //pre-check
        if (!gameOver)
        {
            isLose = ghostKillDetect(ghosts, ghostNum, player);
            isWin = winDetect(ghosts, ghostNum);
        }
        
        if (glfwGetTime() - currentTime < 5.0)
        {
            isFrozen = true;
        }
        else if (glfwGetTime() - currentTime > 5.0 && !gameOver)
        {
            isFrozen = false;
        }
        glm::vec3 static lastPos = player.getViewPosition();
        if (isFrozen)
        {
            player.setViewPosition(lastPos);
            
        }
        else
        {
            for (int i = 0; i != ghostNum; ++i)
            {
                ghosts[i].runAway(std::ref(player));
            }
        }
        lastPos = player.getViewPosition();


        //Input
        player.processInput();
       
        //shooting
        if (!isFrozen && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        {
            //compute if the ghost be captured
            //is aiming percisely?
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
        }

        // render
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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



        //win or lose?
        if (isLose && !isWin)
        {
            gameOver = true;
            player.drawEmoji(shader, isWin);
        }
        else if (isWin && !isLose)
        {
            gameOver = true;
            player.drawEmoji(shader, isWin);
        }

        if (gameOver)
        {
            isFrozen = true;
            static float clostWindowClock = glfwGetTime();
            if (glfwGetTime() - clostWindowClock > 5.0)
            {
                closeWindow = true;
            }
        }

// -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //win or lose?
    if (isLose && !isWin)
    {
        std::cout << "YOU LOSE!" << std::endl;
    }
    else if (isWin && !isLose)
    {
        std::cout << "YOU WIN!" << std::endl;
    }


    glfwTerminate();
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
    //is ghost catch the player?
    for (int i = 0; i != ghostNum; ++i)
    {
        if (!ghosts[i].getIsCaptured())
        {
            return false;
        }
    }
    return true;
}
