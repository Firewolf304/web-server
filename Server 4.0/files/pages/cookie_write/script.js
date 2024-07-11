function add_to_file() {
    var data = { 'name': String(document.getElementById("name").value), 'comment': String(document.getElementById("comment").value) };
    var xhr = fetch("input.php",
        {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: data
        }).then(response =>
    {
        if(response.ok) {
            document.getElementById("status").innerHTML = "ok"
        } else {
            document.getElementById("status").innerText = "error"
        }
    }) .catch(error => {
        console.error('error:', error);
    });
}