<?php

/**
 * @file
 * Theme file for Newspaper solution pack. Includes preprocess functions.
 */

/**
 * Implements hook_preprocess().
 */
function islandora_newspaper_preprocess_islandora_newspaper_wrapper(&$variables) {
  global $base_url;

  $islandora_object = $variables['islandora_object'];
  $newspaper_pid = islandora_newspaper_get_newspaper($islandora_object);
  $newspaper_object = islandora_object_load($newspaper_pid);
  $issue_pager = islandora_get_issue_pager($islandora_object->id, $newspaper_object);

  $newspaper_controls = array();
  // Issue pager.
  if ($issue_pager) {
    $newspaper_controls['issue_pager'] = islandora_newspaper_get_issue_pager_links($issue_pager);
  }
  // All issues.
  if ($newspaper_object) {
    $newspaper_controls['all_issues'] = l(t('All issues'), 'islandora/object/' . $newspaper_pid, array('attributes' => array('title' => t('All issues'))));
  }
  $variables['newspaper_controls_array'] = $newspaper_controls;
  $variables['newspaper_controls'] = theme('item_list', array('items' => $newspaper_controls, NULL, 'ul', 'attributes' => array('class' => array('items', 'inline'))));
}

/**
 * Implements hook_preprocess().
 */
function islandora_newspaper_preprocess_islandora_newspaper(&$variables) {
  global $base_url;
  global $base_path;
  $islandora_object = $variables['islandora_object'];
  $pages = $variables['pages'];
  try {
    $dc = $islandora_object['DC']->content;
    $dc_object = DublinCore::import_from_xml_string($dc);
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_newspaper_page_size', '10') : $_GET['pagesize'];
  $total_count = count($pages);
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : array();
  $variables['islandora_object_label'] = $islandora_object->label;
  $display = (empty($_GET['display'])) ? 'list' : $_GET['display'];
  $variables['theme_hook_suggestions'][] = 'islandora_newspaper__' . str_replace(':', '_', $islandora_object->id);
  if (isset($islandora_object['OBJ'])) {
    $full_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/OBJ/view';
    $variables['islandora_full_img'] = '<img src="' . $full_size_url . '"/>';
  }
  if (isset($islandora_object['TN'])) {
    $thumbnail_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/TN/view';
    $variables['islandora_thumbnail_img'] = '<img src="' . $thumbnail_size_url . '"/>';
  }
  if (isset($islandora_object['MEDIUM_SIZE'])) {
    $medium_size_url = $base_url . '/islandora/object/' . $islandora_object->id . '/datastream/MEDIUM_SIZE/view';
    $variables['islandora_medium_img'] = '<img src="' . $medium_size_url . '"/>';
  }

  $associated_objects_array = array();
  $start = $page_size * ($page_number);
  $end = min($start + $page_size, $total_count);

  for ($i = $start; $i < $end; $i++) {
    $pid = $pages[$i]['object']['value'];
    $fc_object = islandora_object_load($pid);
    if (!isset($fc_object)) {
      // NULL object so don't show in newspaper view.
      continue;
    }
    $associated_objects_array[$pid]['object'] = $fc_object;
    try {
      $dc = $fc_object['DC']->content;
      $dc_object = DublinCore::import_from_xml_string($dc);
      $associated_objects_array[$pid]['dc_array'] = $dc_object->as_formatted_array();
    }
    catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
    $object_url = 'islandora/object/' . $pid;
    $thumbnail_img = '<img src="' . $base_path . $object_url . '/datastream/TN/view"' . '/>';
    $title = $pages[$i]['title']['value'];
    $associated_objects_array[$pid]['pid'] = $pid;
    $associated_objects_array[$pid]['path'] = $object_url;
    $associated_objects_array[$pid]['title'] = $title;
    $associated_objects_array[$pid]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $pid));
    if (isset($fc_object['TN'])) {
      $thumbnail_img = '<img src="' . $base_path . $object_url . '/datastream/TN/view"' . '/>';
    }
    else {
      $image_path = drupal_get_path('module', 'islandora');
      $thumbnail_img = '<img src="' . $base_path . $image_path . '/images/folder.png"/>';
    }
    $associated_objects_array[$pid]['thumbnail'] = $thumbnail_img;
    $associated_objects_array[$pid]['title_link'] = l($title, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
    $associated_objects_array[$pid]['thumb_link'] = l($thumbnail_img, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
  }
  $variables['associated_objects_array'] = $associated_objects_array;
}

/**
 * Implements hook_preprocess().
 */
function islandora_newspaper_preprocess_islandora_newspaper_page(&$variables) {
  $islandora_object = $variables['islandora_object'];
  $repository = $islandora_object->repository;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $path = drupal_get_path('module', 'islandora_newspaper');
  drupal_add_js($path . '/js/islandora_newspaper.js');

  $variables['theme_hook_suggestions'][] = 'islandora_newspaper_page__' . str_replace(':', '_', $islandora_object->id);

  // Thumbnail.
  if (isset($islandora_object['TN'])) {
    $params = array(
      'title' => $islandora_object->label,
      'path' => url("islandora/object/{$islandora_object->id}/datastream/TN/view"),
    );
    $variables['islandora_thumbnail_img'] = theme('image', $params);
  }

  module_load_include('inc', 'islandora', 'includes/solution_packs');
  // Get token to allow access to XACML protected datastreams if necessary.
  if (isset($islandora_object['POLICY'])) {
    module_load_include('inc', 'islandora', 'includes/islandora_authtokens');
    $token = islandora_get_object_token($islandora_object->id, 'JP2', 2);
    $jp2_url = url("islandora/object/{$islandora_object->id}/datastream/JP2/view",
      array(
        'absolute' => TRUE,
        'query' => array('token' => $token),
      ));
  }
  else {
    $jp2_url = url("islandora/object/{$islandora_object->id}/datastream/JP2/view", array('absolute' => TRUE));
  }

  // Display newspaper.
  $viewer = islandora_get_viewer($jp2_url, 'islandora_newspaper_viewers');
  $variables['islandora_content'] = '';
  if ($viewer) {
    $variables['islandora_content'] = $viewer;
  }
  // If no viewer is configured just show the jpeg.
  elseif (isset($islandora_object['JPG'])) {
    $params = array(
      'title' => $islandora_object->label,
      'path' => url("islandora/object/{$islandora_object->id}/datastream/JPG/view"),
    );
    $variables['islandora_content'] = theme('image', $params);
  }
  else {
    $variables['islandora_content'] = NULL;
  }

  $issue_pid = islandora_newspaper_get_issue($islandora_object);
  $issue_object = islandora_object_load($issue_pid);
  $pages = islandora_newspaper_get_pages($issue_object);
  $newspaper_pid = islandora_newspaper_get_newspaper($issue_object);
  $newspaper_object = islandora_object_load($newspaper_pid);
  $current_sequence = islandora_newspaper_get_current_sequence($islandora_object);
  $page_pager = islandora_newspaper_get_page_pager($pages, $current_sequence);

  $newspaper_controls = array();
  // Page select pager.
  $page_select = islandora_newspaper_get_page_select($pages, $current_sequence);
  if (!empty($page_select)) {
    $newspaper_controls['page_select'] = $page_select;
  }
  // Page pager.
  $page_pager_links = islandora_newspaper_get_page_pager_links($pages, $current_sequence);
  if (!empty($page_pager_links)) {
    $newspaper_controls['page_pager'] = $page_pager_links;
  }
  // All pages.
  if ($issue_object) {
    $newspaper_controls['all_pages'] = l(t('All pages'), 'islandora/object/' . $issue_pid, array('attributes' => array('title' => t('All pages'))));
  }
  $issue_pager = islandora_get_issue_pager($issue_pid, $newspaper_object);
  $issue_frontpage_pager = islandora_get_issue_frontpage_pager($issue_pager);
  // Front page issue pager.
  if ($issue_frontpage_pager) {
    $newspaper_controls['issue_pager'] = islandora_newspaper_get_issue_pager_links($issue_frontpage_pager);
  }
  // All issues.
  if ($newspaper_object) {
    $newspaper_controls['all_issues'] = l(t('All issues'), 'islandora/object/' . $newspaper_pid, array('attributes' => array('title' => t('All issues'))));
  }
  // View.
  if ($islandora_object['OCR'] || $islandora_object['PDF']) {
    $view = array();
    // Text view.
    if ($islandora_object['OCR']) {
      $text = $islandora_object['OCR'];
      $view[] = l(t('Text'), islandora_datastream_get_url($text, 'view'), array('attributes' => array('title' => t('View text'), 'class' => 'view-text')));
    }
    // PDF download.
    if ($islandora_object['PDF']) {
      $pdf = $islandora_object['PDF'];
      $view[] = l(t('PDF'), islandora_datastream_get_url($pdf, 'download'), array('attributes' => array('title' => t('Download PDF'), 'class' => 'download-pdf')));
    }
    $newspaper_controls['view'] = '<strong>' . t('View') . ':</strong> ' . implode(' | ', $view);
  }

  // JP2 download.
  if ($islandora_object['JP2']) {
    $jp2_ds = $islandora_object['JP2'];
    $jp2_size = islandora_datastream_get_human_readable_size($jp2_ds);
    $newspaper_controls['jp2_download'] = '<strong>' . t('Download:') . '</strong> ' . l(t('JP2 (@size)', array('@size' => $jp2_size)), islandora_datastream_get_url($jp2_ds, 'download'), array('attributes' => array('title' => 'Download JP2')));
  }
  $variables['newspaper_controls_array'] = $newspaper_controls;
  $variables['newspaper_controls'] = theme('item_list', array('items' => $newspaper_controls, NULL, 'ul', 'attributes' => array('class' => array('items', 'inline'))));
}
