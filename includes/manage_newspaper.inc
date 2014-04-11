<?php

/**
 * @file
 * Contains one local action for ingesting pages.
 */

require_once dirname(__FILE__) . '/utilities.inc';

/**
 * Displays all the book management forms in a set of vertical tabs.
 *
 * @param AbstractObject $object
 *   The newspaper object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   book objects.
 */
function islandora_newspaper_manage_newspaper(AbstractObject $object) {
  drupal_set_title($object->label);
  return '';
}

/**
 * Local menu action, that gets the ingest page form.
 *
 * @param AbstractObject $newspaper
 *   The newspaper to ingest into.
 *
 * @return sting
 *   The HTML repersentation of the ingest page form.
 */
function islandora_newspaper_ingest_issue(AbstractObject $newspaper) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $tuque = islandora_get_tuque_connection();
  $issue = $tuque->repository->constructObject(islandora_get_namespace($newspaper->id));
  $issue->owner = $user->name;
  $issue->label = 'New Issue';
  $issue->models = 'islandora:newspaperIssueCModel';
  // Assume sequential adding of issues for now.
  $issues = islandora_newspaper_get_issues($newspaper);
  $num_issues = count($issues) + 1;
  $issue->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $newspaper->id);
  $issue->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $num_issues, TRUE);
  drupal_set_title(t('Add issue to @newspaper', array('@newspaper' => $newspaper->label)));
  return drupal_get_form('islandora_ingest_form', array(
      'models' => array('islandora:newspaperIssueCModel'),
      'objects' => array($issue),
      'parent' => $newspaper->id,
    ));
}
