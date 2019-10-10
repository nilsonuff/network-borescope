/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#include <stdio.h>
#include <signal.h>
#include "common.h"
#include "contrib/mongoose.h"  

#include "query_processor.h"
#include "terminal.h"

//========================================================================
static clock_t ctrl_c_time = 0;
static int ctrl_c_count = 0;

#define MAX_COUNT 5

void signal_ctrl_c() { 		 
    signal(SIGINT, signal_ctrl_c); /*  */

    ctrl_c_count ++;
    if (ctrl_c_count < MAX_COUNT) {
        fprintf(stderr,"Press Ctrl+C more %d times to Pause/Stop.\n",MAX_COUNT - ctrl_c_count);
    } else if (ctrl_c_count == MAX_COUNT) {
        exit(0);
        fprintf(stderr,"\n\nWeb Server PAUSED!\n\nPress ENTER to STOP or any other key + ENTER to RESUME .\n");
        if (getchar() == 13) exit(0);
        fprintf(stderr,"RESUMED.\n");
        ctrl_c_count = 0;
    } 

}
 
void quitproc() {
    printf("ctrl-\\ pressed to quit\n");
	exit(0); /* normal exit status */
}

//========================================================================

static char *s_http_port = "8001";
static struct mg_serve_http_opts s_http_server_opts;

static PMetaData pmd;
static PSchema   ps;
static PAddr     pa;
static PNode     root;
static void *    record;


static struct mg_connection *cur_nc;

#define QUERY_BUF   60000
static char query_buf[QUERY_BUF];

//-----------------------
//
//
//
//
static int my_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = mg_vprintf(cur_nc, fmt, args);
    // vprintf(fmt, args);
    va_end(args);
    return r;
}


//-----------------------
//
//
//
//
static char * replace_quotes(char *c,char rep) {
    char *s = c;
    for (;*s; s++) {
        if (*s == '\n') *s = ' ';
        else if (*s == '\r') *s = ' ';
        else if (*s == '\"') *s = rep;
    }
    return c;
}

//-----------------------
//
//
//
//
static void on_query(struct mg_connection *nc, struct http_message *hm) {

    int n = mg_url_decode(hm->body.p,(int) hm->body.len, query_buf, QUERY_BUF,1);

    mg_printf(nc,"\r\n");
#if 0
    mg_printf(nc,"\r\n<h1>Hello, </h1>\r\n"
            "You asked for (%s) %.*s\r\n",
            query_buf, (int) hm->uri.len, hm->uri.p);
#endif

    if (n > 0) {
        clock_t start, stop;
        start = clock();

        PQuery pq = query_begin();

        // jump error
        if (setjmp(query_jmp)) {
            mg_printf(nc, "{\"id\":%d,  \"tp\": 0, \"msg\": \"%s\"",pq->id,replace_quotes(query_error_string,'\''));
        } else {
            if (!query_process(pq, query_buf, ps, pmd)) {
                mg_printf(nc, "{\"id\":%d,  \"tp\": 0, \"msg\": \"%s\"",pq->id,replace_quotes(query_error_string,'\''));
            } else {
                // serializa o resultado da query
                mg_printf(nc, "{");        
                cur_nc = nc;
                query_out_as_json(pq, my_printf);
            }
        }

        query_end(pq);

        // computa o tempo total da query
        stop = clock();
        double result = timediff(start, stop);
        // printf("\nTime: %.f ms\n", result);    
        
        mg_printf(nc,",\"ms\":%.f }", result);        
    }
}

//-----------------------
//
//
//
//
static void on_post(struct mg_connection *nc, struct http_message *hm) {
    struct mg_str s2 = mg_mk_str("/jsonquery");
    if (mg_strcmp(hm->uri,s2)==0) {
        on_query(nc, hm);

    } else {
        mg_printf(nc,
                "\r\nInvalid uri for post: %.*s\r\n",              
                (int) hm->uri.len, hm->uri.p);
    }
}

//-----------------------
//
//
//
//
static void on_get(struct mg_connection *nc,struct http_message *hm) {
    struct mg_serve_http_opts opts = { .document_root = "../www" }; 
    mg_serve_http(nc, hm, opts);
}


