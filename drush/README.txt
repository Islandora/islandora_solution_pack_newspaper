Ingesting newspapers - Drush script for ingest - Drush Script for
discovering rows that didn’t get ingested - ITQL Check for missing
datastreams

Ingesting newspapers

There are two drush scripts which are installed on the newspapers repo
server. Drush script for ingest

drush -u 1 --uri=http://yoursite.com islandora_ingest_newspapers
/path/to/your/newspaper/data_sheet.csv TRUE 25

The parameters that need to be changed are 
- the uri setting (set it to your site) 
- the path to your csv file with the page metadata 
- whether to have interactive set to TRUE or FALSE 
- the last number (25 in the example) is the reel number related to the batch of images

The script will read the metadata and ingest the tif files, creating the
MODS and RELS-EXT at the same time. The thumbnail for the issue object is
also created in this process. This command can be stacked with others in
a bash script to ingest more than one reel (I do one reel at a time and resolve any 
problems before moving to the next one - dm). If the ingest of a reel
fails because of a PID conflict or some other reason then the bash
script will continue on to the next reel. This will leave gaps in the
ingested content which will have to be filled later.

Drush Script for discovering rows that didn’t get ingested

The second command can be used to check if the ingest has been
successful.

find /mnt/Digitization/newspapers/ -iname *.csv -exec drush
islandora_check_newspapers {} \;

You could also do a path to the dir you are working with to speed things
up - dm - eg. find
/mnt/Digitization/newspapers/19150506-19150930/processed -iname *.csv
-exec drush islandora_check_newspapers {} \;

This command will go through the csv file(s) and compare the information
there with the objects contained in the repo. If it finds an object that
should be in the repo but isn’t then it will print out the PID of that
object.

If there is a mismatch I’ve been going through the images to check that
the metadata matches the information on the page. I then fix the
metadata and create a csv file that just contains the issues/pages that
didn’t get ingested. Running the first command on this csv file will
ingest them with the correct MODS and RELS-EXT. If the first image in
the csv file is not page 1 then the script will ask which book object
you want to associate the pages with. 

ITQL to check for missing datastreams

You can use this ITQL query to find objects with missing datastreams (you could OR 
a bunch of DS queries together -dm) …
meaning you need to run microservices against that object again.

http://localhost:8080/fedora/risearch

select $object $title from <#ri> where ($object <fedora-model:hasModel>
<info:fedora/islandora:pageCModel> and $object <dc:title> $title and
$object <fedora-model:state>
<info:fedora/fedora-system:def/model#Active>) minus ($object
<fedora-view:disseminates> $ds1 in <#ri> and $ds1
<fedora-view:disseminationType> <info:fedora/*/ENCODED_OCR> in <#ri>)


