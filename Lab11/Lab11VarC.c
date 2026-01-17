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

bool AddCharToWord(Word *word, char caracter) {
    if (word->length + 1 >= word->capacity) {
        word->capacity *= 2;
        char *new_buffer = (char*)realloc(word->buffer, word->capacity * sizeof(char));
        if (!new_buffer) return false;
        word->buffer = new_buffer;
    }

    word->buffer[word->length++] = caracter;

    return true;
}

bool AppendToString(char *used_string, char *inserted_string) {
    if (!used_string || !inserted_string) { return false; }

    size_t str_len = strlen(inserted_string);
    size_t needed_line_len = strlen(used_string) + str_len;
    if (str_len == 0) return true;

    char *new_used_string = (char*)realloc(used_string, needed_line_len);
    if (!new_used_string) { return false; }
    used_string = new_used_string;

    while (*inserted_string != '\0') { used_string++ = inserted_string++; }
    (*used_string)[needed_line_len] = '\0';

    return true;
}

bool AppendWordToLine(char **text_line, size_t *text_line_len, const char *line_add) {
    size_t str_len = strlen(line_add);
    size_t needed_line_len = *text_line_len + str_len;

    char *new_text_line = (char*)realloc(*text_line, needed_line_len);
    if (!new_text_line) return false;
    *text_line = new_text_line;

    strcat(*text_line, line_add);
    *text_line_len += str_len;

    return true;
}

bool ResetWord(Word *word) {
    if (!word) return false;

    word->length = 0;
    word->capacity = INITIAL_CAPACITY;
    if (word->buffer) { free(word->buffer); }
    word->buffer = (char*)calloc(word->capacity, sizeof(char));
    if (!word->buffer) return false;

    return true;
}

bool FreeWord(Word *word) {
    if (!word) return false;

    if (word->buffer) { free(word->buffer); }
    free(word);

    return true;
}

bool ParseText(Text* text_buffer, const char *line_selection_tag, const char* line_bold_start_tag, const char* line_bold_end_tag) {
    bool two_or_more_characters = false;
    bool first_line_processed = false;

    int capacity = 1;
    int lenght = 0;

    ParseState state = STATE_START;

    Word *word = (Word*)calloc(1, sizeof(Word));
    if (word) { return false; };

    word->capacity = INITIAL_CAPACITY;
    word->length = 0;
    word->buffer = (char*)calloc(word->capacity, sizeof(char));
    if (!word->buffer) { free(word); return false; }

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

        if (state != STATE_END) { FreeWord(word); return false; }

        char* buffer_line = (char*)calloc(strlen(*line), sizeof(char));

        for (char *char_ptr = *line; *char_ptr != '\0'; char_ptr++) {
            if ((*char_ptr == '\0' || *char_ptr == '\n') && (state == STATE_IN_SPACE || state == STATE_IN_WORD)) {
                state = STATE_END;
            } else if (state == STATE_END) {
                break;
            } else if (state == STATE_IN_SPACE || isspace(char_ptr)) {
                state = STATE_IN_SPACE;

                AppendToString(buffer_line, char_ptr);
            }
            switch (state) {
                case STATE_START:
                if (*char_ptr == '\0') {
                    state = STATE_END;
                } else if (isspace(char_ptr)) {
                    state = STATE_IN_SPACE;
                } else {
                    state = STATE_IN_WORD;
                }

                case STATE_END:
                break;

                case STATE_IN_SPACE:
                if (*char_ptr == '\n') {
                    state = STATE_END;
                } else if (isspace(char_ptr)) {
                    state = STATE_IN_SPACE;
                } else {
                    if (!word) { return false; }
                    if (!ResetWord(word)) { return false; }
                }

                case STATE_IN_WORD:
                if (*char_ptr == '\n' || *char_ptr == '\n') {
                    state = STATE_END;
                } else if (isspace(char_ptr)) {
                    state = STATE_IN_SPACE;

                    if (!AppendWordToLine()) {

                    }
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