//-----------------------
//
//
//
//
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
 switch (ev) {
    case MG_EV_ACCEPT: {
      char addr[32];
      mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                          MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
      //printf("%p: Connection from %s\r\n", nc, addr);
      break;
    }
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) ev_data;
      char addr[32];
      mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                          MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
      //printf("%p: %.*s %.*s\r\n", nc, (int) hm->method.len, hm->method.p, (int) hm->uri.len, hm->uri.p);

      if (mg_ncasecmp("POST",hm->method.p,hm->method.len) == 0) {
        mg_send_response_line(nc, 200,
                                "Content-Type: text/html\r\n"
                                "Connection: close");
          on_post(nc, hm);
      }

      if (mg_ncasecmp("GET",hm->method.p,hm->method.len) == 0) {
          on_get(nc, hm);
      }

      nc->flags |= MG_F_SEND_AND_CLOSE;

      break;
    }
    case MG_EV_CLOSE: {
      // printf("%p: Connection closed\r4\n", nc);
      break;
    }
  }  
}

//========================================================================

//-----------------------
//
//
//
//
static void initialize(void) {
    map_reset();

    pmd = metadata_create();
    metadata_load(pmd, "schema_rnp.json");

    ps = metadata_create_schema(pmd);
    terminal_set_schema(ps);
    
    root = ps->nfs[0].create_node(0);
    ps->root = root;

    pa = metadata_create_address(pmd);

    record = metadata_create_record(pmd);
}

//-----------------------
//
//
//
//
static void terminate(void) {
    free(record);
    metadata_close_input_file(pmd);
    metadata_release_address(pa);
    metadata_release(pmd);
}

//========================================================================


//-----------------------
//
// Versão para protótipo
//
//
static int incremental_load_from_file(char *day, int *hour, int *minute) {
    static int load_state = 0;

    clock_t start, stop;
    start = clock();

    static char prefix[] = "../../rnp/BR_";
    static char suffix[] = ".csv";
    static char buf[1024];

    if (load_state == 0) {
        printf("Ready to Load data from files: "); fflush(stdout);
        load_state = 1;
    } 
    
    if (load_state == 1) {
        sprintf(buf, "%s%s_%02d_%02d%s",prefix, day, *hour, *minute, suffix);
        FILE *f;
        if ((f = fopen(buf,"r")) == NULL) {
            return 0;
        }
        fclose(f);
        printf("[%02d:%02d", *hour, *minute);  fflush(stdout);
        metadata_open_input_file(pmd, buf);
        load_state = 2;
    }

    if (load_state == 2) {
        while (metadata_read(pmd, record)) {
            metadata_record_to_address(pmd, record, pa);
            tinycubes_insert(ps, root, pa->values, record);
            stop = clock();
            double diff = timediff(start,stop);
            if (diff > 40) {
                // printf("."); fflush(stdout);
                return 1;
            } 
        }
        load_state = 3;
    }

    if (load_state == 3) {
        printf("]");  fflush(stdout);
        metadata_close_input_file(pmd);
        load_state = 1;
        (*minute) ++;
        if (*minute > 59) {
            (*hour) ++;
            if (*hour > 23) return 0;
            *minute = 0;
            load_state = 1;
        } 
        return 1;
    }
    return 0;
}


//-----------------------
//
//
//
//
int webserver_run(int port) {
    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    
    char port_s [10];
    if (port) {
        sprintf(port_s,"%d",port);
        s_http_port = port_s;
    }
    
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }
    printf("Listening on port %s...\n", s_http_port);

    // c
    signal(SIGINT, signal_ctrl_c);
    ctrl_c_time = clock();
    //signal(SIGQUIT, quitproc);
    
    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = ".";  // Serve current directory
    s_http_server_opts.enable_directory_listing = "no";

    int hour = 14;
    int minute = 30;
    
#define READING_TIMEOUT 20
#define WAITING_TIMEOUT 1000

    int timeout = READING_TIMEOUT;
   
    // application loop
    for (;;) {
        mg_mgr_poll(&mgr, timeout);
        if (!incremental_load_from_file("20190806", &hour, &minute)) {
            timeout = WAITING_TIMEOUT;   
        } else {
            timeout = READING_TIMEOUT;     // reading file
        }
    }
    mg_mgr_free(&mgr);

    return 0;
}


//-----------------------
//
//
//
//
int main(int argc, char *argv[]) {
    printf("-------------------------------------------\n");
    printf("Tinycubes Server                Version 0.1\n");
    printf("Universidade Federal Fluminense - UFF, 2019\n");
    printf("-------------------------------------------\n");

    app_init();
    
    int port = 8001;
    if (argc == 3 && (strcmp(argv[1],"-p")==0)) {
        printf("Port: %d\n", port);
        port = atoi(argv[2]);
    }

    initialize();
    webserver_run(port);
    terminate();
    return 0;
}