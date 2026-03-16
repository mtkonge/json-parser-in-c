#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>  

typedef enum JsonValueType {
    JSON_NULL_t,
    JSON_BOOL_t,
    JSON_NUMBER_t,
    JSON_STRING_t,
    JSON_OBJECT_t,
    JSON_ARRAY_t
} JsonValueType;

typedef struct JsonValue {
    JsonValueType type;
    union {
        bool boolean;
        double number;
        char *string;
        struct HashTable *object;
        struct JsonArray *array;
    } value;
} JsonValue;

typedef struct HashEntry {
    char *key;
    JsonValue *value;
    struct HashEntry *next;
} HashEntry;

typedef struct HashTable {
    HashEntry **buckets;
    size_t capacity;
    size_t size;
} HashTable;

typedef struct JsonArray {
    struct JsonValue *value;
    struct JsonArray *next;
} JsonArray;

typedef enum JsonTokenType {
    JSON_TOKEN_STRING_t,
    JSON_TOKEN_NUMBER_t,
    JSON_TOKEN_COMMA_t,
    JSON_TOKEN_COLON_t,
    JSON_TOKEN_TRUE_t,
    JSON_TOKEN_FALSE_t,
    JSON_TOKEN_NULL_t,
    JSON_TOKEN_BRACE_OPEN_t,
    JSON_TOKEN_BRACE_CLOSE_t,
    JSON_TOKEN_BRACKET_CLOSE_t,
    JSON_TOKEN_BRACKET_OPEN_t
} JsonTokenType;

typedef struct JsonTokenEntry {
    JsonTokenType type;
    union {
        char char_value;
        char *string_value;
    } value;
    struct JsonTokenEntry *next;
} JsonTokenEntry;

void append_token(JsonTokenEntry **head, JsonTokenEntry **tail, JsonTokenEntry *new_token) {
    if (*head == NULL) {
        *head = new_token;
        *tail = new_token;
    } else {
        (*tail)->next = new_token;
        *tail = new_token;
    }
}

JsonTokenEntry *create_token(JsonTokenType type) {
    JsonTokenEntry *token = malloc(sizeof(JsonTokenType));
    token->type = type;
    return token;
}

char *json_token_type_to_string(JsonTokenType type) {
    switch (type) {
    case JSON_TOKEN_STRING_t:
        return "JSON_TOKEN_STRING_t";
    case JSON_TOKEN_NUMBER_t:
        return "JSON_TOKEN_NUMBER_t";
    case JSON_TOKEN_COMMA_t:
        return "JSON_TOKEN_COMMA_t";
    case JSON_TOKEN_COLON_t:
        return "JSON_TOKEN_COLON_t";
    case JSON_TOKEN_TRUE_t:
        return "JSON_TOKEN_TRUE_t";
    case JSON_TOKEN_FALSE_t:
        return "JSON_TOKEN_FALSE_t";
    case JSON_TOKEN_NULL_t:
        return "JSON_TOKEN_NULL_t";
    case JSON_TOKEN_BRACE_OPEN_t:
        return "JSON_TOKEN_BRACE_OPEN_t";
    case JSON_TOKEN_BRACE_CLOSE_t:
        return "JSON_TOKEN_BRACE_CLOSE_t";
    case JSON_TOKEN_BRACKET_CLOSE_t:
        return "JSON_TOKEN_BRACKET_CLOSE_t";
    case JSON_TOKEN_BRACKET_OPEN_t:
        return "JSON_TOKEN_BRACKET_OPEN_t";
    default:
        fprintf(stderr, "Unreachable\n");
        exit(EXIT_FAILURE);
    }
}

