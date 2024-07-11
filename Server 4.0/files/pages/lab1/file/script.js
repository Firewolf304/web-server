function add_to_file() {
    var data = { 'name': document.getElementById("name").value, 'comment': document.getElementById("comment").value };
    var xhr = fetch("/apis/input",
        {
            method: 'POST',
            headers: {
            	'Content-Type': 'text/plain'
            },
	    body: JSON.stringify(data)
        }).then(response =>
    {
        if(response.ok) {
            document.getElementById("status").innerHTML = "ok";
            read_file();
        } else {
            document.getElementById("status").innerText = "error";
        }
    }) .catch(error => {
        console.error('error:', error);
    });
}
function read_file() {
    var xhr = fetch("filetxt",
        {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            }
        }).then( response =>
    {
        if(response.ok) {
            return response.text();
        } else {
            throw new Error(`Error! Status: ${response.status}`);
        }
    }).then( data => {
        document.getElementById("read_area").innerHTML = data;
    }).catch(error => {
        console.error('error:', error);
    });
}