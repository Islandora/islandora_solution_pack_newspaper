<?php

/**
 * @file
 * Miscellaneous helper functions.
 */

/**
 * Returns issue object of a given page object.
 *
 * @param object $object
 *   Newspaper page object.
 *
 * @return string|bool
 *   Returns the PID of the issue object of which $object is a page of. If
 *   there is no such membership, returns FALSE.
 */
function islandora_newspaper_get_issue($object) {
  $rels = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $rel = reset($rels);
  if ($rel) {
    return $rel['object']['value'];
  }
  return FALSE;
}

/**
 * Returns the sequence number of a given newsletter page.
 *
 * @param object $object
 *   Newspaper page object.
 *
 * @return string|bool
 *   Returns the sequence number of a page in an issue. If there is no such
 *   membership, returns FALSE.
 */
function islandora_newspaper_get_current_sequence($object) {
  $rels = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber');
  $rel = reset($rels);
  if ($rel) {
    return $rel['object']['value'];
  }
  return FALSE;
}

/**
 * Returns the collection object of a given issue object.
 *
 * @param object $object
 *   Newspaper issue object.
 *
 * @return string|bool
 *   Returns the PID of the newspaper of which $object is a member. If there is
 *   no such membership, returns FALSE.
 */
function islandora_newspaper_get_newspaper($object) {
  $rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $rel = reset($rels);
  if ($rel) {
    return $rel['object']['value'];
  }
  return FALSE;
}

/**
 * Gets all the issues that the given newspaper owns.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   An associative array in the form of:
 *   - pid: The unique persistent identifier for the issue.
 *     - pid: The unique persistent identifier for the issue.
 *     - label: A descriptive label for the issue.
 *     - sequence: The sequence number of the issue, starts at 1.
 *     - issued: A DateTime object representing the date the issue was released.
 */
function islandora_newspaper_get_issues(AbstractObject $object) {

  if (module_exists('islandora_solr') && variable_get('islandora_newspaper_use_solr', FALSE)) {
    $issues = islandora_newspaper_get_issues_solr($object);
  }
  else {
    $issues = islandora_newspaper_get_issues_sparql($object);
  }

  // Grab the PIDs...
  $get_pid = function ($o) {
    return $o['pid'];
  };
  $pids = array_map($get_pid, $issues);
  // Make the PIDs the keys.
  $issues = count($pids) ? array_combine($pids, $issues) : array();
  return $issues;
}

/**
 * Gets all the issues that the given newspaper owns use the resource index.
 *
 * The results are ordered by their RELS-EXT dateIssued property.
 * Older to newer.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   An array with element of the form of:
 *     - pid: The unique persistent identifier for the issue.
 *     - label: A descriptive label for the issue.
 *     - sequence: The sequence number of the issue, starts at 1.
 *     - issued: A DateTime object representing the date the issue was released.
 */
