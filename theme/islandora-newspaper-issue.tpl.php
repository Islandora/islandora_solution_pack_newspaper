<?php

/**
 * @file
 * This is the template file for the object page for a newspaper issue.
 *
 * Available variables:
 * - $object: The newspaper issue object.
 * - $pages: Pages of this object in order.
 * - $description: Rendered metadata descripton for the object.
 * - $metadata: Rendered metadata display for the binary object.
 */
?>
<div class="islandora-newspaper-issue clearfix">
  <span class="islandora-newspaper-issue-navigator">
    <?php print theme('islandora_newspaper_issue_navigator', array('object' => $object)); ?>
  </span>
  <?php if (isset($viewer_id) && $viewer_id != "none"): ?>
    <div id="book-viewer">
      <?php print $viewer; ?>
    </div>
  <?php else: ?>
    <?php print theme('islandora_objects', array('objects' => $pages)); ?>
  <?php endif; ?>
  <div class="islandora-newspaper-issue-metadata">
    <?php print $description; ?>
    <?php print $metadata; ?>
  </div>
</div>
