/*
 * images.h
 *
 *  Created on: Jan 18, 2025
 *      Author: JakubLeb
 */
#ifndef __IMAGES_H
#define __IMAGES_H

#include <stdint.h>

/* Image dimensions definitions */
#define MAX_WIDTH 128
#define MAX_HEIGHT 64
#define ENERGY_IMG_WIDTH 55
#define ENERGY_IMG_HEIGHT 19
#define FUN_IMG_WIDTH 56
#define FUN_IMG_HEIGHT 20
#define FOOD_IMG_WIDTH 55
#define FOOD_IMG_HEIGHT 19
#define ANIMATION_IMG_WIDTH 72
#define ANIMATION_IMG_HEIGHT 64
#define EMOTION_IMG_WIDTH 10
#define EMOTION_IMG_HEIGHT 14
#define FACE_IMG_WIDTH 23
#define FACE_IMG_HEIGHT 19
#define SLIDER_IMG_WIDTH 16
#define SLIDER_IMG_HEIGHT 64
#define BUTTON_SLIDER_IMG_WIDTH 12
#define BUTTON_SLIDER_IMG_HEIGHT 9
#define PLUS_ENERGY_IMG_WIDTH 13
#define PLUS_ENERGY_IMG_HEIGHT 14
#define APPLE_IMG_WIDTH 13
#define APPLE_IMG_HEIGHT 19
#define CHOCOLATE_IMG_WIDTH 10
#define CHOCOLATE_IMG_HEIGHT 17
#define PIZZA_IMG_WIDTH 11
#define PIZZA_IMG_HEIGHT 13
#define GAME_FACE_IMG_WIDTH 30
#define GAME_FACE_IMG_HEIGHT 22
#define PLUS_FOOD_IMG_WIDTH 26
#define PLUS_FOOD_IMG_HEIGHT 18
#define PLUS_FUN_IMG_WIDTH 28
#define PLUS_FUN_IMG_HEIGHT 20

#define DEAFULT_COORDINATE_X 0
#define DEAFULT_COORDINATE_Y 0
#define ENERGY_COORDINATE_X 0
#define ENERGY_COORDINATE_Y 1
#define FUN_COORDINATE_X 0
#define FUN_COORDINATE_Y 23
#define FOOD_COORDINATE_X 0
#define FOOD_COORDINATE_Y 45
#define ANIMATION_COORDINATE_X 55
#define ANIMATION_COORDINATE_Y 0
#define EMOTION_IMG_COORDINATE_X 57
#define EMOTION_IMG_COORDINATE_Y 1
#define FACE_IMG_COORDINATE_X 68
#define FACE_IMG_COORDINATE_Y 2
#define SLIDER_IMG_COORDINATE_X 21
#define SLIDER_IMG_COORDINATE_Y 0
#define BUTTON_SLIDER_IMG_COORDINATE_X 23
#define BUTTON_SLIDER_IMG_COORDINATE_Y_MAX 2
#define BUTTON_SLIDER_IMG_COORDINATE_Y_MIN 53
#define PLUS_ENERGY_IMG_COORDINATE_X 106
#define PLUS_ENERGY_IMG_COORDINATE_Y 8
#define GAME_FACE_DEAFULT_COORDINATE_X 52
#define GAME_FACE_COORDINATE_Y 42
#define APPLE_IMG_DEFAULT_COORDINATE_X 20
#define CHOCOLATE_IMG_DEFAULT_COORDINATE_X 50
#define PIZZA_IMG_DEFAULT_COORDINATE_X 80
#define PLUS_FOOD_IMG_COORDINATE_X 101
#define PLUS_FOOD_IMG_COORDINATE_Y 5
#define PLUS_FUN_IMG_COORDINATE_X 99
#define PLUS_FUN_IMG_COORDINATE_Y 28

/* Image data declarations */
extern const uint8_t DEAFULT_IMG[];
extern const uint8_t ANIMATION_MAIN_LEFT_0[];
extern const uint8_t ANIMATION_MAIN_LEFT_1[];
extern const uint8_t ANIMATION_MAIN_LEFT_2[];
extern const uint8_t ANIMATION_MAIN_RIGHT_0[];
extern const uint8_t ANIMATION_MAIN_RIGHT_1[];
extern const uint8_t ANIMATION_MAIN_RIGHT_2[];
extern const uint8_t SLEEEP_FACE_0[];
extern const uint8_t SLEEEP_FACE_1[];
extern const uint8_t SLEEEP_FACE_2[];
extern const uint8_t ANGRY_FACE_IMG[];
extern const uint8_t START_IMG[];
extern const uint8_t MENU2_START_IMG[];
extern const uint8_t MENU3_START_IMG[];
extern const uint8_t SLIDER_IMG[];
extern const uint8_t BUTTON_SLIDER_IMG[];
extern const uint8_t PLUS_ENERGY_IMG[];
extern const uint8_t APPLE_IMG[];
extern const uint8_t CHOCOLATE_IMG[];
extern const uint8_t PIZZA_IMG[];
extern const uint8_t GAME_FACE_IMG_EMPTY[];
extern const uint8_t GAME_FACE_IMG_FULL[];
extern const uint8_t PLUS_FOOD_IMG[];
extern const uint8_t PLUS_FUN_IMG[];

extern const uint8_t* const ENERGY_ARRAY[];
extern const uint8_t* const FUN_ARRAY[];
extern const uint8_t* const FOOD_ARRAY[];
extern const uint8_t* const ZZZ_ARRAY[];
extern const uint8_t* const XXX_ARRAY[];

/* Image sizes in bytes */
#define MAX_SIZE 1024
#define ENERGY_IMG_SIZE 165
#define FOOD_IMG_SIZE 165
#define FUN_IMG_SIZE 168
#define ANIMATION_IMG_SIZE 576
#define EMOTION_IMG_SIZE 20
#define FACE_IMG_SIZE 69
#define SLIDER_IMG_SIZE 128
#define BUTTON_SLIDER_IMG_SIZE 24
#define PLUS_ENERGY_IMG_SIZE 26
#define APPLE_IMG_SIZE 39
#define CHOCOLATE_IMG_SIZE 30
#define PIZZA_IMG_SIZE 22
#define GAME_FACE_IMG_SIZE 90
#define PLUS_FOOD_SIZE 78
#define PLUS_FUN_SIZE 84

#endif /* __IMAGES_H */
