#pragma once
#include <home-common/common.hpp>

// Boost Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HomeServer
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(initialize)
{
    // Create logger
    Log::Create();
}