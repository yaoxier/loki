#ifndef loki_h
#define loki_h


#ifndef LK_NS_BEGIN
# ifdef __cplusplus
#   define LK_NS_BEGIN extern "C" {
#   define LK_NS_END   }
# else
#   define LK_NS_BEGIN
#   define LK_NS_END
# endif
#endif /* LK_NS_BEGIN */

#ifdef LK_STATIC_API
# ifndef LOKI_IMPLEMENTATION
#  define LOKI_IMPLEMENTATION
# endif
# if __GNUC__
#   define LK_API static __attribute((unused))
# else
#   define LK_API static
# endif
#endif

#if !defined(LK_API) && defined(_WIN32)
# if defined(LOKI_IMPLEMENTATION) || defined(LOKI_MODULE)
#   define LK_API __declspec(dllexport)
# else
#   define LK_API __declspec(dllimport)
# endif
#endif

#ifndef LK_API
# define LK_API extern
#endif

#ifndef LKMOD_API
# define LKMOD_API LK_API
#endif

#include <stdarg.h>
#include <stddef.h>

#define LK_OK      (0)
#define LK_WEAK    (1)
#define LK_ERR     (-1)
#define LK_TIMEOUT (-2)

#define LK_MONITOR_ONUSE     0
#define LK_MONITOR_ONOPEN    1
#define LK_MONITOR_ONWEAK    2
#define LK_MONITOR_ONCLOSE   3
#define LK_MONITOR_EVENTS    4


LK_NS_BEGIN

typedef struct lk_State   lk_State;
typedef struct lk_Service lk_Service;
typedef struct lk_Slot    lk_Slot;

typedef struct lk_Signal {
    lk_Service *src;
    void *data;
    unsigned free : 1;
    unsigned type : 7;
    unsigned size : 24;
    unsigned session;
} lk_Signal;

typedef int   lk_Handler (lk_State *S, lk_Slot *slot, lk_Signal *sig);
typedef void *lk_Allocf  (void *ud, void *ptr, size_t size, size_t osize);


/* global routines */

LK_API lk_State *lk_newstate (const char *name, lk_Allocf *allocf, void *ud);

LK_API void lk_waitclose (lk_State *S);

LK_API int lk_start (lk_State *S, int threads);

LK_API char *lk_getconfig (lk_State *S, const char *key);
LK_API void  lk_setconfig (lk_State *S, const char *key, const char *value);

LK_API int lk_log  (lk_State *S, const char *fmt, ...);
LK_API int lk_vlog (lk_State *S, const char *fmt, va_list l);

#define lk_str_(str) # str
#define lk_str(str) lk_str_(str)
#define lk_loc(str) __FILE__ ":" lk_str(__LINE__) ": " str


/* service routines */

LK_API lk_Service *lk_launch (lk_State *S, const char *name, lk_Handler *h, void *ud);
LK_API void        lk_close  (lk_State *S);

LK_API lk_Service *lk_self (lk_State *S);

LK_API lk_Handler *lk_refactor    (lk_Service *svr);
LK_API void        lk_setrefactor (lk_Service *svr, lk_Handler *h);


/* message routines */

#define LK_SIGNAL { NULL, NULL, 0, 0, 0, 0 }

LK_API lk_Slot *lk_newslot (lk_State *S, const char *name, lk_Handler *h, void *data);
LK_API lk_Slot *lk_slot    (lk_State *S, const char *name);

LK_API lk_Slot *lk_current (lk_State *S);

LK_API int lk_emit       (lk_Slot *slot, const lk_Signal *sig);
LK_API int lk_emitstring (lk_Slot *slot, unsigned type, unsigned session, const char *s);

LK_API lk_Slot *lk_newpoll (lk_State *S, const char *name, lk_Handler *h, void *data);
LK_API int      lk_wait    (lk_Slot *slot, lk_Signal *sig, int waitms);

LK_API const char *lk_name    (lk_Slot *slot);
LK_API lk_Service *lk_service (lk_Slot *slot);
LK_API lk_State   *lk_state   (lk_Slot *slot);

LK_API void *lk_data    (lk_Slot *slot);
LK_API void  lk_setdata (lk_Slot *slot, void *data);

LK_API lk_Handler *lk_slothandler    (lk_Slot *slot);
LK_API void        lk_setslothandler (lk_Slot *slot, lk_Handler *h);


LK_NS_END

#endif /* loki_h */

/****************************************************************************/

#ifndef lk_utils_h
#define lk_utils_h

LK_NS_BEGIN

#include <string.h>

#define LK_MPOOLPAGESIZE 4096

/* memory management */

typedef struct lk_MemPool {
    void *pages;
    void *freed;
    size_t size;
} lk_MemPool;

LK_API void *lk_malloc    (lk_State *S, size_t size);
LK_API void *lk_realloc   (lk_State *S, void *ptr, size_t size, size_t osize);
LK_API void  lk_free      (lk_State *S, void *ptr, size_t osize);
LK_API void  lk_initpool  (lk_MemPool *mpool, size_t size);
LK_API void  lk_freepool  (lk_State *S, lk_MemPool *mpool);
LK_API void *lk_poolalloc (lk_State *S, lk_MemPool *mpool);
LK_API void  lk_poolfree  (lk_MemPool *mpool, void *obj);


/* string routines */

typedef struct lk_Data lk_Data;

LK_API lk_Data *lk_newdata (lk_State *S, size_t size);
LK_API void     lk_deldata (lk_State *S, lk_Data *s);

LK_API size_t lk_len    (lk_Data *s);
LK_API size_t lk_size   (lk_Data *s);
LK_API void   lk_setlen (lk_Data *s, size_t len);

LK_API lk_Data *lk_newstring   (lk_State *S, const char *s);
LK_API lk_Data *lk_newlstring  (lk_State *S, const char *s, size_t len);
LK_API lk_Data *lk_newvfstring (lk_State *S, const char *fmt, va_list l);
LK_API lk_Data *lk_newfstring  (lk_State *S, const char *fmt, ...);

LK_API int lk_emitdata (lk_Slot *slot, unsigned type, unsigned session, lk_Data *data);

LK_API char *lk_strcpy    (char *buff, const char *s, size_t len);
LK_API int   lk_vsnprintf (char *buff, size_t size, const char *fmt, va_list l);


/* table routines */

typedef struct lk_Entry {
    int      next;
    unsigned hash;
    const char *key;
} lk_Entry;

typedef struct lk_Table {
    size_t   size;
    size_t   entry_size;
    size_t   lastfree;
    lk_Entry *hash;
} lk_Table;

typedef struct lk_PtrEntry { lk_Entry entry; void *data; } lk_PtrEntry;

#define lk_key(e) (((lk_Entry*)(e))->key)

LK_API void lk_inittable (lk_Table *t, size_t entry_size);
LK_API void lk_freetable (lk_State *S, lk_Table *t);

LK_API size_t lk_resizetable (lk_State *S, lk_Table *t, size_t len);

LK_API lk_Entry *lk_gettable (lk_Table *t, const char *key);
LK_API lk_Entry *lk_settable (lk_State *S, lk_Table *t, const char *key);

LK_API int lk_nextentry (lk_Table *t, lk_Entry **pentry);


LK_NS_END

#endif /* lk_utils_h */


#ifndef lk_thread_h
#define lk_thread_h

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
# include <Windows.h>
# include <process.h>

typedef DWORD             lk_TlsKey;
typedef CRITICAL_SECTION  lk_Lock;
typedef HANDLE            lk_Event;
typedef HANDLE            lk_Thread;

#define lk_inittls(key)   ((*(key) = TlsAlloc()) != TLS_OUT_OF_INDEXES)
#define lk_freetls(key)   TlsFree(key)
#define lk_gettls(key)    TlsGetValue(key)
#define lk_settls(key, p) TlsSetValue((key),(p))

#define lk_initlock(lock) (InitializeCriticalSection(lock), 1)
#define lk_freelock(lock) DeleteCriticalSection(&(lock))
#define lk_lock(lock)     EnterCriticalSection(&(lock))
#define lk_unlock(lock)   LeaveCriticalSection(&(lock))

