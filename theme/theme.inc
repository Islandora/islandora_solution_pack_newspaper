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

  $newspaper_controls = array(
    'Link to newspaper',
    'prev-next issue',
    'all issues'
  );
  $variables['newspaper_controls'] = $newspaper_controls;
}
 
/**
 * Implements hook_preprocess().
 *
 * @param array $variables
 *   an array of variables that will be passed to the theme function
 */
function islandora_newspaper_preprocess_islandora_newspaper(&$variables) {
  global $base_url;
  global $base_path;
  $islandora_object = $variables['islandora_object'];
  $results = $variables['query_results'];
  try {
    $dc = $islandora_object['DC']->content;
    $dc_object = DublinCore::import_from_xml_string($dc);
  } catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_newspaper_page_size', '10') : $_GET['pagesize'];
  $total_count = count($results);
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
    $pid = $results[$i]['object']['value'];
    $fc_object = islandora_object_load($pid);
    if (!isset($fc_object)) {
      continue; //null object so don't show in newspaper view;
    }
    $associated_objects_array[$pid]['object'] = $fc_object;
    try {
      $dc = $fc_object['DC']->content;
      $dc_object = DublinCore::import_from_xml_string($dc);
      $associated_objects_array[$pid]['dc_array'] = $dc_object->as_formatted_array();
    } catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
    $object_url = 'islandora/object/' . $pid;
    $thumbnail_img = '<img src="' . $base_path . $object_url . '/datastream/TN/view"' . '/>';
    $title = $results[$i]['title']['value'];
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
 *
 * @param array $variables
 *   an array of variables that will be passed to the theme function
 */
function islandora_newspaper_preprocess_islandora_newspaper_page(&$variables) {
  $islandora_object = $variables['islandora_object'];
  $repository = $islandora_object->repository;
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $variables['theme_hook_suggestions'][] = 'islandora_newspaper_page__' . str_replace(':', '_', $islandora_object->id);

  // Thumbnail
  if (isset($islandora_object['TN'])) {
    $params = array(
      'title' => $islandora_object->label,
      'path' => url("islandora/object/{$islandora_object->id}/datastream/TN/view")
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
        'query' => array('token'=>$token),
      ));
  }
  else {
    $jp2_url = url("islandora/object/{$islandora_object->id}/datastream/JP2/view", array('absolute' => TRUE));
  }

  // Display newspaper
  $viewer = islandora_get_viewer($jp2_url, 'islandora_newspaper_viewers');
  $variables['islandora_content'] = '';
  if ($viewer) {
    $variables['islandora_content'] = $viewer;
  }
  elseif (isset($islandora_object['JPG'])) { // If no viewer is configured just show the jpeg.
    $params = array(
      'title' => $islandora_object->label,
      'path' => url("islandora/object/{$islandora_object->id}/datastream/JPG/view")
    );
    $variables['islandora_content'] = theme('image', $params);
  }
  else {
    $variables['islandora_content'] = NULL;
  }

  $newspaper_controls = array(
    'Link to newspaper',
    'Page dropdown',
    'prev-next page',
    'all pages',
    'prev-next issue',
    'all issues',
    'printer friendly (basic info+TN+OCR)',
    'PDF',
    'Download JP2',
    'Clip image'
  );
  $variables['newspaper_controls'] = $newspaper_controls;
}

