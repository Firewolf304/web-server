function get_string (to_do = "get_all", lambda = (text)=>{}, give = {}) {
    var send = {'method': to_do};
    for (var key in give) {
        if (give.hasOwnProperty(key)) {
            send[key] = give[key];
        }
    }
    var xhr = fetch("user_db",
        {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: JSON.stringify(send)
        }).then(response =>
    {
        if(response.ok) {
            return response.text();
        } else {
            //document.getElementById("status").innerText = "error";
        }
    }).then( data => {
        lambda(data);
    }).catch(error => {
        console.error('error:', error);
    });

}
function get(to_do = "get_users", table="user_table", innerHTML = "<tr><td>User id</td><td>Role</td><td>Level</td><td>Login</td><td>Password</td><td>update</td><td>delete</td></tr>") {
    var send = {'method': to_do};
    var xhr = fetch("user_db",
        {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: JSON.stringify(send)
        }).then(response =>
    {
        if(response.ok) {
            return response.text();
            //document.getElementById("status").innerHTML = "ok";
        } else {
            //document.getElementById("status").innerText = "error";
        }
    }).then( data => {
        document.getElementById(table).innerHTML = innerHTML + data;
    }).catch(error => {
        console.error(`error (${table}):`, error);
    });
}
function delete_line(user_id) {
    get_string("delete_user", ()=>{
        console.log("Deleted",user_id);
        get("get_users","user_table");
        get("get_sessions","session_table", "<tr><td>Session id</td><td>Insert date</td><td>Login</td><td>Data</td></tr>");
    }, {"user_id" : user_id});
}
function update(button) {
    var row = button.closest('tr');
    var userId = row.querySelector('#user_id').innerHTML;
    var role = row.querySelector('#role').value;
    var level = row.querySelector('#level').value;
    var login = row.querySelector('#login').value;
    var password = row.querySelector('#password').value;
    console.log("----------------------------------");
    console.log('User ID:', userId);
    console.log('Role:', role);
    console.log('Level:', level);
    console.log('Login:', login);
    console.log('Password:', password);
    get_string("update_user", ()=>{
        console.log("Updated", userId);
        get("get_users","user_table");
        get("get_sessions","session_table", "<tr><td>Session id</td><td>Insert date</td><td>Login</td><td>Data</td></tr>");
    },
    {
        "user_id" : userId,
        "role": role,
        "level": level,
        "login": login,
        "password": password
    });
}
function add(button) {
    var row = button.closest('tr');
    var role = row.querySelector('#role').value;
    var level = row.querySelector('#level').value;
    var login = row.querySelector('#login').value;
    var password = row.querySelector('#password').value;
    console.log("----------------------------------");
    console.log('Role:', role);
    console.log('Level:', level);
    console.log('Login:', login);
    console.log('Password:', password);
    get_string("add_user", ()=>{
        console.log("Added");
        get("get_users","user_table");
    },
    {
        "role": role,
        "level": level,
        "login": login,
        "password": password
    });
}
function delete_sess(button) {
    var row = button.closest('tr');
    var id_session = row.querySelector('#id_session').innerHTML;
    get_string("delete_sessions", ()=>{
        console.log("Session deleted");
        get("get_users","user_table");
        get("get_sessions","session_table", "<tr><td>Session id</td><td>Insert date</td><td>Login</td><td>Data</td></tr>");
    },
    {
        "id_session" : id_session
    });
}
get("get_users","user_table");
get("get_sessions","session_table", "<tr><td>Session id</td><td>Insert date</td><td>Login</td><td>Data</td></tr>");
