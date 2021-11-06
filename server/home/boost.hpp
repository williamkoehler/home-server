#pragma once
#define BOOST_ASIO_USE_TS_EXECUTOR_AS_DEFAULT 

#include <boost/algorithm/algorithm.hpp>
#include <boost/variant.hpp>
#include <boost/regex.hpp>

//IO
#include <boost/filesystem.hpp>

//Container
#include <boost/container/vector.hpp>
#include <boost/container/list.hpp>
#include <boost/container/set.hpp>
#include <boost/container/slist.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/circular_buffer.hpp>

//Threading
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

//Function
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>