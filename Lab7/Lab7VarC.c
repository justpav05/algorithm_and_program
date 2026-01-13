#include "stdio.h"
#include "stdlib.h"

int FindIndex(int *array, int *size, int value);
int FindMin(int *array, int *size);
int FindMax(int *array, int *size);
void AddElement(int **array, int *capacity, int ind, int value);
void InputArray(int **array, int *size);
void UpdateArray(int **array, int *size);
void OutputArray(int *array, int *size);

int main() {
    int *array;
    int size = 0;

    printf("Введите колличество элементов: ");

    while (!(scanf("%d", &size) == 1 && getchar() == '\n')) {
        printf("Ошибка! Не число! Попробуйте ещё раз: ");
        while (getchar() != '\n');
    }

    InputArray(&array, &size);

    UpdateArray(&array, &size);

    OutputArray(array, &size);
}

int FindIndex(int *array, int *size, int value) {
    for (int ind = 0; ind < *size; ind++) {
        if (array[ind] == value) return ind;
    }
    return -1;
}

int FindMin(int *array, int *size) {
    int min = array[0];
    for (int ind = 1; ind < *size; ind++) {
        if (array[ind] < min) min = array[ind];
    }
    return min;
}

int FindMax(int *array, int *size) {
    int max = array[0];
    for (int ind = 1; ind < *size; ind++) {
        if (array[ind] > max) max = array[ind];
    }
    return max;
}

void AddElement(int **array, int *capacity, int ind, int value) {
    if (ind >= *capacity) {
        *capacity = (*capacity == 0) ? 1 : (*capacity) * 2;
        *array = (int*)realloc(*array, (*capacity) * sizeof(int));
    }
    (*array)[ind] = value;
}

void InputArray(int **array, int *size) {
    int element;
    int capacity = 0;

    *array = (int*)calloc(*size, sizeof(int));

    for (int ind = 0; ind < *size; ind++) {
        printf("Введите элемент с новой строки: ");

        while (!(scanf("%d", &element) == 1 && getchar() == '\n')) {
            printf("Ошибка! Не число! Попробуйте ещё раз: ");
            while (getchar() != '\n');
        }
        AddElement(array, &capacity, ind, element);
    }
}

void OutputArray(int *array, int *size) {
    printf("Введённые элементы массива: ");

    for (int ind = 0; ind < *size; ind++) {
        printf("%d ", array[ind]);
    }
}

void UpdateArray(int **array, int *size) {
    int *new_array;

    int min_ind = FindIndex(*array, size, FindMin(*array, size));
    int max_ind = FindIndex(*array, size, FindMax(*array, size));

    int start_ind = (min_ind < max_ind) ? min_ind : max_ind;
    int end_ind = (min_ind > max_ind) ? min_ind : max_ind;

    *size = end_ind - start_ind + 1;

    new_array = (int*)calloc(*size, sizeof(int));

    for (int ind = start_ind; ind <= end_ind; ind++) {
        new_array[ind - start_ind] = (*array)[ind];
    }

    free(*array);
    *array = new_array;
}