#define lk_initevent(evt) ((*(evt)=CreateEvent(NULL,FALSE,FALSE,NULL))!=NULL)
#define lk_freeevent(evt) CloseHandle(evt)
#define lk_signal(evt)    SetEvent(evt)

#define lk_initthread(t,f,ud) ((*(t)=(HANDLE)_beginthreadex(NULL,0,(f),(ud),0,NULL))!=NULL)
#define lk_waitthread(t)  (WaitForSingleObject((t), INFINITE),(void)lk_freethread(t))
#define lk_freethread(t)  ((void)CloseHandle(t))

#else /* POSIX systems */

#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <sys/time.h>

typedef pthread_key_t     lk_TlsKey;
typedef pthread_mutex_t   lk_Lock;
typedef pthread_cond_t    lk_Event;
typedef pthread_t         lk_Thread;

#define lk_inittls(key)   (pthread_key_create((key), NULL) == 0)
#define lk_freetls(key)   pthread_key_delete(key)
#define lk_gettls(key)    pthread_getspecific(key)
#define lk_settls(key, p) pthread_setspecific((key), (p))

#define lk_initlock(lock) (pthread_mutex_init(lock, NULL) == 0)
#define lk_freelock(lock) pthread_mutex_destroy(&(lock))
#define lk_lock(lock)     pthread_mutex_lock(&(lock))
#define lk_unlock(lock)   pthread_mutex_unlock(&(lock))

#define lk_initevent(evt) (pthread_cond_init((evt), NULL) == 0)
#define lk_freeevent(evt) pthread_cond_destroy(&(evt))
#define lk_signal(evt)    pthread_cond_signal(&(evt))

#define lk_initthread(t,f,ud) (pthread_create((t),NULL,(f),(ud)) == 0)
#define lk_waitthread(t)  pthread_join((t),NULL)
#define lk_freethread(t)  pthread_cancel(t)

#endif

#endif /* lk_thread_h */


#ifndef lk_queue_h
#define lk_queue_h

#define lkQ_entry(T) T *next
#define lkQ_type(T)  struct { T *first; T *last; }

#define lkQ_initheader(h,n) ((h)->first = (h)->last = (n))
#define lkQ_clear(h, n)     ((n) = (h)->first, lkQ_init(h))
#define lkQ_empty(h)        ((h)->first == NULL)
#define lkQ_init(h)         lkQ_initheader(h,NULL)

#define lkQ_enqueue(h, n)                                 do { \
    (n)->next = NULL;                                          \
    if (lkQ_empty(h)) lkQ_initheader(h, n);                    \
    else (h)->last->next = (n), (h)->last = (n);             } while (0)

#define lkQ_dequeue(h, n)                                 do { \
    (n) = (h)->first;                                          \
    if ((n) == (h)->last) lkQ_init(h);                         \
    else if ((h)->first != NULL)                               \
        (h)->first = (h)->first->next;                       } while (0)

#define lkQ_merge(h, oh)                                 do { \
    if ((oh)->last) (oh)->last->next = (h)->first;            \
    (oh)->last = (h)->last, *(h) = *(oh);                   } while (0)

#define lkQ_apply(h, type, stmt)                          do { \
    type *cur = (type*)(h)->first;                             \
    while (cur != NULL)                                        \
    { type *next_ = cur->next; stmt; cur = next_; }          } while (0) 

#endif /* lk_queue_h */


#ifndef lk_context_h
#define lk_context_h


#include <setjmp.h>

# if defined(__cplusplus) && !defined(LK_USE_LONGJMP)
#   define lk_throw(S,c) throw(c)
#   define lk_try(S,c,a) do { try { a; } catch(lk_Context *c) {} } while (0)
#   define lk_JmpBuf     int  /* dummy variable */

# elif _WIN32 /* ISO C handling with long jumps */
#   define lk_throw(S,c) longjmp((c)->b, 1)
#   define lk_try(S,c,a) do { if (setjmp((c)->b) == 0) { a; } } while (0)
#   define lk_JmpBuf     jmp_buf

# else /* in POSIX, try _longjmp/_setjmp (more efficient) */
#   define lk_throw(L,c) _longjmp((c)->b, 1)
#   define lk_try(L,c,a) do { if (_setjmp((c)->b) == 0) { a; } } while (0)
#   define lk_JmpBuf     jmp_buf
# endif

LK_NS_BEGIN


typedef int lk_ProtectedHandler (lk_State *S, void *ud);

typedef struct lk_Cleanup {
    lkQ_entry(struct lk_Cleanup);
    lk_ProtectedHandler *h;
    void *ud;
} lk_Cleanup;

typedef struct lk_Context {
    struct lk_Context *prev;
    lk_State   *S;
    lk_Slot    *current;
    lk_Cleanup *cleanups;
    lk_JmpBuf   b;
    int retcode; /* error code */
} lk_Context;

LK_API lk_Context *lk_context (lk_State *S);

LK_API void lk_pushcontext (lk_State *S, lk_Context *ctx, lk_Slot *slot);
LK_API void lk_popcontext  (lk_State *S, lk_Context *ctx);

LK_API int lk_pcall   (lk_State *S, lk_ProtectedHandler *h, void *ud);
LK_API int lk_discard (lk_State *S);

LK_API int lk_addcleanup (lk_State *S, lk_ProtectedHandler *h, void *ud);


LK_NS_END

#endif /* lk_context_h */

/****************************************************************************/

#if defined(LOKI_IMPLEMENTATION) && !defined(lk_implemented)
#define lk_implemented


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef LK_NAME
# define LK_NAME "root"
#endif /* LK_NAME */

#define LK_MAX_THREADS     32
#define LK_MAX_NAMESIZE    32
#define LK_MAX_SLOTNAME    63
#define LK_HASHLIMIT       5
#define LK_MIN_HASHSIZE    8
#define LK_MAX_SIZET       (~(size_t)0u - 100)
#define LK_SMALLSTRING_LEN 64

#define LK_INITIALING 0
#define LK_WORKING    1
#define LK_SLEEPING   2
#define LK_STOPPING   3
#define LK_ZOMBIE     4 /* service stopping but has signal unprocessed */

LK_NS_BEGIN


/* structures */

typedef struct lk_Poll lk_Poll;

typedef struct lk_SignalNode {
    lkQ_entry(struct lk_SignalNode);
    lk_Slot    *slot;
    lk_Signal   data;
} lk_SignalNode;

struct lk_Slot {
    char        name[LK_MAX_SLOTNAME];
    unsigned    is_poll     : 1;
    unsigned    is_svr      : 1;
    unsigned    no_refactor : 1;
    lk_Slot    *next; /* all slots in same service */
    lk_State   *S;
    lk_Service *service;
    lk_Handler *handler;
    void       *data;
};

struct lk_Poll {
    lk_Slot   slot;
    lk_Thread thread;
    lk_Event  event;
    lk_Lock   lock;
    lkQ_type(lk_SignalNode) signals;
    unsigned  status;
};

struct lk_Service {
    lk_Slot     slot;
    lkQ_entry(lk_Service);
    unsigned    status;
    unsigned    pending : 31; /* pending signals in queue */
    unsigned    weak    : 1;
    lkQ_type(lk_SignalNode) signals;
    lk_Slot    *slots;
    lk_Slot    *polls;
    lk_Handler *refactor;
    lk_Lock     lock;
};

struct lk_State {
    lk_Service root;
    unsigned   status;
    unsigned   nthreads  : 16;
    unsigned   nservices : 16;
    lk_Table   slot_names;
    lk_Slot   *logger;
    lk_Slot   *monitor;
    lk_Lock    lock;

    lkQ_type(lk_Service) main_queue;
    lk_Event  queue_event;
    lk_Lock   queue_lock;

    lk_MemPool cleanups;
    lk_MemPool signals;
    lk_MemPool smallstrings;
    lk_MemPool services;
    lk_MemPool slots;
    lk_MemPool polls;
    lk_Lock    pool_lock;

    lk_Table config;
    lk_Lock  config_lock;

