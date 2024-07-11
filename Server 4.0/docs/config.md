# Config docs
The config is divided into sections (server, proxy, etc.) each of which also has subsections.
It has the consistency of a “constructor”, by type, each element can be either present or absent.
Configuration data is located in global variables and contacted with all objects for flexibility.

The config file located in path value.

Some pages:

- [Example](#example-config)
- [Server](#server)
- - [Address](#address-)
- - [Port](#port)
- - [Machine info](#machine-info)
- - [Enable modules](#enable-modules)
- [SQL](#sql)
- - [PSQL](#psql)
- - - [Config](#psql-config)

## Example config:

    {
        "SERVER" : {
            "ADDRESS" : "127.0.0.1",
            "PORT" : "8080"
        }
    }

## Server

### Address 
Default string address value, example:
```json
{
  "ADDRESS": "127.0.0.1"
}
```
Default parameter: ```"127.0.0.1"```

### Port
Default int port value, example:
```json
{
  "PORT" : 8080
}
```
Default parameter: ```8080```

### Machine info
This is the bool parameter get server access to send information about machine in response headers (for fun)
```json
{
  "MACHINE_INFO" : true
}
```
Example response content this parameter:
```http request
HTTP/1.1 200 OK
Architecture: arm64
System: Linux
Version: #1 FEDOR PROSTOKVASHINO4
Release: 6.7.7-1-SYSTEM
Connection: close
Content-Length: 5
```
Default parameter: ```false```

### Enable modules
This is the bool parameter for start init dynamic code apis
```json
{
  "ENABLE_MODULES" : true
}
```
Default parameter: ```true```

Attention: you are not allowed to add any API structures to routes through code! You are trying to add code to something that has no memory allocated for it, you can get around this by typing lightly: 
```c++
firewolf::web_server::content::api::firewolf_main_data.app = std::make_shared<firewolf::web_server::content::api::application>();
```
Then the functions will be entered into memory, but will not be used.
### Pages folder
The string parameter for select pages folder
```json
{
  "PAGES_FOLDER" : "/pages"
}
```
Default parameter: ```"/pages"```

## SQL

Now server support: [psql](#psql)

### PSQL

Psql-client is pqxx library

#### Psql config
A regular json config, similar to the ```pqxx::connection``` [setting](https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-CONNSTRING), the only difference is that it is filled in via json. But you cant use ssh session (it will be added in the future). Enable controls switching on.
```json
{
  "ENABLE": true,
  "CONFIG" : {
    "user" : "postgres",
    "password" : "1234",
    "host" : "127.0.0.1",
    "port" : "5432",
    "dbname" : "test",
    "target_session_attrs" : "read-write"
  }
}
```
Default parameter: ```null```

Attention: if you try user unallocated method - it will throw sigsegv.