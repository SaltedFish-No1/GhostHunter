#include "player.h"


float Player::jumpHeight(1.2f);
float Player::jumpSpeed(sqrt(2 * gravity * jumpHeight));

void Player::processInput()
{
	//timer.tictok(); //timer move into default actions
	processKeyEvent();     //keyboard
	processMouseMovement();//mouse movement
	defaultActions();
}
void Player::jump(float deltaTime)
{
	//can not jump in the air
	if (getPosition().y - ground < 1e-8)
	{
		onGround = false;
		velocity_y = jumpSpeed;
	}
}

void Player::fallProcessing()
{
	//processing vertical movements
	if (onGround == false)
	{
		float currentHeight = getPosition().y + velocity_y * timer.getDeltaTime() - 0.5f * gravity * timer.getDeltaTime() * timer.getDeltaTime();
		float newVelocity = velocity_y - timer.getDeltaTime() * gravity;

		if (currentHeight < ground && newVelocity < 0.0f)
		{ //landed if and only if the direction velocity is downwards and the currentHeight is zero
			viewPosition.y = ground + height;
			velocity_y = 0.0f;
			onGround = true;

		}
		else
		{
			velocity_y = newVelocity;
			viewPosition.y = currentHeight + height;
		}
	}
}

void Player::processKeyEvent()
{
	Camera::processKeyEvent();
	//add jump function
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		jump(timer.getDeltaTime());
	}
}


void Player::defaultActions()
{
	fallProcessing();
	//collision detection
	outerCollisionCheck();
	innerCollisionCheck();
	//update camera
	Camera::defaultActions();
}


