__VERSION__="1"
__AUTHOR__="FIREWOLF"
__COMMENT__="this is a pytest module"

def info() -> str:
    return str('{"version": "' + __VERSION__ + '", "author": "' + __AUTHOR__ + '", "comment": "' + __COMMENT__ + '"}');

# Return values like string!
def start(request: dict) -> dict:
    rep = dict()
    rep["body_text"] = "Hello i`m a test function on Python\n\tGiven: " + str(request)
    rep["header_body"] = "test: allowed\r\nContent-Type: text/*; charset=utf-8\r\n"
    return rep