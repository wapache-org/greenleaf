// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved
//
// This example demonstrates how to handle very large requests without keeping
// them in memory.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mongoose.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

struct file_writer_data {
  FILE *fp;
  size_t bytes_written;
};

static int file_writer_data_init(struct file_writer_data *data) {
    if (data != NULL) {
        return -1;
    }

    data = calloc(1, sizeof(struct file_writer_data));

    data->fp = tmpfile();
    data->bytes_written = 0;

    if (data->fp == NULL) {
        free(data);
        return -2;
    }

    return 0;
}

static int file_writer_data_finallize(struct file_writer_data *data) {
  if (data != NULL) {
      fclose(data->fp);
      free(data);
  }
}

static void handle_upload(struct mg_connection *nc, int ev, void *p) 
{
  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;

  switch (ev) {
    case MG_EV_HTTP_PART_BEGIN: {
        int code = file_writer_data_init(data);
        if (code){
            nc->user_data = (void *) data;
        }else{
            char* msg;
            switch(code){
                case -1:
                    msg = "State invalid";
                    break;
                case -2:
                    msg = "Failed to open a file";
                    break;
                default:
                    msg = "Unknown error";
            }
            mg_printf(nc, 
                "HTTP/1.1 500 %s\r\nContent-Length: 0\r\n\r\n", 
                msg
            );
            nc->flags |= MG_F_SEND_AND_CLOSE;
        }
      break;
    }
    case MG_EV_HTTP_PART_DATA: {

        struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;
      if (fwrite(mp->data.p, 1, mp->data.len, data->fp) != mp->data.len) {
        mg_printf(nc, "%s",
                  "HTTP/1.1 500 Failed to write to a file\r\n"
                  "Content-Length: 0\r\n\r\n");
        nc->flags |= MG_F_SEND_AND_CLOSE;
        return;
      }
      data->bytes_written += mp->data.len;
      break;
    }
    case MG_EV_HTTP_PART_END: {
      mg_printf(nc,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n"
                "Written %ld of POST data to a temp file\n\n",
                (long) ftell(data->fp));

      file_writer_data_finallize(data);

      nc->flags |= MG_F_SEND_AND_CLOSE;
      nc->user_data = NULL;

      break;
    }
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_REQUEST) {
    mg_serve_http(nc, ev_data, s_http_server_opts);
  }
}

int main(void) 
{
  struct mg_mgr mgr;
  mg_mgr_init(&mgr, NULL);

  struct mg_connection *c = mg_bind(&mgr, s_http_port, ev_handler);
  if (c == NULL) {
    fprintf(stderr, "Cannot start server on port %s\n", s_http_port);
    exit(EXIT_FAILURE);
  }

  s_http_server_opts.document_root = ".";  // Serve current directory
  mg_register_http_endpoint(c, "/upload", handle_upload MG_UD_ARG(NULL));

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(c);

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
