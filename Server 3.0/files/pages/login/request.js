var button = document.getElementById("login");
button.addEventListener("click", sendData);
function sendData() {
    // Получите значения или данные, которые вы хотите отправить
    var data = {
        name: "John",
        age: 25,
        // Другие поля данных...
    };

    // Преобразуйте данные в формат JSON
    var jsonData = JSON.stringify(data);

    // Отправьте данные на сервер с использованием AJAX-запроса
    // Здесь можно использовать различные методы, такие как fetch или XMLHttpRequest

    // Пример с использованием fetch:
    fetch('/apis/auth', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'ID': document.getElementById("id").innerHTML
        },
        body: jsonData
    })
        .then(function(response) {
            // Обработка ответа от сервера
            // response может содержать данные ответа или информацию о статусе запроса
        })
        .catch(function(error) {
            // Обработка ошибок
        });
}