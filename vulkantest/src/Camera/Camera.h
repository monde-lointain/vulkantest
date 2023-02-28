#pragma once

#include <glm/mat4x4.hpp>

struct CameraMatrices
{
	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
	glm::mat4 vp_matrix;
};

// Bitmask for combining movement directions together into a single int
enum EMovementState
{
	NOT_MOVING = 0,
	FORWARD = 1 << 0,
	BACKWARD = 1 << 1,
	RIGHT = 1 << 2,
	LEFT = 1 << 3,
	UP = 1 << 4,
	DOWN = 1 << 5,
};

// Bitmask for controlling which types of inputs are allowed
enum EInputMode
{
	INPUT_ENABLED = 0,
	MOUSE_INPUT_DISABLED = 1 << 0,
	KEYBOARD_INPUT_DISABLED = 1 << 1,
	INPUT_DISABLED = MOUSE_INPUT_DISABLED | KEYBOARD_INPUT_DISABLED
};

typedef int MovementState;

class Camera
{
public:
	void update();

	glm::vec3 position;
	glm::vec3 rotation = glm::vec3(0.0f, -90.0f, 0.0f);

	float fov = 60.0f;
	float aspect = 16.0f / 9.0f;
	float znear = 0.1f;
	float zfar = 1000.0f;

	float speed = 0.1f;
	const float SPEED_INC = 0.025f;
	const float MAX_SPEED = 10.0f;
	const float MIN_SPEED = 0.001f;
	glm::vec3 forward = glm::vec3(0.0f);

	CameraMatrices matrices;

	EInputMode input_mode = MOUSE_INPUT_DISABLED;
	bool window_clicked = false;
	bool mouse_has_position = false;

private:
	void process_mouse_movement();
	void update_camera_vectors();
	void update_position();

	glm::vec3 right;
	glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

	MovementState move_state = NOT_MOVING;

	float mouse_sensitivity = 0.15f;

public:
	void set_view();
	void set_projection();
	void set_move_state(EMovementState state, bool set);
};

