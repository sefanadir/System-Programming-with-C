/* 
 *                 SYSTEM PROGRAMMING HOMEWORK 1
 * This homework’s objective is to write a POSIX compatible C  
 * program that admits as command line argument the path of a  
 * black white, uncompressed locally stored tiff file and prints 
 * its pixels on the standard output using 0’s for black an 1’s for white. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#define TIFFTAG_IMAGEWIDTH 10
#define TIFFTAG_IMAGEHEIGHT 11
#define TIFFTAG_PHOTOMETRIC 16
#define TIFF_HEADER_SIZE 8
#define WHITE_COLOR_RGB 255
#define BLACK_COLOR_RGB 0
#define PIXEL_SIZE 3

int tiffDetermineFormat(const char *fileName);
int tiffGetPixelsNumber(const char *fileName, int tiffFormat);
int tiffGetField(const char *fileName, int tag, int tiffFormat);
int tiffConvertHexToDec(int *hexArray, int *intArray, int convertLine);
int tiffConvertDecToHex(unsigned char * array, int arraySize, int number);
int tiffPrintImage(const char *fileName, int pixelsNumber, int width, int photometric);

int main(int argc, char** argv) {
    int width = 0, height = 0, tiffFormat = 0, imagePixels = 0, photometric = 0;
    if (argc != 2) {
        fprintf(stderr, "Usage: ./execute filename");
    } else if (argc == 2) {
        tiffFormat = tiffDetermineFormat(argv[1]);
        /* If image format is Intel */
        if (tiffFormat == 1) {
            printf("Width:  %d\n", width = tiffGetField(argv[1], TIFFTAG_IMAGEWIDTH, tiffFormat));
            printf("Height: %d\n", height = tiffGetField(argv[1], TIFFTAG_IMAGEHEIGHT, tiffFormat));
            photometric = tiffGetField(argv[1], TIFFTAG_PHOTOMETRIC, tiffFormat);
            printf("Byte Order: Intel\n");
            imagePixels = tiffGetPixelsNumber(argv[1], tiffFormat);
            tiffPrintImage(argv[1], imagePixels, width, photometric);
        }/*If image format is Motorola */
        else if (tiffFormat == 2) {
            printf("Width:  %d\n", width = tiffGetField(argv[1], TIFFTAG_IMAGEWIDTH, tiffFormat));
            printf("Height: %d\n", height = tiffGetField(argv[1], TIFFTAG_IMAGEHEIGHT, tiffFormat));
            photometric = tiffGetField(argv[1], TIFFTAG_PHOTOMETRIC, tiffFormat);
            printf("Byte Order: Motorola\n");
            imagePixels = tiffGetPixelsNumber(argv[1], tiffFormat);
            tiffPrintImage(argv[1], imagePixels, width, photometric);
        } else
            fprintf(stderr, "This file is not TIFF FILE\n");
    }
    return 0;
}

/*
 * This function takes the file name, tag code, file format 
 * information from the parameter and calculates the number information of tag
 */
int tiffGetField(const char *fileName, int tag, int tiffFormat) {
    char buffer[2];
    size_t readCharacters;
    int fieldNumber = 0, fd = 0, i = 0;
    int fieldNumberInt[2], fieldNumberHex[4];
    int tag_part1 = tag / 10;
    int tag_part2 = tag % 10;
    int tag_temp = 0;
    if (tiffFormat == 1) {
        tag_temp = tag_part2;
        tag_part2 = tag_part1;
        tag_part1 = tag_temp;
    }
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        do {
            readCharacters = read(fd, buffer, sizeof (buffer));
            if (buffer[0] == tag_part1 && buffer[1] == tag_part2) {
                for (i = 0; i < 4; i++)
                    read(fd, buffer, sizeof (buffer));
                if (tiffFormat == 1) {
                    fieldNumberInt[0] = buffer[1];
                    fieldNumberInt[1] = buffer[0];
                } else if (tiffFormat == 2) {
                    fieldNumberInt[0] = buffer[0];
                    fieldNumberInt[1] = buffer[1];
                }
                fieldNumber = tiffConvertHexToDec(fieldNumberHex, fieldNumberInt, 3);
                close(fd);
                return fieldNumber;
            }
        } while (readCharacters == sizeof (buffer));
    }
    return fieldNumber;
}

/*
 * This function takes the file name and file format 
 * information and calculates the total pixel number of the photo
 */
