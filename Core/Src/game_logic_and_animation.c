/*
 * game_logic_and_animation.c
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#include "game_logic_and_animation.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include "main.h"
#include "images.h"
#include "bh1750.h"
#include "ssd1309.h"

// Definition of initial Tamagotchi attribute values
TamagotchiAtribute g_atribute = {
    .food = 3,
    .fun = 2,
    .energy = 5,
    .menu = 1,
    .wakeup_sensitivity = 300,
    .minigame = false
};
// Structure for comparing changes in Tamagotchi attributes
TamagotchiAtribute g_atribute_comparing = {
    .food = 3,
    .fun = 2,
    .energy = 5,
    .menu = 1,
    .wakeup_sensitivity = 300,
    .minigame = false
};

// Variables storing positions of game elements on the OLED screen
volatile uint32_t game_face_coordinate_x = GAME_FACE_DEAFULT_COORDINATE_X;
volatile uint32_t current_game_face_coordinate_x = GAME_FACE_DEAFULT_COORDINATE_X;
volatile uint32_t current_apple_coordinate_y = 0;
volatile uint32_t current_pizza_coordinate_y = 0;
volatile uint32_t current_chocolate_coordinate_y = 0;
volatile uint32_t last_button_y = BUTTON_SLIDER_IMG_COORDINATE_Y_MIN;

// Timing variables for controlling animations of individual elements
volatile uint32_t last_plus_food_animation_update = 0;
volatile uint32_t last_fill_rectangle_update = 0;
volatile uint32_t last_button_slider_update = 0;
volatile uint32_t last_update_main_time = 0;
volatile uint32_t last_face_update = 0;
volatile uint32_t last_noise_update = 0;
volatile uint32_t last_game_face_update = 0;
volatile uint32_t last_button_update = 0;
volatile uint32_t last_energy_gain_update = 0;
volatile uint32_t last_energy_gain_animation_update = 0;
volatile uint32_t last_pizza_animation_update = 0;
volatile uint32_t last_apple_animation_update = 0;
volatile uint32_t last_chocolate_animation_update = 0;
volatile uint32_t last_update_screen = 0;

// Game state and animation variables
volatile uint8_t current_sleep_noise = 0;
volatile uint8_t current_angry_noise = 0;
volatile uint8_t up_energy_gain_animation = 0;
volatile uint8_t loop_animation = 0;
volatile uint32_t food_count = 0;
volatile uint8_t current_face = 0;
volatile bool angry = false;
volatile bool sleep_interupt = true;
volatile bool eat = false;
volatile bool can_animate_energy_gain = false;
volatile bool game_start = false;

// Function checking collision between two objects on the screen
bool check_collision(int obj1_x, int obj1_y, int obj1_width, int obj1_height,
                     int obj2_x, int obj2_y, int obj2_width, int obj2_height) {
    return (obj1_x + obj1_width >= obj2_x &&
            obj1_x <= obj2_x + obj2_width &&
            obj1_y + obj1_height >= obj2_y &&
            obj1_y <= obj2_y + obj2_height);
}

// Function converting light intensity value to Y coordinate on the screen
int calculateYCoordinate(int lux_value) {
    if (lux_value < 0) lux_value = 0;
    if (lux_value > g_atribute.wakeup_sensitivity) lux_value = g_atribute.wakeup_sensitivity;

    int y = BUTTON_SLIDER_IMG_COORDINATE_Y_MIN + (BUTTON_SLIDER_IMG_COORDINATE_Y_MAX - BUTTON_SLIDER_IMG_COORDINATE_Y_MIN) * lux_value  / g_atribute.wakeup_sensitivity;

    return y;
}

// Function handling button operations (yellow - change menu, blue - start game)
void Button_Functionality() {
    static bool yellow_button_active = false;
    static bool blue_button_active = false;

    // Handling yellow button (menu change)
    if (HAL_GPIO_ReadPin(GPIOC, YellowBt_Pin) == GPIO_PIN_SET) {
        if (!yellow_button_active) {
            yellow_button_active = true;
            g_atribute.menu++;
            if (g_atribute.menu >= 4){
                g_atribute.menu = 1;
            }
        }
    } else {
        if (yellow_button_active) {
            yellow_button_active = false;
        }
    }

    // Handling blue button (start game in menu 3)
    if (HAL_GPIO_ReadPin(GPIOC, BlueBt_Pin) == GPIO_PIN_SET) {
        if (!blue_button_active && g_atribute.menu == 3) {
            game_start = true;
            OLED_Fill(WHITE);
            blue_button_active = true;
        }
    } else {
        if (blue_button_active && g_atribute.menu == 3) {
            blue_button_active = false;
        }
    }
}

// Function updating the display of Tamagotchi attributes on the screen
void Atribute_Update(){
    // Check if any attribute has changed
    if(g_atribute.energy != g_atribute_comparing.energy ||
       g_atribute.fun != g_atribute_comparing.fun ||
       g_atribute.food != g_atribute_comparing.food){

        // Update energy indicator
        OLED_FillRectangle(0,0,60,1,true);
        OLED_PrintImageFromCoords(ENERGY_ARRAY[g_atribute.energy],
            ENERGY_COORDINATE_X, ENERGY_COORDINATE_Y,
            ENERGY_IMG_WIDTH, ENERGY_IMG_HEIGHT);

        // Update fun indicator
        OLED_FillRectangle(0,20,60,3,true);
        OLED_PrintImageFromCoords(FUN_ARRAY[g_atribute.fun],
            FUN_COORDINATE_X, FUN_COORDINATE_Y,
            FUN_IMG_WIDTH, FUN_IMG_HEIGHT);

        // Update food indicator
        OLED_FillRectangle(0,43,60,3,true);
        OLED_PrintImageFromCoords(FOOD_ARRAY[g_atribute.food],
            FOOD_COORDINATE_X, FOOD_COORDINATE_Y,
            FOOD_IMG_WIDTH, FOOD_IMG_HEIGHT);

        // Update comparison values
        g_atribute_comparing.energy = g_atribute.energy;
        g_atribute_comparing.food = g_atribute.food;
        g_atribute_comparing.fun = g_atribute.fun;
    }
}

// Function updating animation in menu 1
void Update_menu1() {
    static uint8_t repeat_count = 0; // Counter for repetitions in current direction
    static bool is_left = true;     // Flag indicating current animation direction

    uint32_t current_time = HAL_GetTick();

    if (current_time - last_update_main_time >= 400) {
        // Arrays storing images for left and right animations
        const uint8_t* left_animation_frames[] = {
            ANIMATION_MAIN_LEFT_0, ANIMATION_MAIN_LEFT_1, ANIMATION_MAIN_LEFT_2, ANIMATION_MAIN_LEFT_1
        };

        const uint8_t* right_animation_frames[] = {
            ANIMATION_MAIN_RIGHT_0, ANIMATION_MAIN_RIGHT_1, ANIMATION_MAIN_RIGHT_2, ANIMATION_MAIN_RIGHT_1
        };

        // Select appropriate array and calculate frame index
        const uint8_t** selected_frames = is_left ? left_animation_frames : right_animation_frames;
        uint8_t frame_index = loop_animation % 4;

        // Draw selected frame
        OLED_PrintImageFromCoords(selected_frames[frame_index], ANIMATION_COORDINATE_X, ANIMATION_COORDINATE_Y, ANIMATION_IMG_WIDTH, ANIMATION_IMG_HEIGHT);

        // Increment animation counter
        loop_animation++;

        // If one full animation is completed (4 frames)
        if (frame_index == 3) {
            repeat_count++;

            // After three repetitions change animation direction
            if (repeat_count >= 3) {
                is_left = !is_left;  // Change direction
                repeat_count = 0;   // Reset repetition counter
            }
        }

        // Update time
        last_update_main_time = current_time;
    }
}

// Function initializing menu 1 (main screen)
void Start_menu1(){
    // Resetting animation variables
    loop_animation = 0;
    last_update_main_time = 0;

    // Clearing screen and drawing interface
    OLED_Clear();

    // Drawing status indicators
    OLED_FillRectangle(0,0,60,1,true);  // Top line
    OLED_PrintImageFromCoords(ENERGY_ARRAY[g_atribute.energy],
        ENERGY_COORDINATE_X, ENERGY_COORDINATE_Y,
        ENERGY_IMG_WIDTH, ENERGY_IMG_HEIGHT);  // Energy indicator

    OLED_FillRectangle(0,20,60,3,true);  // Middle line
    OLED_PrintImageFromCoords(FUN_ARRAY[g_atribute.fun],
        FUN_COORDINATE_X, FUN_COORDINATE_Y,
        FUN_IMG_WIDTH, FUN_IMG_HEIGHT);  // Fun indicator

    OLED_FillRectangle(0,43,60,3,true);  // Bottom line
    OLED_PrintImageFromCoords(FOOD_ARRAY[g_atribute.food],
        FOOD_COORDINATE_X, FOOD_COORDINATE_Y,
        FOOD_IMG_WIDTH, FOOD_IMG_HEIGHT);  // Food indicator

    // Drawing initial animation
    OLED_PrintImageFromCoords(ANIMATION_MAIN_LEFT_0,
        ANIMATION_COORDINATE_X, ANIMATION_COORDINATE_Y,
        ANIMATION_IMG_WIDTH, ANIMATION_IMG_HEIGHT);
}

// Function initializing menu 2 (sleep mode)
void Start_menu2(){
    // Resetting all flags and counters
    angry = false;
    sleep_interupt = true;
    can_animate_energy_gain = false;
    last_face_update = 0;
    current_face = 0;
    last_noise_update = 0;
    current_angry_noise = 0;
    last_button_slider_update = 0;
    last_fill_rectangle_update = 0;
    last_energy_gain_update = 0;
    last_energy_gain_animation_update = 0;
    up_energy_gain_animation = 0;

    // Screen initialization
    OLED_Clear();  // Clearing screen

    // Drawing interface elements
    OLED_PrintImageFromCoords(MENU2_START_IMG,
        DEAFULT_COORDINATE_X,
        DEAFULT_COORDINATE_Y,
        MAX_WIDTH,
        MAX_HEIGHT);  // Menu 2 background

    // Drawing initial sleeping Tamagotchi face
    OLED_PrintImageFromCoords(SLEEEP_FACE_0,
        FACE_IMG_COORDINATE_X,
        FACE_IMG_COORDINATE_Y,
        FACE_IMG_WIDTH,
        FACE_IMG_HEIGHT);

    // Drawing initial "ZZZ" animation
    OLED_PrintSmallImageFromCoords(ZZZ_ARRAY[9],
        EMOTION_IMG_COORDINATE_X,
        EMOTION_IMG_COORDINATE_Y,
        EMOTION_IMG_WIDTH,
        EMOTION_IMG_HEIGHT);

    // Drawing slider and its button
    OLED_PrintImageFromCoords(SLIDER_IMG,
        SLIDER_IMG_COORDINATE_X,
        SLIDER_IMG_COORDINATE_Y,
        SLIDER_IMG_WIDTH,
        SLIDER_IMG_HEIGHT);
    OLED_PrintSmallImageFromCoords(BUTTON_SLIDER_IMG,
        BUTTON_SLIDER_IMG_COORDINATE_X,
        BUTTON_SLIDER_IMG_COORDINATE_Y_MIN,
        BUTTON_SLIDER_IMG_WIDTH,
        BUTTON_SLIDER_IMG_HEIGHT);
}

// Function updating menu 2 (sleep mode)
void Update_menu2(){
    uint32_t current_time = HAL_GetTick();

    // Face animation update (every 500ms)
    if (current_time - last_face_update >= 500) {
        if (angry){
            // Display angry face when Tamagotchi is angry
            OLED_PrintImageFromCoords(ANGRY_FACE_IMG,
                FACE_IMG_COORDINATE_X,
                FACE_IMG_COORDINATE_Y,
                FACE_IMG_WIDTH,
                FACE_IMG_HEIGHT);
        } else {
            // Sleeping face animation
            switch(current_face) {
                case 0:
                    OLED_PrintImageFromCoords(SLEEEP_FACE_0,
                        FACE_IMG_COORDINATE_X,
                        FACE_IMG_COORDINATE_Y,
                        FACE_IMG_WIDTH,
                        FACE_IMG_HEIGHT);
                    break;
                case 1:
                case 3:
                    OLED_PrintImageFromCoords(SLEEEP_FACE_1,
                        FACE_IMG_COORDINATE_X,
                        FACE_IMG_COORDINATE_Y,
                        FACE_IMG_WIDTH,
                        FACE_IMG_HEIGHT);
                    break;
                case 2:
                    OLED_PrintImageFromCoords(SLEEEP_FACE_2,
                        FACE_IMG_COORDINATE_X,
                        FACE_IMG_COORDINATE_Y,
                        FACE_IMG_WIDTH,
                        FACE_IMG_HEIGHT);
                    break;
            }
            current_face = (current_face + 1) % 4;  // Move to next animation frame
        }
        last_face_update = current_time;
    }

    // Sound animation update (every 300ms)
    if (current_time - last_noise_update >= 300) {
        if (angry){
            // Anger sound animation (XXX)
            if (current_angry_noise < 6) {
                OLED_PrintSmallImageFromCoords(XXX_ARRAY[current_angry_noise],
                    EMOTION_IMG_COORDINATE_X,
                    EMOTION_IMG_COORDINATE_Y,
                    EMOTION_IMG_WIDTH,
                    EMOTION_IMG_HEIGHT);
                current_angry_noise = (current_angry_noise + 1) % 6;
            }
        } else {
            // Sleep sound animation (ZZZ)
            if (current_sleep_noise < 10) {
                OLED_PrintSmallImageFromCoords(ZZZ_ARRAY[current_sleep_noise],
                    EMOTION_IMG_COORDINATE_X,
                    EMOTION_IMG_COORDINATE_Y,
                    EMOTION_IMG_WIDTH,
                    EMOTION_IMG_HEIGHT);
                current_sleep_noise = (current_sleep_noise + 1) % 10;
            }
        }
        last_noise_update = current_time;
    }

    // Slider position update based on light sensor (every 100ms)
    if (current_time - last_button_slider_update >= 100) {
        int current_y = calculateYCoordinate(BH1750_ReadLux());

        // Refreshing slider background
        if (current_time - last_fill_rectangle_update >= 100){
            if (current_y != last_button_y){
                OLED_FillRectangle(22, 1 ,14,62,true);
            }
        }

        // Drawing slider button
        OLED_PrintSmallImageFromCoords(BUTTON_SLIDER_IMG,
            BUTTON_SLIDER_IMG_COORDINATE_X,
            current_y,
            BUTTON_SLIDER_IMG_WIDTH,
            BUTTON_SLIDER_IMG_HEIGHT);

        // Checking slider position and setting anger state
        if (current_y<20){
            angry = true;
            sleep_interupt = true;
        } else {
            angry = false;
        }

        last_button_slider_update = current_time;
        last_button_y = current_y;
    }

    // Energy update during sleep (every 5000ms)
    if (current_time - last_energy_gain_update >= 5000){
        if (sleep_interupt == false){
            // If sleep is not interrupted, increase energy and decrease other attributes
            can_animate_energy_gain = true;
            if(g_atribute.food-1>=0){
                g_atribute.food--;  // Decrease food level
            }
            if(g_atribute.fun-1>=0){
                g_atribute.fun--;   // Decrease fun level
            }
            if(g_atribute.energy+1<=5){
                g_atribute.energy++;  // Increase energy level
            }
        } else {
            can_animate_energy_gain = false;
            sleep_interupt = false;
        }
        last_energy_gain_update = current_time;
    }

    // Energy gain animation (every 250ms)
    if (current_time - last_energy_gain_animation_update>=250){
        if (can_animate_energy_gain == true){
            OLED_PrintSmallImageFromCoords(PLUS_ENERGY_IMG,
                PLUS_ENERGY_IMG_COORDINATE_X,
                PLUS_ENERGY_IMG_COORDINATE_Y-up_energy_gain_animation,
                PLUS_ENERGY_IMG_WIDTH,
                PLUS_ENERGY_IMG_HEIGHT);

            up_energy_gain_animation++;

            // Clearing previous position of symbol
            OLED_FillRectangle(PLUS_ENERGY_IMG_COORDINATE_X,
                (PLUS_ENERGY_IMG_COORDINATE_Y - up_energy_gain_animation + PLUS_ENERGY_IMG_HEIGHT),
                PLUS_ENERGY_IMG_WIDTH,1,true);

            // Ending animation after reaching specified height
            if(up_energy_gain_animation==7){
                OLED_FillRectangle(PLUS_ENERGY_IMG_COORDINATE_X,
                    (PLUS_ENERGY_IMG_COORDINATE_Y - up_energy_gain_animation),
                    PLUS_ENERGY_IMG_WIDTH,PLUS_ENERGY_IMG_HEIGHT,true);
                can_animate_energy_gain= false;
                up_energy_gain_animation=0;
            }
        }
        last_energy_gain_animation_update = current_time;
    }
}

// Function initializing menu 3 (mini-game)
void Start_menu3(){
    // Resetting all game variables
    game_start = false;
    eat = false;
    food_count = 0;
    game_face_coordinate_x = GAME_FACE_DEAFULT_COORDINATE_X;

    // Resetting all animation timers
    last_game_face_update = 0;
    last_button_update = 0;
    current_apple_coordinate_y = 0;
    last_apple_animation_update = 0;
    last_chocolate_animation_update = 0;
    last_pizza_animation_update = 0;

    // Clearing screen and drawing initial interface
    OLED_Clear();
    OLED_PrintImageFromCoords(MENU3_START_IMG,
        DEAFULT_COORDINATE_X,
        DEAFULT_COORDINATE_Y,
        MAX_WIDTH,
        MAX_HEIGHT);
}

void Update_menu3() {
    if (game_start == true) {
        uint32_t current_time = HAL_GetTick();
        OLED_FillRectangle((PLUS_FUN_IMG_COORDINATE_X-1),0,1,64,false);

        // Updating game face - switching between empty and full when eating
        if (current_time - last_game_face_update >= 100) {
            if (eat) {
                OLED_PrintSmallImageFromCoords(GAME_FACE_IMG_FULL,
                    game_face_coordinate_x,
                    GAME_FACE_COORDINATE_Y,
                    GAME_FACE_IMG_WIDTH,
                    GAME_FACE_IMG_HEIGHT);
                eat = false;
            } else {
                OLED_PrintSmallImageFromCoords(GAME_FACE_IMG_EMPTY,
                    game_face_coordinate_x,
                    GAME_FACE_COORDINATE_Y,
                    GAME_FACE_IMG_WIDTH,
                    GAME_FACE_IMG_HEIGHT);
            }
            last_game_face_update = current_time;
        }

        // Button handling - green moves left, red moves right
        if (current_time - last_button_update >= 100) {
            if (HAL_GPIO_ReadPin(GPIOC, GreenBt_Pin) == GPIO_PIN_SET) {
                game_face_coordinate_x--;
                if (game_face_coordinate_x < 0) {
                    game_face_coordinate_x = 0;
                }
                OLED_FillRectangle((game_face_coordinate_x+GAME_FACE_IMG_WIDTH), GAME_FACE_COORDINATE_Y, 1, GAME_FACE_COORDINATE_Y - GAME_FACE_IMG_HEIGHT,true);
            }

            if (HAL_GPIO_ReadPin(GPIOC, RedBt_Pin) == GPIO_PIN_SET) {
                game_face_coordinate_x++;
                if (game_face_coordinate_x > (MAX_WIDTH - GAME_FACE_IMG_WIDTH - PLUS_FUN_IMG_WIDTH - 2)) {
                    game_face_coordinate_x = (MAX_WIDTH - GAME_FACE_IMG_WIDTH - PLUS_FUN_IMG_WIDTH - 2);
                }
                OLED_FillRectangle(game_face_coordinate_x-1, GAME_FACE_COORDINATE_Y, 1, GAME_FACE_COORDINATE_Y - GAME_FACE_IMG_HEIGHT,true);
            }
            last_button_update = current_time;
        }

        // Falling apple animation
        if (current_time - last_apple_animation_update >= 100) {
            current_apple_coordinate_y++;
            OLED_PrintSmallImageFromCoords(APPLE_IMG,
                APPLE_IMG_DEFAULT_COORDINATE_X,
                current_apple_coordinate_y,
                APPLE_IMG_WIDTH,
                APPLE_IMG_HEIGHT);
            OLED_FillRectangle(APPLE_IMG_DEFAULT_COORDINATE_X,
                    current_apple_coordinate_y - 1,
                    APPLE_IMG_WIDTH, 1,true);

            // Reset apple position when it reaches bottom and check collision
            if (current_apple_coordinate_y > (MAX_HEIGHT - APPLE_IMG_HEIGHT - 1)) {
                current_apple_coordinate_y = 0;
                OLED_FillRectangle(APPLE_IMG_DEFAULT_COORDINATE_X, 0,
                        APPLE_IMG_WIDTH, 64,true);
            }
            if (check_collision(APPLE_IMG_DEFAULT_COORDINATE_X, current_apple_coordinate_y, APPLE_IMG_WIDTH, APPLE_IMG_HEIGHT,
                    game_face_coordinate_x+4, GAME_FACE_COORDINATE_Y, GAME_FACE_IMG_WIDTH-4, GAME_FACE_IMG_HEIGHT)) {
                current_apple_coordinate_y = 0;
                OLED_FillRectangle(APPLE_IMG_DEFAULT_COORDINATE_X, 0,
                    APPLE_IMG_WIDTH, 64,true);
                eat = true;
                food_count++;
            }
            last_apple_animation_update = current_time;
        }

        // Falling chocolate animation - similar logic to apple but slower falling
        if (current_time - last_chocolate_animation_update >= 300) {
            current_chocolate_coordinate_y++;
            OLED_PrintSmallImageFromCoords(CHOCOLATE_IMG,
                CHOCOLATE_IMG_DEFAULT_COORDINATE_X,
                current_chocolate_coordinate_y,
                CHOCOLATE_IMG_WIDTH,
                CHOCOLATE_IMG_HEIGHT);
            OLED_FillRectangle(CHOCOLATE_IMG_DEFAULT_COORDINATE_X,
                    current_chocolate_coordinate_y - 1,
                    CHOCOLATE_IMG_WIDTH, 1,true);

            if (current_chocolate_coordinate_y > (MAX_HEIGHT - CHOCOLATE_IMG_HEIGHT - 1)) {
                current_chocolate_coordinate_y = 0;
                OLED_FillRectangle(CHOCOLATE_IMG_DEFAULT_COORDINATE_X, 0,
                        CHOCOLATE_IMG_WIDTH, 64,true);
            }
            if (check_collision(CHOCOLATE_IMG_DEFAULT_COORDINATE_X, current_chocolate_coordinate_y, CHOCOLATE_IMG_WIDTH, CHOCOLATE_IMG_HEIGHT,
                    game_face_coordinate_x+4, GAME_FACE_COORDINATE_Y, GAME_FACE_IMG_WIDTH-4, GAME_FACE_IMG_HEIGHT)) {
                current_chocolate_coordinate_y = 0;
                OLED_FillRectangle(CHOCOLATE_IMG_DEFAULT_COORDINATE_X, 0,
                    CHOCOLATE_IMG_WIDTH, 64,true);
                eat = true;
                food_count++;
            }
            last_chocolate_animation_update = current_time;
        }

        // Falling pizza animation - medium falling speed
        if (current_time - last_pizza_animation_update >= 200) {
            current_pizza_coordinate_y++;
            OLED_PrintSmallImageFromCoords(PIZZA_IMG,
                PIZZA_IMG_DEFAULT_COORDINATE_X,
                current_pizza_coordinate_y,
                PIZZA_IMG_WIDTH,
                PIZZA_IMG_HEIGHT);
            OLED_FillRectangle(PIZZA_IMG_DEFAULT_COORDINATE_X,
                    current_pizza_coordinate_y - 1,
                    PIZZA_IMG_WIDTH, 1,true);

            if (current_pizza_coordinate_y > (MAX_HEIGHT - PIZZA_IMG_HEIGHT - 1)) {
                current_pizza_coordinate_y = 0;
                OLED_FillRectangle(PIZZA_IMG_DEFAULT_COORDINATE_X, 0,
                        PIZZA_IMG_WIDTH, 64,true);
            }
            if (check_collision(PIZZA_IMG_DEFAULT_COORDINATE_X, current_pizza_coordinate_y, PIZZA_IMG_WIDTH, PIZZA_IMG_HEIGHT,
                    game_face_coordinate_x+4, GAME_FACE_COORDINATE_Y, GAME_FACE_IMG_WIDTH-4, GAME_FACE_IMG_HEIGHT)) {
                current_pizza_coordinate_y = 0;
                OLED_FillRectangle(PIZZA_IMG_DEFAULT_COORDINATE_X, 0,
                    PIZZA_IMG_WIDTH, 64,true);
                eat = true;
                food_count++;
            }
            last_pizza_animation_update = current_time;
        }

        // Displaying bonus after collecting 5 food items
        static uint32_t display_start_time = 0;

        if (food_count >= 5) {
            if (display_start_time == 0) {
                display_start_time = current_time;
                OLED_PrintSmallImageFromCoords(PLUS_FOOD_IMG,
                        PLUS_FOOD_IMG_COORDINATE_X, PLUS_FOOD_IMG_COORDINATE_Y,
                        PLUS_FOOD_IMG_WIDTH, PLUS_FOOD_IMG_HEIGHT);
                OLED_PrintSmallImageFromCoords(PLUS_FUN_IMG,
                                        PLUS_FUN_IMG_COORDINATE_X, PLUS_FUN_IMG_COORDINATE_Y,
                                        PLUS_FUN_IMG_WIDTH, PLUS_FUN_IMG_HEIGHT);
            } else if (current_time - display_start_time >= 2000) {
                OLED_FillRectangle(PLUS_FUN_IMG_COORDINATE_X, PLUS_FOOD_IMG_COORDINATE_Y,
                                    PLUS_FUN_IMG_WIDTH, PLUS_FOOD_IMG_HEIGHT+5+PLUS_FUN_IMG_HEIGHT,true);
                food_count = 0;  // Resetting food counter
                // Updating character attributes
                if(g_atribute.food+1<=5){
                    g_atribute.food++;
                }
                if(g_atribute.fun+1<=5){
                    g_atribute.fun++;
                }
                if(g_atribute.energy-1>=0){
                    g_atribute.energy--;
                }
                display_start_time = 0;  // Resetting display timer
            }
        }
    }
}

// Function managing menu change and its update
void Menu_change_and_update(){
    // Check if menu has changed
    if (g_atribute.menu != g_atribute_comparing.menu){
        switch(g_atribute.menu){
            case 1:
                Start_menu1();
                break;
            case 2:
                Start_menu2();
                break;
            case 3:
                Start_menu3();
                break;
        }
        g_atribute_comparing.menu=g_atribute.menu;
    }

    // Setting minigame flag
    if(g_atribute.menu ==2 || g_atribute.menu==3)g_atribute.minigame=true;
    else g_atribute.minigame=false;

    // Updating appropriate menu
    switch(g_atribute.menu){
        case 1:
            Atribute_Update();
            Update_menu1();
            break;
        case 2:
            Update_menu2();
            break;
        case 3:
            Update_menu3();
            break;
    }
}
