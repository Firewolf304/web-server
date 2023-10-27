#include "library.h"
#include "scripts/variables.h"

//using namespace::nlohmann;
extern "C" const char*  info() {
    string version = __VERSION__;
    string author = __AUTHOR__;
    string comment = __COMMENT__;
    return static_cast<const char*> ( ((string)("{ {'version'," + version + " }, {'author'," +  author +"}, {'comment', " + comment + "} }")).c_str() );
}

extern "C" void start(string * s, responser::response* rep, requester::request_data req, void * f) {
    rep->header_body += "test: allowed\r\nContent-Type: text/*; charset=utf-8\r\n";
    main_funcs * funcs = (main_funcs*)f;
    auto sslInfo = [funcs]() {
        json d = funcs->ssl_info->Read_SSL(funcs->ssl_info->operator()());
        string out = d.dump(4).c_str();
        return out;
    };
    struct utsname unameData;
    uname(&unameData);
    *s = "# -> Hostname: " + (string)unameData.nodename + "@" + (string) (unameData.sysname) + " " + (string) (unameData.release) + " " + (string) (unameData.machine) + "\n";
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
    *s += "Info funcs:\n"
          "\ttimeoutclient=" + to_string(*funcs->timeoutclient) + "\n" +
          "\tapp_ptr: {\n" +
            "\t\troutes_len=" + to_string(funcs->app_ptr->routes.size()) +"\n" +
            "\t\tmap_funcs_len=" + to_string(funcs->app_ptr->map_funcs.size()) +"\n\t}\n" +
          "\tip=" + *(funcs->ip) +"\n" +
          "\tport=" + to_string( *(funcs->portptr)) +"\n" +
          "\tssl_info_status=" + (*funcs->ssl_info->secure_status ? "true" : "false") +"\n" +
          "\tactive_epols_len=" + to_string(*funcs->active_epols) + "\n" +
          "\tapis_path=" + (funcs->app_ptr->path_dirs)  + "\n" +
          "\tserver_path=" + (funcs->app_ptr->app_path)  + "\n" +
          "\tpages=" + *funcs->pages + "\n" +
          "\tmachine_info=" + to_string(funcs->machine_info) + "\n" +
          "\tproxy_path: {\n" +
            [funcs]() ->string {
                string text = "";
                for(auto d : funcs->proxy_path) {
                    text += "\t\t" + d["PATH"].get<string>() + (string)" to " + d["ADRESS"].get<string>() + "\n";
                }
                return text;
            }() +"\t}\n" +
          "\tlog_config: {\n" +
            "\t\tthread_id=" + funcs->log->get_id_thread() + "\n" +
            "\t\tformat=" + funcs->log->config->format_type  + "\n" +
            "\t\tshow_id_thread=" + (funcs->log->config->show_id_thread ? "true\n" : "false\n") +
            "\t\tshow_time=" + (funcs->log->config->show_time ? "true\n" : "false\n") +
            "\t\twait=" + to_string(funcs->log->config->wait) + "\n\t}\n"
          "\tssl_info=" + ((*funcs->ssl_info->secure_status) ? (string)(funcs->ssl_info->Read_SSL((*funcs->ssl_info)()).dump()) : "null") + "\n";
}
