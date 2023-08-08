#pragma once

#include <functional>

#include <QCoreApplication>
#include <QThread>

namespace detail {

struct QExecutorOperationImpl : QObject
{
    using fn_t = std::function<void()>;

    Q_OBJECT

public slots:
    void run(fn_t f)
    {
        f();
    }
};

} // namespace detail

template <class F, class... Args>
void qPost(F&& f, Args&&... args)
{
    static detail::QExecutorOperationImpl executor_op{};
    static auto _ = []() {
        auto app = qApp;
        Q_ASSERT(app);

        auto ui_thrd = app->thread();
        Q_ASSERT(ui_thrd);

        executor_op.moveToThread(ui_thrd);
        return 0;
    }(); // call once

    if constexpr (sizeof...(args) > 0) {
        auto op = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        QMetaObject::invokeMethod(&executor_op, "run", Q_ARG(detail::QExecutorOperationImpl::fn_t, std::move(op)));
    } else {
        QMetaObject::invokeMethod(&executor_op, "run", Q_ARG(detail::QExecutorOperationImpl::fn_t, std::forward<F>(f)));
    }
}

template <class F, class... Args>
void qDispatch(F&& f, Args&&... args)
{
    auto app = qApp;
    if (app && QThread::currentThread() == app->thread()) {
        std::forward<F>(f)(std::forward<Args>(args)...);
        return ;
    }
    qPost(std::forward<F>(f), std::forward<Args>(args)...);
}