function islandora_newspaper_get_issues_sparql(AbstractObject $object) {
  $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
PREFIX fedora-rels-ext: <info:fedora/fedora-system:def/relations-external#>
SELECT DISTINCT ?object ?sequence ?label ?issued
FROM <#ri>
WHERE {
  ?object fedora-rels-ext:isMemberOf <info:fedora/{$object->id}> ;
       <fedora-model:hasModel> <info:fedora/islandora:newspaperIssueCModel> ;
       <fedora-model:label> ?label .
  ?object islandora-rels-ext:isSequenceNumber ?sequence .
  ?object islandora-rels-ext:dateIssued ?issued
  !optionals
  !filters
}
ORDER BY ?sequence
EOQ;

  // XXX: Can't really use the fully featured query_filter and query_statements
  // hook as it will return some things we don't want. Perhaps change the return
  // structure in the future to specify which module they are coming from? For
  // now we will just get XACML's directly.
  $query_optionals = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_optionals', 'view');
  $query_filters = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_filters');

  $filter_map = function ($filter) {
    return "FILTER($filter)";
  };

  $query = format_string($query, array(
    '!optionals' => !empty($query_optionals) ? ('OPTIONAL {{' . implode('} UNION {', $query_optionals) . '}}') : '',
    '!filters' => implode(' ', array_map($filter_map, $query_filters)),
  ));

  $results = $object->repository->ri->sparqlQuery($query);
  // Map the results using a default Datetime for missing issued dates.
  $map_results = function ($o) {
    try {
      @$issued = new DateTime($o['issued']['value']);
    }
    catch (Exception $e) {
      // Use the current time as a place holder.
      $issued = new DateTime();
      $msg = 'Failed to get issued date from SPARQL query for @pid';
      $vars = array('@pid' => $o['object']['value']);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
    return array(
      'pid' => $o['object']['value'],
      'label' => $o['label']['value'],
      'sequence' => $o['sequence']['value'],
      'issued' => $issued,
    );
  };
  $issues = array_map($map_results, $results);
  return $issues;
}

/**
 * Gets all the issues that the given newspaper owns using Solr.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   An array with elements of the form of:
 *     - pid: The unique persistent identifier for the issue.
 *     - label: A descriptive label for the issue.
 *     - sequence: The sequence number of the issue, starts at 1.
 *     - issued: A DateTime object representing the date the issue was released.
 */
function islandora_newspaper_get_issues_solr(AbstractObject $object) {
  $parent_field = variable_get('islandora_newspaper_parent_issue_solr_field', 'RELS_EXT_isMemberOf_uri_ms');
  $date_field = variable_get('islandora_newspaper_solr_date_field', 'RELS_EXT_dateIssued_literal_ms');
  $sequence_field = variable_get('islandora_newspaper_solr_sequence_field', 'RELS_EXT_isSequenceNumber_literal_ms');
  // Counter for looping the start.
  $start = -1;
  // Number of records to return at once.
  $rows = 10000;
  $count = 0;
  $results = array();

  $solr_build = new IslandoraSolrQueryProcessor();
  $solr_query = format_string('!parent_field:("info:fedora/!pid" OR "!pid") AND'
    . ' RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:newspaperIssueCModel"',
    array(
      '!parent_field' => $parent_field,
      '!pid' => $object->id,
    )
  );

  do {
    $start += 1;
    $solr_param = array(
      'rows' => $rows,
      'limit' => $rows,
      'fl' => format_string('!date, !sequence, PID, fgs_label_s', array(
        '!date' => $date_field,
        '!sequence' => $sequence_field,
      )),
      'start' => ($rows * $start),
      'hl' => 'false',
      'facet' => 'false',
    );

    $solr_build->buildQuery($solr_query, $solr_param);
    $solr_build->solrParams = array_replace_recursive($solr_build->solrParams, $solr_param);
    if (variable_get('islandora_newspaper_solr_remove_base_filters', 1)) {
      module_load_include('inc', 'islandora_solr', 'includes/utilities');
      $solr_build->solrParams = islandora_solr_remove_base_filters($solr_build->solrParams);
    }
    try {
      $solr_build->executeQuery(FALSE);
      $count = $solr_build->islandoraSolrResult['response']['numFound'];
      $results = array_merge($results, $solr_build->islandoraSolrResult['response']['objects']);
    }
    catch (Exception $error) {
      drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $error->getMessage(), 'error');
    }

  } while ($count > ($rows * $start + $rows) && !isset($error));

  // Check the Solr doc array top and inside 'solr_doc'.
  $check_fn = function ($o, $name) {
    $value = "";
    if (isset($o[$name])) {
      $value = $o[$name];
    }
    elseif (isset($o['solr_doc'][$name])) {
      $value = $o['solr_doc'][$name];
    }
    if (is_array($value)) {
      return reset($value);
    }
    else {
      return $value;
    }
  };

  // Map the results using a default Datetime for missing issued dates.
  $map_results = function ($o) use ($date_field, $check_fn, $sequence_field) {
    try {
      @$issued = new DateTime($check_fn($o, $date_field));
    }
    catch (Exception $e) {
      // Use the current time as a place holder.
      $issued = new DateTime();
      $msg = 'Failed to get issued date from Solr query for @pid';
      $vars = array('@pid' => $o['PID']);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
    $sequence = $check_fn($o, $sequence_field);
    if (empty($sequence)) {
      $sequence = 0;
    }
    return array(
      'pid' => $o['PID'],
      'label' => $o['object_label'],
      'sequence' => $sequence,
      'issued' => $issued,
    );
  };
  $issues = array_map($map_results, $results);

  return $issues;
}

/**
 * Groups the issues by their relvant dates.
 *
 * @param array $issues
 *   Expected to be in the format as returned by islandora_newspaper_get_issues.
 *
 * @return array
 *   An associative array where the issues are grouped like so:
 *   - 1989: The year of the grouped issues.
 *     - 01: The month of the grouped issues.
 *       - 28: The day of the grouped issues contains actual issue descriptions.
 */
function islandora_newspaper_group_issues(array $issues) {
  $output = array();
  foreach ($issues as $issue) {
    $year = $issue['issued']->format('Y');
    $month = $issue['issued']->format('m');
    $day = $issue['issued']->format('d');
    $output[$year][$month][$day][] = $issue;
  }
  return $output;
}

/**
 * Attempts to get the date issued from the mods document.
 *
 * @param AbstractDatastream $datastream
 *   The datastream containing MODS.
 *
 * @return DateTime
 *   The date issued if found, otherwise the current time.
 */
function islandora_newspaper_get_date_issued_from_mods(AbstractDatastream $datastream) {
  $out = FALSE;
  $dom = new DOMDocument();
  if ($dom->loadXML($datastream->content)) {
    $xpath = new DomXPath($dom);
    $xpath->registerNamespace('ns', 'http://www.loc.gov/mods/v3');
    // Assumes the canonical date issued exists in the first mods document under
    // origin info and is not specified as a point, additional logic could be
    // added to process different encodings.
    $dates = $xpath->query('//ns:mods/ns:originInfo/ns:dateIssued[not(@point)]')->item(0);
    try {
      if (empty($dates)) {
        throw new Exception('mods:dateIssued element was empty.');
      }
      $result = $dates->nodeValue;
      $out = new DateTime($result);
    }
    catch (Exception $e) {
      $msg = 'Failed to get issued date from MODS for @pid';
      $vars = array('@pid' => $datastream->parent->id);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
  }
  return $out;
}

/**
 * Set's the date issued literal in the objects RELS-EXT.
 *
 * @param AbstractObject $object
 *   Newspaper issue object.
 * @param DateTime $date
 *   The date issued, will be stored in ISO-8601 YYYY-MM-DD format.
 */
function islandora_newspaper_set_date_issued(AbstractObject $object, DateTime $date) {
  $object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'dateIssued', NULL, TRUE);
  $object->relationships->add(ISLANDORA_RELS_EXT_URI, 'dateIssued', $date->format("Y-m-d"), TRUE);
}

/**
 * Fetchs the date issued from the objects RELS-EXT.
 *
 * If the date issue is not defined the current time is returned.
 *
 * @param AbstractObject $object
 *   Newspaper issue object.
 *
 * @return DateTime
 *   The date issued. If the date issue is not defined the current time is used.
 */
function islandora_newspaper_get_date_issued(AbstractObject $object) {
  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'dateIssued', NULL, TRUE);
  $date = empty($results) ? new DateTime() : new DateTime($results[0]['object']['value']);
  return $date;
}

