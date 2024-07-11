<?php
include "/home/user111/public_html/login/login.php";
$access = 10;
$updateaccess = 1000;
$deleteaccess = 1000;
$user_access = check_level_user($_COOKIE['id']);                                                                                # user rank

function get_users() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    $result = mysqli_query($SQL, "SELECT id, role, level, login, password FROM users ORDER BY role DESC, level DESC;");         # request to server
    $numRows = mysqli_num_rows($result);                                                                                        # get lenght of rows
    while ($row=mysqli_fetch_assoc($result)) {                                                                                  # iterate result
    //for ($i = 0; $i < $numRows; $i++) {
        //$row = mysqli_fetch_assoc($result);
        $jsonFileContent_acess = file_get_contents('/home/user111/meta_data/access.json');                                      # get access
        if ($jsonFileContent_acess === false) {
            echo 'Error, no access file';
            return;
        }
        $jsonData = json_decode($jsonFileContent_acess, true);
        echo "<tr><form class='line'>";
        /*foreach($row as $value) {                                                                                             # iterate line columns
            echo "<td>".$value."</td>";                                                                                         # value of column
        }*/
        echo "<td><a id='user_id'>".$row["id"]."</a></td>";
        echo "<td><select id='role'>";
        foreach ($jsonData as $role => $userInfo) {
            echo '<option value="'.$role.'"';
            if($role == $row["role"]) {
                echo 'selected';
            }
            echo '>'.$role.'</option>';
        }
        echo "</select></td>";
        echo '<td><input type="number" id="level" value="'.$row["level"].'"></td>';
        echo "<td><input type='text' id='login' value='".$row["login"]."'></td>";
        echo "<td><input type='text' id='password' value='".$row["password"]."'></td>";
        if( $user_access >= $updateaccess ) {
            echo "<td> <button onclick='update(this)'>update</button> </td>";
        }
        if( $user_access >= $deleteaccess ) {
            //echo "<td><a href='"."user_db.php?method=delete_user&user_id=".$_COOKIE['id']."'>update</a></td>";
            /*echo "<td> <a onclick='(function() {
            })'>update</a> </td>";*/
            echo '<td> <button onclick="delete_line(\'' .$row["id"]. '\')">delete</button> </td>';
            //echo "<td> <button onclick=\"delete_line(\"".$_COOKIE['user_id']."\")\">delete</button> </td>";
        }
        echo "</tr></form>";
    }

    if( $user_access >= $deleteaccess ) {
        echo "<tr><form class='line'>";
        echo "<td></td>";
        echo "<td><select id='role'>";
        foreach ($jsonData as $role => $userInfo) {
            echo '<option value="'.$role.'">'.$role.'</option>';
        }
        echo "</select></td>";
        echo '<td><input type="number" id="level" value="1"></td>';
        echo "<td><input type='text' id='login' value='login'></td>";
        echo "<td><input type='text' id='password' value='password'></td>";
        echo "<td> <button onclick='add(this)'>add</button> </td>";
        echo "</tr></form>";
    }

}
function update_user() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    if($user_access >= $deleteaccess) {
        try {
            $result = mysqli_query($SQL, "update users set login='". $post["login"] ."', password='". $post["password"] ."', level=". $post["level"] .", role='" . $post["role"] . "' where id='". $post["user_id"] ."';");
            //$result = mysqli_query($SQL, "update users set password='".."', login='".."', role='".."', level=".." where id='".."';");
            echo $post["user_id"]." ".$post["password"];
            echo "Updated";
        } catch(Exception $e) {
            echo "Error: ".$e;
        }

    }
}
function delete_user() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    if($user_access >= $deleteaccess) {
        try {
            $result = mysqli_query($SQL, "delete from users where id = '".$post["user_id"]."';");
            //echo $post["user_id"];
            echo "OK";
        } catch(Exception $e) {
            echo "Error: ".$e;
        }
    }
}
function add_user() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    if($user_access >= $deleteaccess) {
        try {
            //insert into users values (UUID(), 'user1', 'user1', 2, 'private');
            $result = mysqli_query($SQL, "insert into users values (UUID(), '". $post["login"] ."', '". $post["password"] ."', ". $post["level"] .", '" . $post["role"] . "');");
            //echo $post["user_id"];
            echo "OK ";
        } catch(Exception $e) {
            echo "Error: ".$e;
        }
    }
}
function get_sessions() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    $result = mysqli_query($SQL, "select session.id_session, session.insert_date, users.login, session.data from session inner join users on users.id = session.user_id;");
    while ($row=mysqli_fetch_assoc($result)) {                                                                                  # iterate result

        echo "<tr><form class='line'>";
        /*foreach($row as $value) {                                                                                             # iterate line columns
            echo "<td>".$value."</td>";                                                                                         # value of column
        }*/
        echo "<td><a id='id_session'>".$row["id_session"]."</a></td>";
        echo "<td><a>".$row["insert_date"]."</a></td>";
        echo "<td><a>".$row["login"]."</a></td>";
        echo "<td><a>".$row["data"]."</a></td>";
        if( $user_access >= $deleteaccess ) {
            echo "<td> <button onclick='delete_sess(this)'>delete</button> </td>";
        }
        echo "</tr></form>";
    }
}
function delete_session() {
    global $jsonFileContent_acess, $SQL, $post, $updateaccess, $access, $deleteaccess, $user_access;
    if($user_access >= $deleteaccess) {
        try {
            $result = mysqli_query($SQL, "delete from session where id_session = '".$post["id_session"]."';");
            //echo $post["user_id"];
            echo "OK";
        } catch(Exception $e) {
            echo "Error: ".$e;
        }
    }
}

if($_SERVER['REQUEST_METHOD'] === 'POST'){
    if($user_access >= $access) {
        if($post["method"] == "get_users" || $_POST['method'] == "get_users") {
            get_users();
        } elseif($post["method"] == "delete_user" || $_POST['method'] == "delete_user") {
            delete_user();
        } elseif($post["method"] == "update_user" || $_POST['method'] == "update_user") {
            update_user();
        } elseif($post["method"] == "add_user" || $_POST['method'] == "add_user") {
            add_user();
        } elseif($post["method"] == "get_sessions" || $_POST['method'] == "get_sessions") {
            get_sessions();
        } elseif($post["method"] == "delete_sessions" || $_POST['method'] == "delete_sessions") {
            delete_session();
        }
    }
} elseif($_SERVER['REQUEST_METHOD'] === 'GET') {
    if(check_level_user($_COOKIE['id']) >= $access) {

    }
    /*if($_GET['method'] == "delete_cookie") {
        delete_cookie();
    }*/
}
?>
