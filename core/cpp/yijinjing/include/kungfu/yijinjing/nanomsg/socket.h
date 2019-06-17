//
// Created by Keren Dong on 2019-05-25.
//

#ifndef KUNGFU_NANOMSG_SOCKET_H
#define KUNGFU_NANOMSG_SOCKET_H

#include <string>
#include <cstring>
#include <algorithm>
#include <exception>
#include <nlohmann/json.hpp>
#include <nanomsg/nn.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/pipeline.h>

#include <kungfu/yijinjing/common.h>

#define MAX_MSG_LENGTH 16 * 1024
#define DEFAULT_NOTICE_TIMEOUT 1000

namespace kungfu {
    namespace yijinjing {

        namespace nanomsg {

            enum class protocol : int {
                UNKNOWN = -1,
                REPLY = NN_REP,
                REQUEST = NN_REQ,
                PUSH = NN_PUSH,
                PULL = NN_PULL,
                PUBLISH = NN_PUB,
                SUBSCRIBE = NN_SUB
            };

            inline const std::string get_protocol_name(protocol p) {
                switch (p) {
                    case protocol::REPLY:
                        return "reply";
                    case protocol::REQUEST:
                        return "request";
                    case protocol::PUSH:
                        return "push";
                    case protocol::PULL:
                        return "pull";
                    case protocol::PUBLISH:
                        return "publish";
                    case protocol::SUBSCRIBE:
                        return "subscribe";
                    default:
                        return "unknown";
                }
            }

            inline const protocol get_opposite_protol(protocol p) {
                switch (p) {
                    case protocol::REPLY:
                        return protocol::REQUEST;
                    case protocol::REQUEST:
                        return protocol::REPLY;
                    case protocol::PUSH:
                        return protocol::PULL;
                    case protocol::PULL:
                        return protocol::PUSH;
                    case protocol::PUBLISH:
                        return protocol::SUBSCRIBE;
                    case protocol::SUBSCRIBE:
                        return protocol::PUBLISH;
                    default:
                        return protocol::UNKNOWN;
                }
            }

            class url_factory {
            public:
                virtual const std::string make_url_bind(const data::location_ptr location, protocol p) const = 0;
                virtual const std::string make_url_connect(const data::location_ptr location, protocol p) const = 0;
            };
            DECLARE_PTR(url_factory)

            static const char *symbol(int i, int *value);

            static void term();

            class nn_exception : public std::exception {
            public:
                nn_exception() : errno_(nn_errno()) {}

                virtual const char *what() const throw();

                int num() const;

            private:
                int errno_;
            };
            DECLARE_PTR(nn_exception)

            class socket {
            public:
                socket(protocol p) : socket(AF_SP, p, MAX_MSG_LENGTH) {};

                socket(int domain, protocol p) : socket(domain, p, MAX_MSG_LENGTH) {};

                socket(int domain, protocol p, int buffer_size);

                ~socket();

                void setsockopt(int level, int option, const void *optval, size_t optvallen);

                void setsockopt_str(int level, int option, std::string value);

                void setsockopt_int(int level, int option, int value);

                void getsockopt(int level, int option, void *optval, size_t *optvallen);

                int getsockopt_int(int level, int option);

                int bind(const std::string &url);

                int connect(const std::string &url);

                void shutdown(int how = 0);

                void close();

                int send(const std::string &msg, int flags = 0) const;

                int recv(int flags = 0);

                const std::string &recv_msg(int flags = 0);

                int send_json(const nlohmann::json &msg, int flags = 0) const;

                nlohmann::json recv_json(int flags = 0);

                const std::string &request(const std::string &json_message);

                const protocol get_protocol() const {return protocol_;};
                const std::string &get_url() const {return url_;};
                const std::string &last_message() const {return message_;};

            private:
                int sock_;
                protocol protocol_;
                std::string url_;
                std::vector<char> buf_;
                std::string message_;

                /*  Prevent making copies of the socket by accident. */
                socket(const socket &);

                void operator=(const socket &);
            };
            DECLARE_PTR(socket)

            class nanomsg_json : public event {
            public:
                nanomsg_json(const std::string &msg): binding_(nlohmann::json::parse(msg)) {};

                inline int64_t gen_time() const override {return binding_["gen_time"];}

                inline int64_t trigger_time() const override {return binding_["trigger_time"];}

                inline int32_t msg_type() const override {return binding_["msg_type"];}

                inline uint32_t source() const override {return binding_["source"];}

            protected:
                const void* data_address() const override { return &binding_["data"];}

            private:
                const nlohmann::json binding_;
            };
            DECLARE_PTR(nanomsg_json)
        }
    }
}

#endif //KUNGFU_NANOMSG_SOCKET_H
