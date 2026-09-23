#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_contents(uint8_t* data, size_t size);

int main(int argc, char** argv) {
  // Make sure we have a file input
  if (argc != 2) {
    fprintf(stderr, "Please specify an input filename.\n");
    exit(1);
  }

  // Try to open the file
  FILE* input = fopen(argv[1], "r");
  if (input == NULL) {
    perror("Unable to open input file");
    exit(1);
  }

  // Seek to the end of the file so we can get its size
  if (fseek(input, 0, SEEK_END) != 0) {
    perror("Unable to seek to end of file");
    exit(2);
  }

  // Get the size of the file
  size_t size = ftell(input);

  // Seek back to the beginning of the file
  if (fseek(input, 0, SEEK_SET) != 0) {
    perror("Unable to seek to beginning of file");
    exit(2);
  }

  // Allocate a buffer to hold the file contents. We know the size in bytes, so
  // there's no need to multiply to get the size we pass to malloc in this case.
  uint8_t* data = malloc(size);

  // Read the file contents
  if (fread(data, 1, size, input) != size) {
    fprintf(stderr, "Failed to read entire file\n");
    exit(2);
  }

  // Make sure the file starts with the .ar file signature
  if (memcmp(data, "!<arch>\n", 8) != 0) {
    fprintf(stderr, "Input file is not in valid .ar format\n");
    exit(1);
  }

  // Call the code to print the archive contents
  print_contents(data, size);

  // Clean up
  free(data);
  fclose(input);

  return 0;
}


/**
 * This function should print the name of each file in the archive followed by its
 * contents.
 *
 * \param data This is a pointer to the first byte in the file.
 * \param size This is the number of bytes in the file.
 */
void print_contents(uint8_t* data, size_t size) {
    
    //make sure to skip over the first 8 bytes
    uint8_t* dataPointer = data + 8;
    // find out the end of the file
    uint8_t* end = data + size;

    // every file is 60 bytes of header + file size, go until end
    while (dataPointer + 60 <= end) {
        
        //make sure theres room for a null characyer
        char sizeText[11];
        for(int i = 0; i < 11; i++) {
            sizeText[i] = 0;
        }

        memcpy(sizeText, dataPointer + 48, 10);

        size_t fileSize = 0;
        sscanf(sizeText, "%zu", &fileSize);

        // go through the file up until weve hit 16 bytes or until we find '/', ' ', '\0'
        int nameLen = 0;
        while (nameLen < 16 && dataPointer[nameLen] != '/' && dataPointer[nameLen] != ' ' && dataPointer[nameLen] != '\0') {
            nameLen++;
        }

        // Print the filename we found
        printf("%.*s\n", nameLen, (char*)dataPointer);

       uint8_t* content = dataPointer + 60;
       size_t len = fileSize;
       if (content + len > end) {
        len = end - content;
        }


        // checks to make sure the file isnt empty + prints
        if (len > 0) {
            fwrite(content, 1, len, stdout);
        }
        printf("\n");

        // Move to our next file entry
        dataPointer += 60 + fileSize;

        //make sure to add extra padding for odd file sizes
        if (fileSize % 2 != 0) {
            dataPointer += 1;
        }
    }
}