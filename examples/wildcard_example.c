#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The main function that checks if two given strings 
// match. The first string may contain wildcard characters 
int recursive_match(char *first, char * second) 
{ 
    // If we reach at the end of both strings, we are done 
    if (*first == '\0' && *second == '\0') 
        return 0; 
  
    // Make sure that the characters after '*' are present 
    // in second string. This function assumes that the first 
    // string will not contain two consecutive '*' 
    if (*first == '*' && *(first+1) != '\0' && *second == '\0') 
        return 1; 
  
    // If the first string contains '?', or current characters 
    // of both strings match 
    if (*first == '?' || *first == *second) 
        return match(first+1, second+1); 
  
    // If there is *, then there are two possibilities 
    // a) We consider current character of second string 
    // b) We ignore current character of second string. 
    if (*first == '*') 
        return match(first+1, second) || match(first, second+1); 
    return 1; 
} 


/**
 * * --> Matches with 0 or more instances of any character or set of characters.
 * ? --> Matches with any one character.
 */
int non_recursive_match(char f[], char s[])
{
    int m = strlen(f);
    int n = strlen(s);
    int i = 0, j = 0;
    int c = 0;

    while (i <= m && j <= n)
    {

        if (j == n && i == m)
        {
            c = 1;
            break;
        }
        else if (f[i] == s[j])
        {
            i++;
            j++;
        }

        if (f[i] == '*')
        {
            i++;
            while (f[i] != s[j] && j < n)
            {
                j++;
                if (f[i] == '?')
                {
                    j++;
                    i++;
                }
            }

            if (j == n)
            {
                return 0;
            }
            else
            {
                i++;
                j++;
            }
        }

        if (f[i] == '?' && j != n)
        {
            j++;
            i++;
        }

        if (f[i] != s[j])
            return c;
    }
    return c;
}

/**
 * 
 * https://developpaper.com/using-apache-ant-path-matching-library-under-openresty/
 * https://stackoverflow.com/questions/69835/how-do-i-use-nant-ant-naming-patterns
 * 
 * The rules are:
 * 
 *     a single star (*) matches zero or more characters within a path name
 *     a double star (**) matches zero or more characters across directory levels
 *     a question mark (?) matches exactly one character within a path name
 * 
 * Another way to think about it is double star (**) matches slash (/) but single star (*) does not.
 */
int ant_style_path_match(char f[], char s[])
{
    // 实现思路1
    // 将它翻译成正则,然后用正则来实现匹配
    // 将**转换为([!/]/)*
    // 将*转换为[!/]*
    // 将?转换为[!/]?
    // 将${xxx}转换为(xxx:[a-zA-Z\d_\-]+), 命名分组

    // 实现思路2
    // 构造一个ant_path结构体, 
    // 先将字符串解析成ant_path
    // 将目标路径转换成数组
    // 再写一个匹配程序

    return 0;
}


int main()
{
    char f[] = "g**k?s";
    char s[] = "geemnkts";

    if (non_recursive_match(f, s))
        printf("Matched");
    else
        printf("Not matched");
    return 0;
}