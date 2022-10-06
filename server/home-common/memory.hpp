#pragma once
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_unique.hpp>

#ifdef _WIN32
typedef INT_PTR ssize_t;
#endif

template <class T>
using Ref = boost::shared_ptr<T>;

template<class T>
using UniqueRef = std::unique_ptr<T>;

template <class T>
using WeakRef = boost::weak_ptr<T>;

#define SAFE_DELETE(e)                                                                                                 \
    if (e)                                                                                                             \
    {                                                                                                                  \
        delete e;                                                                                                      \
        e = nullptr;                                                                                                   \
    }
#define SAFE_DELETE_ARRAY(e)                                                                                           \
    if (e)                                                                                                             \
    {                                                                                                                  \
        delete[] e;                                                                                                    \
        e = nullptr;                                                                                                   \
    }
#define SAFE_DELETE_METHOD(e, meth)                                                                                    \
    if (e)                                                                                                             \
    {                                                                                                                  \
        e->##meth();                                                                                                   \
        e = nullptr;                                                                                                   \
    }
