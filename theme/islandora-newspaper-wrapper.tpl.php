<?php

/**
 * @file
 * islandora-newspaper-wrapper.tpl.php
 * 
 * @TODO: needs documentation about file and variables
 */
?>

<div class="islandora-newspaper-wrapper">
  <div class="islandora-newspaper clearfix">
    <?php if ($newspaper_controls): ?>
      <div class="islandora-newspaper-controls">
        <?php print theme('item_list', array('items' => $newspaper_controls, NULL, 'ul', 'attributes' => array('class' => array('items', 'inline'))));?>
      </div>
    <?php endif; ?>
    <?php print $newspaper_pager; ?>
    <?php print $newspaper_content; ?>
    <?php print $newspaper_pager; ?>
  </div>
</div>