/**
 * Fetch the issues of a newspaper that do not have a date.
 *
 * @param AbstractObject|null $object
 *   Newspaper object or empty for all issues.
 *
 * @return array
 *   Listing of the PIDs of objects without the date issued defined.
 */
function islandora_newspaper_get_empty_dates($object = NULL) {
  $tuque = islandora_get_tuque_connection();

  if (is_null($object)) {
    $where = <<<EOQ
  ?object <fedora-model:hasModel> <info:fedora/islandora:newspaperIssueCModel> ;
EOQ;
  }
  else {
    $where = <<<EOQ
  ?object fedora-rels-ext:isMemberOf <info:fedora/{$object->id}> ;
    <fedora-model:hasModel> <info:fedora/islandora:newspaperIssueCModel> ;
EOQ;
  }
  $islandora_rels_ext = ISLANDORA_RELS_EXT_URI;
  $fedora_rels_ext = FEDORA_RELS_EXT_URI;
  $query = <<<EOQ
PREFIX islandora-rels-ext: <{$islandora_rels_ext}>
PREFIX fedora-rels-ext: <{$fedora_rels_ext}>
SELECT ?object ?label
FROM <#ri>
WHERE {
  $where
    <fedora-model:label> ?label .
  !optionals
  !filters
}
ORDER BY ?label
EOQ;

  // XXX: Can't really use the fully featured query_filter and query_statements
  // hook as it will return some things we don't want. Perhaps change the return
  // structure in the future to specify which module they are coming from? For
  // now we will just get XACML's directly.
  $query_optionals = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_optionals', 'manage');
  $query_optionals[] = '?object islandora-rels-ext:dateIssued ?issued';
  $query_filters = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_filters');
  $query_filters[] = '!bound(?issued)';

  $filter_map = function ($filter) {
    return "FILTER($filter)";
  };

  $query = format_string($query, array(
    '!optionals' => !empty($query_optionals) ? ('OPTIONAL {{' . implode('} UNION {', $query_optionals) . '}}') : '',
    '!filters' => implode(' ', array_map($filter_map, $query_filters)),
  ));
  $results = $tuque->repository->ri->sparqlQuery($query);
  $map_results = function ($o) {
    return array(
      'pid' => $o['object']['value'],
      'label' => $o['label']['value'],
    );
  };
  $issues = array_map($map_results, $results);
  return $issues;
}

