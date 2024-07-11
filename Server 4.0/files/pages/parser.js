
function createList(obj) {

    const list = document.createElement('ul');
    list.className = 'json-list';
    for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
            const listItem = document.createElement('li');
            listItem.className = 'json-object';

            if (typeof obj[key] === 'object') {
                const nestedList = createList(obj[key]);
                nestedList.classList.add('hidden');

                const toggleLink = document.createElement('a');
                toggleLink.setAttribute('href', '#');
                toggleLink.appendChild(document.createTextNode(key));

                listItem.appendChild(toggleLink);
                listItem.appendChild(nestedList);

                toggleLink.addEventListener('click', function(event) {
                    event.stopPropagation();
                    toggleVisibility(nestedList);
                });
            } else {
                const contentLink = document.createElement('a');

                if (!Array.isArray(obj)) {
                    contentLink.innerHTML = key + " = " + obj[key];
                } else {
                    contentLink.innerHTML = obj[key];
                    contentLink.setAttribute("href", obj[key]);
                }

                listItem.appendChild(contentLink);
            }

            list.appendChild(listItem);
        }
    }

    return list;
}

function toggleVisibility(element) {
    element.classList.toggle('hidden');
}

const xhttp_api = new XMLHttpRequest();
xhttp_api.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
        const jsonOutput = document.getElementById('api');
        console.log(this.responseText);
        var data = JSON.parse(this.responseText);
        const initialList = createList(data);
        jsonOutput.appendChild(initialList);
    }
    else {
        console.log(this.responseText);
    }
};
let randomParam = Math.floor(Math.random() * 10000);
xhttp_api.open("GET", "/apis/tools?random=" + randomParam, true);
xhttp_api.send();
const xhttp_pages = new XMLHttpRequest();
xhttp_pages.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
        const jsonOutput = document.getElementById('pages');
        console.log(this.responseText);
        var data = JSON.parse(this.responseText);
        const initialList = createList(data);
        jsonOutput.appendChild(initialList);
    }
    else {
        console.log(this.responseText);
    }
};

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
xhttp_pages.open("GET", "/apis/getpages?random=" + randomParam, true);
xhttp_pages.send();

document.getElementById("background").setAttribute("src", massive[Math.floor(Math.random() * (massive.length-1))]);

