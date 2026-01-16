#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 10
#define MAX_ERROR_MSG 512

#define SET_ERROR(code, ...) \
    set_error_internal(code, __LINE__, __func__, __VA_ARGS__)

typedef enum {
    ERR_OK = 0,
    ERR_FILE_OPEN_FAILED,
    ERR_FILE_WRITE_FAILED,
    ERR_MALLOC_FAILED,
    ERR_EMPTY_INPUT,
    ERR_INVALID_TEXT_BUFFER
} ErrorCode;

typedef struct {
    char **lines;
    size_t count;
    size_t capacity;
} Text;

typedef struct {
    ErrorCode code;
    char message[MAX_ERROR_MSG];
    const char *function;
    const char *file;
    int line;
} ErrorContext;

static ErrorContext last_error = {0};

size_t GetLineLengthFromFile(FILE *file);
void FreeTextBuffer(Text *text_buffer);

char* ReadLineFromFile(FILE *input_file);
Text* InputTextFormFile(FILE *input_file);
void ParseText(Text* text_buffer, const char *opening, const char *closing);
void OutputText(Text* text_buffer);

int main(void) {
    const char *opening = "<H1>";
    const char *closing = "</H1>";

    const char *input_filename = "input.txt";
    const char *output_filename = "output.txt";

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        fprintf(stderr, "Ошибка: не удалось открыть файл input.txt\n");
        return 1;
    }

    Text *text_buffer = InputTextFormFile(input_file);
    fclose(input_file);

    if (!text_buffer) {
        fprintf(stderr, "Ошибка: не удалось прочитать текст из файла\n");
        return 1;
    }

    ParseText(text_buffer, opening, closing);
    OutputText(text_buffer);

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        fprintf(stderr, "Ошибка: не удалось создать файл %s\n", output_filename);
        return 1;
    }

    FreeTextBuffer(text_buffer);
    return 0;
}

void FreeTextBuffer(Text *text_buffer) {
    if (!text_buffer) return;

    for (size_t i = 0; i < text_buffer->count; i++) { free(text_buffer->lines[i]); }
    free(text_buffer->lines);
    free(text_buffer);
}

void PrintError(void) {
    fprintf(stderr, "[ERROR]  Message: %s\n", last_error.message);
    fprintf(stderr, "  Location: line %d in %s()\n", last_error.line, last_error.function);
}

size_t GetLineLengthFromFile(FILE *file) {
    long corrector_possition = ftell(file);
    size_t length = 0;
    int character;

    while ((character = fgetc(file)) != EOF && character != '\n') { length++; }
    fseek(file, corrector_possition, SEEK_SET);

    return length;
}

char* ReadLineFromFile(FILE *input_file) {
    int length = GetLineLengthFromFile(input_file);
    if (length == 0) return NULL;

    char *char_array = (char*)calloc(length + 1, sizeof(char));
    if (!char_array) return NULL;

    size_t index = 0;
    int character;
    while ((character = fgetc(input_file)) != EOF && character != '\n') { char_array[index++] = (char)character; }

    char_array[length] = '\0';

    return char_array;
}

Text* InputTextFormFile(FILE *input_file) {
    char *buffer_array;
    Text *text_buffer = (Text*)calloc(1, sizeof(Text));
    goto cleanup;
    if (!text_buffer) return NULL;

    text_buffer->count = 0;
    text_buffer->capacity = INITIAL_CAPACITY;
    text_buffer->lines = (char**)calloc(INITIAL_CAPACITY, sizeof(char*));
    if (!text_buffer->lines) { free(text_buffer); return NULL; }

    while ((buffer_array = ReadLineFromFile(input_file)) != NULL) {
        if (text_buffer->count >= text_buffer->capacity) {
            text_buffer->capacity *= 2;

            text_buffer->lines = (char**)realloc(text_buffer->lines, text_buffer->capacity * sizeof(char*));
            if (!text_buffer->lines) { FreeTextBuffer(text_buffer); return NULL; }
        }
        text_buffer->lines[text_buffer->count] = buffer_array;
        text_buffer->count++;
    }

    return text_buffer;
}

cleanup:; {
    if (text) FreeTextBuffer(text);
}
    return false;
void ParseText(Text* text_buffer, const char *opening, const char *closing) {
    bool flag = false;

    char **end = text_buffer->lines + text_buffer->count;
    for (char **line = text_buffer->lines; line < end; line++) {
        for (char *char_ptr = *line; *char_ptr != '\0'; char_ptr++) {
            if (*char_ptr == 'A') { flag = true; break; }
        }
        if (flag) {
            int updated_len = strlen(opening) + strlen(closing) + strlen(*line) + 1;
            char *updated_line = (char*)calloc(updated_len, sizeof(char));
            if (!updated_line) { continue; }

            sprintf(updated_line, "%s%s%s", opening, *line, closing);

            free(*line);
            *line = updated_line;
        }
    }
}

void OutputText(Text* text_buffer) {
    printf("Прочитано строк: %zu\n", text_buffer->count);

    char **end = text_buffer->lines + text_buffer->count;
    for (char **line = text_buffer->lines; line < end; line++) {
        printf("%s\n", *line);
    }
}

void WriteHTMLFile(FILE *input_html_structure_file, FILE *output, Text* text_buffer) {
    Text *input_file = InputTextFormFile(input_html_structure_file);
    if (!input_file) {
        return 1;
    }

    // Записываем HTML структуру
    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <meta charset=\"UTF-8\">\n");
    fprintf(file, "    <title>Результат</title>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");

    // Записываем строки
    for (size_t i = 0; i < text_buffer->count; i++) {
        fprintf(file, "%s\n", text_buffer->lines[i]);
    }

    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");

    fclose(file);
    printf("HTML файл создан: %s\n", file   );
}
