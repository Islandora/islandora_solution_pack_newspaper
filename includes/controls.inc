<?php

/**
 * @file
 * Contains callback functions for controls on top of newspaper pages
 */

/**
 * Returns issue object of a given page object.
 *
 * @param object $object
 *   Newspaper page object
 *
 * @return string
 *   Returns the PID of the issue object of which $object is a page of.
 */
function islandora_newspaper_get_issue($object) {
  $rels = $object->relationships->get('http://islandora.ca/ontology/relsext#', 'isPageOf');
  return $rels[0]['object']['value'];
}

/**
 * Returns the sequence number of a given newsletter page.
 *
 * @param object $object
 *   Newspaper page object
 *
 * @return string
 *   Returns the sequence number of a page in an issue.
 */
function islandora_newspaper_get_current_sequence($object) {
  $rels = $object->relationships->get('http://islandora.ca/ontology/relsext#', 'isSequenceNumber');
  return $rels[0]['object']['value'];
}

/**
 * Returns the collection object of a given issue object.
 *
 * @param object $object
 *   Newspaper issue object
 *
 * @return string
 *   Returns the PID of the collection of which $object is a member of.
 */
function islandora_newspaper_get_newspaper($object) {
  // Is this the old way of using rels ext?
  $rels = $object->relationships->get('info:fedora/fedora-system:def/relations-external#', 'isMemberOfCollection');
  return $rels[0]['object']['value'];
}

/**
 * Get the pages of a newspaper issue. Mimics the basic collection
 * functionality.
 *
 * @param object $object
 *   Newspaper issue object
 * @param int $page_number
 *   Page offset number
 * @param int $page_size
 *   Amount of objects to be displayed per page
 *
 * @return array
 *   An array containing the results of the query
 *
 * @see islandora_basic_collection_get_objects()
 */
function islandora_newspaper_get_pages($object, $page_number = 1, $page_size = 5) {
  // @TODO: this should later be split up in 2 queries.
  // One for visible page and one for total count.
  $query = 'SELECT ?object ?sequence ?page ?title
WHERE {
  ?object <http://islandora.ca/ontology/relsext#isPageOf> <info:fedora/' . $object->id . '> .
  ?object <http://islandora.ca/ontology/relsext#isSequenceNumber> ?sequence .
  ?object <http://islandora.ca/ontology/relsext#isPageNumber> ?page .
  ?object <info:fedora/fedora-system:def/model#label> ?title .
  ?object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
}
ORDER BY ?sequence';

  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
    'pid' => $object->id,
    'page_size' => $page_size,
    'page_number' => $page_number,
  );
  drupal_alter('islandora_newspaper_query', $query_array);
  try {
    $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  }
  catch (Exception $e) {
    drupal_set_message(t('Islandora Error getting related objects for %s', array('%s' => $object->id)), 'error');
    return '';
  }
  return $results;
}

/**
 * Returns object details for the next and previous page if available in page
 * sequence.
 *
 * @param array $pages
 *   An array containing info about all pages in a newspaper issue.
 * @param int $current_sequence
 *   A sequence number of the current page.
 *
 * @return array
 *   Returns an array containing info about the previous and next pages relative
 *   to the current page.
 */
function islandora_newspaper_get_page_pager($pages, $current_sequence) {
  $total = count($pages);
  foreach ($pages as $key => $page) {
    if ($page['sequence']['value'] == $current_sequence) {
      $page_pager = array();
      if ($key + 1 <= $total - 1) {
        $page_pager['next'] = $pages[$key + 1];
      }
      if ($key - 1 >= 0) {
        $page_pager['previous'] = $pages[$key - 1];
      }
      return $page_pager;
    }
  }
}

/**
 * Creates rendered pager links for issue pages.
 *
 * @param array $pages
 *   An array containing info about all pages in a newspaper issue.
 * @param int $current_sequence
 *   A sequence number of the current page.
 *
 * @return array
 *   Returns an array containing the rendered previous and next pager links
 */
function islandora_newspaper_get_page_pager_links($pages, $current_sequence) {
  $page_pager = islandora_newspaper_get_page_pager($pages, $current_sequence);
  if (!empty($page_pager)) {
    $page_pager_links = '';
    if (isset($page_pager['previous'])) {
      $prev_pid = $page_pager['previous']['object']['value'];
      $page_pager_links .= l(t('Prev'), 'islandora/object/' . $prev_pid, array('attributes' => array('title' => t('Previous page'), 'class' => 'page-prev')));
    }
    $page_pager_links .= ' <strong>' . t('Page') . '</strong> ';
    if (isset($page_pager['next'])) {
      $next_pid = $page_pager['next']['object']['value'];
      $page_pager_links .= l(t('Next'), 'islandora/object/' . $next_pid, array('attributes' => array('title' => t('Next page'), 'class' => 'page-next')));
    }
    return $page_pager_links;
  }
  else {
    return NULL;
  }
}

