//
// Created by firewolf304 on 18.03.24.
//

#ifndef SERVER4_0_INCLUDE_HPP
#define SERVER4_0_INCLUDE_HPP
#include <iostream>
#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <future>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <filesystem>
#include <zlib.h>
#include <fstream>
#include <dlfcn.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <functional>
//#include <gflags/gflags.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/url.hpp>
#include <boost/url/parse.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <pqxx/pqxx>


#include "module/custom_code.hpp"
#include "config.hpp"
#include "../src/methods.hpp"
//#include <glog/logging.h>
//#include <gflags/gflags.h>

#endif //SERVER4_0_INCLUDE_HPP