    lk_Allocf *allocf; void *alloc_ud;
    lk_TlsKey tls_index;
    lk_Thread threads[LK_MAX_THREADS];
};


/* memory management */

struct lk_Data {
    unsigned size : 16;
    unsigned len  : 16;
};

LK_API void lk_poolfree (lk_MemPool *mpool, void *obj)
{ *(void**)obj = mpool->freed; mpool->freed = obj; }

LK_API size_t lk_len  (lk_Data *data) { return data ? data[-1].len  : 0; }
LK_API size_t lk_size (lk_Data *data) { return data ? data[-1].size : 0; }

LK_API void lk_setlen(lk_Data *data, size_t len)
{ if (data) data[-1].len = len < lk_size(data) ? len : lk_size(data); }

static void *lkM_outofmemory (void) {
    fprintf(stderr, "out of memory\n");
    abort();
    return NULL;
}

LK_API int lk_vsnprintf(char *buff, size_t size, const char *fmt, va_list l) {
#if !defined(_WIN32) || defined(__MINGW32__)
    return vsnprintf(buff, size, fmt, l);
#else
    int count = -1;
    if (size  !=  0) count = _vsnprintf_s(buff, size, _TRUNCATE, fmt, l);
    if (count == -1) count = _vscprintf(fmt, l);
    return count;
#endif
}

LK_API char *lk_strcpy (char *buff, const char *s, size_t len) {
    size_t srclen = strlen(s);
    if (srclen >= len - 1) {
        memcpy(buff, s, len-1);
        buff[len-1] = '\0';
    }
    else {
        memcpy(buff, s, srclen);
        memset(buff+srclen, 0, len-srclen);
    }
    return buff;
}

LK_API void *lk_malloc (lk_State *S, size_t size) {
    void *ptr = S->allocf(S->alloc_ud, NULL, size, 0);
    if (ptr == NULL) return lkM_outofmemory();
    return ptr;
}

LK_API void *lk_realloc (lk_State *S, void *ptr, size_t size, size_t osize) {
    void *newptr = S->allocf(S->alloc_ud, ptr, size, osize);
    if (newptr == NULL) return lkM_outofmemory();
    return newptr;
}

LK_API void lk_free (lk_State *S, void *ptr, size_t osize) {
    void *newptr = ptr ? S->allocf(S->alloc_ud, ptr, 0, osize) : NULL;
    assert(newptr == NULL);
}

LK_API void lk_initpool (lk_MemPool *mpool, size_t size) {
    size_t sp = sizeof(void*);
    assert(((sp - 1) & sp) == 0);
    mpool->pages = NULL;
    mpool->freed = NULL;
    if (size < sp)      size = sp;
    if (size % sp != 0) size = (size + sp - 1) & ~(sp - 1);
    mpool->size = size;
    assert(LK_MPOOLPAGESIZE / size > 2);
}

LK_API void lk_freepool (lk_State *S, lk_MemPool *mpool) {
    const size_t offset = LK_MPOOLPAGESIZE - sizeof(void*);
    while (mpool->pages != NULL) {
        void *next = *(void**)((char*)mpool->pages + offset);
        lk_free(S, mpool->pages, LK_MPOOLPAGESIZE);
        mpool->pages = next;
    }
    lk_initpool(mpool, mpool->size);
}

LK_API void *lk_poolalloc (lk_State *S, lk_MemPool *mpool) {
    void *obj = mpool->freed;
    if (obj == NULL) {
        const size_t offset = LK_MPOOLPAGESIZE - sizeof(void*);
        void *end, *newpage = lk_malloc(S, LK_MPOOLPAGESIZE);
        *(void**)((char*)newpage + offset) = mpool->pages;
        mpool->pages = newpage;
        end = (char*)newpage + (offset/mpool->size-1)*mpool->size;
        while (end != newpage) {
            *(void**)end = mpool->freed;
            mpool->freed = end;
            end = (char*)end - mpool->size;
        }
        return end;
    }
    mpool->freed = *(void**)obj;
    return obj;
}

LK_API lk_Data *lk_newdata (lk_State *S, size_t size) {
    size_t rawlen = sizeof(lk_Data) + size;
    lk_Data *data;
    if (rawlen <= LK_SMALLSTRING_LEN) {
        lk_lock(S->pool_lock);
        data = (lk_Data*)lk_poolalloc(S, &S->smallstrings);
        data->size = LK_SMALLSTRING_LEN-sizeof(lk_Data);
        lk_unlock(S->pool_lock);
    }
    else {
        data = (lk_Data*)lk_malloc(S, rawlen);
        data->size = rawlen-sizeof(lk_Data);
    }
    data->len = 0;
    return data + 1;
}

LK_API void lk_deldata (lk_State *S, lk_Data *s) {
    if (!s) return;
    if (s[-1].size+sizeof(lk_Data) != LK_SMALLSTRING_LEN)
        lk_free(S, s-1, s[-1].size+sizeof(lk_Data));
    else {
        lk_lock(S->pool_lock);
        lk_poolfree(&S->smallstrings, s-1);
        lk_unlock(S->pool_lock);
    }
}

LK_API lk_Data *lk_newstring (lk_State *S, const char *s) {
    size_t len = s ? strlen(s) : 0;
    lk_Data *data = lk_newdata(S, len+1);
    memcpy(data, s, len+1);
    lk_setlen(data, len);
    return data;
}

LK_API lk_Data *lk_newlstring (lk_State *S, const char *s, size_t len) {
    lk_Data *data = lk_newdata(S, len);
    memcpy(data, s, len);
    lk_setlen(data, len);
    return data;
}

LK_API lk_Data *lk_newvfstring (lk_State *S, const char *fmt, va_list l) {
    va_list l_count;
    lk_Data *data;
    int len;
#ifdef va_copy
    va_copy(l_count, l);
#else
    __va_copy(l_count, l);
#endif
    len = lk_vsnprintf(NULL, 0, fmt, l_count);
    va_end(l_count);
    if (len <= 0) return NULL;
    data = lk_newdata(S, len+1);
    lk_vsnprintf((char*)data, len+1, fmt, l);
    lk_setlen(data, len);
    return data;
}

LK_API lk_Data *lk_newfstring (lk_State *S, const char *fmt, ...) {
    lk_Data *data;
    va_list l;
    va_start(l, fmt);
    data = lk_newvfstring(S, fmt, l);
    va_end(l);
    return data;
}


/* hashtable routines */

#define lk_offset(lhs, rhs) ((char*)(lhs) - (char*)(rhs))
#define lk_index(lhs, rhs)  ((lk_Entry*)((char*)(lhs) + (rhs)))

LK_API void lk_inittable (lk_Table *t, size_t entry_size)
{ memset(t, 0, sizeof(*t)); t->entry_size = entry_size; }

static size_t lkH_hashsize (lk_Table *t, size_t len) {
    size_t newsize = LK_MIN_HASHSIZE;
    const size_t maxsize = LK_MAX_SIZET/2/t->entry_size;
    while (newsize < maxsize && newsize < len)
        newsize <<= 1;
    assert(newsize < maxsize);
    return newsize < maxsize ? newsize : 0;
}

static size_t lkH_countsize (lk_Table *t) {
    size_t i, size = t->size * t->entry_size;
    size_t count = 0;
    for (i = 0; i < size; i += t->entry_size) {
        lk_Entry *e = lk_index(t->hash, i);
        if (e->key != NULL) ++count;
    }
    return count;
}

static lk_Entry *lkH_mainposition (lk_Table *t, unsigned hash) {
    assert((t->size & (t->size - 1)) == 0);
    return lk_index(t->hash, (hash & (t->size - 1))*t->entry_size);
}

