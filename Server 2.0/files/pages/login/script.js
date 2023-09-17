const xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
        console.log(this.responseText);
        var data = JSON.parse(this.responseText);
        var items = data.items;
        var summator = items.slice(0, 10).reduce(function (a, b) {
            return a + b;
        }, 0);
        document.getElementById("id").innerHTML += `${summator.toString()}`;
    }
};
xhttp.open("GET", "/apis/rand", true);
xhttp.send();
