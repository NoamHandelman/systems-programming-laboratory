#ifndef ASSEMBLER_TRANSITIONS_H
#define ASSEMBLER_TRANSITIONS_H

#include <stdio.h>
#include <string.h>
#include "./strings.h"
#include "./globals.h"
#include "./lexer.h"
#include "./code_conversions.h"

int exec_first_pass(const char *);

int exec_second_pass(const char *);

#endif