static lk_Entry *lkH_newkey (lk_State *S, lk_Table *t, lk_Entry *entry) {
    lk_Entry *mp;
    if (entry->key == NULL
            || (t->size == 0 && lk_resizetable(S, t, LK_MIN_HASHSIZE) == 0))
        return NULL;
redo:
    mp = lkH_mainposition(t, entry->hash);
    if (mp->key != NULL) {
        lk_Entry *f = NULL, *othern;
        while (t->lastfree > 0) {
            lk_Entry *e = lk_index(t->hash, t->lastfree -= t->entry_size);
            if (e->key == NULL && e->next == 0)  { f = e; break; }
        }
        if (f == NULL) {
            if (lk_resizetable(S, t, lkH_countsize(t)*2) == 0)
                return NULL;
            goto redo; /* return lkH_newkey(t, entry); */
        }
        othern = lkH_mainposition(t, mp->hash);
        if (othern != mp) {
            lk_Entry *next;
            while ((next = lk_index(othern, othern->next)) != mp)
                othern = next;
            othern->next = lk_offset(f, othern);
            memcpy(f, mp, t->entry_size);
            if (mp->next != 0) {
                f->next += lk_offset(mp, f);
                mp->next = 0;
            }
        }
        else {
            if (mp->next != 0)
                f->next = lk_offset(mp, f) + mp->next;
            else assert(f->next == 0);
            mp->next = lk_offset(f, mp);
            mp = f;
        }
    }
    mp->key  = entry->key;
    mp->hash = entry->hash;
    return mp;
}

static unsigned lkH_calchash (const char *s, size_t len) {
    size_t l1;
    size_t step = (len >> LK_HASHLIMIT) + 1;
    unsigned h = (unsigned)len;
    for (l1 = len; l1 >= step; l1 -= step)
        h ^= (h<<5) + (h>>2) + (unsigned char)s[l1 - 1];
    return h;
}

LK_API void lk_freetable (lk_State *S, lk_Table *t) {
    lk_free(S, t->hash, t->size*t->entry_size);
    lk_inittable(t, t->entry_size);
}

LK_API size_t lk_resizetable (lk_State *S, lk_Table *t, size_t len) {
    size_t i, size = t->size*t->entry_size;
    lk_Table nt = *t;
    nt.size = lkH_hashsize(t, len);
    if (nt.size == 0) return 0;
    nt.lastfree = nt.size*nt.entry_size;
    nt.hash = (lk_Entry*)lk_malloc(S, nt.lastfree);
    memset(nt.hash, 0, nt.lastfree);
    for (i = 0; i < size; i += t->entry_size) {
        lk_Entry *olde = lk_index(t->hash, i);
        lk_Entry *newe = lkH_newkey(S, &nt, olde);
        assert(newe != NULL);
        if (newe != NULL && t->entry_size > sizeof(lk_Entry))
            memcpy(newe + 1, olde + 1, t->entry_size - sizeof(lk_Entry));
    }
    lk_free(S, t->hash, size);
    *t = nt;
    return t->size;
}

LK_API lk_Entry *lk_gettable (lk_Table *t, const char *key) {
    unsigned hash;
    lk_Entry *e;
    if (t->size == 0 || key == NULL) return NULL;
    hash = lkH_calchash(key, strlen(key));
    e = lkH_mainposition(t, hash);
    for (;;) {
        int next = e->next;
        if (e->hash == hash && e->key && strcmp(e->key, key) == 0)
            return e;
        if (next == 0) return NULL;
        e = lk_index(e, next);
    }
}

LK_API lk_Entry *lk_settable (lk_State *S, lk_Table *t, const char *key) {
    lk_Entry e, *ret;
    if (key == NULL) return NULL;
    if ((ret = lk_gettable(t, key)) != NULL)
        return ret;
    e.key  = key;
    e.hash = lkH_calchash(key, strlen(key));
    if ((ret = lkH_newkey(S, t, &e)) == NULL)
        return NULL;
    if (t->entry_size > sizeof(lk_Entry))
        memset(ret + 1, 0, t->entry_size - sizeof(lk_Entry));
    return ret;
}

LK_API int lk_nextentry (lk_Table *t, lk_Entry **pentry) {
    size_t i = *pentry ? lk_offset(*pentry, t->hash) + t->entry_size : 0;
    const size_t size = t->size * t->entry_size;
    for (; i < size; i += t->entry_size) {
        lk_Entry *e = lk_index(t->hash, i);
        if (e->key != NULL) { *pentry = e; return 1; }
    }
    *pentry = NULL;
    return 0;
}


/* context routines */

LK_API lk_Context *lk_context (lk_State *S)
{ return S ? (lk_Context*)lk_gettls(S->tls_index) : NULL; }

LK_API void lk_pushcontext (lk_State *S, lk_Context *ctx, lk_Slot *slot) {
    ctx->prev = lk_context(S);
    ctx->S = S;
    ctx->current  = slot;
    ctx->cleanups = NULL;
    ctx->retcode  = LK_OK;
    lk_settls(S->tls_index, ctx);
}

LK_API void lk_popcontext (lk_State *S, lk_Context *ctx) {
    lk_Cleanup *cleanups = ctx ? ctx->cleanups : NULL;
    lk_lock(S->pool_lock);
    while (cleanups) {
        lk_Cleanup *next = cleanups->next;
        lk_poolfree(&S->cleanups, next);
        cleanups = next;
    }
    lk_unlock(S->pool_lock);
    lk_settls(S->tls_index, ctx->prev);
}

LK_API int lk_pcall (lk_State *S, lk_ProtectedHandler *h, void *ud) {
    int ret = LK_OK;
    lk_Context ctx;
    lk_pushcontext(S, &ctx, NULL);
    lk_try(S, &ctx, ret = h(S, ud));
    lk_popcontext(S, &ctx);
    return ctx.retcode == LK_ERR ? LK_ERR : ret;
}

LK_API int lk_discard (lk_State *S) {
    lk_Context *ctx = lk_context(S);
    if (ctx == NULL) {
        fprintf(stderr, "unproected errors\n");
        abort();
    }
    if (ctx->cleanups != NULL) {
        lk_Cleanup *cleanups = ctx->cleanups;
        while (cleanups != NULL) {
            lk_Cleanup *next = cleanups->next;
            cleanups->h(S, cleanups->ud);
            cleanups = next;
        }
        lk_lock(S->pool_lock);
        while (cleanups != NULL) {
            lk_Cleanup *next = cleanups->next;
            lk_poolfree(&S->cleanups, cleanups);
            cleanups = next;
        }
        lk_unlock(S->pool_lock);
    }
    ctx->retcode = LK_ERR;
    lk_throw(S, ctx);
    return LK_ERR;
}

LK_API int lk_addcleanup (lk_State *S, lk_ProtectedHandler *h, void *ud) {
    lk_Context *ctx = lk_context(S);
    lk_Cleanup *cleanup;
    if (ctx == NULL)
        return LK_ERR;
    lk_lock(S->pool_lock);
    cleanup = (lk_Cleanup*)lk_poolalloc(S, &S->cleanups);
    lk_unlock(S->pool_lock);
    cleanup->h = h;
    cleanup->ud = ud;
    cleanup->next = ctx->cleanups;
    ctx->cleanups = cleanup;
    return LK_OK;
}


/* platform specific routines */

static void lkS_dispatchGS (lk_State *S, lk_Service *svr);

#ifdef _WIN32

