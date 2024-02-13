#ifndef MENU_H
#define MENU_H

#include "file_operations.h"
#include <stdbool.h>
#include <stdio.h>

// Define a function pointer type for menu functions
typedef void (*MenuFunction)();

// Define a structure to hold a menu item, consisting of a description and a
// function pointer
typedef struct {
  const char *description; // Description of the menu item
  MenuFunction
      function; // Pointer to the function associated with the menu item
} MenuItem;

void printMenu(const MenuItem *menuItems, int numItems);
void menu(int choice, const MenuItem *menuItems, int numMenuItems);
int getValidChoice(int numOptions);
void runMenu(const MenuItem *menuItems, int numMenuItems);
void exitFunction();
void goodbye();
void createAndSaveFile();
void readFiles();
void inputString(const char *prompt, char *str, size_t size);

#endif /* MENU_H */