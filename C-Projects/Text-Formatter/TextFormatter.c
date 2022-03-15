#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 0
#if DEBUG
#define DUMP_DBL(x) printf("line %d: %s = %d\n", __LINE__, #x, x)
#else
#define DUMP_DBL(x)
#endif

#define CHARS_PER_LINE 999

typedef char line_t[CHARS_PER_LINE + 1];

/*reduce number of function arguments*/
typedef struct
{
    int *insert_blank;
    int line_len;
    int input_len;
    int nxt_wrd_len;
    int *margin;
    int *max_line_len;
    int *valid_b;
    int *valid_p;
    int *section;
} var_t;

int mygetchar(void);
int get_line(char L[], int limit);
int next_word_len(char input[], int start, int max);
void print_margin(int *m);
int compress_blanks(char input[], int start, int *blank);
int print_big_word(char input[], int start, int max, int *m);
int get_number(line_t input, int start, int max);
int is_member(char x, char *string);
int command(char x, int num, var_t box);
int c_command(char input[], var_t box);
int h_command(char input[], int num, var_t box);
int print_line(line_t input, var_t box, int first_line);

int main(int argc, char *argv[])
{
    line_t one_line;
    int repeat = 1, first = 1, first_line = 1;
    int zero = 0, one = 1, uno = 1, four = 4, fifty = 50;
    var_t variables = {&zero, 0, 0, 0, &four, &fifty, &one, &uno};
    int arr[5] = {0};
    variables.section = &arr[0];

    while (repeat)
    {
        /*get_line will return 0 at EOF, but still operates on the
        last line*/
        repeat = get_line(one_line, CHARS_PER_LINE);

        if (first)
        {
            print_margin(variables.margin);
        }
        first = 0;

        /*print_line returns line_len to preserve its value through multiple
        calls to the function*/
        variables.line_len = print_line(one_line, variables, first_line);

        first_line = 0;
    }
    return 0;
}

int print_line(line_t input, var_t box, int first_line)
{
    int i = 0, num;
    char x;
    box.input_len = strlen(input);

    for (i = 0; i < box.input_len; i++)
    {

        *box.insert_blank = 0;

        /*discard current line if it starts with a period*/
        if (!i && input[i] == '.')
        {
            i += 1;
            x = input[i];

            if (is_member(x, "bp"))
            {
                box.line_len = command(x, 0, box);
                i += 1;
            }
            else if (is_member(x, "lw"))
            {
                i += 2;
                /*num is the new width or new margin length*/
                num = get_number(input, i, box.input_len);
                i += 2;
                box.line_len = command(x, num, box);
            }
            else if (x == 'c')
            {
                box.line_len = c_command(input, box);
                break;
            }
            else if (x == 'h')
            {
                i += 2;
                num = get_number(input, i, box.input_len);
                box.line_len = h_command(input, num, box);
                break;
            }
            else
            {
                break;
            }
        }

        /*skip the index over multiple blank spaces*/
        i += compress_blanks(input, i, box.insert_blank);

        box.nxt_wrd_len = next_word_len(input, i, box.input_len);
        /*if the current line length + length of next word exceeds the max
        line length, print a new line. The potential blank space will be
        added as well*/
        if ((box.line_len + box.nxt_wrd_len + *box.insert_blank) > *box.max_line_len)
        {
            printf("\n");
            print_margin(box.margin);
            box.line_len = 0;
        }

        /*if we haven't entered a new line, the potential blank is now valid
        to be printed. note that when i is 0, the previous character is \n,
        so we also want to insert blank between words either side of \n*/
        else if ((*box.insert_blank || !i) && box.line_len)
        {
            printf(" ");
            box.line_len += 1;
        }

        /*massive word*/
        if (box.nxt_wrd_len > *box.max_line_len)
        {
            if (!*box.valid_p)
            {
                print_margin(box.margin);
                *box.valid_p = 1;
            }
            i += print_big_word(input, i, box.input_len, box.margin);
            box.line_len = 0;

            *box.valid_p = 1;
            *box.valid_b = 1;
        }

        else if (i < box.input_len)
        {
            /*to avoid printing multiple margins through consecutive commands,
            this ensures only one margin is printed by the time the code
            reaches a non-blank character*/
            if (!*box.valid_p)
            {
                print_margin(box.margin);
                *box.valid_p = 1;
            }
            /*input[i] is a valid, non-white space character in the
            correct line, we can print and increment line_len*/
            printf("%c", input[i]);
            box.line_len += 1;

            *box.valid_p = 1;
            *box.valid_b = 1;
        }
    }
    return box.line_len;
}

int mygetchar(void)
{
    int c;
    while ((c = getchar()) == '\r')
    {
    }
    return c;
}

/*fetch the next line from stdin*/
int get_line(char L[], int limit)
{
    int c, len = 0;

    while ((c = mygetchar()) != EOF && (c != '\n') && (len < limit))
    {
        L[len++] = c;
    }
    L[len] = '\0';

    if (c == EOF)
    {
        return 0;
    }
    /*this is used to keep the main loop running*/
    return 1;
}

/*take the input text and return the next word length
after the starting position.*/
int next_word_len(char input[], int start, int max)
{
    int j = start, len = 0;
    /*punctuation is included for nice formatting*/
    while (j < max && input[j] != ' ' && input[j] != '\n')
    {
        len += 1;
        j += 1;
    }
    return len;
}

