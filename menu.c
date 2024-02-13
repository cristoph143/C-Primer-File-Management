#include "menu.h"
#include "file_operations.h"
#include <stdio.h>
#include <stdlib.h>

void printMenu(const MenuItem *menuItems, int numItems) {
  printf("------------------------------------------------------------\n");
  printf("Menu\n");
  printf("------------------------------------------------------------\n");

  for (int i = 0; i < numItems; i++) {
    printf("%d. %s\n", i + 1, menuItems[i].description);
  }

  printf("0. Exit\n");
  printf("------------------------------------------------------------\n");
  printf("Enter Choice: ");
}

// Implementation of the menu function
void menu(int choice, const MenuItem *menuItems, int numMenuItems) {
  // Check if the choice is within the range of menu items
  if (choice >= 0 && choice < numMenuItems) {
    // Execute the function associated with the chosen menu item
    menuItems[choice].function();
    // Prompt the user to press a key to continue
    printf("Press any key to continue...");
    getchar(); // Wait for user input
    // Clear the newline character from the input buffer
    while (getchar() != '\n')
      ;
  } else {
    // Print an error message for an invalid choice
    printf("Invalid choice. Please enter a valid option.\n");
  }
}

void exitFunction() { printf("Exiting...\n"); }

/**
 * @brief Gets a valid choice from the user.
 *
 * This function prompts the user for input until a valid choice is entered.
 *
 * @param numOptions The number of options available.
 * @return The valid choice entered by the user.
 */
int getValidChoice(int numOptions) {
  int choice;
  // printf("Enter Choice: ");
  while (scanf("%d", &choice) != 1 || choice < 0 || choice > numOptions) {
    // Clear input buffer
    while (getchar() != '\n')
      ;
    printf("Invalid choice. Please enter a valid option: \n");
  }
  return choice;
}

void runMenu(const MenuItem *menuItems, int numMenuItems) {
  int choice = -1;

  while (choice != 0) {
    // Clear the screen
    system("cls || clear");
    printMenu(menuItems, numMenuItems);
    choice = getValidChoice(numMenuItems);
    if (choice != 0) {
      menu(choice - 1, menuItems, numMenuItems);
    }
  }
}

void goodbye() { printf("Goodbye\n"); }

// Step 1: Create and Save File
void createAndSaveFile() {
  // Write Process
  HeaderInfo header;
  header.fileID = generateRandomFileID();
  header.readCount = 0;

  char data[MAX_DATA_SIZE];
  printf("Enter data: ");
  scanf(" %[^\n]", data);

  // Encrypt data for security
  encryptData(data);

  // Write data to file
  writeToFile(header, data);
}

// Step 2: Read File
void readFiles() {
  // Read Process
  char filename[100];
  printf("Enter filename to read: ");
  scanf("%s", filename);

  // Read data from file
  readFromFile(filename);

  // decryptData(data);
}

void inputString(const char *prompt, char *str, size_t size) {
  printf("%s", prompt);
  scanf("%s", str);
}
