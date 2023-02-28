#include "Camera.h"

#include <iostream>

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "../Utils/string_ops.h"

void Camera::set_move_state(EMovementState state, bool set)
{
    if (set)
    {
        move_state |= state;
    }
    else
    {
        move_state &= ~state;
    }
}

void Camera::set_projection()
{
    projection_matrix =
        glm::perspective(glm::radians(fov), aspect, znear, zfar);

    // Correction matrix for Vulkan's coordinate system (Vulkan has flipped y
    // and half z)
    const glm::mat4 clip(1.0f,  0.0f, 0.0f, 0.0f, 
                         0.0f, -1.0f, 0.0f, 0.0f, 
                         0.0f,  0.0f, 0.5f, 0.0f, 
                         0.0f,  0.0f, 0.5f, 1.0f);

    projection_matrix = clip * projection_matrix;
}

void Camera::set_view()
{
    const glm::vec3 target = position + forward;
    view_matrix = glm::lookAt(position, target, world_up);

    // Concatenate view and projection matrices
    vp_matrix = projection_matrix * view_matrix;
}

void Camera::update_camera_vectors()
{
    const float pitch = glm::radians(rotation.x);
    const float yaw = glm::radians(rotation.y);
    const glm::vec3 front(
        cosf(yaw) * cosf(pitch),
        sinf(pitch),
        sinf(yaw) * cosf(pitch)
    );
    forward = glm::normalize(front);

    right = glm::normalize(glm::cross(forward, world_up));
}

void Camera::process_mouse_movement()
{
    if (input_mode & MOUSE_INPUT_DISABLED)
    {
        update_camera_vectors();
        return;
    }

    // Get the x and y coordinates of the mouse
    int x, y;

    SDL_GetRelativeMouseState(&x, &y);

    // Set the mouse focus manually when the window is first clicked
    if (window_clicked && !mouse_has_position)
    {
        SDL_GetRelativeMouseState(&x, &y);
        x = 0;
        y = 0;
        mouse_has_position = true;
    }

    // Adjust to be relative to the center of the window
    const float xoffset = (float)x * mouse_sensitivity;
    const float yoffset = (float)y * mouse_sensitivity;

    rotation.y += xoffset;
    // Flip to correct for inverted y coordinates
    rotation.x -= yoffset;

    // Clamp the pitch so we can't rotate the camera backwards
    rotation.x = glm::clamp(rotation.x, -89.0f, 89.0f);

    update_camera_vectors();
}

void Camera::update_position()
{
    if (input_mode & KEYBOARD_INPUT_DISABLED)
    {
        return;
    }

    glm::vec3 move_dir(0.0f);

    if (move_state & FORWARD)
    {
        move_dir += forward;
    }
    if (move_state & BACKWARD)
    {
        move_dir -= forward;
    }
    if (move_state & RIGHT)
    {
        move_dir += right;
    }
    if (move_state & LEFT)
    {
        move_dir -= right;
    }
    if (move_state & UP)
    {
        move_dir += world_up;
    }
    if (move_state & DOWN)
    {
        move_dir -= world_up;
    }

    // Our velocity is zero if we press two buttons going in opposite directions
    // (left+right/up+down/etc.) and aren't pressing any other buttons. This
    // will cause an error in the direction computations so we need to early out
    // if that's the case
    if (move_dir == glm::vec3(0.0f))
    {
        return;
    }

    move_dir = glm::normalize(move_dir);

    position += move_dir * speed;
}

void Camera::update()
{
    process_mouse_movement();
    update_position();
    set_view();
}
