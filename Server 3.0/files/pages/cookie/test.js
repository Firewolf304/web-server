var defaultname = "who are you?";
function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) {
        return parts.pop().split(';').shift();
    }
}
function SetCookie(value) {
    document.cookie = "name" + "=" + document.getElementById("input").value + ";";
    document.getElementById("name").innerHTML = "Hello, " + getCookie("name");
}
function ClearCookie() {
    document.cookie = "name=";
    document.getElementById("name").innerHTML = "Hello, " + defaultname;
}
function CheckCookie() :  {
    //var cookies = document.cookie;
    console.log("cookie: " + document.cookie);
    try {
        return getCookie("name").length > 0;
    } catch (error) {
        return false;
    }
}
if(CheckCookie()) {
    document.getElementById("name").innerHTML = "Hello, " + getCookie("name");
}
else {
    document.getElementById("name").innerHTML = "Hello, " + defaultname;
}