/**
 * Creates page pager as a select element.
 *
 * @param array $pages
 *   An array containing info about all pages in a newspaper issue.
 * @param int $current_sequence
 *   A sequence number of the current page.
 *
 * @return string
 *   Returns a rendered select item to navigate through pages.
 */
function islandora_newspaper_get_page_select($pages, $current_sequence) {
  $total = count($pages);
  $options = array();
  foreach ($pages as $key => $page) {
    $pid = $page['object']['value'];
    $sequence = $page['sequence']['value'];
    $options[$pid] = $sequence;
    if ($sequence == $current_sequence) {
      $default = $pid;
    }
  }
  $variables = array(
    '#options' => $options,
    '#value' => $default,
    '#attributes' => array('class' => array('page-select')),
  );
  $select = '<strong>' . t('Image') . ':</strong> ';
  $select .= theme('select', array('element' => $variables));
  $select .= ' ' . t('of') . ' ' . $total;
  return $select;
}

/**
 * Finds next and previous issues given a newspaper object and a current issue
 * PID.
 *
 * @param string $issue_pid
 *   The PID of the issue object.
 * @param object $newspaper_object
 *   Newspaper object - this is the parent object of issue objects.
 *
 * @return array
 *   Returns an array containing info about the first previous and next issue
 *   relative to the $issue_pid object.
 */
function islandora_newspaper_get_issue_pager($issue_pid, $newspaper_object) {
  $issues = islandora_basic_collection_get_objects($newspaper_object);
  $total = count($issues);
  foreach ($issues as $key => $issue) {
    if ($issue['object']['value'] == $issue_pid) {
      $issue_pager = array();

      if ($key + 1 <= $total - 1) {
        $issue_pager['next'] = $issues[$key + 1];
      }
      if ($key - 1 >= 0) {
        $issue_pager['previous'] = $issues[$key - 1];
      }
      return $issue_pager;
    }
  }
}

/**
 * Given the next and previous issues, this function changes this info into the
 * front pages of these issues instead.
 *
 * @param array $issue_pager
 *   An array containing info about the first previous and next issues relative
 *   to a certain issue.
 *
 * @return array
 *   Return the $issue_pager array where the issue information has been replaced
 *   by that issues' first page.
 */
function islandora_get_issue_frontpage_pager($issue_pager) {
  if (isset($issue_pager['next'])) {
    $issue_object = $object = islandora_object_load($issue_pager['next']['object']['value']);
    $front_page = islandora_newspaper_get_issue_frontpage($issue_object);
    $issue_pager['next'] = $front_page;
  }
  if (isset($issue_pager['previous'])) {
    $issue_object = $object = islandora_object_load($issue_pager['previous']['object']['value']);
    $front_page = islandora_newspaper_get_issue_frontpage($issue_object);
    $issue_pager['previous'] = $front_page;
  }
  return $issue_pager;
}

/**
 * Returns the first page of an issue, given an issue object.
 *
 * @param object $object
 *   An issue object
 *
 * @return array
 *   An array containing query results describing the first page of a given
 *   issue.
 */
function islandora_newspaper_get_issue_frontpage($object) {
  // We only need the front page, but with collection queries don't add amounts
  // to the query yet. Once that's in place, we'd need to change this in
  // islandora_newspaper_get_pages() too.
  $pages = islandora_newspaper_get_pages($object);
  return $pages[0];
}

/**
 * Creates rendered pager links for issues.
 *
 * @param array $issue_pager
 *   An array describing the previous and next issues relative to a certain
 *   issue.
 *
 * @return string
 *   Returns rendered previous and next links.
 */
function islandora_newspaper_get_issue_pager_links($issue_pager) {
  if (!empty($issue_pager)) {
    $issue_pager_links = '';
    if (isset($issue_pager['previous'])) {
      $prev_pid = $issue_pager['previous']['object']['value'];
      $issue_pager_links .= l(t('Prev'), 'islandora/object/' . $prev_pid, array('attributes' => array('title' => t('Previous issue'), 'class' => 'issue-prev')));
    }
    $issue_pager_links .= ' <strong>' . t('Issue') . '</strong> ';
    if (isset($issue_pager['next'])) {
      $next_pid = $issue_pager['next']['object']['value'];
      $issue_pager_links .= l(t('Next'), 'islandora/object/' . $next_pid, array('attributes' => array('title' => t('Next issue'), 'class' => 'issue-next')));
    }
    return $issue_pager_links;
  }
  else {
    return NULL;
  }
}