JsonTokenEntry *json_tokenize(char *json, size_t size) {
    JsonTokenEntry *head = NULL;
    JsonTokenEntry *tail = NULL;
    
    for (size_t i = 0; i < size; i++) {
        if (json[i] == ' ' || json[i] == '\n') {
            continue;
        }
        if (json[i] == ':') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_COLON_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == ',') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_COMMA_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == '{') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_BRACE_OPEN_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == '}') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_BRACE_CLOSE_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == '[') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_BRACKET_OPEN_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == ']') {
            JsonTokenEntry *token = create_token(JSON_TOKEN_BRACKET_CLOSE_t);
            token->value.char_value = json[i];
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] == '"') {
            char buffer[256];
            size_t j = 0;
            i++;

            while(json[i] != '"' && i < size) {
                buffer[j++] = json[i++];
            }
            buffer[j] = '\0';
            JsonTokenEntry *token = create_token(JSON_TOKEN_STRING_t);
            token->value.string_value = strdup(buffer);
            append_token(&head, &tail, token);
            continue;
        }
        if (json[i] >= 48 && json[i] <= 57) {
            char buffer[256];
            size_t j = 0;
            while((json[i] >= 48 && json[i] <= 57) || json[i] == '.') {
                buffer[j++] = json[i++];
            }
            buffer[j] = '\0';
            JsonTokenEntry *token = create_token(JSON_TOKEN_NUMBER_t);
            token->value.string_value = strdup(buffer);
            append_token(&head, &tail, token);
            i--;
            continue;
        }
        if (json[i] == 'n') {
            char buffer[5];
            size_t j = 0;
            while(j < 4 && i < size) {
                buffer[j++] = json[i++];
            }
            buffer[j] = '\0';
            if (strcmp(buffer, "null") != 0) {
                fprintf(stderr, "Error: could not parse value '%s'\n", buffer);
                exit(EXIT_FAILURE);
            }
            JsonTokenEntry *token = create_token(JSON_TOKEN_NULL_t);
            token->value.string_value = strdup(buffer);
            append_token(&head, &tail, token);
            i--;
            continue;
        }
        if (json[i] == 't') {
            char buffer[5];
            size_t j = 0;
            while(j < 4 && i < size) {
                buffer[j++] = json[i++];
            }
            buffer[j] = '\0';
            if (strcmp(buffer, "true") != 0) {
                fprintf(stderr, "Error: could not parse value '%s'\n", buffer);
                exit(EXIT_FAILURE);
            }
            JsonTokenEntry *token = create_token(JSON_TOKEN_TRUE_t);
            token->value.string_value = strdup(buffer);
            append_token(&head, &tail, token);
            i--;
            continue;
        }
        if (json[i] == 'f') {
            char buffer[6];
            size_t j = 0;
            while(j < 5 && i < size) {
                buffer[j++] = json[i++];
            }
            buffer[j] = '\0';
            if (strcmp(buffer, "false") != 0) {
                fprintf(stderr, "Error: could not parse value '%s'\n", buffer);
                exit(EXIT_FAILURE);
            }
            JsonTokenEntry *token = create_token(JSON_TOKEN_FALSE_t);
            token->value.string_value = strdup(buffer);
            append_token(&head, &tail, token);
            i--;
            continue;
        }
        fprintf(stderr, "Error: could not parse char '%c'\n", json[i]);
        exit(EXIT_FAILURE);
    }
    return head;   
}

void next_token_in_object(JsonTokenEntry **token_entry) {
    if (!(*token_entry)->next) {
        fprintf(stderr, "Missing closing brace\n");
        exit(EXIT_FAILURE);
    }
    *token_entry = (*token_entry)->next;
}

void next_token_in_array(JsonTokenEntry **token_entry) {
    if (!(*token_entry)->next) {
        fprintf(stderr, "Missing closing bracket\n");
        exit(EXIT_FAILURE);
    }
    *token_entry = (*token_entry)->next;
}

JsonValue *parse_value();

size_t djb2_hash(const char *str) {
    size_t hash = 5381;
    unsigned char c;

    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c*/
    }

    return hash;
}

HashTable *create_table(size_t capacity) {
    HashTable *table = malloc(sizeof(HashTable));
    table->capacity = capacity;
    table->size = 0;
    table->buckets = calloc(capacity, sizeof(HashEntry*));
    return table;
}

