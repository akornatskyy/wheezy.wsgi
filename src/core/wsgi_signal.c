
#include <wsgi_core.h>


static void wsgi_signal_shutdown_handler(int signo);


typedef struct {
    int         signo;
    void        (*handler)(int signo);
} wsgi_signal_t;


volatile sig_atomic_t wsgi_signal_shutdown = 0;


static wsgi_signal_t signals[] = {
    { SIGINT, wsgi_signal_shutdown_handler },
    { SIGTERM, wsgi_signal_shutdown_handler },
    { SIGQUIT, wsgi_signal_shutdown_handler },
    { SIGSYS, SIG_IGN },
    { SIGPIPE, SIG_IGN },
    { 0, 0 }
};


int
wsgi_signal_init(wsgi_log_t *log)
{
    wsgi_signal_t *s;
    struct sigaction sa;

    for (s = signals; s->signo != 0; s++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = s->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(s->signo, &sa, NULL) == -1) {
            wsgi_log_error(log, WSGI_LOG_SOURCE_CORE,
                           "sigaction: errno %d: %s",
                           errno, strerror(errno));
            return WSGI_ERROR;
        }
    }

    return WSGI_OK;
}

void
wsgi_signal_shutdown_handler(int signo)
{
    /* Erace ^C echo from terminal */
    printf("\r");
    /* TODO: add logging of signal received */
    wsgi_signal_shutdown = 1;
}
