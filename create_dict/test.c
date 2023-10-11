#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "dict_0.c"
#include "dict_1.c"
#include "dict_2.c"
#include "dict_3.c"
#include "dict_4.c"
#include "dict_5.c"
#include "dict_6.c"
#include "dict_7.c"
#include "dict_8.c"
#include "dict_9.c"
#include "dict_10.c"
#include "dict_11.c"

#define MORE_MASK 0x800000
#define LETTER_MASK 0x7C0000
#define END_MASK 0x20000
#define INDEX_MASK 0x1FFFF

#define BAG_STEP 4904

void dict_switch(const int** d, int i)
{
     if (i  < BAG_STEP)
    {
        *d = &dict_0[0];
        *d += i*3;
    }
    else if (i < BAG_STEP*2)
    {
        *d = &dict_1[0];
        *d += (i - BAG_STEP)*3;
    }
    else if (i < BAG_STEP*3)
    {
        *d = &dict_2[0];
        *d += (i - BAG_STEP*2)*3;
    }
    else if (i < BAG_STEP*4)
    {
        *d = &dict_3[0];
        *d += (i - BAG_STEP*3)*3;
    }
    else if (i < BAG_STEP*5)
    {
        *d = &dict_4[0];
        *d += (i - BAG_STEP*4)*3;
    }
    else if (i < BAG_STEP*6)
    {
        *d = &dict_5[0];
        *d += (i - BAG_STEP*5)*3;
    }
    else if (i < BAG_STEP*7)
    {
        *d = &dict_6[0];
        *d += (i - BAG_STEP*6)*3;
    }
    else if (i < BAG_STEP*8)
    {
        *d = &dict_7[0];
        *d += (i - BAG_STEP*7)*3;
    }
    else if (i < BAG_STEP*9)
    {
        *d = &dict_8[0];
        *d += (i - BAG_STEP*8)*3;
    }
    else if (i < BAG_STEP*10)
    {
        *d = &dict_9[0];
        *d += (i - BAG_STEP*9)*3;
    }
    else if (i < BAG_STEP*11)
    {
        *d = &dict_10[0];
        *d += (i - BAG_STEP*10)*3;
    }
    else
    {
        *d = &dict_11[0];
        *d += (i - BAG_STEP*11)*3;
    }
}

int is_word(unsigned char* word, int len)
{
    int l = 0; // word letter index
    int i = 0; // dictionary entry index

    while (1)
    {
        const int *dict;

        dict_switch(&dict, i);

        int c = word[l]-97;
        int entry = (*(dict) << 16) | (*(++dict) << 8) | *(++dict);

        int more = ((entry & MORE_MASK) >> 23);
        int letter = ((entry & LETTER_MASK) >> 18);
        int end = ((entry & END_MASK) >> 17);
        int index = ((entry & INDEX_MASK));

        printf("letter: %c\n", letter+97);

        if (letter == c && (end) == (l == len-1))
        {
            printf("i: %u\n", i);
            printf("index: %u\n", index);
            if (end) return 1;
            i = index;
            l++;
        }
        else if (more) i++;
        else return 0;
    }
}

int main() {
    while(1)
    {
        char word[31];
        printf("Enter word: ");
        scanf("%30s", word);
        int len = strlen(word);

        struct timeval stop, start;
        gettimeofday(&start, NULL);
        if(is_word(word, len)) printf("> yes!\n");
        else printf("> no.\n");
        gettimeofday(&stop, NULL);
        printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    }
    return 0;
}