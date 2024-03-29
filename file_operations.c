#include "file_operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createDirectory(const char *path) {
  char command[256];
  sprintf(command, "mkdir -p %s", path);
  system(command);
}

char *constructFilePath(const char *filename) {
  char *filepath = malloc(strlen(FILE_FOLDER) + strlen(filename) + 1);
  if (filepath == NULL) {
    perror("Memory allocation failed");
    exit(EXIT_FAILURE);
  }
  strcpy(filepath, FILE_FOLDER);
  strcat(filepath, filename);
  return filepath;
}

void writeToFile(const HeaderInfo header, const char *data) {
  // Calculate CRC
  unsigned char crc = calculateCRC(header, data);

  // Create folder if it doesn't exist
  createDirectory(FILE_FOLDER);

  // Construct file name
  char filename[100];
  snprintf(filename, sizeof(filename), "%d_%d.txt", header.fileID,
           header.readCount);

  // Create file path
  char *filepath = constructFilePath(filename);

  // Write header, data, and CRC to file
  writeHeaderAndDataToFile(header, data, filepath);
  // Create duplicates
  for (int i = 0; i < NUM_DUPLICATES; i++) {
    char duplicateFilename[100];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             "%s%s_duplicates%d.txt", FILE_FOLDER, filename, i + 1);
    writeHeaderAndDataToFile(header, data, duplicateFilename);
  }

  printf("File saved successfully.\n");
  free(filepath);
}

void writeHeaderAndDataToFile(const HeaderInfo header, const char *data,
                              const char *filepath) {
  // Open file for writing
  FILE *file = fopen(filepath, "wb");
  if (file == NULL) {
    perror("Error opening file for writing");
    free((char *)filepath);
    exit(EXIT_FAILURE);
  }

  // Write header, data, and CRC to file
  fwrite(&header, sizeof(HeaderInfo), 1, file);
  fwrite(data, sizeof(char), strlen(data), file);
  unsigned char crc = calculateCRC(header, data);
  fwrite(&crc, sizeof(unsigned char), 1, file);

  // Close file
  fclose(file);
}

// Function to calculate CRC (One's Complement)
unsigned char calculateCRC(HeaderInfo header, const char *data) {
  unsigned int sum = header.fileID + header.readCount;
  printf("Sum: %d\n", sum);
  while (*data) {
    sum += *data;
    data++;
  }
  printf("data %s\n", data);
  return ~(sum & 0xFF); // One's complement
}

// Function to generate a random file ID
unsigned char generateRandomFileID() {
  return rand() % 256; // Generates a random number between 0 and 255
}

bool isFileCorrupted(const char *filename) {
  // Create the path for the original file
  char originalPath[256];
  snprintf(originalPath, sizeof(originalPath), FILE_FOLDER "%s", filename);

  // Read the content of the original file
  FILE *originalFile = fopen(originalPath, "rb");
  if (originalFile == NULL) {
    printf("Error opening original file '%s' for reading.\n", originalPath);
    return true; // Consider as corrupted if original file cannot be opened
  }

  fseek(originalFile, 0, SEEK_END);
  long originalSize = ftell(originalFile);
  fseek(originalFile, 0, SEEK_SET);
  char originalData[originalSize + 1];
  if (fread(originalData, sizeof(char), originalSize, originalFile) !=
      originalSize) {
    fclose(originalFile);
    printf("Error reading original file '%s'.\n", originalPath);
    return true; // Consider as corrupted if unable to read original file
  }
  originalData[originalSize] = '\0';
  fclose(originalFile);

  // Check if the original file is corrupted
  bool isOriginalCorrupted = true;

  // Iterate through each duplicate file
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    // Create the path for the duplicate file
    char duplicatePath[256];
    snprintf(duplicatePath, sizeof(duplicatePath), "%s%s_duplicates%d.txt",
             FILE_FOLDER, filename, i);

    // Read the content of the duplicate file
    FILE *duplicateFile = fopen(duplicatePath, "rb");
    if (duplicateFile == NULL) {
      printf("Error opening duplicate file '%s' for reading.\n", duplicatePath);
      continue; // Continue with the next duplicate if this one cannot be opened
    }

    fseek(duplicateFile, 0, SEEK_END);
    long duplicateSize = ftell(duplicateFile);
    fseek(duplicateFile, 0, SEEK_SET);
    char duplicateData[duplicateSize + 1];
    if (fread(duplicateData, sizeof(char), duplicateSize, duplicateFile) !=
        duplicateSize) {
      fclose(duplicateFile);
      printf("Error reading duplicate file '%s'.\n", duplicatePath);
      continue; // Continue with the next duplicate if unable to read
    }
    duplicateData[duplicateSize] = '\0';
    fclose(duplicateFile);

    // Compare the content of the duplicate with the original
    if (strcmp(originalData, duplicateData) == 0 && !isOriginalCorrupted) {
      // Found a matching duplicate, file is not corrupted
      printf("File '%s' is not corrupted. Match found in duplicate %d.\n",
             filename, i);
      isOriginalCorrupted = false;
      break;
    }
  }

  // If the original file is corrupted, return true
  if (isOriginalCorrupted) {
    printf("Original file '%s' is corrupted.\n", filename);
    return true;
  }

  // None of the duplicates match the original, check if any duplicates are
  // corrupted
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    // Create the path for the duplicate file
    char duplicatePath[256];
    snprintf(duplicatePath, sizeof(duplicatePath), "%s%s_duplicates%d.txt",
             FILE_FOLDER, filename, i);

    // Read the content of the duplicate file
    FILE *duplicateFile = fopen(duplicatePath, "rb");
    if (duplicateFile == NULL) {
      printf("Error opening duplicate file '%s' for reading.\n", duplicatePath);
      continue; // Continue with the next duplicate if this one cannot be opened
    }

    fseek(duplicateFile, 0, SEEK_END);
    long duplicateSize = ftell(duplicateFile);
    fseek(duplicateFile, 0, SEEK_SET);
    char duplicateData[duplicateSize + 1];
    if (fread(duplicateData, sizeof(char), duplicateSize, duplicateFile) !=
        duplicateSize) {
      fclose(duplicateFile);
      printf("Error reading duplicate file '%s'.\n", duplicatePath);
      continue; // Continue with the next duplicate if unable to read
    }
    duplicateData[duplicateSize] = '\0';
    fclose(duplicateFile);

    // Compare the content of the duplicate with the original
    if (strcmp(originalData, duplicateData) != 0) {
      // Found a mismatch, duplicate is corrupted
      printf("Duplicate %d of file '%s' is corrupted.\n", i, filename);
      return true;
    }
  }

  // All duplicates match the original, file is not corrupted
  printf("File '%s' is not corrupted.\n", filename);
  return false;
}

