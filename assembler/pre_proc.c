#include "headers/pre_proc.h"

/**
 * @brief Function to validate macro definition.
 * @param name The name of the macro.
 * @param macro_list The list of macros.
 * @param line The line that caused the error.
 * @param line_number The line number where the error occurred.
 * @param am_filename The name of the am file.
 * @return 1 if the macro is valid, 0 otherwise (If the name length is bigger then 31, or if the name is a reserved word, or if its already defined).
 */

int validate_macro(const char *name, Macro *macro_list, char *line, int line_number, const char *am_filename)
{
    if (strlen(name) > MAX_SYMBOL_LENGTH)
    {
        display_error(line, line_number, "Macro name is too long", am_filename);
        return 0;
    }

    if (get_opcode(name) >= 0 || is_valid_instruction(name) || get_register(name) >= 0)
    {
        display_error(line, line_number, "Macro name can not be a reserved word", am_filename);
        return 0;
    }

    if (find_macro(macro_list, name))
    {
        display_error(line, line_number, "Macro name already defined", am_filename);
        return 0;
    }

    return 1;
}

/**
 * @brief Handle error in the pre proccess stage.
 * @param message The error message to display.
 * @param line The line that caused the error.
 * @param line_number The line number where the error occurred.
 * @param macro_list The list of macros.
 * @param am_filename The name of the am file.
 * @param as_file The file pointer for the as file.
 * @param am_file The file pointer for the am file.
 * @return NULL (to indicate an error occurred).
 */

void *handle_preproc_error(const char *message, char *line, int line_number, Macro *macro_list, char *am_filename, FILE *as_file, FILE *am_file)
{
    display_error(line, line_number, message, am_filename);
    free_macros(macro_list);
    free(am_filename);
    fclose(as_file);
    fclose(am_file);
    return NULL;
}

/**
 * @brief The main function for the pre proccess stage.
 * @param input_filename The name of the input file.
 * @return the path for the am file.
 */

char *exec_preproc(const char *input_filename)
{
    FILE *as_file, *am_file = NULL;
    /**
     * In the pre process stage we will hold the line on a very large buffer and only on the firs pass we will valid the line length.
     */
    char line[INITIAL_BUFFER_SIZE];
    char *am_filename;
    Macro *macro_list = NULL, *current_macro = NULL;
    int in_macro = 0;
    int should_continue = 1;
    int line_number = 0;

    am_filename = create_file(input_filename, ".am");
    if (!am_filename)
    {
        printf("ERROR: Failed to create file path for %s\n", input_filename);
        return 0;
    }

    as_file = fopen(input_filename, "r");
    if (!as_file)
    {
        printf("ERROR: Failed to open file %s\n", input_filename);
        free(am_filename);
        return 0;
    }

    am_file = fopen(am_filename, "w");
    if (!am_file)
    {
        printf("ERROR: Failed to open file %s\n", input_filename);
        free(am_filename);
        fclose(as_file);
        return 0;
    }

    while (fgets(line, sizeof(line), as_file))
    {
        char *token, *macro_name;
        char line_copy[INITIAL_BUFFER_SIZE];
        line_number++;
        strcpy(line_copy, line);
        if (is_empty_line(line))
        {
            continue;
        }
        token = strtok(line, " \t\n");
        if (token)
        {
            if (strcmp(token, "macr") == 0)
            {
                macro_name = strtok(NULL, " \t\n");
                if (macro_name)
                {
                    char *rest;
                    rest = strstr(line_copy, macro_name) + strlen(macro_name);
                    /**
                     * Check for extra characters after the macro name.
                     */
                    if (check_for_extra_chars(rest))
                    {
                        display_error(line_copy, line_number, "Extra characters after macro name", input_filename);
                        should_continue = 0;
                    }

                    /**
                     * Validate the macro name.
                     */
                    if (!validate_macro(macro_name, macro_list, line_copy, line_number, input_filename))
                    {
                        should_continue = 0;
                    }
                    current_macro = create_and_add_macro(&macro_list, macro_name);

                    /**
                     * Check if the macro was created successfully, if not exit the program.
                     */
                    if (!current_macro)
                    {
                        return handle_preproc_error("Failed to create macro", line, line_number, macro_list, am_filename, as_file, am_file);
                    }
                    in_macro = 1;
                }
                else
                {
                    display_error(line_copy, line_number, "No macro name provided", input_filename);
                    should_continue = 0;
                }
            }
            else if (strcmp(token, "endmacr") == 0)
            {
                /**
                 * Check for extra characters after end of macro declaration.
                 */
                char *rest;
                rest = strstr(line_copy, "endmacr") + strlen("endmacr");
                if (check_for_extra_chars(rest))
                {
                    display_error(line_copy, line_number, "Extra characters after end of macro declaration", input_filename);
                    should_continue = 0;
                }
                in_macro = 0;
                current_macro = NULL;
            }
            else if (in_macro)
            {
                /**
                 * Add the line to the current macro, if the allocation failed exit the program.
                 */
                if (!add_macro_line(current_macro, line_copy))
                {
                    return handle_preproc_error("Failed to add line to macro", line, line_number, macro_list, am_filename, as_file, am_file);
                };
            }
            else
            {
                Macro *macro;
                macro = find_macro(macro_list, token);
                if (macro)
                {
                    int i;
                    for (i = 0; i < macro->line_count; i++)
                    {
                        fprintf(am_file, "%s", macro->content[i]);
                    }
                }
                else
                {
                    fprintf(am_file, "%s", line_copy);
                }
            }
        }
        else if (!in_macro)
        {
            fprintf(am_file, "%s", line_copy);
        }
    }

    fclose(as_file);
    fclose(am_file);
    free_macros(macro_list);
    return should_continue ? am_filename : NULL;
}
