<?php
if(isset($_POST)){
	$fp = fopen("filetxt", "a");
	$data = file_get_contents("php://input");
	$post = json_decode($data, true);
	echo "Values Post:";
	foreach ($post as $key => $value) {
    		echo $key . ' = ' . $value . '<br>';
	}
	fwrite($fp, date("D M j G:i:s T Y")." Name: '".$post["name"]."' Comment: ".$post["comment"]."\n");
	fclose($fp);
}
?>

