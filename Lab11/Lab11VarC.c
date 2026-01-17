#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define INITIAL_CAPACITY 10
#define MAX_ERROR_MSG 512

typedef enum {
    STATE_START,
    STATE_IN_WORD,
    STATE_IN_SPACE,
    STATE_END
} ParseState;

typedef struct {
    char *buffer;
    size_t length;
    size_t capacity;
} Word;

typedef struct {
    char **lines;
    size_t count;
    size_t capacity;
} Text;

size_t GetLineLengthFromFile(FILE*);
bool FreeTextBuffer(Text*);

char* ReadLineFromFile(FILE*);
bool InputTextFormFile(FILE*, Text**);
bool ParseText(Text*, const char*, const char*, const char*);
bool WriteHTMLFile(FILE*, FILE*, Text*, Text*, const char*);
bool OutputText(Text*);

int main(void) {
    char *marker = "{{CONTENT}}";
    const char *line_selection_tag = "<H1>";
    const char *line_bold_start_tag = "<B><I>";
    const char *line_bold_end_tag = "</I></B>";

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

    if (!ParseText(text_buffer, line_selection_tag, line_bold_start_tag, line_bold_end_tag)) {
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

bool ProcessWord(Word *word, char **result, size_t *result_len, size_t *result_capacity) {
    if (word->length == 0) return true;

    size_t needed_size = *result_len;

    if (word->has_A) {
        // Нужно место для <B><I>слово</I></B>
        needed_size += strlen("<B><I>") + word->length + strlen("</I></B>");
    } else {
        needed_size += word->length;
    }

    // Расширяем буфер результата при необходимости
    while (needed_size + 1 >= *result_capacity) {
        *result_capacity *= 2;
        char *new_result = (char*)realloc(*result, *result_capacity);
        if (!new_result) return false;
        *result = new_result;
    }

    if (word->has_A) {
        // Заменяем A на * и оборачиваем в теги
        strcat(*result, "<B><I>");
        for (size_t i = 0; i < word->length; i++) {
            char c = word->buffer[i];
            if (c == 'A' || c == 'a') {
                c = '*';
            }
            size_t len = strlen(*result);
            (*result)[len] = c;
            (*result)[len + 1] = '\0';
        }
        strcat(*result, "</I></B>");
    } else {
        strcat(*result, word->buffer);
    }

    *result_len = strlen(*result);
    return true;
}

bool ParseText(Text* text_buffer, const char *line_selection_tag, const char* line_bold_start_tag, const char* line_bold_end_tag) {
    bool two_or_more_characters = false;
    bool first_line_processed = false;

    ParseState state = STATE_START;

    int lenght = 0;
    int capacity = 1;

    char **end = text_buffer->lines + text_buffer->count;

    for (char **line = text_buffer->lines; line < end; line++) {
        if (!first_line_processed) {
            int first_line_new_len = strlen(line_selection_tag) + strlen(line_selection_tag) + strlen(*line) + 1;
            char *new_first_line = (char*)calloc(first_line_new_len, sizeof(char));
            if (!new_first_line) { return false; }

            sprintf(new_first_line, "%s%s%s", line_selection_tag, *line, line_selection_tag);
            free(*line);
            *line = new_first_line;

            first_line_processed = true;
            continue;
        }

        if (state != STATE_END) { return false; }

        Word *current_word = (Word*)calloc(1, sizeof(Word));
        if (current_word) { return false; };

        for (char *char_ptr = *line; *char_ptr != '\0'; char_ptr++) {
            switch (state) {
                case STATE_START:
                if (isspace(char_ptr)) {
                    state = STATE_IN_SPACE;

                    if (current_word->buffer) {
                    }
                }
                case STATE_IN_SPACE:
                if (isalpha(char_ptr) || isdigit(char_ptr)) {
                    state = STATE_IN_WORD;

                    if (lenght >= capacity) {
                        capacity *= 2;
                        char *new_buffer = (char*)realloc(current_word->buffer, capacity);
                        if (!new_buffer) { return false; }
                        current_word->buffer = new_buffer;
                    }

                    if (*char_ptr == 'A') {
                        if (capacity <= strlen(line_bold_start_tag) + strlen(line_bold_end_tag) + 1) {
                            capacity *= 2;
                            char *new_buffer = (char*)realloc(current_word->buffer, capacity);
                            if (!new_buffer) { return false; }
                            current_word->buffer = new_buffer;
                        }

                        current_word->buffer[lenght] = *line_bold_start_tag;
                        current_word->buffer[lenght + 1] = *char_ptr;
                        current_word->buffer[lenght + 2] = *line_bold_end_tag;
                    }

                    current_word->buffer[lenght++] = *char_ptr;
                }

            }
            int line_new_len = strlen(line_bold_start_tag) + strlen(line_bold_end_tag) + strlen(*line) + 1;
            char *new_first_line = (char*)calloc(line_new_len, sizeof(char));
            if (!new_first_line) { return false; }
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
