#include "../headers/code_conversions.h"

#define MACHINE_WORD_SIZE 15

unsigned short encode_addressing_mode(int addressing_mode)
{
    unsigned short word = 0;
    if (addressing_mode == 0)
        word = 1;
    else if (addressing_mode == 1)
        word = 2;
    else if (addressing_mode == 2)
        word = 4;
    else if (addressing_mode == 3)
        word = 8;
    return word;
}

void encode_instruction(Instruction *instruction, Machine_Code_Image *code_image, int *IC)
{
    int opcode_index = get_opcode(instruction->op_code.opcode);
    int i;

    code_image[*IC].value = 0;

    code_image[*IC].value |= (opcode_index << 11);

    if (instruction->operand_count > 0)
    {
        code_image[*IC].value |= (encode_addressing_mode(instruction->operands[0].addressing_mode) << 7);
    }

    if (instruction->operand_count > 1)
    {
        code_image[*IC].value |= (encode_addressing_mode(instruction->operands[1].addressing_mode) << 3);
    }

    code_image[*IC].value |= (1 << 2);

    printf("Machine code value: ");
    for (i = 0; i < MACHINE_WORD_SIZE; i++)
    {
        printf("%d", (code_image[*IC].value >> (MACHINE_WORD_SIZE - 1 - i)) & 1);
    }
    printf("\n");

    (*IC)++;
    printf("Done processing first word of instruction\n");

    for (i = 0; i < instruction->operand_count; i++)
    {
        code_image[*IC].value = 0;
        code_image[*IC].symbol = NULL;

        if (instruction->operands[i].addressing_mode == 0)
        {
            code_image[*IC].value |= (instruction->operands[i].value.num << 3);
        }
        else if (instruction->operands[i].addressing_mode == 1)
        {
            code_image[*IC].symbol = instruction->operands[i].value.symbol;
        }
        else if (instruction->operands[i].addressing_mode == 2 || instruction->operands[i].addressing_mode == 3)
        {
            if (instruction->operand_count == 2 &&
                (instruction->operands[0].addressing_mode == 2 || instruction->operands[0].addressing_mode == 3) &&
                (instruction->operands[1].addressing_mode == 2 || instruction->operands[1].addressing_mode == 3))
            {
                code_image[*IC].value |= (instruction->operands[0].value.reg << 6);
                code_image[*IC].value |= (instruction->operands[1].value.reg << 3);
                code_image[(*IC)++].value |= (1 << 2);
                break;
            }
            else
            {
                if (i == 0)
                {
                    code_image[*IC].value |= (instruction->operands[i].value.reg << 6);
                }
                else
                {
                    code_image[*IC].value |= (instruction->operands[i].value.reg << 3);
                }
            }
            code_image[(*IC)++].value |= (1 << 2);
        }
    }
}