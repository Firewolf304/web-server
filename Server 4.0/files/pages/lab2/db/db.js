function get(to_do = "get_all") {
    var send = {'method': to_do};
    var xhr = fetch("db.php",
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
        document.getElementById("table").innerHTML += data;
    }).catch(error => {
        console.error('error:', error);
    });
}

function get_string (to_do = "get_all", lambda = (text)=>{}, give = {}) {
    var send = {'method': to_do};
    for (var key in give) {
        if (give.hasOwnProperty(key)) {
            send[key] = give[key];
        }
    }
    var xhr = fetch("db.php",
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
            throw new Error('Error response');
        }
    }).then( data => {
        lambda(data);
    }).catch(error => {
        console.error('error:', error);
    });

}

function hide(id, lambda = () => {}, clear = () => {}){
    var tag = document.getElementById(id);
    if(tag.style.display == 'none') {
        tag.style.display = 'block';
        lambda();
    } else {
        tag.style.display = 'none';
        clear();
    }

}
function dump() {
    get_string('dump', () => {});
    get();
}
get();