void readFromFile(const char *filename) {
  // Create full file path by appending directory path
  char filepath[256]; // Adjust size as needed
  snprintf(filepath, sizeof(filepath), FILE_FOLDER "%s", filename);
  printf("filepath %s\n", filepath);

  // Open file for reading
  FILE *file = fopen(filepath, "rb");
  if (file == NULL) {
    printf("Error opening file '%s' for reading.\n", filepath);
    return;
  }

  // Read header from file
  HeaderInfo header;
  if (fread(&header, sizeof(HeaderInfo), 1, file) != 1) {
    printf("Error reading header from file '%s'.\n", filepath);
    fclose(file);
    return;
  }

  // Calculate data size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, sizeof(HeaderInfo), SEEK_SET);
  long dataSize = fileSize - sizeof(HeaderInfo) - sizeof(unsigned char);

  // Allocate memory for data
  char *data = (char *)malloc(dataSize + 1);
  if (data == NULL) {
    printf("Error allocating memory for data.\n");
    fclose(file);
    return;
  }

  // Read data from file
  if (fread(data, sizeof(char), dataSize, file) != dataSize) {
    printf("Error reading data from file '%s'.\n", filepath);
    free(data);
    fclose(file);
    return;
  }
  data[dataSize] = '\0'; // Null-terminate the data

  // Allocate memory for temporary decrypted data
  char *tempData = (char *)malloc(dataSize + 1);
  if (tempData == NULL) {
    printf("Error allocating memory for tempData.\n");
    free(data);
    fclose(file);
    return;
  }

  // Copy data to tempData
  strcpy(tempData, data);
  printf("Before decryption: %s\n", tempData); // Troubleshooting

  // Decrypt tempData
  decryptData(tempData);
  printf("After decryption: %s\n", tempData); // Troubleshooting

  // Read CRC from file
  unsigned char crc;
  if (fread(&crc, sizeof(unsigned char), 1, file) != 1) {
    printf("Error reading CRC from file '%s'.\n", filepath);
    free(data);
    free(tempData);
    fclose(file);
    return;
  }

  // Close file
  fclose(file);

  // Validate CRC
  unsigned char calculatedCRC = calculateCRC(header, data);
  if (calculatedCRC != crc) {
    printf("CRC validation failed for file '%s'. Data may be corrupted.\n",
           filepath);
    // Check and repair corrupted file
    if (!checkAndRepairCorruptedFile(filename, filename)) {
      printf("Failed to repair file '%s'.\n", filepath);
    }
    free(data);
    free(tempData);
    return;
  }

  // Validate Header Information
  if (header.fileID < 0 || header.fileID > 255) {
    printf("Invalid file ID for file '%s'.\n", filepath);
    free(data);
    free(tempData);
    return;
  }

  // Increment read count
  header.readCount++;

  // Display file content and header information
  printf("File ID: %d\n", header.fileID);
  printf("Read Count: %d\n", header.readCount);
  printf("Data: %s\n", tempData);

  // Recalculate CRC based on updated data
  crc = calculateCRC(header, data);

  // Rewrite the file with updated read count and CRC
  file = fopen(filepath, "wb");
  if (file == NULL) {
    printf("Error opening file '%s' for writing.\n", filepath);
    free(data);
    free(tempData);
    return;
  }

  fwrite(&header, sizeof(HeaderInfo), 1, file);
  fwrite(data, sizeof(char), dataSize, file);
  fwrite(&crc, sizeof(unsigned char), 1, file);
  fclose(file);

  // Copy content from the original file to all duplicates
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    char duplicateFilename[256];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             FILE_FOLDER "%s_duplicates%d.txt", filename, i);
    if (copyFileContent(filepath, duplicateFilename)) {
      printf("Content copied from original file to duplicate %d.\n", i);
    }
  }

  // Free allocated memory
  free(data);
  free(tempData);
}

