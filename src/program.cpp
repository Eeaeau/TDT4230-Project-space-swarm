// Local headers
#include "program.hpp"
#include "utilities/window.hpp"
#include "gamelogic.h"
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/System/Time.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utilities/timeutils.h>


void runProgram(GLFWwindow* window, CommandLineOptions options)
{
    // Enable depth (Z) buffer (accept "closest" fragment)
  /*  glEnable(GL_DEPTH_TEST);
    ;*/

    // Configure miscellaneous OpenGL settings
    //glDisable(GL_CULL_FACE);
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    
    // Enables Multisampling
    glEnable(GL_MULTISAMPLE);

    // Enable built-in dithering
    glEnable(GL_DITHER);

    // Enables the Depth Buffer
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);

    //// Enable transparency
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Set default colour after clearing the colour buffer
    //glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	initGame(window, options);

    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
	    // Clear colour and depth buffers
        
	    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        updateFrame(window);
        renderFrame(window);





        // Handle other events
        glfwPollEvents();
        handleKeyboardInput(window);

        // Flip buffers
        glfwSwapBuffers(window);
    }
    deleteGame();
}


void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
