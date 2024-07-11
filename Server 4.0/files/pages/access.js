function getCookie(name) {
    let matches = document.cookie.match(new RegExp(
        "(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"
    ));
    return matches ? decodeURIComponent(matches[1]) : undefined;
}

var id = getCookie("id");
if(id == undefined) {
    id = "noone";
}
var data = {
    id: id
};
var value;
var xhr = fetch("/apis/access",
        {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: JSON.stringify(data)
        }).then(response =>
    {
        if(response.ok) {
            return response.text();
        } else {
            document.getElementById("list").innerHTML = "error api";
        }
    }).then( data => {
        console.log("Role: " + data);
        value = JSON.parse(data);
        if(value["user_name"] != 'undefined') {
            document.getElementById("user_name").innerHTML = "You auth like " + value["user_name"];
        }
        document.getElementById("list").innerHTML = value["value"];
    }).catch(error => {
	    document.getElementById("list").innerHTML = "error api";
        console.error('error:', error);
    });

xhr.finally(()=>{
    var parent = document.getElementById('window').parentElement.getElementsByTagName('a')[0];
    parent.addEventListener('click', function(event) {
        var panel = window.open("", "User changer", 'width=1300, height=333, status=no, toolbar=no, resizable=yes, scrollbars=no, menubar=no');
        panel.document.write("<iframe src=\"/user_db/main.html\"></iframe>");
        panel.document.write("<style>iframe{width: 100%;height: 100%;border: none;} body, html {margin: 0;padding: 0;height: 100%;}</style>");
    });
});