/**
 * Set mods:dateIssued from form.
 *
 * @param AbstractDatastream $datastream
 *   MODS datastream.
 * @param DateTime $date
 *   New date issued.
 */
function islandora_newspaper_set_mods_date_issued(AbstractDatastream $datastream, DateTime $date) {
  $out = FALSE;
  $dom = new DOMDocument();
  if ($dom->loadXML($datastream->content)) {
    $xpath = new DomXPath($dom);
    $xpath->registerNamespace('ns', 'http://www.loc.gov/mods/v3');
    // Assumes the canonical date issued exists in the first mods document under
    // origin info and is not specified as a point, additional logic could be
    // added to process different encodings.
    $dates = $xpath->query('//ns:mods/ns:originInfo/ns:dateIssued[not(@point)]');
    foreach ($dates as $d) {
      $d->parentNode->removeChild($d);
    }
    $new_date = $dom->createElementNS($dom->firstChild->namespaceURI, 'dateIssued', $date->format("Y-m-d"));
    $new_date->setAttribute('encoding', 'iso8601');
    $origin = $xpath->query('//ns:mods/ns:originInfo')->item(0);
    if (!$origin) {
      $origin = $dom->createElementNS($dom->firstChild->namespaceURI, 'originInfo');
      $origin = $dom->firstChild->appendChild($origin);
    }
    $new_date = $origin->appendChild($new_date);
    try {
      $datastream->setContentFromString($dom->saveXML());
      $out = TRUE;
    }
    catch (Exception $e) {
      $msg = 'Failed to get save MODS datastream for @pid';
      $vars = array('@pid' => $datastream->parent->id);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
  }
  return $out;
}
