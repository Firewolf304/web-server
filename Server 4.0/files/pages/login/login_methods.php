<?php

    include_once('login.php');
    try {

        if(isset($_SERVER['REQUEST_METHOD'])) {

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
        }

    } catch (Exception $e) {

        echo 'api error: ',  $e->getMessage(), "\n";
    }
?>
