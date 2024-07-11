
function getCookie(name) {
    let matches = document.cookie.match(new RegExp(
        "(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"
    ));
    return matches ? decodeURIComponent(matches[1]) : undefined;
}
function deleteCookie(name) {
    document.cookie = name + '=; Max-Age=0';
}
function setCookie_old(name, value, options = {}) {
    options = {
        path: '/',
        ...options
    };
    if (options.expires instanceof Date) {
        options.expires = options.expires.toUTCString();
    }
    let updatedCookie = encodeURIComponent(name) + "=" + encodeURIComponent(value);
    for (let optionKey in options) {
        updatedCookie += "; " + optionKey;
        let optionValue = options[optionKey];
        if (optionValue !== true) {
            updatedCookie += "=" + optionValue;
        }
    }
    document.cookie = updatedCookie;
}



/*var button = document.getElementById("login_event");
button.addEventListener("submit", function (event) {
    event.preventDefault();
    var data = {
        method: "create_cookie",
        login: document.getElementById("login").value,
        password: document.getElementById("password").value
    };
    var xhr = fetch("/login/login_methods",
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
        }
    }).then( data => {
        console.log("Response:". data);
        var js = JSON.parse(data);
        if(js["ERROR"] === undefined && data !== undefined) {
            const expires = new Date();
            expires.setTime(expires.getTime() + (7 * 24 * 60 * 60 * 1000));
            document.cookie = `level=${js["level"]}; expires=${expires.toUTCString()}; path=/ `;
            document.cookie = `user_id=${js["user"]}; expires=${expires.toUTCString()}; path=/ `;
            document.cookie = `id=${js["id_session"]}; expires=${expires.toUTCString()}; path=/ `;
            document.location.href = "/";
        } else {
            document.getElementById("status").innerHTML = js["ERROR"];
        }

    }).catch(error => {
        document.getElementById("status").innerHTML = "Incorrect input";
        console.error('error:', error);
    });
} );*/

$(document).ready(function(){
    var options = {
        type: "post",
        success: function() {
            alert("hello");
        },
        timeout: 3000 // тайм-аут
    };
    $.validator.messages.required = '';
    $("#login_event").validate({
        focusInvalid: false,
        focusCleanup: true,
        rules: {
            login : {
                required: true,
                minLength: 4,
                maxLength: 64
            },
            password : {
                required: true,
                minLength: 4,
                maxLength: 64
            }
        },
        messages: {
            login: {
                required: "Login must be required!"
            },
            password: {
                required: "Password must be required!"
            }
        },
        onkeyup: false,
        errorPlacement: function(error,element) {
            if(error !== undefined) {
                console.log("Error input: " + error);
                //element.parent().find('em').text(error[0].innerText);
                error.appendTo(element.parent().find('em'));
            } else {
                element.parent().find('em').text("");
            }
        },
        submitHandler: function (form) {
            var data = {
                method: "create_cookie",
                login: document.getElementById("login").value,
                password: document.getElementById("password").value
            };
            var xhr = fetch("/login/login_methods",
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
                }
            }).then( data => {
                console.log("Response:". data);
                var js = JSON.parse(data);
                if(js["ERROR"] === undefined && data !== undefined) {
                    const expires = new Date();
                    expires.setTime(expires.getTime() + (7 * 24 * 60 * 60 * 1000));
                    document.cookie = `level=${js["level"]}; expires=${expires.toUTCString()}; path=/ `;
                    document.cookie = `user_id=${js["user"]}; expires=${expires.toUTCString()}; path=/ `;
                    document.cookie = `id=${js["id_session"]}; expires=${expires.toUTCString()}; path=/ `;
                    document.location.href = "/";
                } else {
                    document.getElementById("status").innerHTML = js["ERROR"];
                }

            }).catch(error => {
                document.getElementById("status").innerHTML = "Incorrect input";
                console.error('error:', error);
            });
        }
    });

});