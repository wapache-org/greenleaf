#ifndef CS_MONGOOSE_SRC_STRINGS_H_
#define CS_MONGOOSE_SRC_STRINGS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// int str_split(char* str, char* split, char** strs, int* size)
// {
//   char *p;
//   while((p=strtok(&str, split))!=NULL) // strtok, strsep
//   {
//     printf("%s\n", p);
//   }
// }

// char** str_split(char sep, const char *str, int *size)
// {
//     int count = 0, i;
//     for(i = 0; i < strlen(str); i++)
//     {       
//         if (str[i] == sep)
//         {       
//             count ++;
//         }
//     }

//     char **ret = calloc(++count, sizeof(char *));

//     int lastindex = -1;
//     int j = 0;

//     for(i = 0; i < strlen(str); i++)
//     {
//         if (str[i] == sep)
//         {       
//             ret[j] = calloc(i - lastindex, sizeof(char));
//             memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
//             j++;
//             lastindex = i;
//         }
//     }
    
//     if (lastindex <= strlen(str) - 1)
//     {
//         ret[j] = calloc(strlen(str) - lastindex, sizeof(char));
//         memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
//         j++;
//     }

//     *size = j;
//     return ret;
// }

// void str_split_free(char** strs, int size)
// {
//     for(int i = 0; i < size; i++)
//     {
//         free(strs[i]);
//     }
//     free(strs);
// }

#endif