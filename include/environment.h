#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SDL2/SDL.h>

extern float cameraX;
extern float cameraY;
extern float cameraZ;

extern float rotX;
extern float rotY;

extern bool quit;

extern float delta;

extern float mouseX;
extern float mouseY;

const float mouseSensitivity = 0.3;
extern const Uint8 *keyboard_state;

extern bool isMultiplayer;
extern int mySocket;

#endif
