<?php

/**
 * @file
 */
?>
<div class="islandora-newspaper-issue clearfix">
  <span class="islandora-newspaper-issue-navigator">
    <?php print theme('islandora_newspaper_issue_navigator', array('object' => $object)); ?>
  </span>
  <?php print theme('islandora_objects', array('objects' => $pages)); ?>
</div>
