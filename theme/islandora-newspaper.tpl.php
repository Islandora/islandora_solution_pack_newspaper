<?php

/**
 * @file
 * islandora-newspaper.tpl.php
 *
 * @TODO: needs documentation about file and variables
 */
?>
<div class="islandora islandora-newspaper">
  <div class="islandora-newspaper-grid clearfix">
  <?php foreach($associated_objects_array as $key => $value): ?>
    <dl class="islandora-newspaper-object <?php print $value['class']; ?>">
        <dt class="islandora-newspaper-thumb"><?php print $value['thumb_link']; ?></dt>
        <dd class="islandora-newspaper-caption"><?php print $value['title_link']; ?></dd>
    </dl>
  <?php endforeach; ?>
  </div>
</div>