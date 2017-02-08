/**
 * NGINX TMS 2 XYZ Module
 *
 * Nginx module to convert TMS tile URL into that of WMTS.
 * This flips y tile index.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
  ngx_flag_t enable;
} ngx_tms_2_xyz_module_loc_conf_t;

static char * ngx_tms_2_xyz_module(
  ngx_conf_t * cf,
  ngx_command_t * cmd,
  void *conf
);

static void * ngx_tms_2_xyz_module_create_loc_conf(ngx_conf_t * cf);

static ngx_command_t ngx_tms_2_xyz_module_commands[] = {
  {
    ngx_string("TMS2XYZ"),
    NGX_HTTP_LOC_CONF | NGX_CONF_TAKE3, // location setting with 2 args(zoom and y)
    ngx_tms_2_xyz_module,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t ngx_tms_2_xyz_module_module_ctx = {
  NULL, /* preconfiguration */
  NULL, /* postconfiguration */
  NULL, /* create main configuration */
  NULL, /* init main configuration */
  NULL, /* create server configuration */
  NULL, /* merge server configuration */
  ngx_tms_2_xyz_module_create_loc_conf, /* create location configuration */
  NULL
};

ngx_module_t ngx_tms_2_xyz_module_module = {
  NGX_MODULE_V1,
  &ngx_tms_2_xyz_module_module_ctx, /* module context */
  ngx_tms_2_xyz_module_commands,    /* module directives */
  NGX_HTTP_MODULE,                  /* module type */
  NULL,                             /* init master */
  NULL,                             /* init module */
  NULL,                             /* init process */
  NULL,                             /* init thread */
  NULL,                             /* exit thread */
  NULL,                             /* exit process */
  NULL,                             /* exit master */
  NGX_MODULE_V1_PADDING
};

static void * ngx_tms_2_xyz_module_create_loc_conf(ngx_conf_t *cf)
{
  ngx_tms_2_xyz_module_loc_conf_t *conf;
  conf = ngx_pcalloc(cf->pool, sizeof(ngx_tms_2_xyz_module_loc_conf_t));
  if (conf == NULL) {
    return NGX_CONF_ERROR;
  }
  return conf;
}

static ngx_int_t ngx_tms_2_xyz_module_handler(ngx_http_request_t *r)
{
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t out;
    unsigned char *buff;
    char hello[124] = "Hello Sample!";
    int l = strlen(hello);
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "***Call Handler***");
    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }
    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK && rc != NGX_AGAIN) {
        return rc;
    }
    if (r->headers_in.if_modified_since) {
        return NGX_HTTP_NOT_MODIFIED;
    }
    // header設定
    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = l;
    if (r->method == NGX_HTTP_HEAD) {
        rc = ngx_http_send_header(r);
        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    out.buf = b;
    out.next = NULL;
    buff = ngx_palloc(r->pool, l);
    if (buff == NULL) {
     ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate memory.");
     return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_memcpy(buff, hello, l);
    b->pos = buff;
    b->last = buff + l;
    b->memory = 1;
    b->last_buf = 1;
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "Success! sample");
    return ngx_http_output_filter(r, &out);
}

static char * ngx_tms_2_xyz_module(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;
    // lget ocation configuration and configure Handler
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_tms_2_xyz_module_handler;
    return NGX_CONF_OK;
}