void resize_table(HashTable *table) {
    size_t new_capacity = table->capacity * 2;
    HashEntry **new_buckets = calloc(new_capacity, sizeof(HashEntry*));

    for (size_t i = 0; i < table->capacity; i++) {
        HashEntry *entry = table->buckets[i];

        while (entry) {
            HashEntry *next = entry->next;

            size_t index = djb2_hash(entry->key) % new_capacity;

            entry->next = new_buckets[index];
            new_buckets[index] = entry;

            entry = next;
        }
    }
    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;
}

bool add_entry(HashTable *table, const char *key, JsonValue *value) {
    unsigned long bucket_idx = djb2_hash(key) % table->capacity;
    HashEntry *entry = table->buckets[bucket_idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }
    HashEntry *new_entry = malloc(sizeof(HashEntry));
    if (!new_entry) {
        return false;
    }

    new_entry->value = value;
    new_entry->key = strdup(key);
    new_entry->next = table->buckets[bucket_idx];
    table->buckets[bucket_idx] = new_entry;
    table->size++;

    float load_factor = (float)table->size / table->capacity;

    if (load_factor > 0.75) {
        resize_table(table);
    }

    return true;
}

JsonValue *get_entry(HashTable *table, const char *key) {
    unsigned long bucket_idx = djb2_hash(key) % table->capacity;

    HashEntry *entry = table->buckets[bucket_idx];
    while (entry) {
        printf("key: %s\n", key);
        printf("entry key: %s\n", entry->key);
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

JsonValue *parse_object(JsonTokenEntry **token_entry) {
    HashTable *table = create_table(2);

    next_token_in_object(token_entry);
    while ((*token_entry)->type != JSON_TOKEN_BRACE_CLOSE_t) {
        if ((*token_entry)->type != JSON_TOKEN_STRING_t) {
            fprintf(stderr, "Expected string key in object: %s\n", json_token_type_to_string((*token_entry)->type));
            exit(EXIT_FAILURE);
        }
        char *key = (*token_entry)->value.string_value;
        next_token_in_object(token_entry);
        if ((*token_entry)->type != JSON_TOKEN_COLON_t) {
            fprintf(stderr, "Expected ':' in key-value pair got %s\n", json_token_type_to_string((*token_entry)->type));
            exit(EXIT_FAILURE);
        }
        next_token_in_object(token_entry);

        JsonValue *value = parse_value(token_entry); 
        add_entry(table, key, value);
        
        next_token_in_object(token_entry);

        if ((*token_entry)->type == JSON_TOKEN_COMMA_t) {
            next_token_in_object(token_entry);
        }
    }
    JsonValue *json_value = malloc(sizeof(JsonValue));
    json_value->type = JSON_OBJECT_t;
    json_value->value.object = table;
    return json_value;
}

JsonValue *parse_array(JsonTokenEntry **token_entry) {
    JsonArray *node_head = NULL;
    JsonArray *node_tail = NULL;
    next_token_in_array(token_entry);
    while ((*token_entry)->type != JSON_TOKEN_BRACKET_CLOSE_t) {
        JsonValue *value = parse_value(token_entry);
        if (node_head == NULL) {
            node_head = malloc(sizeof(JsonArray));
            node_head->value = value;
            node_tail = node_head;
        } else {
            JsonArray *new_node = malloc(sizeof(JsonArray));
            new_node->value = value;
            node_tail->next = new_node;
            node_tail = new_node;
        }
        next_token_in_array(token_entry);

        if ((*token_entry)->type == JSON_TOKEN_COMMA_t) {
            next_token_in_array(token_entry);
        } 
    }
    JsonValue *node = malloc(sizeof(JsonValue));
    node->type = JSON_ARRAY_t;
    node->value.array = node_head;
    return node;
}

JsonValue *parse_value(JsonTokenEntry **token_entry) {
    if (*token_entry == NULL) {
        fprintf(stderr, "Unable to pass NULL value\n");
        exit(EXIT_FAILURE);
    }
    JsonValue *json_value = malloc(sizeof(JsonValue));
    switch ((*token_entry)->type) {
        case JSON_TOKEN_STRING_t: {
            json_value->type = JSON_STRING_t;
            json_value->value.string = (*token_entry)->value.string_value;
            return json_value;
        }
        case JSON_TOKEN_NUMBER_t:
            json_value->type = JSON_NUMBER_t;
            char *end;
            json_value->value.number = strtod((*token_entry)->value.string_value, &end);
            if (strcmp(end, (*token_entry)->value.string_value) == 0) {
                fprintf(stderr, "Could not parse string to double: %s\n", (*token_entry)->value.string_value);
                exit(EXIT_FAILURE);
            }
            return json_value;
        case JSON_TOKEN_TRUE_t: {
            json_value->type = JSON_BOOL_t;
            json_value->value.boolean = true;
            if (strcmp("true", (*token_entry)->value.string_value) != 0) {
                fprintf(stderr, "Could not parse string to boolean, expected 'true' got %s\n", (*token_entry)->value.string_value);
                exit(EXIT_FAILURE);
            }
            return json_value;
        }
        case JSON_TOKEN_FALSE_t: {
            json_value->type = JSON_BOOL_t;
            json_value->value.boolean = false;
            if (strcmp("false", (*token_entry)->value.string_value) != 0) {
                fprintf(stderr, "Could not parse string to boolean, expected 'false' got %s\n", (*token_entry)->value.string_value);
                exit(EXIT_FAILURE);
            }
            return json_value;
        }
        case JSON_TOKEN_BRACE_OPEN_t:
            free(json_value);
            return parse_object(token_entry);
        case JSON_TOKEN_BRACKET_OPEN_t:
            free(json_value);
            return parse_array(token_entry);
        case JSON_TOKEN_NULL_t:
            json_value->type = JSON_NULL_t;
            return json_value;
        default:
            fprintf(stderr, "Unexpected token type %s\n", json_token_type_to_string((*token_entry)->type));
            exit(EXIT_FAILURE);
    }
} 

int main(int argc, char *argv[]) {

    if (argc == 1) {
        printf("No argument given for filename\n");
        printf("Usage: parse_json <filename>\n");
        exit(1);
    }

    char *filename = argv[1];
    FILE *file;
    file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Error: could not open file %s, does it exist?\n", filename);
        return 1;
    }

    size_t capacity = 1024;
    size_t top = 0;
    char *buffer = malloc(capacity);

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (top == capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
        }
        buffer[top++] = (char)ch;
    }

    fclose(file);

    JsonTokenEntry *token_entry = json_tokenize(buffer, top);
    // printf("value: %s\n", json_token_type_to_string(token_entry_head->type));
    // printf("next value: %s\n", json_token_type_to_string(token_entry_head->next->type));
    // printf("next value: %s\n", json_token_type_to_string(token_entry_head->next->next->type));
    // printf("next value: %s\n", json_token_type_to_string(token_entry_head->next->next->next->type));
    // printf("next far value: %s\n", json_token_type_to_string(token_entry_head->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->type));
    // printf("_________________________________________\n");

    JsonValue *parsed_json = parse_value(&token_entry);
    // HashTable *table = parsed_json->value.object;
    // if (!get_entry(table, "a_boolean")) {
    //     printf("a_boolean not found\n");
    // }
    // printf("a_string: '%f'\n", get_entry(table, "a_number")->value.number);
    // printf("a_number: '%s'\n", get_entry(table, "a_string")->value.string);
    // printf("a_boolean: '%d'\n", get_entry(table, "a_boolean")->value.boolean);
    // printf("another_boolean: '%d'\n", get_entry(table, "another_boolean")->value.boolean);

    free(token_entry);
    free(parsed_json);
    free(buffer);
}
