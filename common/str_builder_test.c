#include <stdio.h>
#include <stdlib.h>
#include "str_builder.h"

int main(int argc, char **argv)
{
    str_builder_t *builder;
    char          *str;

    builder = str_builder_create();
    printf("create\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str_builder_add_str(builder, "Test", 0);
    printf("Add 'Test'\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str_builder_add_str(builder, " ", 0);
    printf("Add ' '\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str_builder_add_int(builder, 123);
    printf("Add int 123\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str_builder_truncate(builder, str_builder_len(builder)-2);
    printf("Truncate -2\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str_builder_drop(builder, 3);
    printf("Drop 3\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));

    str = str_builder_dump(builder, NULL);
    printf("Dump\n");
    printf("\tsb len=%zu\n\tsb='%s'\n", str_builder_len(builder), str_builder_peek(builder));
    printf("\tstr='%s'\n", str);
    free(str);

    str_builder_destroy(builder);
    return 0;
}