static size_t lkP_getcpucount (void) {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

static unsigned __stdcall lkP_poller (void *lpParameter) {
    lk_Context ctx;
    lk_Slot *slot  = (lk_Slot*)lpParameter;
    lk_Poll *poll = (lk_Poll*)slot;
    lk_State *S = slot->S;
    lk_pushcontext(S, &ctx, slot);
    lk_try(S, &ctx, slot->handler(S, slot, NULL));
    lk_popcontext(S, &ctx);
    poll->status = LK_STOPPING;
    return 0;
}

static unsigned __stdcall lkP_worker (void *lpParameter) {
    int status  = LK_WORKING;
    lk_State *S = (lk_State*)lpParameter;
    while (status == LK_WORKING) {
        lk_Service *svr;
        WaitForSingleObject(S->queue_event, INFINITE);
        lk_lock(S->queue_lock);
        lkQ_dequeue(&S->main_queue, svr);
        if ((status = S->status) >= LK_STOPPING)
            lk_signal(S->queue_event);
        lk_unlock(S->queue_lock);
        while (svr != NULL) {
            lkS_dispatchGS(S, svr);
            lk_lock(S->queue_lock);
            lkQ_dequeue(&S->main_queue, svr);
            lk_unlock(S->queue_lock);
        }
    }
    return 0;
}

LK_API void lk_waitclose (lk_State *S) {
    WaitForMultipleObjects(S->nthreads, S->threads, TRUE, INFINITE);
}

LK_API int lk_wait (lk_Slot *slot, lk_Signal* sig, int waitms) {
    lk_Poll *poll = (lk_Poll*)slot;
    lk_SignalNode *node;
    if (!slot->is_poll) return LK_ERR;
    lk_lock(poll->lock);
    lkQ_dequeue(&poll->signals, node);
    lk_unlock(poll->lock);
    if (waitms != 0 && (!node || poll->status < LK_STOPPING)) {
        DWORD timeout = waitms < 0 ? INFINITE : (DWORD)waitms;
        WaitForSingleObject(poll->event, timeout);
    }
    if (node) {
        lk_State *S = slot->S;
        if (sig) *sig = node->data;
        lk_lock(S->pool_lock);
        lk_poolfree(&S->signals, node);
        lk_unlock(S->pool_lock);
        return LK_OK;
    }
    return poll->status >= LK_STOPPING ? LK_ERR : LK_TIMEOUT;
}

#else

static size_t lkP_getcpucount (void) {
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN);
}

static void *lkP_poller (void *ud) {
    lk_Context ctx;
    lk_Slot *slot  = (lk_Slot*)ud;
    lk_Poll *poll = (lk_Poll*)slot;
    lk_State *S = slot->S;
    lk_pushcontext(S, &ctx, slot);
    lk_try(S, &ctx, slot->handler(S, slot, NULL));
    lk_popcontext(S, &ctx);
    poll->status = LK_STOPPING;
    return NULL;
}

static void *lkP_worker (void *ud) {
    int status = LK_WORKING;
    lk_State *S = (lk_State*)ud;
    lk_lock(S->queue_lock);
    while (status == LK_WORKING) {
        lk_Service *svr;
        for (;;) {
            lkQ_dequeue(&S->main_queue, svr);
            status = S->status;
            if (svr != NULL || status != LK_WORKING)
                break;
            pthread_cond_wait(&S->queue_event, &S->queue_lock);
        }
        if (status >= LK_STOPPING)
            pthread_cond_broadcast(&S->queue_event);
        while (svr != NULL) {
            lk_unlock(S->queue_lock);
            lkS_dispatchGS(S, svr);
            lk_lock(S->queue_lock);
            lkQ_dequeue(&S->main_queue, svr);
        }
    }
    lk_unlock(S->queue_lock);
    return NULL;
}

static int lkP_timedwait (lk_Event *event, lk_Lock *lock, int waitms) {
    struct timeval tv;
    struct timespec ts;
    int sec  = waitms / 1000;
    int usec = waitms % 1000 * 1000;
    gettimeofday(&tv, NULL);
    if (tv.tv_usec + usec > 1000000) {
        sec += 1;
        usec = (tv.tv_usec + usec) - 1000000;
    }
    ts.tv_sec = tv.tv_sec + sec;
    ts.tv_nsec = (tv.tv_usec + usec) * 1000;
    return pthread_cond_timedwait(event, lock, &ts);
}

LK_API void lk_waitclose (lk_State *S) {
    size_t i;
    for (i = 0; i < S->nthreads; ++i)
        pthread_join(S->threads[i], NULL);
    S->nthreads = 0;
}

LK_API int lk_wait (lk_Slot *slot, lk_Signal* sig, int waitms) {
    lk_Poll *poll = (lk_Poll*)slot;
    lk_SignalNode *node;
    if (!slot->is_poll) return LK_ERR;
    lk_lock(poll->lock);
    lkQ_dequeue(&poll->signals, node);
    if (waitms != 0 && (!node || poll->status < LK_STOPPING)) {
        if (waitms < 0)
            pthread_cond_wait(&poll->event, &poll->lock);
        else
            lkP_timedwait(&poll->event, &poll->lock, waitms);
    }
    lk_unlock(poll->lock);
    if (node) {
        if (sig) *sig = node->data;
        return LK_OK;
    }
    return poll->status >= LK_STOPPING ? LK_ERR : LK_TIMEOUT;
}

#endif


/* singal slot routines */

static void lkS_active (lk_State *S, lk_Service *svr, int sig);

LK_API const char *lk_name        (lk_Slot *slot) { return slot ? slot->name    : NULL; }
LK_API lk_Service *lk_service     (lk_Slot *slot) { return slot ? slot->service : NULL; }
LK_API lk_State   *lk_state       (lk_Slot *slot) { return slot ? slot->S       : NULL; }
LK_API void       *lk_data        (lk_Slot *slot) { return slot ? slot->data    : NULL; }
LK_API lk_Handler *lk_slothandler (lk_Slot *slot) { return slot ? slot->handler : NULL; }

LK_API lk_Slot *lk_current (lk_State *S)
{ lk_Context *ctx = lk_context(S); return ctx ? ctx->current : &S->root.slot; }

LK_API void lk_setdata (lk_Slot *slot, void *data)
{ if (slot) slot->data = data; }

LK_API void lk_setslothandler (lk_Slot *slot, lk_Handler *h)
{ if (slot) slot->handler = h; }

static void lkE_name (char *buff, const char *q, const char *name) {
    size_t qlen = strlen(q), namelen;
    assert(qlen < LK_MAX_NAMESIZE);
    memcpy(buff, q, qlen); buff += qlen;
    if (name == NULL) return;
    *buff++ = '.';
    namelen = strlen(name);
    assert(namelen < LK_MAX_NAMESIZE);
    memcpy(buff, name, namelen); buff += namelen;
    *buff = '\0';
}

static lk_Slot *lkE_new (lk_State *S, size_t sz, const char *svr, const char *name) {
    lk_Slot *slot = NULL;
    lk_MemPool *pool = NULL;
    switch (sz) {
    case sizeof(lk_Service): pool = &S->services; break;
    case sizeof(lk_Slot):    pool = &S->slots; break;
    case sizeof(lk_Poll):    pool = &S->polls; break;
    default: assert(!"slot size error"); return NULL;
    }
    assert(pool->size == sz);
    lk_lock(S->pool_lock);
    slot = (lk_Slot*)lk_poolalloc(S, pool);
    lk_unlock(S->pool_lock);
    memset(slot, 0, sz);
    lkE_name(slot->name, svr, name);
    slot->S = S;
    return slot;
}

static lk_Slot *lkE_register (lk_State *S, lk_Slot *slot) {
    lk_Entry *e = lk_settable(S, &S->slot_names, slot->name);
    if ((lk_Slot*)e->key != slot) {
        if (&slot->service->slot == slot)
            lk_freelock(slot->service->lock);
        lk_lock(S->pool_lock);
        if (slot->is_svr)       lk_poolfree(&S->services, slot);
        else if (slot->is_poll) lk_poolfree(&S->polls, slot);
        else                    lk_poolfree(&S->slots, slot);
        lk_unlock(S->pool_lock);
        return NULL;
    }
    return slot;
}

static int lkE_startpoll (lk_Poll *poll) {
    lk_State *S = poll->slot.S;
    lkQ_init(&poll->signals);
    poll->status = LK_WORKING;
    if (!lk_initlock(&poll->lock))   goto err_lock;
    if (!lk_initevent(&poll->event)) goto err_event;
    if (!lk_initthread(&poll->thread, lkP_poller, poll)) {
        lk_freeevent(poll->event);
err_event:
        lk_freelock(poll->lock);
err_lock:
        lk_lock(S->pool_lock);
        lk_poolfree(&S->polls, poll);
        lk_unlock(S->pool_lock);
        return LK_ERR;
    }
    return LK_OK;
}