void print_margin(int *m)
{
    int j;
    for (j = 0; j < *m; j++)
    {
        printf(" ");
    }
}

int compress_blanks(char input[], int start, int *blank)
{
    int j = start;
    /*jump over excess blank spaces*/
    while (input[j] == '\t' || input[j] == ' ')
    {
        /*all blank spaces get collapsed to one potential blank*/
        *blank = 1;
        j += 1;
    }
    return j - start;
}

/*word being greater than the max line length, gets printed on
its own line.*/
int print_big_word(char input[], int start, int max, int *m)
{
    int j = start;
    while (input[j] != ' ' && input[j] != '\n' && j < max)
    {
        printf("%c", input[j]);
        j += 1;
    }

    printf("\n");
    print_margin(m);

    return j - start;
}

/*STAGE 2*/
int get_number(line_t input, int start, int max)
{
    int k = start;
    /* nn is at most 99 */
    char number[100];
    while (k < max && atoi(&input[k]))
    {
        k += 1;
    }
    /*copy the string of read digits and then convert to int*/
    strncpy(number, &input[start], k);
    return atoi(number);
}

int is_member(char x, char *string)
{
    int k;
    for (k = 0; k < strlen(string); k++)
    {
        if (string[k] == x)
        {
            return 1;
        }
    }
    return 0;
}

/*return 0 to reset line length*/
int command(char x, int num, var_t box)
{
    if (x == 'b' && *box.valid_b)
    {
        printf("\n");
        print_margin(box.margin);
        /*don't print multiple breaks through consecutive commands*/
        *box.valid_b = 0;
    }
    else if (x == 'p' && *box.valid_p)
    {
        printf("\n\n");
        *box.valid_p = 0;
    }
    else if (x == 'l')
    {
        /*margin in the struct points to specified number*/
        *box.margin = num;
        if (*box.valid_p)
        {
            printf("\n\n");
        }
        *box.valid_p = 0;
    }
    else if (x == 'w')
    {
        *box.max_line_len = num;
        if (*box.valid_p)
        {
            printf("\n\n");
        }
        *box.valid_p = 0;
    }
    return 0;
}

/*STAGE 3*/
int c_command(char input[], var_t box)
{
    int j, length = 0, blanks = 0, not_cool = 0;

    if (strlen(input) == 2)
    {
        if (*box.valid_p)
        {
            printf("\n");
            *box.valid_p = 0;
        }
        return 0;
    }
    if (*box.valid_p)
    {
        printf("\n");
    }
    /*construct the length of this line*/
    for (j = 2; input[j] != '\0'; j++)
    {
        while (input[j] == '\t' || input[j] == ' ')
        {
            blanks = 1;
            j += 1;
        }
        if (j == 3 || input[j] == '\0')
        {
            blanks = 0;
        }
        length += blanks + 1;
        blanks = 0;
    }
    print_margin(box.margin);

    /*centre the line, rounding down, by adding blanks*/
    blanks = (*box.max_line_len - length) / 2;
    for (j = 0; j < blanks; j++)
    {
        printf(" ");
    }

    blanks = 0;
    length = 0;
    /*time to print out the line*/
    for (j = 2; input[j] != '\0'; j++)
    {
        while (input[j] == '\t' || input[j] == ' ')
        {
            j += 1;
            /*only print one space between words*/
            blanks = 1;
            if (input[j] == '\0')
            {
                not_cool = 1;
            }
        }
        if (not_cool)
        {
            break;
        }

        /*don't print blank if it's the very first char seen*/
        if (blanks && length)
        {
            printf(" ");
        }
        printf("%c", input[j]);
        length = 1;
        blanks = 0;
    }
    /*to avoid multiple line breaks with consecutive commands, keep tabs on
    a pointer to valid paragraph*/
    printf("\n");
    *box.valid_p = 0;
    return 0;
}

int h_command(char input[], int num, var_t box)
{
    int j, level = num - 1, blanks = 0;

    if (*box.valid_p)
    {
        printf("\n\n");
    }
    print_margin(box.margin);

    /*on first level calls, we print some -*/
    if (!level)
    {
        for (j = 0; j < *box.max_line_len; j++)
        {
            printf("-");
        }
        printf("\n");
        print_margin(box.margin);
    }

    /*modify level and reset any further ones*/
    box.section[level] += 1;
    for (j = level + 1; j < 5; j++)
    {
        box.section[j] = 0;
    }

    /*print the section number with periods in the middle*/
    for (j = 0; j < 5; j++)
    {
        if (!box.section[j])
        {
            break;
        }
        if ((0 < j) && (j < 5))
        {
            printf(".");
        }
        printf("%d", box.section[j]);
    }

    /*ready to print title*/
    for (j = 4; input[j] != '\0'; j++)
    {
        while (input[j] == '\t' || input[j] == ' ')
        {
            j += 1;
            blanks = 1;
        }

        if (blanks)
        {
            printf(" ");
        }
        /*printing ze heading*/
        printf("%c", input[j]);
        blanks = 0;
    }

    printf("\n\n");
    /*for consecutive line-breaking commands*/
    *box.valid_p = 0;
    return 0;
}
