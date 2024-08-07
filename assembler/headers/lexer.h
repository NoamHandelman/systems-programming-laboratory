#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/globals.h"
#include "../headers/data_struct.h"
#include "../headers/code_conversions.h"
#include "../headers/errors.h"

#define MAX_DATA_NUM 16383
#define MIN_DATA_NUM -16384

int get_opcode(const char *);

int get_register(const char *);

int is_valid_instruction(const char *);

int is_valid_symbol(const char *, Symbol **, char *, int, const char *);

void handle_data_or_string(char *, Symbol **, int *, Machine_Code_Image *, int *, int, const char *);

int handle_extern(char *, Symbol **, int *, int *, int, const char *);

int handle_entry(char *, Symbol **, Declaration **);

int handle_instruction(char *, Symbol **, int *, Machine_Code_Image *, int *, int, const char *);

#endif
