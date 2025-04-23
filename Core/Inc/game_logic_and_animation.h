/*
 * game_logic_and_animation.h
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#ifndef INC_GAME_LOGIC_AND_ANIMATION_H_
#define INC_GAME_LOGIC_AND_ANIMATION_H_

#include <stdbool.h>
#include <stdint.h>

// Structure representing Tamagotchi attributes
typedef struct {
    uint16_t food;
    uint16_t fun;
    uint16_t energy;
    uint16_t menu;
    uint16_t wakeup_sensitivity;
    uint16_t speed;
    bool minigame ;
} TamagotchiAtribute;

/// Function checking collision between two objects
bool check_collision(int obj1_x, int obj1_y, int obj1_width, int obj1_height,
                     int obj2_x, int obj2_y, int obj2_width, int obj2_height);

// Function calculating the Y coordinate based on lux value
int calculateYCoordinate(int lux_value);

// Button functionality
void Button_Functionality();

// Attribute update
void Atribute_Update();

// Functions for updating and launching the menu
void Update_menu1();
void Start_menu1();
void Update_menu2();
void Start_menu2();
void Update_menu3();
void Start_menu3();
void Menu_change_and_update();

// Global Tamagotchi attributes
extern TamagotchiAtribute g_atribute;          // Aktualne atrybuty
extern TamagotchiAtribute g_atribute_comparing; // Atrybuty do porównania

#endif /* INC_GAME_LOGIC_AND_ANIMATION_H_ */