bool compareFiles(const char *file1, const char *file2) {
  FILE *f1 = fopen(file1, "rb");
  FILE *f2 = fopen(file2, "rb");

  if (f1 == NULL || f2 == NULL) {
    // Error opening files
    if (f1)
      fclose(f1);
    if (f2)
      fclose(f2);
    return false;
  }

  char buffer1[BUFFER_SIZE];
  char buffer2[BUFFER_SIZE];

  size_t bytesRead1, bytesRead2;

  do {
    bytesRead1 = fread(buffer1, 1, BUFFER_SIZE, f1);
    bytesRead2 = fread(buffer2, 1, BUFFER_SIZE, f2);

    if (bytesRead1 != bytesRead2 || memcmp(buffer1, buffer2, bytesRead1) != 0) {
      fclose(f1);
      fclose(f2);
      return false; // Files are different
    }
  } while (bytesRead1 > 0);

  fclose(f1);
  fclose(f2);
  return true; // Files are identical
}

// Function to copy content from one file to another
bool copyFileContent(const char *sourceFilename,
                     const char *destinationFilename) {
  FILE *sourceFile = fopen(sourceFilename, "rb");
  FILE *destinationFile = fopen(destinationFilename, "wb");

  if (sourceFile == NULL || destinationFile == NULL) {
    if (sourceFile)
      fclose(sourceFile);
    if (destinationFile)
      fclose(destinationFile);
    return false;
  }

  // Copy header
  HeaderInfo header;
  if (fread(&header, sizeof(HeaderInfo), 1, sourceFile) != 1) {
    fclose(sourceFile);
    fclose(destinationFile);
    return false;
  }
  fwrite(&header, sizeof(HeaderInfo), 1, destinationFile);

  // Copy data
  char buffer[BUFFER_SIZE];
  size_t bytesRead;

  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0) {
    fwrite(buffer, 1, bytesRead, destinationFile);
  }

  fclose(sourceFile);
  fclose(destinationFile);
  return true;
}

// Function to repair corrupted file
void repairFile(const char *filename) {
  printf("Attempting to repair file '%s'...\n", filename);

  char originalFilename[256];
  snprintf(originalFilename, sizeof(originalFilename), FILE_FOLDER "%s",
           filename);

  // Attempt to repair from duplicates
  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    char duplicateFilename[256];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             FILE_FOLDER "%s_duplicates%d.txt", filename, i);

    if (copyFileContent(duplicateFilename, originalFilename)) {
      printf("File repaired successfully using duplicate %d.\n", i);
      return;
    }
  }

  printf("File repair failed. No valid duplicates found.\n");
}

// Function to repair corrupted file using duplicates
bool isRepairFile(const char *originalFilename, const char *corruptedFilename) {
  printf("Repairing file '%s' using duplicates.\n", corruptedFilename);

  char originalPath[256];
  snprintf(originalPath, sizeof(originalPath), FILE_FOLDER "%s",
           originalFilename);

  for (int i = 1; i <= NUM_DUPLICATES; i++) {
    char duplicateFilename[100];
    snprintf(duplicateFilename, sizeof(duplicateFilename),
             "%s_duplicates%d.txt", originalFilename, i);

    if (compareFiles(originalPath, duplicateFilename)) {
      if (copyFileContent(duplicateFilename, originalPath)) {
        printf("File '%s' repaired successfully using duplicate %d.\n",
               corruptedFilename, i);
        return true;
      }
    }
  }

  printf("Repair failed for file '%s'. No valid duplicates found.\n",
         corruptedFilename);
  return false;
}
bool checkAndRepairCorruptedFile(const char *originalFilename,
                                 const char *corruptedFilename) {
  if (!compareFiles(originalFilename, corruptedFilename)) {
    printf("File '%s' is corrupted. Attempting repair...\n", corruptedFilename);
    repairFile(corruptedFilename); // Attempt to repair the corrupted file
    printf("File '%s' repaired successfully.\n", corruptedFilename);
    return true;
  } else {
    printf("File '%s' is not corrupted.\n", corruptedFilename);
    return true;
  }
}

void decryptData(char *data) {
  while (*data) {
    *data = ~(*data);
    data++;
  }
}

// Function to encrypt data (simple XOR encryption for demonstration purposes)
void encryptData(char *data) {
  while (*data) {
    *data = ~(*data);
    data++;
  }
}
