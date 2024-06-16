#include <threads.h>

struct _thrd_ctx {
	size_t nr_online;
	cnd_t cond;
	mtx_t mutex;
};
typedef struct _thrd_ctx thrd_ctx;

struct _thrd_cb {
	void *arg;
	thrd_start_t func;
	thrd_ctx *mctx;
};
typedef struct _thrd_cb thrd_cb;

static int thrd_ctx_init(thrd_ctx *mctx)
{
	int ret;

	mctx->nr_online = 0;
	ret = mtx_init(&mctx->mutex, mtx_plain);
	if (ret != thrd_success)
		return ret;

	ret = cnd_init(&mctx->cond);
	if (ret != thrd_success)
		mtx_destroy(&mctx->mutex);

	return ret;
}

static void thrd_ctx_destroy(thrd_ctx *mctx)
{
	cnd_destroy(&mctx->cond);
	mtx_destroy(&mctx->mutex);
}

static void thrd_joinv(thrd_ctx *mctx)
{
	mtx_lock(&mctx->mutex);
	while (mctx->nr_online > 0)
		cnd_wait(&mctx->cond, &mctx->mutex);
	mtx_unlock(&mctx->mutex);
}

static int __thrd_entry(void *arg)
{
	thrd_cb *ctx = arg;
	thrd_ctx *mctx = ctx->mctx;
	int ret;

	ret = ctx->func(ctx->arg);
	mtx_lock(&mctx->mutex);
	if (!--mctx->nr_online)
		cnd_signal(&mctx->cond);
	mtx_unlock(&mctx->mutex);
	return ret;
}

static int thrd_createv(thrd_ctx *mctx, thrd_cb *ctx, thrd_start_t func, void *arg)
{
	thrd_t thr;
	int ret;

	ctx->func = func;
	ctx->arg = arg;
	ctx->mctx = mctx;

	mtx_lock(&mctx->mutex);
	mtx_unlock(&mctx->mutex);
	ret = thrd_create(&thr, &__thrd_entry, ctx);
	if (ret != thrd_success) {
		mtx_lock(&mctx->mutex);
		mtx_unlock(&mctx->mutex);
	} else {
                ++mctx->nr_online;
		thrd_detach(thr);
	}

	return ret;
}