static void lkE_delpollG (lk_State *S, lk_Poll *poll) {
    lk_lock(poll->lock);
    poll->status = LK_STOPPING;
    lk_signal(poll->event);
    lk_unlock(poll->lock);
    lk_waitthread(poll->thread);
    lk_freeevent(poll->event);
    lk_freelock(poll->lock);
    lk_lock(S->pool_lock);
    {
        lk_SignalNode *node = poll->signals.first;
        while (node) {
            lk_SignalNode *next = node->next;
            lk_poolfree(&S->signals, node);
            node = next;
        }
    }
    lk_poolfree(&S->polls, poll);
    lk_unlock(S->pool_lock);
}

static lk_Slot *lkE_findslotG (lk_State *S, const char *name) {
    lk_Slot *slot = NULL;
    lk_Entry *e;
    lk_lock(S->lock);
    e = lk_gettable(&S->slot_names, name);
    if (e != NULL) slot = (lk_Slot*)e->key;
    lk_unlock(S->lock);
    return slot;
}

static void lkE_emitpollP (lk_Poll *poll, lk_SignalNode *node) {
    lk_lock(poll->lock);
    lkQ_enqueue(&poll->signals, node);
    lk_signal(poll->event);
    lk_unlock(poll->lock);
}

static void lkE_emitslotS (lk_State *S, lk_Service *svr, lk_SignalNode *node) {
    lk_lock(svr->lock);
    lkQ_enqueue(&svr->signals, node);
    if (svr->status == LK_SLEEPING) {
        svr->status = LK_WORKING;
        lkS_active(S, svr, 1);
    }
    lk_unlock(svr->lock);
}

static int lkE_check (lk_State *S, const char *tag, const char *name) {
    if (name == NULL)
        lk_log(S, "E[%s]" lk_loc("slot name required"));
    else if (strlen(name) >= LK_MAX_NAMESIZE)
        lk_log(S, "E[%s]" lk_loc("slot name '%s' too long"), tag, name);
    else if (lk_self(S)->status < LK_STOPPING)
        return 1;
    return 0;
}

static lk_Service *lkE_checkservice (lk_State *S, lk_Slot *slot, lk_Service *src) {
    lk_Service *dst = slot ? slot->service : NULL;
    if (S == NULL || src == NULL || dst == NULL) return NULL;
    if (dst->slot.S->status >= LK_STOPPING)
        return NULL; /*lk_log(S, "E[emit]" lk_loc("host is stopping"));*/
    else if (src->status >= LK_STOPPING && (lk_Slot*)dst == S->logger)
        return dst; /* lk_log(S, "E[emit]" lk_loc("source service is stopping")); */
    else if (dst->status >= LK_STOPPING && (lk_Slot*)dst != S->logger)
        lk_log(S, "E[emit]" lk_loc("destination service is stopping"));
    else
        return dst;
    return NULL;
}

LK_API lk_Slot *lk_newslot (lk_State *S, const char *name, lk_Handler *h, void *data) {
    lk_Service *svr = lk_self(S);
    lk_Slot *slot = NULL;
    if (S == NULL || svr == NULL || !lkE_check(S, "newslot", name))
        return NULL;
    slot = lkE_new(S, sizeof(lk_Slot), svr->slot.name, name);
    slot->service = svr;
    slot->handler = h;
    slot->data    = data;
    lk_lock(S->lock);
    if ((slot = lkE_register(S, slot)) != NULL) {
        slot->next = svr->slots;
        svr->slots = slot;
    }
    lk_unlock(S->lock);
    if (slot == NULL)
        lk_log(S, "E[newslot]", lk_loc("slot '%s' exists"), name);
    return slot;
}

LK_API lk_Slot *lk_newpoll (lk_State *S, const char *name, lk_Handler *h, void *data) {
    lk_Service *svr = lk_self(S);
    lk_Poll *poll;
    if (S == NULL || svr == NULL || !lkE_check(S, "newpoll", name))
        return NULL;
    poll = (lk_Poll*)lkE_new(S, sizeof(lk_Poll), svr->slot.name, name);
    poll->slot.is_poll = 1;
    poll->slot.service = svr;
    poll->slot.handler = h;
    poll->slot.data    = data;
    if (lkE_startpoll(poll) != LK_OK) return NULL;
    lk_lock(S->lock);
    if ((poll = (lk_Poll*)lkE_register(S, &poll->slot)) != NULL) {
        poll->slot.next = svr->polls;
        svr->polls = &poll->slot;
    }
    lk_unlock(S->lock);
    if (poll == NULL) {
        lk_log(S, "E[newpoll]", lk_loc("poll '%s' exists"), name);
        return NULL;
    }
    return &poll->slot;
}

LK_API lk_Slot *lk_slot (lk_State *S, const char *name) {
    lk_Service *svr = lk_self(S);
    lk_Slot *slot = NULL;
    if (S == NULL || svr == NULL || !lkE_check(S, "slot", name))
        return NULL;
    if (strchr(name, '.') == NULL) {
        char qname[LK_MAX_NAMESIZE];
        lkE_name(qname, svr->slot.name, name);
        slot = lkE_findslotG(S, qname);
    }
    if (slot == NULL)
        slot = lkE_findslotG(S, name);
    if (slot == NULL)
        lk_log(S, "E[slot]" lk_loc("slot '%s' not exists"), name);
    return slot;
}

LK_API int lk_emit (lk_Slot *slot, const lk_Signal *sig) {
    lk_State *S = slot ? slot->S : NULL;
    lk_Service *src = sig && sig->src ? sig->src : lk_self(S);
    lk_Service *dst = lkE_checkservice(S, slot, src);
    lk_SignalNode *node = NULL;
    if (sig == NULL || dst == NULL) return LK_ERR;
    lk_lock(S->pool_lock);
    node = (lk_SignalNode*)lk_poolalloc(S, &S->signals);
    lk_unlock(S->pool_lock);
    node->slot = slot;
    node->data = *sig;
    node->data.src = src;
    if (slot->is_poll)
        lkE_emitpollP((lk_Poll*)slot, node);
    else
        lkE_emitslotS(S, dst, node);
    if (!slot->is_poll) {
        lk_lock(src->lock);
        ++src->pending;
        lk_unlock(src->lock);
    }
    return LK_OK;
}

LK_API int lk_emitstring (lk_Slot *slot, unsigned type, unsigned session, const char *s) {
    lk_Signal sig = LK_SIGNAL;
    sig.free    = 1;
    sig.type    = type;
    sig.session = session;
    if (slot) {
        sig.data = lk_newstring(slot->S, s);
        sig.size = lk_len((lk_Data*)sig.data);
    }
    return lk_emit(slot, &sig);
}

LK_API int lk_emitdata (lk_Slot *slot, unsigned type, unsigned session, lk_Data *data) {
    lk_Signal sig = LK_SIGNAL;
    sig.free    = 1;
    sig.type    = type;
    sig.session = session;
    if (slot && data) {
        sig.size = lk_len(data);
        sig.data = data;
    }
    return lk_emit(slot, &sig);
}


/* service routines */

static void lkG_onuse   (lk_State *S, lk_Service *svr);
static void lkG_onopen  (lk_State *S, lk_Service *svr);
static void lkG_onweak  (lk_State *S, lk_Service *svr);
static void lkG_onclose (lk_State *S, lk_Service *svr);

LK_API lk_Handler *lk_refactor (lk_Service *svr)
{ return svr ? svr->refactor : NULL; }

LK_API void lk_setrefactor (lk_Service *svr, lk_Handler *h)
{ if (svr) svr->refactor = h; }

LK_API lk_Service *lk_self (lk_State *S)
{ lk_Slot *slot = lk_current(S); return slot ? slot->service : NULL; }

