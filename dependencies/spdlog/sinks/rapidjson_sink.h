#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/log_msg_buffer.h"
#include "spdlog/details/null_mutex.h"

#include <mutex>
#include <rapidjson/document.h>

namespace spdlog {
    namespace sinks {
        /*
         * Ring buffer sink
         */
        template<typename Mutex>
        class ringbuffer_sink final : public base_sink<Mutex>
        {
        public:
            explicit ringbuffer_sink(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
                : value(value), allocator(allocator)
            {
                value.SetArray();
            }

        protected:
            void sink_it_(const details::log_msg& msg) override
            {
                memory_buf_t formatted;
                base_sink<Mutex>::formatter_->format(msg, formatted);

                value.PushBack(rapidjson::Value(formatted.data(), formatted.size(), allocator), allocator);
            }
            void flush_() override {}

        private:
            const rapidjson::Value& value;
            const rapidjson::Document::AllocatorType& allocator;
        };

        using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
        using ringbuffer_sink_st = ringbuffer_sink<details::null_mutex>;

    } // namespace sinks

} // namespace spdlog
