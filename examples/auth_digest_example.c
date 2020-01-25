/*

# Digest Authentication

Mongoose has a built-in Digest (MD5) authentication support. In order to
enable Digest authentication, create a file `.htpasswd` in the directory
you would like to protect. That file should be in the format that Apache's
`htdigest` utility.

You can use either Apache `htdigest` utility, or
Mongoose pre-build binary at https://www.cesanta.com/binary.html
to add new users into that file:

```
mongoose -A /path/to/.htdigest mydomain.com joe joes_password
```


 */
#include "mongoose.h"

int main(int argc, char* argv[])
{
    // there are two way to du the digest auth:
    // 1. use auth file, just set the http options and add .htpasswd to the dir.
    // 2. use custom callback function, the example code see api_server_example.c

    return 0;
}
