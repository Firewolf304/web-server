<?php
if(isset($_POST)){
    $data = file_get_contents("php://input");
	$post = json_decode($data, true);
	#echo "Values Post:";
	#foreach ($post as $key => $value) {
    #		echo $key . ' = ' . $value . '<br>';
	#}
    $SQL = mysqli_connect('localhost', 'user111','Dancen228','user111');
    if (!$SQL) { echo "Can't connect to database!"; exit;}

    function get_all () {
        global $SQL;
        $result=mysqli_query($SQL, "SELECT  auto.name_auto, brand.name_brand, type_auto.name_type, price.price from auto inner join brand on auto.id_brand = brand.id_brand inner join type_auto on type_auto.id_type_auto = auto.id_type_auto inner join price on price.id_auto = auto.id_auto;");
        #insert into price (id_auto, price) values (0,9999);
        $nf=mysqli_field_count($SQL);
        #$fields=mysqli_fetch_fields($result);
        #    foreach ($fields as $value) {
        #        echo "<th>".$value->name."<br> ";
        #    }
        while ($row=mysqli_fetch_assoc($result))
        #$row=mysqli_fetch_assoc($result);
        {
            echo "<tr>";
            foreach($row as $value) {
                echo "<td>".$value."</td>";
            }
            echo "</tr>";
        }
    }
    function insert () {
        global $SQL, $post;
        $result=mysqli_query($SQL, "INSERT into price(id_auto, price) values(".$post["model_id"].", ".$post["price"].");" );
        echo "OK";
    }
    function get_models() {
        global $SQL;
        $result=mysqli_query($SQL, "SELECT * from auto;" );
        $nf=mysqli_field_count($SQL);
        while ($row=mysqli_fetch_assoc($result))
        {
            echo $row["name_auto"]."\n";
        }
    }

    if($post["method"] == "get_all") {
        get_all();
    } elseif ($post["method"] == "insert") {
        #insert();
    } elseif ($post["method"] == "get_models") {
        get_models();
    }
}
?>
