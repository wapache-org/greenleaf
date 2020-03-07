/*
 * authentication.c
 * This file contains an example of how to do an authentication to a
 * SSH server using libssh
 */

/*
Copyright 2003-2009 Aris Adamantiadis

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libssh/libssh.h"
#include "ssh_common.h"
/*
ssh_userauth_autopubkey: Authenticating with public keys, find key at "~/.ssh/". The return values are the following:

SSH_AUTH_ERROR: some serious error happened during authentication
SSH_AUTH_DENIED: no key matched
SSH_AUTH_SUCCESS: you are now authenticated
SSH_AUTH_PARTIAL: some key matched but you still have to provide an other mean of authentication (like a password).

 */

struct ssh_user_auth_methods {
    int unknown;
    int none;
    int password;
    int public_key;
    int host_based;
    int interactive;
    int gssapi;
};

void ssh_print_supported_auth_methods(int method_mark);
void ssh_parse_supported_auth_methods(int method_mark, struct ssh_user_auth_methods * methods);

static int auth_keyfile(ssh_session session, char* keyfile);

void ssh_print_error(ssh_session session);


// ////////////////////////////////////////////////////////////////////////////


// 匿名访问
int authenticate_none(ssh_session session)
{
  int rc;
  
  rc = ssh_userauth_none(session, NULL);
  return rc;
}

// 公钥
int authenticate_pubkey(ssh_session session)
{
  int rc;
  
  rc = ssh_userauth_publickey_auto(session, NULL, NULL);
  
  if (rc == SSH_AUTH_ERROR) ssh_print_error(session);
  
  return rc;
}

// 密码
int authenticate_password(ssh_session session, char *password)
{
  int rc;
  if(password == NULL)
    password = getpass("Enter your password: ");
  rc = ssh_userauth_password(session, NULL, password);
  if (rc == SSH_AUTH_ERROR) ssh_print_error(session);
  
  return rc;
}

// 交互
int authenticate_kbdint(ssh_session session, const char *password)
{
    int err;

    err = ssh_userauth_kbdint(session, NULL, NULL);
    while (err == SSH_AUTH_INFO) {
        const char *instruction;
        const char *name;
        char buffer[128];
        int i, n;

        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        n = ssh_userauth_kbdint_getnprompts(session);

        if (name && strlen(name) > 0) {
            printf("%s\n", name);
        }

        if (instruction && strlen(instruction) > 0) {
            printf("%s\n", instruction);
        }

        for (i = 0; i < n; i++) {
            const char *answer;
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(session, i, &echo);
            if (prompt == NULL) {
                break;
            }

            if (echo) {
                char *p;

                printf("%s", prompt);

                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    return SSH_AUTH_ERROR;
                }

                buffer[sizeof(buffer) - 1] = '\0';
                if ((p = strchr(buffer, '\n'))) {
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                    return SSH_AUTH_ERROR;
                }

                memset(buffer, 0, strlen(buffer));
            } else {
                if (password && strstr(prompt, "Password:")) {
                    answer = password;
                } else {
                    buffer[0] = '\0';

                    if (ssh_getpass(prompt, buffer, sizeof(buffer), 0, 0) < 0) {
                        return SSH_AUTH_ERROR;
                    }
                    answer = buffer;
                }
                err = ssh_userauth_kbdint_setanswer(session, i, answer);
                memset(buffer, 0, sizeof(buffer));
                if (err < 0) {
                    return SSH_AUTH_ERROR;
                }
            }
        }
        err=ssh_userauth_kbdint(session,NULL,NULL);
    }

    return err;
}

// 以上4种认证方式的综合利用
int authenticate_console(ssh_session session)
{
    int rc;
    int method;
    char password[128] = {0};
    char *banner;

    // Try to authenticate
    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        ssh_print_error(session);
        return rc;
    }

    method = ssh_userauth_list(session, NULL);
    ssh_print_supported_auth_methods(method);

    while (rc != SSH_AUTH_SUCCESS) {

        printf(">>>>>>>>>>>>>>>>>>>>>>>>");

        if (method & SSH_AUTH_METHOD_GSSAPI_MIC){
            rc = ssh_userauth_gssapi(session);
            if(rc == SSH_AUTH_ERROR) {
                ssh_print_error(session);
                return rc;
            } else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }

        // Try to authenticate with public key first
        if (method & SSH_AUTH_METHOD_PUBLICKEY) {
            rc = ssh_userauth_publickey_auto(session, NULL, NULL);
            if (rc == SSH_AUTH_ERROR) {
                ssh_print_error(session);
                return rc;
            } else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }
        // Try to authenticate with private key
        // {
        //     char buffer[128] = {0};
        //     char *p = NULL;

        //     printf("Automatic pubkey failed. "
        //            "Do you want to try a specific key? (y/n)\n");
        //     if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        //         break;
        //     }
        //     if ((buffer[0]=='Y') || (buffer[0]=='y')) {
        //         printf("private key filename: ");

        //         if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        //             return SSH_AUTH_ERROR;
        //         }

        //         buffer[sizeof(buffer) - 1] = '\0';
        //         if ((p = strchr(buffer, '\n'))) {
        //             *p = '\0';
        //         }

        //         rc = auth_keyfile(session, buffer);

        //         if(rc == SSH_AUTH_SUCCESS) {
        //             break;
        //         }
        //         fprintf(stderr, "failed with key\n");
        //     }
        // }

        // Try to authenticate with keyboard interactive";
        if (method & SSH_AUTH_METHOD_INTERACTIVE) {
            rc = authenticate_kbdint(session, NULL);
            if (rc == SSH_AUTH_ERROR) {
                ssh_print_error(session);
                return rc;
            } else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }

        // Try to authenticate with password
        if (method & SSH_AUTH_METHOD_PASSWORD) {
            // if (ssh_getpass("Password: ", password, sizeof(password), 0, 0) < 0) {
            //     return SSH_AUTH_ERROR;
            // }
            rc = ssh_userauth_password(session, NULL, "vimicro");
            if (rc == SSH_AUTH_ERROR) {
                ssh_print_error(session);
                return rc;
            } else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
            memset(password, 0, sizeof(password));
        }
    }

    banner = ssh_get_issue_banner(session);
    if (banner) {
        printf("banner: %s\n",banner);
        SSH_STRING_FREE_CHAR(banner);
    }

    return rc;
}


