# thrdv
C Threads with multiple join support that only call atleast *1 futex wait* or more so it can reduce system call.
Because C ISO didn't specify multiple thread join and `futex_waitv` isn't used.
## Interface
```c
int thrd_ctx_init(thrd_ctx *mctx);
int thrd_ctx_destory(thrd_ctx *mctx);
int thrd_createv(thrd_ctx *mctx, thrd_cb *ctx, thrd_start_func, void *arg);
void thrd_joinv(thrd_ctx *mctx);
```
### Example
```c
#include <thrdv.h>
#include <stdio.h>

int main(void)
{
    thrd_cb ctx[10];
    thrd_ctxv mctx;
    int ret;
    ret = thrd_ctxv_init(&mctx);
    if (ret != thrd_success) {
        fprintf(stderr, "thrd_ctxv_init failed\n");
        return 1;
    }

    for (size_t i = 0; i < 10; i++) {
        ret = thrd_createv(&mctx, &ctx[i], &my_func, (void *)i);
    }

	thrd_joinv(&mctx);
	thrd_ctxv_destroy(&mctx);
}
```