static void lkS_freeslotsG (lk_State *S, lk_Service *svr) {
    lk_Slot *polls;
    lk_lock(S->lock);
    lk_lock(S->pool_lock);
    while (svr->slots != NULL) {
        lk_Slot *next = svr->slots->next;
        lk_Entry *e = lk_gettable(&S->slot_names, svr->slots->name);
        assert(e && (lk_Slot*)e->key == svr->slots);
        if (!svr->slots->is_svr)
            lk_poolfree(&S->slots, svr->slots);
        e->key = NULL;
        svr->slots = next;
    }
    lk_unlock(S->pool_lock);
    polls = svr->polls;
    while (svr->polls != NULL) {
        lk_Slot *next = svr->polls->next;
        lk_Entry *e = lk_gettable(&S->slot_names, svr->polls->name);
        assert(e && (lk_Slot*)e->key == svr->polls);
        e->key = NULL;
        svr->polls = next;
    }
    lk_unlock(S->lock);
    while (polls != NULL) {
        lk_Slot *next = polls->next;
        lkE_delpollG(S, (lk_Poll*)polls);
        polls = next;
    }
}

static void lkS_delserviceG (lk_State *S, lk_Service *svr) {
    if (svr->slot.handler) {
        lk_Context ctx;
        lk_pushcontext(S, &ctx, &svr->slot);
        lk_try(S, &ctx, svr->slot.handler(S, &svr->slot, NULL));
        lk_popcontext(S, &ctx);
    }
    lkS_freeslotsG(S, svr);
    lk_freelock(svr->lock);
    assert(lkQ_empty(&svr->signals));
    if (svr != &S->root) {
        lk_lock(S->pool_lock);
        lk_poolfree(&S->services, svr);
        lk_unlock(S->pool_lock);
    }
    lk_lock(S->lock);
    if (!svr->weak) --S->nservices;
    if ((S->nservices == 1 && S->root.slot.handler == NULL)
            || S->nservices == 0) {
        S->status = LK_STOPPING;
        lk_lock(S->queue_lock);
        lk_signal(S->queue_event);
        lk_unlock(S->queue_lock);
    }
    lk_unlock(S->lock);
}

static void lkS_active (lk_State *S, lk_Service *svr, int sig) {
    lk_lock(S->queue_lock);
    lkQ_enqueue(&S->main_queue, svr);
    if (sig) lk_signal(S->queue_event);
    lk_unlock(S->queue_lock);
}

static void lkS_callslot (lk_State *S, lk_SignalNode *node, lk_Context *ctx) {
    int ret = LK_ERR;
    lk_Slot *slot = node->slot;
    lk_Service *src = node->data.src;
    assert(src != NULL);
    ctx->current = slot;
    if (src->refactor && !slot->no_refactor)
        lk_try(S, ctx, ret = src->refactor(S, slot, &node->data));
    if (ret == LK_ERR && slot->handler)
        lk_try(S, ctx, slot->handler(S, slot, &node->data));
    if (node->data.free)
        lk_deldata(S, (lk_Data*)node->data.data);
}

static void lkS_callslotsS (lk_State *S, lk_Service *svr) {
    lk_Context ctx;
    lk_SignalNode *node;
    /* fetch all signal */
    lk_lock(svr->lock);
    lkQ_clear(&svr->signals, node);
    lk_unlock(svr->lock);

    /* call signal handler */
    lk_pushcontext(S, &ctx, &svr->slot);
    while (node != NULL) {
        lk_SignalNode *next = node->next;
        lk_Service *src = node->data.src;
        lkS_callslot(S, node, &ctx);
        lk_lock(S->pool_lock);
        lk_poolfree(&S->signals, node);
        lk_unlock(S->pool_lock);
        lk_lock(src->lock);
        if (--src->pending == 0 && src->status == LK_ZOMBIE)
            lkS_active(S, src, 0);
        lk_unlock(src->lock);
        node = next;
    }
    lk_popcontext(S, &ctx);
}

static void lkS_dispatchGS (lk_State *S, lk_Service *svr) {
    int should_delete = 0;
    lkS_callslotsS(S, svr);

    lk_lock(svr->lock);
    if (!lkQ_empty(&svr->signals))
        lkS_active(S, svr, 0);
    if (svr->status == LK_STOPPING && svr->pending != 0)
        svr->status = LK_ZOMBIE;
    else if (svr->status < LK_STOPPING)
        svr->status = LK_SLEEPING;
    else if (svr->pending == 0)
        should_delete = 1;
    lk_unlock(svr->lock);

    if (should_delete) lkS_delserviceG(S, svr);
}

static int lkS_check (lk_State *S, const char *name) {
    if (strlen(name) >= LK_MAX_NAMESIZE) {
        lk_log(S, "E[launch]" lk_loc("serivce name '%s' too long"), name);
        return 0;
    }
    if (S->status >= LK_STOPPING)
        return 0;
    return 1;
}

static int lkS_callinit (lk_State *S, lk_Service *svr) {
    lk_Context ctx;
    int ret = LK_ERR;
    lk_Handler *h = svr->slot.handler;
    lk_pushcontext(S, &ctx, &svr->slot);
    lk_try(S, &ctx, ret = h(S, NULL, NULL));
    lk_popcontext(S, &ctx);
    if (ret < LK_OK || svr->status >= LK_STOPPING) {
        lk_log(S, "E[launch]" lk_loc("serivce '%s' initialize failure"),
                svr->slot.name);
        lkS_delserviceG(S, svr);
        return 0;
    }
    if (ret == LK_WEAK) {
        svr->weak = 1;
        lk_lock(S->lock);
        --S->nservices;
        if (&svr->slot != S->monitor)
            lkG_onweak(S, svr);
        lk_unlock(S->lock);
    }
    return 1;
}

static lk_Service *lkS_initserviceGS (lk_State *S, lk_Service *svr, lk_Handler *h, void *data) {
    svr->slot.handler = h;
    svr->slot.data    = data;
    lk_lock(S->lock);
    ++S->nservices;
    lkG_onopen(S, svr);
    lk_unlock(S->lock);
    if (h && !lkS_callinit(S, svr))
        return NULL;
    lk_lock(svr->lock);
    svr->status = LK_SLEEPING;
    if (!lkQ_empty(&svr->signals)) {
        svr->status = LK_WORKING;
        lkS_active(S, svr, 0);
    }
    lk_unlock(svr->lock);
    return svr;
}

LK_API lk_Service *lk_launch (lk_State *S, const char *name, lk_Handler *h, void *data) {
    lk_Service *svr;
    if (S == NULL || name == NULL || h == NULL) return NULL;
retry:
    if ((svr = (lk_Service*)lkE_findslotG(S, name)) != NULL) {
        lkG_onuse(S, svr);
        return svr;
    }
    if (!lkS_check(S, name)) return NULL;
    svr = (lk_Service*)lkE_new(S, sizeof(lk_Service), name, NULL);
    if (!svr) return NULL; /* avoid MSVC warning */
    if (!lk_initlock(&svr->lock)) {
        lk_lock(S->pool_lock);
        lk_poolfree(&S->services, svr);
        lk_unlock(S->pool_lock);
        return NULL;
    }
    svr->slot.is_svr  = 1;
    svr->slot.service = svr;
    svr->slots = &svr->slot;
    lkQ_init(&svr->signals);
    lk_lock(S->lock);
    if (!lkE_register(S, &svr->slot))
        svr = NULL; /* already created? retry lounch */
    lk_unlock(S->lock);
    if (svr == NULL) goto retry;
    /* only one thread could reach here */
    return lkS_initserviceGS(S, svr, h, data);
}


/* global routines */

static void lkG_onuse (lk_State *S, lk_Service *svr) {
    if (S->monitor) {
        lk_Signal sig = LK_SIGNAL;
        sig.type = LK_MONITOR_ONUSE;
        sig.data = svr;
        lk_emit(S->monitor, &sig);
    }
}

static void lkG_onopen (lk_State *S, lk_Service *svr) {
    if (S->monitor) {
        lk_Signal sig = LK_SIGNAL;
        sig.type = LK_MONITOR_ONOPEN;
        sig.data = svr;
        lk_emit(S->monitor, &sig);
    }
    if (!S->logger && strcmp(svr->slot.name, "log") == 0) {
        S->logger = &svr->slot;
        svr->slot.no_refactor = 1;
    }
    if (!S->monitor && strcmp(svr->slot.name, "monitor") == 0) {
        S->monitor = &svr->slot;
        svr->slot.no_refactor = 1;
    }
}