int test_several_auth_methods(ssh_session session)
{
  int method, rc;
  
  rc = ssh_userauth_none(session, NULL);
  if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR) {
      return rc;
  }
  
  method = ssh_userauth_list(session, NULL);
  
  if (method & SSH_AUTH_METHOD_NONE)
  { // For the source code of function authenticate_none(),
    // refer to the corresponding example
    rc = authenticate_none(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_PUBLICKEY)
  { // For the source code of function authenticate_pubkey(),
    // refer to the corresponding example
    rc = authenticate_pubkey(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_INTERACTIVE)
  { // For the source code of function authenticate_kbdint(),
    // refer to the corresponding example
    rc = authenticate_kbdint(session, NULL);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_PASSWORD)
  { // For the source code of function authenticate_password(),
    // refer to the corresponding example
    rc = authenticate_password(session, NULL);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  return SSH_AUTH_ERROR;
}

/**
 * 
 * @param keyfile 不带后缀的文件名, 公钥文件是keyfile.pub, 私钥文件是keyfile
 */
static int auth_keyfile(ssh_session session, char* keyfile)
{
    int rc;
    
    ssh_key key = NULL;
    char pubkey[132] = {0}; // +".pub"

    snprintf(pubkey, sizeof(pubkey), "%s.pub", keyfile);

    // 公钥
    rc = ssh_pki_import_pubkey_file( pubkey, &key);
    if (rc != SSH_OK)
        return SSH_AUTH_DENIED;
    // 尝试使用公钥进行用户认证
    rc = ssh_userauth_try_publickey(session, NULL, key);
    ssh_key_free(key);
    if (rc!=SSH_AUTH_SUCCESS)
        return SSH_AUTH_DENIED;

    // 私钥
    rc = ssh_pki_import_privkey_file(keyfile, NULL, NULL, NULL, &key);
    if (rc != SSH_OK)
        return SSH_AUTH_DENIED;

    // 使用公钥进行用户认证
    rc = ssh_userauth_publickey(session, NULL, key);

    ssh_key_free(key);

    return rc;
}

void ssh_print_error(ssh_session session)
{
    fprintf(stderr,"Authentication failed: %s\n",ssh_get_error(session));
}

void ssh_parse_supported_auth_methods(int method_mark, struct ssh_user_auth_methods * methods){
    if(method_mark & SSH_AUTH_METHOD_UNKNOWN) methods->unknown = 1;
    if(method_mark & SSH_AUTH_METHOD_NONE) methods->none = 1;
    if(method_mark & SSH_AUTH_METHOD_PASSWORD) methods->password = 1;
    if(method_mark & SSH_AUTH_METHOD_PUBLICKEY) methods->public_key = 1;
    if(method_mark & SSH_AUTH_METHOD_HOSTBASED) methods->host_based = 1;
    if(method_mark & SSH_AUTH_METHOD_INTERACTIVE) methods->interactive = 1;
    if(method_mark & SSH_AUTH_METHOD_GSSAPI_MIC) methods->gssapi = 1;
}
void ssh_print_supported_auth_methods(int method_mark){
    printf("supported methods: ");
    if(method_mark & SSH_AUTH_METHOD_UNKNOWN) printf("unknown, ");
    if(method_mark & SSH_AUTH_METHOD_NONE) printf("none, ");
    if(method_mark & SSH_AUTH_METHOD_PASSWORD) printf("password, ");
    if(method_mark & SSH_AUTH_METHOD_PUBLICKEY) printf("public_key, ");
    if(method_mark & SSH_AUTH_METHOD_HOSTBASED) printf("host_based, ");
    if(method_mark & SSH_AUTH_METHOD_INTERACTIVE) printf("interactive, ");
    if(method_mark & SSH_AUTH_METHOD_GSSAPI_MIC) printf("gssapi, ");
    printf("\n");
}