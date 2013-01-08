<?php

/**
 * @file
 * Contains callback functions for controls on top of newspaper pages
 */

/**
 * Returns issue object of a given page object.
 */
function islandora_newspaper_get_issue($object) {
  $rels = $object->relationships->get('http://islandora.ca/ontology/relsext#', 'isPageOf');
  return $rels[0]['object']['value'];
}

/**
 * Returns the collection object of a given issue object
 */
function islandora_newspaper_get_newspaper($object) {
  $rels = $object->relationships->get('info:fedora/fedora-system:def/relations-external#', 'isMemberOfCollection'); // is this the old way of rels ext?
  return $rels[0]['object']['value'];
}

/**
 * Returns the sequence number of a given newsletter page.
 */
function islandora_newspaper_get_current_sequence($object) {
  $rels = $object->relationships->get('http://islandora.ca/ontology/relsext#', 'isSequenceNumber');
  return $rels[0]['object']['value'];
}

/**
 * Get the pages of a newspaper issue. Mimics the basic collection functionality.
 *
 * @see islandora_basic_collection_get_objects()
 */
function islandora_newspaper_get_pages($object, $page_number = 1, $page_size = 5) {
  // @TODO: this should later be split up in 2 queries. One for visible page and one for total count.
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
    'page_number' => $page_number
  );
  drupal_alter('islandora_newspaper_query', $query_array);
  try {
    $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  } catch (Exception $e) {
    drupal_set_message(t('Islandora Error getting related objects for %s', array('%s' => $object->id)), 'error');
    return '';
  }
  return $results;
}

/**
 * Returns object details for the next and previous page if available in page sequence.
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
 */
function islandora_newspaper_get_page_pager_links($pages, $current_sequence) {
  $page_pager = islandora_newspaper_get_page_pager($pages, $current_sequence);
  if (!empty($page_pager)) {
    $page_pager_links = '';
    if (isset($page_pager['previous'])) {
      $prev_pid = $page_pager['previous']['object']['value'];
      $page_pager_links .= l('Prev', 'islandora/object/' . $prev_pid, array('attributes' => array('title' => t('Previous page'), 'class' => 'page-prev')));
    }
    $page_pager_links .= ' <strong>' . t('Page') . '</strong> ';
    if (isset($page_pager['next'])) {
      $next_pid = $page_pager['next']['object']['value'];
      $page_pager_links .= l('Next', 'islandora/object/' . $next_pid, array('attributes' => array('title' => t('Next page'), 'class' => 'page-next')));
    }
    return $page_pager_links;
  }
  else {
    return NULL;
  }
}

/**
 * Creates page pager as a select element.
 */
function islandora_newspaper_get_page_select($pages, $current_sequence) {
  $path = drupal_get_path('module', 'islandora_newspaper');
  drupal_add_js($path . '/js/islandora_newspaper.js');
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
    '#id' => 'page-select',
  );
  $select = '<strong>' . t('Image') . ':</strong> ';
  $select .= theme('select', array('element' => $variables));
  $select .= ' ' . t('of') . ' ' . $total;
  return $select;  
}

/**
 * Finds next and previous issues given a newspaper object and a current issue PID.
 */
function islandora_get_issue_pager($issue_pid, $newspaper_object) {
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
 * Returns the first page of an issue, given an issue PID.
 */
function islandora_newspaper_get_issue_frontpage($object) {
  // We only need the front page, but with collection queries don't add amounts to the query yet. Once that's
  // in place, we'd need to change this in islandora_newspaper_get_pages() too.
  $pages = islandora_newspaper_get_pages($object);
  return $pages[0];
}

/**
 * Creates rendered pager links for issues.
 */
function islandora_newspaper_get_issue_pager_links($issue_pager) {
  if (!empty($issue_pager)) {
    $issue_pager_links = '';
    if (isset($issue_pager['previous'])) {
      $prev_pid = $issue_pager['previous']['object']['value'];
      $issue_pager_links .= l('Prev', 'islandora/object/' . $prev_pid, array('attributes' => array('title' => t('Previous issue'), 'class' => 'issue-prev')));
    }
    $issue_pager_links .= ' <strong>' . t('Issue') . '</strong> ';
    if (isset($issue_pager['next'])) {
      $next_pid = $issue_pager['next']['object']['value'];
      $issue_pager_links .= l('Next', 'islandora/object/' . $next_pid, array('attributes' => array('title' => t('Next issue'), 'class' => 'issue-next')));
    }
    return $issue_pager_links;
  }
  else {
    return NULL;
  }
}
