<?php

/**
 * @file
 * Miscellaneous helper functions.
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
  $rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
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
  $rels = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
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
  $rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  return $rels[0]['object']['value'];
}
