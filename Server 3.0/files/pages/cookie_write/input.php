<?php
$fp = fopen("filetxt", "w");
	#fwrite($fp, "Code=>".(count($arr)+1).", Name=>".$_POST["Name"].", SecondName=>".$_POST["SecondName"].", Fakultet=>".$_POST["Fakultet"]."\n");
    fwrite($fp, date()." Name: '".$_POST["name"]." Comment: ".$_POST["comment"]);
    fclose($fp);
	#header("Location: 217.71.139.74");
?>
