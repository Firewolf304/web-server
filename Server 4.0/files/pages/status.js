var url = 'https://nstu.ru/';

var xhr = fetch(url,
        {
            method: 'POST',
            headers: {
            },
            body: 'this is check site'
        }).then(response =>
    {
        if(response.ok) {
            document.getElementById("status").innerHTML = "online";
        } else {
            document.getElementById("status").innerHTML = "offline";
        }
    }) .catch(error => {
	    document.getElementById("status").innerHTML = "offline";
        console.error('error:', error);
    });
var massive = [
    "https://media.tenor.com/2EVga7MfSRsAAAAC/kirby-falling.gif",
    "https://c.tenor.com/eqx4UXyyELQAAAAd/tenor.gif",
    "https://c.tenor.com/_Z_UIdYyNlMAAAAC/tenor.gif",
    "https://c.tenor.com/_pSTChwI1mcAAAAC/tenor.gif",
    "https://c.tenor.com/P49xhh2DMcMAAAAd/tenor.gif",
    "https://c.tenor.com/TSJ8PZ22LR8AAAAC/tenor.gif",
    "https://c.tenor.com/z9GajXe9H9wAAAAC/tenor.gif",
    "https://c.tenor.com/VZe9cdNrmIoAAAAC/tenor.gif",
    "https://c.tenor.com/EWuNdhEYlVkAAAAd/tenor.gif",
    "https://c.tenor.com/RdTJtSL1KFYAAAAC/tenor.gif"
];


randomParam = Math.floor(Math.random() * 10000);
document.getElementById("background").setAttribute("src", massive[Math.floor(Math.random() * (massive.length-1))]);
