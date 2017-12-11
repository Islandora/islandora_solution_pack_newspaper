<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_newspaper_admin_settings_form(array $form, array &$form_state) {
  $get_default_value = function ($name, $default) use (&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $form = array(
    'local_derivative_settings' => array(
      '#type' => 'fieldset',
      '#title' => t('Create Page/Issue Derivatives Locally'),
      '#description' => t('These options allow you to create derivatives automatically when ingesting pages or issues, using this server. If you intend to only use microservices to generate derivatives, you should not have any of these settings enabled.'),
      'islandora_newspaper_ingest_derivatives' => array(
        '#type' => 'checkboxes',
        '#element_validate' => array('islandora_newspaper_admin_settings_form_ingest_derivatives_validate'),
        '#options' => array(
          'pdf' => t('PDF datastream. <strong>Requires</strong> <em>ImageMagick</em>'),
          'image' => t('Image datastreams (TN, JPEG, JP2). <strong>Requires</strong> <em>Large Image Solution Pack</em>'),
          'ocr' => t('OCR datastreams (OCR, HOCR). <strong>Requires</strong> <em>Islandora OCR Module</em>'),
        ),
        '#default_value' => $get_default_value('islandora_newspaper_ingest_derivatives', drupal_map_assoc(
                array('pdf', 'image', 'ocr'))),
      ),
    ),
  );

  $form['islandora_newspaper_solr_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Solr settings'),
    'islandora_newspaper_parent_issue_solr_field' => array(
      '#type' => 'textfield',
      '#title' => t('Parent Solr Field'),
      '#description' => t("Solr field containing the parent issue's PID."),
      '#default_value' => variable_get('islandora_newspaper_parent_issue_solr_field', 'RELS_EXT_isMemberOf_uri_ms'),
      '#size' => 100,
      '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    ),
    'islandora_newspaper_use_solr' => array(
      '#type' => 'checkbox',
      '#title' => t('Use Solr for Newspaper display'),
      '#disabled' => (!module_exists('islandora_solr')),
      '#description' => t('Use Solr to generate lists of issues for a newspaper object.'),
      '#default_value' => variable_get('islandora_newspaper_use_solr', FALSE),
    ),
    'islandora_newspaper_solr_wrapper' => array(
      '#type' => 'container',
      '#states' => array(
        'visible' => array(
          ':input[name="islandora_newspaper_use_solr"]' => array('checked' => TRUE),
        ),
      ),
      'islandora_newspaper_solr_newspaper_parent_field' => array(
        '#type' => 'textfield',
        '#title' => t('Newspaper issue parent field'),
        '#description' => t('Solr field that contains the parent newspaper of the newspaper issue.'),
        '#size' => 100,
        '#default_value' => variable_get('islandora_newspaper_solr_newspaper_parent_field', 'RELS_EXT_isMemberOf_uri_ms'),
        '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
        '#states' => array(
          'required' => array(
            ':input[name="islandora_newspaper_use_solr"]' => array('checked' => TRUE),
          ),
        ),
      ),
      'islandora_newspaper_solr_date_field' => array(
        '#type' => 'textfield',
        '#title' => t('Issued date field'),
        '#description' => t('Solr field that contains the date issued of the newspaper issue.'),
        '#size' => 100,
        '#default_value' => variable_get('islandora_newspaper_solr_date_field', 'RELS_EXT_dateIssued_literal_ms'),
        '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
        '#states' => array(
          'required' => array(
            ':input[name="islandora_newspaper_use_solr"]' => array('checked' => TRUE),
          ),
        ),
      ),
      'islandora_newspaper_solr_sequence_field' => array(
        '#type' => 'textfield',
        '#title' => t('Sequence field'),
        '#description' => t('Solr field that contains the sequence number of the issues in a newspaper.'),
        '#size' => 100,
        '#default_value' => variable_get('islandora_newspaper_solr_sequence_field', 'RELS_EXT_isSequenceNumber_literal_ms'),
        '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
        '#states' => array(
          'required' => array(
            ':input[name="islandora_newspaper_use_solr"]' => array('checked' => TRUE),
          ),
        ),
      ),
      'islandora_newspaper_solr_remove_base_filters' => array(
        '#type' => 'checkbox',
        '#title' => t('Remove base Solr filters'),
        '#description' => t('This option removes your configured Solr base filters from these queries. If you want your filters to be applied even though they could affect which newspaper issue objects are returned in the list, uncheck this option.'),
        '#default_value' => variable_get('islandora_newspaper_solr_remove_base_filters', TRUE),
      ),
    ),
  );
  $form['#validate'][] = 'islandora_newspaper_admin_settings_form_validate';

  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $form += islandora_viewers_form('islandora_newspaper_issue_viewers', array('application/pdf'), 'islandora:newspaperIssueCModel');
  $form['issue_viewers'] = $form['viewers'];
  $form['issue_viewers']['#title'] = t('Issue Viewers');
  unset($form['viewers']);
  $form += islandora_viewers_form('islandora_newspaper_page_viewers', array('image/jp2', 'application/pdf'));
  $form['page_viewers'] = $form['viewers'];
  $form['page_viewers']['#title'] = t('Page Viewers');
  unset($form['viewers']);
  return system_settings_form($form);
}

/**
 * Implements hook_FORM_ID_validate().
 */
function islandora_newspaper_admin_settings_form_validate(array $form, array &$form_state) {
  $error = array();
  if (isset($form_state['values']['islandora_newspaper_use_solr']) &&
    $form_state['values']['islandora_newspaper_use_solr']) {
    if (!module_exists('islandora_solr')) {
      $error['islandora_newspaper_use_solr'] = t('This option requires the Islandora Solr Search module.');
    }
    if (empty($form_state['values']['islandora_newspaper_solr_newspaper_parent_field'])) {
      $error['islandora_newspaper_solr_newspaper_parent_field'] = t('Your must set the Newspaper issue parent field');
    }
    if (empty($form_state['values']['islandora_newspaper_solr_date_field'])) {
      $error['islandora_newspaper_solr_date_field'] = t('You must set the date issued field.');
    }
    if (empty($form_state['values']['islandora_newspaper_solr_sequence_field'])) {
      $error['islandora_newspaper_solr_sequence_field'] = t('You must set the sequence field.');
    }
  }
  if (count($error) > 0) {
    foreach ($error as $field => $message) {
      form_set_error($field, check_plain($message));
    }
  }
}

/**
 * Check if the required resouces are enabled.
 *
 * @param array $element
 *   The element to check.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_newspaper_admin_settings_form_ingest_derivatives_validate(array $element, array &$form_state, array $form) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  // form_error() doesn't seem to work here, it prevents the submit but
  // also prevents the value from being set. So we are using
  // drupal_set_message().
  if (array_key_exists('pdf', $element['#value']) && !islandora_paged_content_can_create_pdf()) {
    $element['#value']['pdf'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('ImageMagick must be enabled to derive the PDF datastream.'), 'error');
  }
  if (array_key_exists('image', $element['#value']) && !islandora_paged_content_can_create_images()) {
    $element['#value']['image'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('The Large Image Solution Pack must be enabled to derive image datastreams.'), 'error');
  }
  if (array_key_exists('ocr', $element['#value']) && !(module_exists('islandora_ocr'))) {
    $element['#value']['ocr'] = 0;
    form_set_value($element, $element['#value'], $form_state);
    drupal_set_message(t('The Islandora OCR module must be enabled to derive OCR datastreams.'), 'error');
  }
}
