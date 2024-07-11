<?php
    $data = file_get_contents("php://input");
    $post = json_decode($data, true);
    $SQL = mysqli_connect('localhost', 'user111','Dancen228','user111');
    if (!$SQL) { echo "Can't connect to database!"; exit;}
    function check_cookie() {
        global $SQL, $post;
        $result = mysqli_query($SQL, "SELECT * FROM session where id_session = '".$post["id"]."';");
        if(mysqli_num_rows($result) == 1) {
            http_response_code(202);
            echo "OK";
        } else {
            http_response_code(401);
            echo "ERROR";
        }
    }

    function create_cookie() {
        global $SQL, $post;
        $result = mysqli_query($SQL, "select id, level from users where login = '".$post["login"]."' and password = '".$post["password"]."';");
        echo "";
        if(mysqli_num_rows($result) === 1) {
            $row = $result->fetch_assoc();
            $user = $row["id"];
            $level = $row["level"];
            try {
                $result = mysqli_query($SQL, "insert into session values(UUID(), now(), '".$user."', '{}');");
            } catch (Exception $e) {
                $result = mysqli_query($SQL, "delete from session where user_id = '".$user."';");
                $result = mysqli_query($SQL, "insert into session values(UUID(), now(), '".$user."', '{}');");
            }
            $result = mysqli_query($SQL, "select id_session from session where user_id = '".$user."';");
            $row = $result->fetch_assoc();
            echo "{\"id_session\" : \"".$row["id_session"]."\", \"user\" : \"".$user."\", \"level\" : \"".$level."\"}";
            #$result = mysqli_query($SQL, "delete from session;");
        } else {
            echo "{\"ERROR\" : \"Incorrect password or login\"}";
            #http_response_code(401);
        }
    }
    function check_level_user($id_session) {
        global $SQL, $post;
        #SELECT session.id_session, session.insert_date, session.user_id, session.data, users.login, users.password, users.level FROM session inner join users on users.id = session.user_id;
        $result = mysqli_query($SQL, "SELECT session.id_session, session.insert_date, session.user_id, session.data, users.login, users.password, users.level FROM session inner join users on users.id = session.user_id where id_session = '".$id_session."';");
        if(mysqli_num_rows($result) == 1) {
            $row = $result->fetch_assoc();
            return $row["level"];
        } else {
            return 0;
        }
    }
    function delete_cookie () {
        global $SQL, $post;
        try {
            echo $_GET['method']." ".$_COOKIE['id']." ".$_COOKIE['user_id']."\n";
            if($post["user_id"] !== null) {
                echo "selected post";
                $result = mysqli_query($SQL, "delete from session where user_id = '".$post["user_id"]."' and id_session = '".$post["is_session"]."' ;");
            } elseif($_POST["user_id"] !== null) {
                echo "selected _post";
                $result = mysqli_query($SQL, "delete from session where user_id = '".$_POST["user_id"]."' and id_session = '".$_POST["is_session"]."' ;");
            } else {
                echo "selected cookie";
                //$result = mysqli_query($SQL, "delete from session where user_id = '".$_COOKIE['user_id']."' and id_session = '".$_COOKIE['id']."';");
                $result = mysqli_query($SQL, "DELETE FROM session where id_session = '".$_COOKIE['id']."' and user_id = '".$_COOKIE['user_id']."';");
                if(!$result) {
                    echo "oops";
                }
            }
            echo "Deleted";
            header("Location: /~user111/");
        } catch (Exception $e) {
            echo "NO USER";
        }
    }
    if($_SERVER['REQUEST_METHOD'] === 'POST'){
        if($post["method"] == "check_cookie" || $_POST['method'] == "check_cookie") {
            check_cookie();
        } elseif( $post["method"] == "create_cookie" || $_POST['method'] == "create_cookie" ) {
            create_cookie();
        } elseif( $post["method"] == "delete_cookie" || $_POST['method'] == "delete_cookie" ) {
            delete_cookie();
        }
    } elseif($_SERVER['REQUEST_METHOD'] === 'GET') {
        if($_GET['method'] == "delete_cookie") {
            delete_cookie();
        }
    }
?>