/**
 * Create all default pager controls for newspaper issues.
 *
 * @param object $islandora_object
 *   The newspaper issue object to create pager controls for.
 *
 * @return array
 *   An array returning all rendered control elements.
 */
function islandora_newspaper_get_newspaper_controls($islandora_object) {
  $newspaper_pid = islandora_newspaper_get_newspaper($islandora_object);
  $newspaper_object = islandora_object_load($newspaper_pid);
  $issue_pager = islandora_newspaper_get_issue_pager($islandora_object->id, $newspaper_object);

  $newspaper_controls = array();
  // Issue pager.
  if ($issue_pager) {
    $newspaper_controls['issue_pager'] = islandora_newspaper_get_issue_pager_links($issue_pager);
  }
  // All issues.
  if ($newspaper_object) {
    $newspaper_controls['all_issues'] = l(t('All issues'), 'islandora/object/' . $newspaper_pid, array('attributes' => array('title' => t('All issues'))));
  }
  return $newspaper_controls;
}

/**
 * Create all default pager controls for newspaper pages.
 *
 * @param object $islandora_object
 *   The newspaper page object to create pager controls for.
 *
 * @return array
 *   An array returning all rendered control elements.
 */
function islandora_newspaper_get_page_controls($islandora_object) {
  $issue_pid = islandora_newspaper_get_issue($islandora_object);
  $issue_object = islandora_object_load($issue_pid);
  // Print error and return if no parent object.
  if (!$issue_object) {
    watchdog("Missing newspaper object.", "$issue_pid is missing,
    $islandora_object is the page that expected to find it.");

    return;
  }
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
  $issue_pager = islandora_newspaper_get_issue_pager($issue_pid, $newspaper_object);
  $issue_frontpage_pager = islandora_get_issue_frontpage_pager($issue_pager);
  // Front page issue pager.
  if ($issue_frontpage_pager) {
    $newspaper_controls['issue_pager'] = islandora_newspaper_get_issue_pager_links($issue_frontpage_pager);
  }
  // All issues.
  if ($newspaper_object) {
    $newspaper_controls['all_issues'] = l(t('All issues'), 'islandora/object/' . $newspaper_pid, array('attributes' => array('title' => t('All issues'))));
  }

  // Text view.
  if ($islandora_object['OCR']) {
    $text = $islandora_object['OCR'];
    $view= l(t('Text'), islandora_datastream_get_url($text, 'view'), array('attributes' => array('title' => t('View text'), 'class' => 'view-text')));
    $newspaper_controls['view'] = '<strong>' . t('View') . ':</strong> ' . $view;
  }


  // PDF download.
  if ($islandora_object['PDF']) {
    $pdf = $islandora_object['PDF'];
    $pdf_size = islandora_datastream_get_human_readable_size($pdf);
    $newspaper_controls['pdf_download'] = '<strong>' . t('Download:') . '</strong> ' . l(t('pdf (@size)', array('@size' => $pdf_size)), islandora_datastream_get_url($pdf, 'download'), array('attributes' => array('title' => 'Download PDF')));
  }
  // JP2 download.
  if ($islandora_object['JP2']) {
    $jp2_ds = $islandora_object['JP2'];
    $jp2_size = islandora_datastream_get_human_readable_size($jp2_ds);
    $newspaper_controls['jp2_download'] = '<strong>' . t('Download:') . '</strong> ' . l(t('JP2 (@size)', array('@size' => $jp2_size)), islandora_datastream_get_url($jp2_ds, 'download'), array('attributes' => array('title' => 'Download JP2')));
  }

  // TIFF download.
  if ($islandora_object['OBJ']) {
    $tiff_ds = $islandora_object['OBJ'];
    $tiff_size = islandora_datastream_get_human_readable_size($tiff_ds);
    $newspaper_controls['tiff_download'] = '<strong>' . t('Download:') . '</strong> ' . l(t('TIFF (@size)', array('@size' => $tiff_size)), islandora_datastream_get_url($tiff_ds, 'download'), array('attributes' => array('title' => 'Download TIFF')));
  }

  return $newspaper_controls;
}
