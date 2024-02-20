#include "library.h"


//using namespace::nlohmann;
extern "C" const char*  info() {
    string version = __VERSION__;
    string author = __AUTHOR__;
    string comment = __COMMENT__;
    return static_cast<const char*> ( ((string)("{ {'version'," + version + " }, {'author'," +  author +"}, {'comment', " + comment + "} }")).c_str() );
}

extern "C" void start(string * s, firewolf::responser::response* rep, firewolf::requester::request_data req, void * f) {
    rep->header_body += "test: allowed\r\nContent-Type: text/html; charset=utf-8\r\nContent-Disposition: inline\r\n";
    main_funcs * funcs = static_cast<main_funcs *>(f);
    auto sslInfo = [funcs]() {
        json d = funcs->ssl_info->Read_SSL(funcs->ssl_info->operator()());
        string out = d.dump(4).c_str();
        return out;
    };
    struct utsname unameData;
    uname(&unameData);
    *s = "<!doctype html><html> <style>body {background-color: #3e3e3e;color: #fff;}pre {white-space: pre-wrap;}"
         "</style><pre>\n";
    *s += "# -> Hostname: " + (string)unameData.nodename + "@" + (string) (unameData.sysname) + " " + (string) (unameData.release) + " " + (string) (unameData.machine) + "\n";
    *s += "# -> Version: " + (string)(__VERSION__) + "\n";
    *s += "# -> Author: " + (string)(__AUTHOR__) + "\n";
    *s += "# -> Comment: " + (string)(__COMMENT__) + "\n";
    *s += "---------------------------------------------------------------------------------------\n"
          "███████╗██╗██████╗ ███████╗██╗    ██╗ ██████╗ ██╗     ███████╗██████╗  ██████╗ ██╗  ██╗\n"
          "██╔════╝██║██╔══██╗██╔════╝██║    ██║██╔═══██╗██║     ██╔════╝╚════██╗██╔═████╗██║  ██║\n"
          "█████╗  ██║██████╔╝█████╗  ██║ █╗ ██║██║   ██║██║     █████╗   █████╔╝██║██╔██║███████║\n"
          "██╔══╝  ██║██╔══██╗██╔══╝  ██║███╗██║██║   ██║██║     ██╔══╝   ╚═══██╗████╔╝██║╚════██║\n"
          "██║     ██║██║  ██║███████╗╚███╔███╔╝╚██████╔╝███████╗██║     ██████╔╝╚██████╔╝     ██║\n"
          "╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚══════╝╚═╝     ╚═════╝  ╚═════╝      ╚═╝\n"
          "---------------------------------------------------------------------------------------\n"
          "hello, i`m test module for main functions. For now this is config output\n";
    *s += "Info funcs:\n";
    *s +=  "\ttimeoutclient=" + to_string(*funcs->timeoutclient) + "\n";
    *s += "\tapp_ptr: {\n";
    *s +=         "\t\troutes_len=" + to_string(funcs->app_ptr->routes.size()) +"\n";
    *s +=         "\t\tmap_funcs_len=" + to_string(funcs->app_ptr->map_funcs.size()) +"\n\t}\n";
    *s += "\tip=" + *(funcs->ip) +"\n";
    *s += "\tport=" + to_string( *(funcs->portptr)) +"\n";
    *s += std::string("\tssl_info_status=") + (*funcs->ssl_info->secure_status ? "true" : "false") +"\n";
    *s += "\tapis_path=" + (funcs->app_ptr->path_dirs)  + "\n";
    *s += "\tserver_path=" + (funcs->app_ptr->app_path)  + "\n";
    *s += "\tpages=" + *funcs->pages + "\n";
    *s += "\tmachine_info=" + to_string(funcs->machine_info) + "\n";
    *s += "\tproxy_path: {\n";
    *s +=   [funcs]() ->string {
                string text = "";
                for(auto d : funcs->proxy_path) {
                    text += "\t\t" + d["PATH"].get<string>() + (string)" to " + d["ADRESS"].get<string>() + "\n";
                }
                return text;
            }() +"\t}\n";
    *s += "\tlog_config: {\n";
    *s +=   "\t\tthread_id=" + funcs->log->get_id_thread() + "\n";
    *s += "\t\tformat=" + funcs->log->config->format_type  + "\n";
    *s += std::string("\t\tshow_id_thread=") + (funcs->log->config->show_id_thread ? "true\n" : "false\n");
    *s +=   std::string("\t\tshow_time=") + (funcs->log->config->show_time ? "true\n" : "false\n");
    *s +=   "\t\twait=" + to_string(funcs->log->config->wait) + "\n\t}\n";
    *s += "\tssl_info=" + ((*funcs->ssl_info->secure_status) ? (string)(funcs->ssl_info->Read_SSL((*funcs->ssl_info)()).dump()) : "null") + "\n";
    *s += "</pre></html>";
}