int tiffGetPixelsNumber(const char *fileName, int tiffFormat) {
    int fd = 0;
    unsigned char buffer[8];
    int pixelsNumberHex[8];
    int pixelsNumberInt[4];
    int pixelNumber = 0;
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        if (tiffFormat == 1) {
            read(fd, buffer, sizeof (buffer));
            pixelsNumberInt[0] = buffer[7];
            pixelsNumberInt[1] = buffer[6];
            pixelsNumberInt[2] = buffer[5];
            pixelsNumberInt[3] = buffer[4];
            pixelNumber = tiffConvertHexToDec(pixelsNumberHex, pixelsNumberInt, 7);
        } else if (tiffFormat == 2) {
            read(fd, buffer, sizeof (buffer));
            pixelsNumberInt[0] = buffer[4];
            pixelsNumberInt[1] = buffer[5];
            pixelsNumberInt[2] = buffer[6];
            pixelsNumberInt[3] = buffer[7];
            pixelNumber = tiffConvertHexToDec(pixelsNumberHex, pixelsNumberInt, 7);
        }
        close(fd);
    }
    return pixelNumber;
}

/*
 * This function takes the file name on parameter and checks
 * out whether the tiff file is in intel or motorola format
 */
int tiffDetermineFormat(const char *fileName) {
    int fd = 0;
    unsigned char buffer[2];
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        read(fd, buffer, sizeof (buffer));
        close(fd);
    }
    if (buffer[0] == 'I' && buffer[1] == 'I')
        return 1;
    else if (buffer[0] == 'M' && buffer[1] == 'M')
        return 2;
    else
        return 0;
}

/*
 * This function converts hexadecimal number to decimal number
 */
int tiffConvertHexToDec(int *hexArray, int *intArray, int convertLine) {
    int fieldNumber = 0, i = 0, j = 0;
    for (i = 0, j = 0; i < convertLine; i += 2, ++j) {
        hexArray[i] = intArray[j] / 16;
        hexArray[i + 1] = intArray[j] % 16;
    }
    for (i = convertLine; i >= 0; --i)
        fieldNumber += hexArray[i] * pow(16, convertLine - i);
    return fieldNumber;
}

/*
 * This function converts decimal number to hexadecimal   
 * number and fills this hexadecimal  number to char array
 * If this operation is success, function returns 1
 */
int tiffConvertDecToHex(unsigned char * array, int arraySize, int number) {
    int modOperation = 0, i = 0, index = arraySize - 1;
    for (i = number; index >= 0; i /= 2, --index) {
        modOperation = i % 2;
        if (modOperation == 1)
            array[index] = '1';
        else if (modOperation == 0)
            array[index] = '0';
    }
    return 1;
}

/*
 * This function takes the file name, the number of pixels, the width 
 * of the photo, and photometric interpretation information and prints 
 * its pixels on the standard output using 0’s for black and 1’s for white. 
 */
int tiffPrintImage(const char *fileName, int pixelsNumber, int width, int photometric) {
    int fd = 0, i = 0;
    unsigned char buffer[1];
    unsigned char binary[8];
    int countRowSize = 0, countBytes = 0;
    int whiteColor = 0, blackColor = 0;
    int decimalNumber = 0;
    pixelsNumber = pixelsNumber - TIFF_HEADER_SIZE;
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        perror("Failed to open file");
        exit(0);
    } else {
        for (i = 0; i < TIFF_HEADER_SIZE; i++) {
            read(fd, buffer, sizeof (buffer));
        }
        if (photometric == 2) {
            do {
                read(fd, buffer, sizeof (buffer));
                ++countBytes;
                ++countRowSize;
                if (buffer[0] == WHITE_COLOR_RGB) {
                    ++whiteColor;
                    if (whiteColor == PIXEL_SIZE) {
                        printf("1");
                        whiteColor = 0;
                    }
                } else if (buffer[0] == BLACK_COLOR_RGB) {
                    ++blackColor;
                    if (blackColor == PIXEL_SIZE) {
                        printf("0");
                        blackColor = 0;
                    }
                }
                if (countRowSize == width * PIXEL_SIZE) {
                    countRowSize = 0;
                    printf("\n");
                }
            } while (countBytes < pixelsNumber);

        } else if (photometric == 1 || photometric == 0) {
            do {
                if (photometric == 0) {
                    whiteColor = 0;
                    blackColor = 1;
                } else if (photometric == 1) {
                    whiteColor = 1;
                    blackColor = 0;
                }
                read(fd, buffer, sizeof (buffer));
                ++countBytes;
                decimalNumber = buffer[0];
                tiffConvertDecToHex(binary, 8, decimalNumber);
                for (i = 0; i < 8; ++i) {
                    if (countRowSize < width) {
                        if (binary[i] == '0') {
                            printf("%d", blackColor);
                        } else if (binary[i] == '1') {
                            printf("%d", whiteColor);
                        }
                        ++countRowSize;
                    } else {
                        countRowSize = 0;
                        printf("\n");
                        break;
                    }
                }
            } while (countBytes < pixelsNumber);
        }
        close(fd);
    }
    return 1;
}