#pragma once

#include <functional>

#include <QCoreApplication>

namespace detail {

struct Poster : QObject
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

template <typename F>
void qPost2Ui(F&& f)
{
    static detail::Poster p{};
    static auto _ = []() {
        auto app = qApp;
        Q_ASSERT(app);

        auto ui_thrd = app->thread();
        Q_ASSERT(ui_thrd);

        p.moveToThread(ui_thrd);
        return 0;
    }(); // call once
    QMetaObject::invokeMethod(&p, "run", Q_ARG(detail::Poster::fn_t, std::forward<F>(f)));
}
