#include "../headers/lexer.h"

OP_CODE OP_CODES[] = {
    {"mov", 2},
    {"cmp", 2},
    {"add", 2},
    {"sub", 2},
    {"lea", 1},
    {"clr", 1},
    {"not", 2},
    {"inc", 1},
    {"dec", 1},
    {"jmp", 1},
    {"bne", 1},
    {"red", 1},
    {"prn", 1},
    {"jsr", 1},
    {"rts", 0},
    {"stop", 0}};

/* Define the registers */
char *REGISTERS[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

/* Define the instructions */
char *INSTRUCTIONS[] = {".data", ".string", ".extern", ".entry"};

/**
 * @brief Function tp extract the numbers from the .data directive and add them to the data image.
 * @return 1 if the process was successful, 0 otherwise.
 */

int parse_data_dir(char *, int *, Machine_Code_Image *, int, const char *, char *);

/**
 * @brief Function to extract each char from the .string directive and add the asci code to the data image.
 * @return 1 if the process was successful, 0 otherwise.
 */

int parse_string_dir(char *, int *, Machine_Code_Image *, int, const char *, char *);

int get_addressing_mode(const char *);

Instruction *parse_instruction(const char *);

int get_opcode(const char *op)
{
    int i;
    for (i = 0; i < OP_CODES_COUNT; i++)
    {
        if (strcmp(op, OP_CODES[i].opcode) == 0)
        {
            return i;
        }
    }
    return -1;
}

int get_register(const char *reg)
{
    int i;
    for (i = 0; i < REGISTERS_COUNT; i++)
    {
        if (strcmp(reg, REGISTERS[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

int is_valid_instruction(const char *inst)
{
    int i;
    for (i = 0; i < INSTRUCTIONS_COUNT; i++)
    {
        if (strcmp(inst, INSTRUCTIONS[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_valid_symbol(const char *symbol, Symbol **symbol_table, char *line, int line_number, const char *input_filename)
{
    int i;

    if (strlen(symbol) > MAX_SYMBOL_LENGTH)
    {
        /**
         *         fprintf(stderr, "Symbol %s has illegal name length\n", symbol);
         */
        display_error(line, line_number, "Symbol has illegal name length", input_filename);
        return 0;
    }

    if (find_symbol(*symbol_table, symbol))
    {
        /**
         *        fprintf(stderr, "Symbol %s already defined\n", symbol);
         */
        display_error(line, line_number, "Symbol already defined", input_filename);
        return 0;
    }

    if (get_opcode(symbol) >= 0 || get_register(symbol) >= 0 || is_valid_instruction(symbol))
    {
        /**
         *       fprintf(stderr, "Symbol %s is a reserved word\n", symbol);
         */
        display_error(line, line_number, "Symbol can not be a reserved word", input_filename);
        return 0;
    }

    if (!isalpha(symbol[0]))
    {
        /**
         *      fprintf(stderr, "Symbol %s must start with a letter\n", symbol);
         */
        display_error(line, line_number, "Symbol must start with a letter", input_filename);
        return 0;
    }

    for (i = 1; i < strlen(symbol); i++)
    {
        if (!isalnum(symbol[i]))
        {
            /**
             *        fprintf(stderr, "Symbol %s must contains only letters or numbers\n", symbol);
             */
            display_error(line, line_number, "Symbol must contains only letters or numbers", input_filename);
            return 0;
        }
    }

    return 1;
}

int parse_data_dir(char *line, int *DC, Machine_Code_Image *data_image, int line_number, const char *input_filename, char *full_line)
{
    char *token;
    char *endptr;
    int value;
    int expecting_number = 1;
    int error_found = 1;

    if (line[0] == ',' || line[strlen(line) - 1] == ',')
    {
        error_found = 0;
        display_error(full_line, line_number, "Line should not start or end with a comma", input_filename);
    }

    token = strtok(line, " ");
    while (token)
    {
        if (expecting_number)
        {
            value = strtol(token, &endptr, 10);
            if (*endptr == '\0')
            {
                if (value < MIN_DATA_NUM || value > MAX_DATA_NUM)
                {
                    display_error(full_line, line_number, "Number out of valid range", input_filename);
                    error_found = 0;
                }
                else
                {
                    data_image[(*DC)++].value = value;
                    printf("Parsed number: %d\n", value);
                }
                expecting_number = 0;
            }
            else
            {
                error_found = 0;
                expecting_number = 0;
                display_error(full_line, line_number, "Found invalid token (not a number)", input_filename);
            }
        }
        else
        {
            if (strcmp(token, ",") == 0)
            {
                expecting_number = 1;
            }
            else
            {
                display_error(full_line, line_number, "Numbers should be separated by comma", input_filename);
                error_found = 0;
            }
        }
        token = strtok(NULL, " ");
    }

    if (expecting_number)
    {
        display_error(full_line, line_number, "Expected number but not found", input_filename);
        error_found = 0;
    }

    return error_found;
}

int parse_string_dir(char *line, int *DC, Machine_Code_Image *data_image, int line_number, const char *input_filename, char *full_line)
{
    char *line_copy;
    int i, error_found = 1;
    char *start_quote, *end_quote;

    printf("initial line : %s\n", line);

    start_quote = strchr(line, '"');

    if (start_quote)
    {
        end_quote = strchr(start_quote + 1, '"');
    }

    if (!start_quote || !end_quote || start_quote == end_quote)
    {
        display_error(full_line, line_number, "String should start and end with a double quote", input_filename);
        error_found = 0;
    }

    for (i = 0; i < start_quote - line; i++)
    {
        if (!isspace(line[i]))
        {
            display_error(full_line, line_number, "Unexpected characters before the start of the string", input_filename);
            error_found = 0;
            break;
        }
    }

    for (i = end_quote - line + 1; i < strlen(line); i++)
    {
        if (!isspace(line[i]))
        {
            display_error(full_line, line_number, "Unexpected characters after the end of the string", input_filename);
            error_found = 0;
            break;
        }
    }

    line_copy = start_quote + 1;
    while (line_copy < end_quote)
    {
        printf("ASCII value of %c: %d\n", *line_copy, *line_copy);
        data_image[(*DC)++].value = *line_copy++;
    }

    printf("End of string sign: 0\n");
    data_image[(*DC)++].value = '\0';

    return error_found;
}

int get_addressing_mode(const char *operand)
{
    if (operand[0] == '#')
    {
        return 0;
    }

    if (operand[0] == '*')
    {
        return 2;
    }

    if (operand[0] == 'r')
    {
        return 3;
    }

    return 1;
}

/**
 * should also valid address mode is ok for each instruction
 */

Instruction *parse_instruction(const char *line)
{
    char line_copy[MAX_LINE_LENGTH];
    char *token;
    Instruction *instr;
    int operand_count = 0;

    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';

    instr = (Instruction *)malloc(sizeof(Instruction));
    if (!instr)
    {
        return NULL;
    }

    instr->operand_count = 0;

    token = strtok(line_copy, " ");

    if (!token)
    {
        free(instr);
        return NULL;
    }

    if (get_opcode(token) == -1)
    {
        printf("Invalid opcode: %s\n", token);
        free(instr);
        return NULL;
    }

    instr->op_code.opcode = (char *)malloc(strlen(token) + 1);
    if (!instr->op_code.opcode)
    {
        free(instr);
        return NULL;
    }

    strcpy(instr->op_code.opcode, token);

    while ((token = strtok(NULL, " ,")) && operand_count < 2)
    {
        int addressing_mode;
        addressing_mode = get_addressing_mode(token);
        instr->operands[operand_count].addressing_mode = addressing_mode;
        if (addressing_mode == 0)
        {
            instr->operands[operand_count].value.num = atoi(token + 1);
        }
        else if (addressing_mode == 2 || addressing_mode == 3)
        {
            instr->operands[operand_count].value.reg = atoi(token + (addressing_mode == 2 ? 2 : 1));
        }
        else
        {
            instr->operands[operand_count].value.symbol = (char *)malloc(strlen(token) + 1);
            if (!instr->operands[operand_count].value.symbol)
            {
                free(instr->op_code.opcode);
                free(instr);
                return NULL;
            }
            strcpy(instr->operands[operand_count].value.symbol, token);
        }
        operand_count++;
    }
    instr->operand_count = operand_count;

    return instr;
}

/**
 * @brief Main function to process data or string directive.
 * @param line The line to process.
 * @param symbol_table The symbol table.
 * @param DC The data counter.
 * @param data_image The data image array.
 * @param should_continue A flag to indicate if the process should continue.
 * @return 1 if the process was successful, 0 otherwise.
 */

void handle_data_or_string(char *line, Symbol **symbol_table, int *DC, Machine_Code_Image *data_image, int *should_continue, int line_number, const char *input_filename)
{
    char symbol_name[MAX_LINE_LENGTH];
    char *current = line;
    char *token;
    char *directive;
    char original_line[MAX_LINE_LENGTH];

    strncpy(original_line, line, MAX_LINE_LENGTH);

    token = strtok(current, " ");
    if (token && token[strlen(token) - 1] == ':')
    {
        strncpy(symbol_name, token, strlen(token) - 1);
        symbol_name[strlen(token) - 1] = '\0';

        if (!is_valid_symbol(symbol_name, symbol_table, original_line, line_number, input_filename))
        {
            should_continue = 0;
        }
        else
        {
            if (!create_and_add_symbol(symbol_table, symbol_name, *DC, 0, 1))
            {
                display_error(original_line, line_number, "Failed to create and add symbol, memory allocation failed", input_filename);
                *should_continue = -1;
                return;
            };
        }

        token = strtok(NULL, " ");
    }

    if (token)
    {
        directive = token;
        current = strtok(NULL, "");

        if (strcmp(directive, ".data") == 0)
        {
            *should_continue = parse_data_dir(current, DC, data_image, line_number, input_filename, original_line);
        }

        else if (strcmp(directive, ".string") == 0)
        {

            *should_continue = parse_string_dir(current, DC, data_image, line_number, input_filename, original_line);
        }
        else
        {
            display_error(original_line, line_number, "Invalid directive", input_filename);
        }
    }
}

int handle_extern(char *line, Symbol **symbol_table, int *externs_count, int *should_continue, int line_number, const char *input_filename)
{
    char symbol_name[MAX_SYMBOL_LENGTH];
    char extra_forbidden_symbol[MAX_SYMBOL_LENGTH];

    if (sscanf(line, ".extern %s %s", symbol_name, extra_forbidden_symbol) == 1)
    {
        if (is_valid_symbol(symbol_name, symbol_table, line, line_number, input_filename))
        {
            (*externs_count)++;
            return create_and_add_symbol(symbol_table, symbol_name, 0, 1, 0);
        }
        else
        {
            printf("Symbol %s is not valid\n", symbol_name);
        }
    }

    return 1;
}

int handle_entry(char *line, Symbol **symbol_table, Declaration **entries)
{
    char symbol_name[MAX_SYMBOL_LENGTH];
    char extra_forbidden_symbol[MAX_SYMBOL_LENGTH];

    if (sscanf(line, ".entry %s %s", symbol_name, extra_forbidden_symbol) == 1)
    {
        return create_and_add_declaration(entries, symbol_name);
    }
    else
    {
        printf("Invalid entry declaration on line %s\n", line);
    }

    return 1;
}

int handle_instruction(char *line, Symbol **symbol_table, int *IC, Machine_Code_Image *code_image, int *should_continue, int line_number, const char *input_filename)
{
    char symbol_name[MAX_SYMBOL_LENGTH + 1];
    char *current = line;
    char *token;
    Instruction *instruction;

    char initial_line[MAX_LINE_LENGTH];
    strncpy(initial_line, line, MAX_LINE_LENGTH);
    initial_line[MAX_LINE_LENGTH - 1] = '\0';

    token = strtok(current, " ");
    if (token && token[strlen(token) - 1] == ':')
    {
        strncpy(symbol_name, token, strlen(token) - 1);
        symbol_name[strlen(token) - 1] = '\0';

        if (is_valid_symbol(symbol_name, symbol_table, line, line_number, input_filename))
        {
            create_and_add_symbol(symbol_table, symbol_name, *IC, 0, 0);
        }

        token = strtok(NULL, "");
    }

    else
    {
        token = initial_line;
    }

    if (token)
    {
        instruction = parse_instruction(token);
        if (!instruction)
        {
            fprintf(stderr, "Error parsing instruction\n");
            return 0;
        }

        encode_instruction(instruction, code_image, IC);
    }

    return 1;
}
