#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 10
#define MAX_ERROR_MSG 512

typedef struct {
    char **lines;
    size_t count;
    size_t capacity;
} Text;

size_t GetLineLengthFromFile(FILE*);
bool FreeTextBuffer(Text*);

char* ReadLineFromFile(FILE*);
bool InputTextFormFile(FILE*, Text**);
bool ParseText(Text*, const char*);
bool WriteHTMLFile(FILE*, FILE*, Text*, Text*, const char*);
bool OutputText(Text*);

int main(void) {
    char *marker = "{{CONTENT}}";
    const char *handler = "<H1>";

    const char *input_filename = "input.txt";
    const char *output_filename = "output.txt";
    const char *html_structure_filename = "html_structure.txt";

    Text *text_buffer = (Text*)calloc(1, sizeof(Text));
    if (!text_buffer) {
        fprintf(stderr, "Ошибка: не удалось выделить память\n");
        return 1;
    }

    Text *html_structure = (Text*)calloc(1, sizeof(Text));
    if (!html_structure) {
        fprintf(stderr, "Ошибка: не удалось выделить память\n");
        return 1;
    }

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        fprintf(stderr, "Ошибка: не удалось считать файл input.txt\n");
        return 1;
    }

    FILE *input_html_structure_file = fopen(html_structure_filename, "r");
    if (!input_html_structure_file) {
        fprintf(stderr, "Ошибка: не удалось считать файл html_structure.txt\n");
        return 1;
    }

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        fprintf(stderr, "Ошибка: не удалось создать файл %s\n", output_filename);
        return 1;
    }

    if (!InputTextFormFile(input_file, &text_buffer)) {
        fprintf(stderr, "Ошибка: не удалось прочитать текст из файла\n");
        return 1;
    }

    fclose(input_file);

    if (!ParseText(text_buffer, handler)) {
        fprintf(stderr, "Ошибка: парс текста из файла неудался\n");
        return 1;
    }

    if (!OutputText(text_buffer)) {
        fprintf(stderr, "Ошибка: не удалось вывести текст\n");
        return 1;
    }

    if (!WriteHTMLFile(input_html_structure_file, output, html_structure, text_buffer, marker)) {
        fprintf(stderr, "Ошибка: не удалось вывести текст в файл html\n");
        return 1;
    }


    if (!FreeTextBuffer(text_buffer)) {
        fprintf(stderr, "Ошибка: не удалось освободить память\n");
        return 1;
    }

    if (!FreeTextBuffer(html_structure)) {
        fprintf(stderr, "Ошибка: не удалось освободить память\n");
        return 1;
    }

    return 0;
}

bool FreeTextBuffer(Text *text_buffer) {
    if (!text_buffer) return false;

    for (size_t i = 0; i < text_buffer->count; i++) { free(text_buffer->lines[i]); }
    free(text_buffer->lines);
    free(text_buffer);

    return true;
}

size_t GetCountLinesFromFile(FILE *file) {
    long corrector_possition = ftell(file);
    size_t length = 0;

    int character;
    while ((character = fgetc(file)) != EOF && character != '\n') { length++; }
    fseek(file, corrector_possition, SEEK_SET);

    return length;
}

char* ReadLineFromFile(FILE *input_file) {
    int length = GetCountLinesFromFile(input_file);
    if (length == 0) return NULL;

    char *char_array = (char*)calloc(length + 1, sizeof(char));
    if (!char_array) return NULL;

    size_t index = 0;
    int character;
    while ((character = fgetc(input_file)) != EOF && character != '\n') { char_array[index++] = (char)character; }

    char_array[length] = '\0';

    return char_array;
}

bool InputTextFormFile(FILE *input_file, Text **text_buffer) {
    char *buffer_array;

    (*text_buffer)->count = 0;
    (*text_buffer)->capacity = INITIAL_CAPACITY;
    (*text_buffer)->lines = (char**)calloc(INITIAL_CAPACITY, sizeof(char*));
    if (!(*text_buffer)->lines) { free(text_buffer); return NULL; }

    while ((buffer_array = ReadLineFromFile(input_file)) != NULL) {
        if ((*text_buffer)->count >= (*text_buffer)->capacity) {
            (*text_buffer)->capacity *= 2;

            (*text_buffer)->lines = (char**)realloc((*text_buffer)->lines, (*text_buffer)->capacity * sizeof(char*));
            if (!(*text_buffer)->lines) { FreeTextBuffer((*text_buffer)); return NULL; }
        }
        (*text_buffer)->lines[(*text_buffer)->count] = buffer_array;
        (*text_buffer)->count++;
    }

    return true;
}

bool ParseText(Text* text_buffer, const char *handler) {
    bool two_or_more_characters = false;
    bool first_line_processed = false;
    char present_char;

    char **end = text_buffer->lines + text_buffer->count;

    for (char **line = text_buffer->lines; line < end; line++) {
        present_char = '\0';

        if (!first_line_processed) {
            int first_line_new_len = strlen(handler) + strlen(handler) + strlen(*line) + 1;
            char *new_first_line = (char*)calloc(first_line_new_len, sizeof(char));

            if (!new_first_line) { return false; }

            sprintf(new_first_line, "%s%s%s", handler, *line, handler);
            free(*line);
            *line = new_first_line;

            first_line_processed = true;
            continue;
        }

        for (char *char_ptr = *line; *char_ptr != '\0'; char_ptr++) {
            if (*char_ptr == present_char) { two_or_more_characters = true; break; }

            present_char = *char_ptr;
        }
        if (two_or_more_characters) {
            two_or_more_characters = false;

            int updated_len = strlen(handler) + strlen(handler) + strlen(*line) + 1;
            char *updated_line = (char*)calloc(updated_len, sizeof(char));
            if (!updated_line) { continue; }

            sprintf(updated_line, "%s%s%s", handler, *line, handler);

            free(*line);
            *line = updated_line;
        }
    }

    return true;
}

bool WriteHTMLFile(FILE *input_html_structure_file, FILE *output, Text* html_structure, Text* text_buffer, const char *marker) {
    if (!InputTextFormFile(input_html_structure_file, &html_structure)) { return false; }

    char **end = html_structure->lines + html_structure->count;
    for (char **structure_line = html_structure->lines; structure_line < end; structure_line++) {
        char *record_line = strstr(*structure_line, marker);

        if (record_line) {
            *record_line = '\0';
            fprintf(output, "%s", *structure_line);

            char **text_end = text_buffer->lines + text_buffer->count;
            for (char **text_line = text_buffer->lines; text_line < text_end; text_line++) {
                fprintf(output, "%s\n", *text_line);
            }
            fprintf(output, "%s", record_line + strlen(marker));
        } else {
            fprintf(output, "%s\n", *structure_line);
        }
    }

    return true;
}

bool OutputText(Text* text_buffer) {
    printf("Прочитано строк: %zu\n", text_buffer->count);

    char **end = text_buffer->lines + text_buffer->count;
    for (char **line = text_buffer->lines; line < end; line++) {
        printf("%s\n", *line);
    }

    return true;
}
