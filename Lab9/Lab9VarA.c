#include <stdio.h>
#include <stdlib.h>

void InputArrayFromFile(FILE* file, int*** double_array, int* rows, int* cols);
void OutputArray(int** array, int rows, int cols);

int main() {
    const char* filename = "input.txt";
    int** array;
    int rows, cols;

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Ошибка открытия файла!\n");
        return -1;
    }

    InputArrayFromFile(file, &array, &rows, &cols);
    OutputArray(array, rows, cols);

    fclose(file);
}

void InputArrayFromFile(FILE* file, int*** double_array, int* rows, int* cols) {
    fscanf(file, "%d %d", rows, cols);

    *double_array = (int**)calloc(*rows, sizeof(int*));
    for (int ind = 0; ind < *rows; ind++) (*double_array)[ind] = (int*)calloc(*cols, sizeof(int));

    for (int ind = 0; ind < *rows; ind++) {
        for (int jnd = 0; jnd < *cols; jnd++) {
            fscanf(file, "%d", &(*double_array)[ind][jnd]);
        }
    }
}

void OutputArray(int** array, int rows, int cols) {
    for (int ind = 0; ind < rows; ind++) {
        for (int jnd = 0; jnd < cols; jnd++) {
            printf("%d ", array[ind][jnd]);
        }
        printf("\n");
    }
}
