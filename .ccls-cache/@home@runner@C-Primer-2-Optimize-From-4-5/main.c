#include "menu.h"
#include <stdio.h>

int main() {
  // Define an array of menu items
  const MenuItem menuItems[] = {{"Create and save file", createAndSaveFile},
                                {"Read file", readFiles}};
  int numMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);
  runMenu(menuItems, numMenuItems);
  exitFunction();
  return 0;
}
