
const jsonOutput = document.getElementById('jsonOutput');
function createList(obj) {
    const list = document.createElement('ul');
    list.className = 'json-list';

    for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
            //const listItem = createListItem(key, obj[key]);
            //
            const listItem = document.createElement('li');
            listItem.className = 'json-object';

            if (typeof obj[key] === 'object') {

                const nestedList = createList(obj[key]);
                nestedList.classList.add('hidden');
                listItem.appendChild(document.createTextNode(key + ":"));
                listItem.appendChild(nestedList);
                listItem.addEventListener('click', function (event) {
                    event.stopPropagation();
                    toggleVisibility(nestedList);
                });

            } else {
                if(!Array.isArray(obj)) {
                    listItem.innerHTML = key + " = " + obj[key];
                }
                else {
                    listItem.innerHTML = obj[key];
                }
            }
            //
            list.appendChild(listItem);
        }
    }

    return list;
}

function toggleVisibility(element) {
    element.classList.toggle('hidden');
}
const xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
        console.log(this.responseText);
        var data = JSON.parse(this.responseText);
        const initialList = createList(data);
        jsonOutput.appendChild(initialList);
    }
};
xhttp.open("GET", "/apis/tools", true);
xhttp.send();

