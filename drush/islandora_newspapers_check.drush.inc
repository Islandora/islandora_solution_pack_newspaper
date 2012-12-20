<?php

function islandora_newspapers_check_drush_command() {
  $items = array();

  $items['islandora_check_newspapers'] = array(
    'description' => "Checks that all of the newspaper objects have been created properly.",
    'arguments' => array(
      'query_file' => 'The path to a CSV file that contains the information about the newspaper pages',
    ),
    'examples' => array(
      'drush -u 1 --uri=http://localhost islandora_ingest_newspapers_check /path/to/pages.csv',
    ),
    'aliases' => array('islandoranewscheck'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN, // we can pass in users id on the command line using drush -u.
  );
  return $items;
}

function drush_islandora_newspapers_check_islandora_check_newspapers($query_file) {

  $path_name = dirname($query_file);
  
  $fh = fopen($query_file, 'r');
  $line_number = 0;

  while (($data = fgetcsv($fh)) !== FALSE) {
    $line_number++;
    if ($line_number != 1) {
      if ($data[7] == '1') {
        $book = check_object('guardian:' . $data[3]);
        if (strlen($data[7]) == 1) {
          $page_no = '00' . $data[7];
          $page = check_object('guardian:'. $data[3] . '-' . $page_no);
        }
        else {
          $page_no = '0' . $data[7];
          $page = check_object('guardian:'. $data[3] . '-' . $page_no);
        }
      }
      else {
        if (strlen($data[7]) == 1) {
          $page_no = '00' . $data[7];
          $page = check_object('guardian:'. $data[3] . '-' . $page_no);
        }
        else {
          $page_no = '0' . $data[7];
          $page = check_object('guardian:'. $data[3] . '-' . $page_no);
        }
      }
    }
  }
}


function check_object($pid) {

$ch = curl_init("http://localhost:8080/fedora/objects/$pid");
curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
curl_exec($ch);
if (!curl_errno($ch)) {
  $info = curl_getinfo($ch);
  $code = $info['http_code'];
  if ($code == '404') {
    print "Pid: $pid\n";
    print "Code: " . $info['http_code'] . "\n\n";
  }
}

}

?>
