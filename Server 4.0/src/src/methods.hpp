//
// Created by firewolf304 on 30.03.24.
//

#ifndef SERVER4_0_METHODS_HPP
#define SERVER4_0_METHODS_HPP
#include "../include/include.hpp"
#define _XPLATSTR(x) x
namespace firewolf::methods {
    std::string open_file( std::ifstream & file ) {
        std::string line, text;
        if (file.is_open()) {
            while (getline(file, line)) {
                text += line + "\n";
            }
            file.close();
        }
        return text;
    }
    void create_file(std::string const & path, std::string const & value) {
        std::ofstream file(path, std::ios::trunc);
        file << value;
        file.close();
    }
    std::map<std::string, std::string> split_query(const std::string &query)
    {
        std::map<std::string, std::string> results;

        // Split into key value pairs separated by '&'.
        size_t prev_amp_index = 0;
        while(prev_amp_index != std::string::npos)
        {
            size_t amp_index = query.find_first_of(_XPLATSTR('&'), prev_amp_index);
            if (amp_index == std::string::npos)
                amp_index = query.find_first_of(_XPLATSTR(';'), prev_amp_index);

            std::string key_value_pair = query.substr(
                    prev_amp_index,
                    amp_index == std::string::npos ? query.size() - prev_amp_index : amp_index - prev_amp_index);
            prev_amp_index = amp_index == std::string::npos ? std::string::npos : amp_index + 1;

            size_t equals_index = key_value_pair.find_first_of(_XPLATSTR('='));
            if(equals_index == std::string::npos)
            {
                continue;
            }
            else if (equals_index == 0)
            {
                std::string value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
                results[_XPLATSTR("")] = value;
            }
            else
            {
                std::string key(key_value_pair.begin(), key_value_pair.begin() + equals_index);
                std::string value(key_value_pair.begin() + equals_index + 1, key_value_pair.end());
                results[key] = value;
            }
        }

        return results;
    }
    inline std::vector<std::string> save_split (const std::string &s, char simp) {
        std::vector<std::string> result;
        std::stringstream ss (s);
        std::string item;
        while (getline (ss, item, simp)) {
            result.push_back (item);
        }
        return result;
    }
    inline nlohmann::json cookie_to_json(std::string cookie) // shitty method until the cookies come out experimental
    {
        cookie.erase(std::remove_if(cookie.begin(), cookie.end(), [](char c) { return c == ';'; }), cookie.end()); //cookie.replace(cookie.begin(), cookie.end(), ";", "");
        nlohmann::json convert;
        std::vector<std::string> med = save_split(cookie, ' ');
        int i = 1;
        while(i < med.size())
        {
            std::vector<std::string> d = save_split(med[i], '=');
            convert[d[0]] = d[1];
            i++;
        }
        return convert;
    }

    std::ostringstream gzip_compression_boost (std::ifstream & stream) {
        if(stream.good()) {
            boost::iostreams::filtering_streambuf<boost::iostreams::input > inbuf;
            inbuf.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_speed)));
            inbuf.push(stream);
            std::ostringstream compressedBuffer;
            boost::iostreams::copy(inbuf, compressedBuffer);
            boost::iostreams::close(inbuf);
            return compressedBuffer;
        }
        throw std::runtime_error("Cant open file");
    }

    namespace client {
        boost::beast::http::response<boost::beast::http::string_body> http_client_response( std::string url, boost::beast::http::verb method = boost::beast::http::verb::get ) {
            boost::asio::io_context ioc;

        }
    }

}
#endif //SERVER4_0_METHODS_HPP
