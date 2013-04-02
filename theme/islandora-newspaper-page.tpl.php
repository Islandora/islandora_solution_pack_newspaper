<?php

/**
 * @file
 * islandora-newspaper-page.tpl.php
 * This is the template file for the object page for newspaper
 *
 * Available variables:
 * - $islandora_object: The Islandora object rendered in this template file
 * - $islandora_dublin_core: The DC datastream object
 * - $dc_array: The DC datastream object values as a sanitized array. This
 *   includes label, value and class name.
 * - $islandora_object_label: The sanitized object label.
 * - $parent_collections: An array containing parent collection(s) info.
 *   Includes collection object, label, url and rendered link.
 * - $islandora_thumbnail_img: A rendered thumbnail image.
 * - $islandora_content: A rendered image. By default this is the JPG datastream
 *   which is a medium sized image. Alternatively this could be a rendered
 *   viewer which displays the JP2 datastream image.
 *
 * @see template_preprocess_islandora_large_image()
 * @see theme_islandora_large_image()
 *
 * @TODO: review documentation about file and variables
 */
?>
<div class="islandora-newspaper-object">
  <div class="islandora-newspaper-controls">
    <?php print theme('islandora_newspaper_page_controls', array('object' => $object)); ?>
  </div>
  <div class="islandora-newspaper-content-wrapper clearfix">
    <?php if ($content): ?>
      <div class="islandora-newspaper-content">
        <?php print $content; ?>
      </div>
    <?php endif; ?>
  </div>
</div>