static void lkG_onweak (lk_State *S, lk_Service *svr) {
    if (S->monitor) {
        lk_Signal sig = LK_SIGNAL;
        sig.src  = svr;
        sig.type = LK_MONITOR_ONWEAK;
        lk_emit(S->monitor, &sig);
    }
}

static void lkG_onclose (lk_State *S, lk_Service *svr) {
    if (&svr->slot == S->logger)  S->logger = NULL;
    if (&svr->slot == S->monitor) S->monitor = NULL;
    if (S->monitor && S->monitor != &svr->slot) {
        lk_Signal sig = LK_SIGNAL;
        sig.type = LK_MONITOR_ONCLOSE;
        lk_emit(S->monitor, &sig);
    }
}

static int lkG_initroot (lk_State *S, const char *name) {
    lk_Service *svr = &S->root;
    lk_strcpy(svr->slot.name, name, LK_MAX_NAMESIZE);
    svr->slot.is_svr = 1;
    svr->slot.S = S;
    svr->slot.service = svr;
    svr->slots = &svr->slot;
    lkQ_init(&svr->signals);
    return lk_initlock(&svr->lock);
}

static void lkG_delstate (lk_State *S) {
    lk_Entry *e = NULL;
    size_t i;
    while (lk_nextentry(&S->slot_names, &e)) {
        lk_Slot *slot = (lk_Slot*)e->key;
        if (slot->is_svr) {
            assert(slot->service->weak);
            lkS_delserviceG(S, (lk_Service*)slot);
        }
    }
    while (lk_nextentry(&S->config, &e))
        lk_deldata(S, (lk_Data*)e->key);
    lk_freepool(S, &S->cleanups);
    lk_freepool(S, &S->signals);
    lk_freepool(S, &S->smallstrings);
    lk_freepool(S, &S->services);
    lk_freepool(S, &S->slots);
    lk_freepool(S, &S->polls);
    lk_freetable(S, &S->config);
    lk_freetable(S, &S->slot_names);
    for (i = 0; i < S->nthreads; ++i)
        lk_freethread(S->threads[i]);
    lk_freeevent(S->queue_event);
    lk_freetls(S->tls_index);
    lk_freelock(S->config_lock);
    lk_freelock(S->queue_lock);
    lk_freelock(S->lock);
    S->allocf(S->alloc_ud, S, 0, sizeof(lk_State));
}

static void *default_allocf (void *ud, void *ptr, size_t size, size_t osize) {
    (void)ud, (void)osize;
    if (size == 0) { free(ptr); return NULL; }
    return realloc(ptr, size);
}

LK_API lk_State *lk_newstate (const char *name, lk_Allocf *allocf, void *ud) {
    lk_State *S;
    allocf = allocf ? allocf : default_allocf;
    S = (lk_State*)allocf(ud, NULL, sizeof(lk_State), 0);
    if (S == NULL) return NULL;
    memset(S, 0, sizeof(*S));
    S->allocf   = allocf;
    S->alloc_ud = ud;
    name = name ? name : LK_NAME;
    if (!lk_inittls(&S->tls_index))     goto err_tls;
    if (!lk_initevent(&S->queue_event)) goto err_event;
    if (!lk_initlock(&S->lock))         goto err_lock;
    if (!lk_initlock(&S->queue_lock))   goto err_queuelock;
    if (!lk_initlock(&S->config_lock))  goto err_configlock;
    if (!lk_initlock(&S->pool_lock))    goto err_poollock;
    if (!lkG_initroot(S, name))         goto err;
    lk_initpool(&S->cleanups, sizeof(lk_Cleanup));
    lk_initpool(&S->signals, sizeof(lk_SignalNode));
    lk_initpool(&S->smallstrings, LK_SMALLSTRING_LEN);
    lk_initpool(&S->services, sizeof(lk_Service));
    lk_initpool(&S->slots, sizeof(lk_Slot));
    lk_initpool(&S->polls, sizeof(lk_Poll));
    lk_inittable(&S->config, sizeof(lk_PtrEntry));
    lk_inittable(&S->slot_names, sizeof(lk_Entry));
    lk_settable(S, &S->slot_names, S->root.slot.name);
    return S;
err:            lk_freelock(S->pool_lock);
err_poollock:   lk_freelock(S->config_lock);
err_configlock: lk_freelock(S->queue_lock);
err_queuelock:  lk_freelock(S->lock);
err_lock:       lk_freeevent(S->queue_event);
err_event:      lk_freetls(S->tls_index);
err_tls:        free(S);
    return NULL;
}

LK_API void lk_close (lk_State *S) {
    lk_Context *ctx = lk_context(S);
    lk_Service *svr;
    if (ctx == NULL && S->status >= LK_STOPPING)
        lkG_delstate(S);
    if (ctx != NULL && (svr = ctx->current->service) != NULL) {
        unsigned status;
        lk_lock(S->lock);
        lkG_onclose(S, svr);
        lk_lock(svr->lock);
        status = svr->status;
        svr->status = LK_STOPPING;
        if (status == LK_SLEEPING)
            lkS_active(S, svr, 1);
        lk_unlock(svr->lock);
        lk_unlock(S->lock);
    }
}

LK_API int lk_start (lk_State *S, int threads) {
    size_t i, count = 0;
    lkS_initserviceGS(S, &S->root, S->root.slot.handler, S->root.slot.data);
    lk_lock(S->lock);
    S->nthreads = threads;
    if (S->nthreads <= 0)
        S->nthreads = lkP_getcpucount();
    S->status = LK_WORKING;
    for (i = 0; i < S->nthreads; ++i) {
        if (!lk_initthread(&S->threads[i], lkP_worker, S))
            break;
    }
    count = S->nthreads = i;
    lk_unlock(S->lock);
    return count;
}

LK_API char *lk_getconfig (lk_State *S, const char *key) {
    lk_PtrEntry *e;
    char *value = NULL;
    lk_lock(S->config_lock);
    if ((e = (lk_PtrEntry*)lk_gettable(&S->config, key)) != NULL)
        value = (char*)lk_newstring(S, (const char*)e->data);
    lk_unlock(S->config_lock);
    return value;
}

LK_API void lk_setconfig (lk_State *S, const char *key, const char *value) {
    lk_PtrEntry *e;
    char *data;
    size_t ksize, vsize;
    assert(key != NULL);
    lk_lock(S->config_lock);
    if (value == NULL) {
        e = (lk_PtrEntry*)lk_gettable(&S->config, key);
        if (e) lk_deldata(S, (lk_Data*)lk_key(e)), lk_key(e) = NULL;
        goto out;
    }
    e = (lk_PtrEntry*)lk_settable(S, &S->config, key);
    ksize = strlen(key);
    vsize = strlen(value);
    if (e->data && strlen((const char*)e->data) >= vsize) {
        memcpy(e->data, value, vsize+1);
        goto out;
    }
    data = (char*)lk_newdata(S, ksize+vsize+2);
    lk_key(e) = data;
    e->data = data + ksize + 1;
    memcpy(data, key, ksize+1);
    memcpy(e->data, value, vsize+1);
out: lk_unlock(S->config_lock);
}

LK_API int lk_log (lk_State *S, const char *fmt, ...) {
    va_list l;
    int ret;
    va_start(l, fmt);
    ret = lk_vlog(S, fmt, l);
    va_end(l);
    return ret;
}

LK_API int lk_vlog (lk_State *S, const char *fmt, va_list l) {
    lk_Slot *logger = S->logger;
    lk_Signal sig = LK_SIGNAL;
    if (!logger) return LK_OK;
    sig.free = 1;
    sig.data = lk_newvfstring(S, fmt, l);
    sig.size = lk_len((lk_Data*)sig.data);
    return lk_emit(logger, &sig);
}


LK_NS_END

#endif

/* win32cc: flags+='-Wextra -s -O3 -mdll -DLOKI_IMPLEMENTATION -std=c90 -pedantic -xc'
 * win32cc: output='loki.dll'
 * unixcc: flags+='-Wextra -s -O3 -fPIC -shared -DLOKI_IMPLEMENTATION -xc'
 * unixcc: output='loki.so' */

