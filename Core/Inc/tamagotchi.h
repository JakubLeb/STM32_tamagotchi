/*
 * tamagotchi.h
 *
 *  Created on: Jan 17, 2025
 *      Author: JakubLeb
 */

#ifndef __TAMAGOTCHI_H
#define __TAMAGOTCHI_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WRCMD ,
    WRFRAME,
    WRVALUE,
    WRMENU,
    WRWAITGAME,
    ACCEPTMENU,
    ACCEPTADDAT,
    ACCEPTSUBAT,
    ACCEPTRESAT
} Communication;
const char* CommunicationStrings[] = {
    "WRCMD",
    "WRFRAME",
    "WRVALUE",
    "WRMENU",
    "WRWAITGAME",
    "ACCEPTMENU",
    "ACCEPTADDAT",
    "ACCEPTSUBAT",
    "ACCEPTRESAT"
};
typedef struct {
    uint16_t food;
    uint16_t fun;
    uint16_t energy;
    uint16_t menu;
    uint16_t wakeup_sensitivity;
    uint16_t jume_sensitivity;
    uint16_t speed;
    bool minigame;
} TamagotchiAtribute;

void Commends(const char* head, const char* instruction, const char* value);
bool value_check(const char* head, const char* instruction, const char* value);
bool isNumber(const char* value);
int handleNumber(const char* value);
bool isInRange(int value, int min, int max);

extern TamagotchiAtribute g_atribute;

#endif /* __TAMAGOTCHI_H *//* INC_TAMAGOTCHI_H_ */
