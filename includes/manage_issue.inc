<?php

/**
 * @file
 * Provides the management interface for Newspaper Issue objects.
 */

require_once dirname(__FILE__) . '/utilities.inc';

/**
 * Local menu action, that gets the ingest page form.
 *
 * @global $user
 *
 * @param AbstractObject $issue
 *   The newspaper to ingest into.
 *
 * @return sting
 *   The HTML repersentation of the ingest page form.
 */
function islandora_newspaper_ingest_page(AbstractObject $issue) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $tuque = islandora_get_tuque_connection();
  $page = $tuque->repository->constructObject(islandora_get_namespace($issue->id));
  $page->owner = $user->name;
  $page->label = 'New Page';
  $page->models = 'islandora:newspaperPageCModel';
  $newspaper = islandora_object_load(islandora_newspaper_get_newspaper($issue));
  drupal_set_title(t('Add page to @newspaper issue @issue', array('@newspaper' => $newspaper->label, '@issue' => $issue->label)));
  return drupal_get_form('islandora_ingest_form', array(
      'models' => array('islandora:newspaperPageCModel'),
      'issue' => $issue,
      'object' => $page,
      'parent' => $issue->id,
    ));
}

/**
 * Local menu action to present a zipped file ingest form.
 *
 * @param AbstractObject $object
 *   The issue to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function islandora_newspaper_ingest_zipped_pages(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  module_load_include('inc', 'islandora', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  return drupal_get_form('islandora_newspaper_zipped_upload_form', $object->id);
}

/**
 * Defines the zipped page form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $issue_pid
 *   PID of issue into which pages are being ingested.
 *
 * @return array
 *   Drupal form definition.
 */
function islandora_newspaper_zipped_upload_form(array $form, array &$form_state, $issue_pid) {
  module_load_include('inc', 'islandora_ocr', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $issue_object = islandora_object_load($issue_pid);
  $current_pages = islandora_paged_content_get_pages($issue_object);
  $last_page_number = count($current_pages);
  $languages = module_exists('islandora_ocr') ? islandora_ocr_get_enabled_tesseract_languages() : array();
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('zip');
  $form = array();
  $derivatives = variable_get('islandora_newspaper_ingest_derivatives', array('ocr'));
  $do_ocr = in_array('ocr', $derivatives);
  $message = t("This sequence currently has @count pages. Additional pages will be appended to the end of the sequence by default. !break", array("@count" => $last_page_number, '!break' => '<br />'));
  $message .= t("Choose a number lower than @count to insert page(s) at a specific location in the sequence.", array("@count" => $last_page_number, '!break' => '<br />'));

  $form['language'] = array(
    '#access' => module_exists('islandora_ocr') && $do_ocr,
    '#title' => t('Language'),
    '#type' => 'select',
    '#description' => t('Please select the language the page is written in.'),
    '#options' => $languages,
    '#default_value' => 'English',
  );

  if ($current_pages) {
    $form['insertion_point'] = array(
      '#type' => 'textfield',
      '#title' => t('Last sequence number'),
      '#default_value' => $last_page_number,
      '#description' => check_plain($message),
      '#size' => 5,
    );
  }

  // Value behaves more consistently when passed as a string.
  $form['current_pages'] = array(
    '#type' => 'hidden',
    '#value' => serialize($current_pages),
  );

  $form['file'] = array(
    '#title' => t('Compressed images file.'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions ,
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),

  );

  $form['issue_pid'] = array(
    '#type' => 'hidden',
    '#value' => $issue_pid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add files to issue.'),
  );
  return $form;
}

/**
 * Submit handler for uploaded zip files.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_newspaper_zipped_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $issue_pid = $form_state['values']['issue_pid'];
  $namespace = substr($issue_pid, 0, strpos($issue_pid, ":"));
  $tmp_dir = uniqid();
  $issue_object = islandora_object_load($issue_pid);
  if (!$issue_pid) {
    drupal_set_message(t("This issue does not exist in this repository"), 'warning');
    return;
  }

  $current_pages = unserialize($form_state['values']['current_pages']);

  $insertion_point = isset($form_state['values']['insertion_point']) ? (int) $form_state['values']['insertion_point'] : 0;
  $pages_to_renumber = array();
  foreach ($current_pages as $current_page) {
    if ((int) $current_page['page'] > $insertion_point) {
      $pages_to_renumber[] = $current_page;
    }
  }
  // Extract file.
  $zip_file = file_load($form_state['values']['file']);
  $zip_uri = drupal_realpath($zip_file->uri);
  $zip = new ZipArchive();
  $zip->open($zip_uri);
  $destination_dir = drupal_realpath("temporary://$tmp_dir");

  // Extract zipped file to named directory.
  if (!$zip->extractTo($destination_dir)) {
    drupal_set_message(t('Ingest failed.'), 'warning');
    return;
  }
  $zip->close();
  file_delete($zip_file);

  $allowed_extensions = array('tif', 'tiff', 'jpg', 'jpeg', 'jp2');
  $callback = function ($element) use ($allowed_extensions) {
    $ext = pathinfo($element, PATHINFO_EXTENSION);
    $ext = drupal_strtolower($ext);

    // An allowed extension and does /not/ contain __MACOSX.
    return in_array($ext, $allowed_extensions) && preg_match('/__MACOSX/', $element) === 0;
  };

  $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($destination_dir), RecursiveIteratorIterator::SELF_FIRST);
  foreach ($objects as $file => $object) {
    $unfiltered[] = $file;
  }
  $files_to_add = array_values(array_filter($unfiltered, $callback));
  // Sort files based on name.
  $comparator = function ($a, $b) {
    $file_a = pathinfo($a, PATHINFO_FILENAME);
    $file_b = pathinfo($b, PATHINFO_FILENAME);
    return ($file_a < $file_b) ? -1 : 1;
  };
  usort($files_to_add, $comparator);

  $renumber_count = count($pages_to_renumber);
  $add_count = count($files_to_add);
  $status_message = format_plural(
    $add_count,
    'adding 1 page to issue',
    'adding @count pages to issue'
  );

  if ($renumber_count) {
    $status_message = format_plural(
      $renumber_count,
      'Renumbering 1 page and @adding_message.',
      'Renumbering @count pages and @adding_message.',
      array(
        '@adding_message' => $status_message,
      )
    );
  }

  if ($add_count > 0) {
    $batch = array(
      'title' => ucfirst($status_message),
      'progress_message' => t('Completed @current operations out of @total.'),
      'operations' => array(),
      'file' => drupal_get_path('module', 'islandora_newspaper') . '/includes/manage_issue.inc',
      'finished' => 'islandora_newspaper_zipped_upload_ingest_finished',
    );
    $file_count = count($files_to_add);
    $config = array(
      'issue_pid' => $issue_pid,
      'destination_dir' => $destination_dir,
      'namespace' => $namespace,
      'language' => $form_state['values']['language'],
    );
    $offset = count($files_to_add);
    if (isset($pages_to_renumber[0])) {
      foreach ($pages_to_renumber as $page) {
        $batch['operations'][] = array(
          'islandora_newspaper_insert_sequence_gap',
          array($page, $offset),
        );
      }
    }
    foreach ($files_to_add as $image) {
      $config['page_number'] = ++$insertion_point;
      $config['image'] = $image;
      $batch['operations'][] = array(
        'islandora_newspaper_add_pages',
        array($repository, $config, $destination_dir),
      );
    }

    batch_set($batch);
  }
  else {
    drupal_set_message(t('No files were found to add, in the provided ZIP file.'), 'warning');
  }
}

/**
 * Creates page objects and associates them with issue object.
 *
 * @param Repository $repository
 *   Active repository object to build NewFedoraObject
 * @param array $config
 *   Associative array of required values.
 * @param string $pages_directory
 *   This is a hack.  The finished callback needs this value so it can delete
 *   the temporary directory used to store issue pages.
 */
function islandora_newspaper_add_pages($repository, $config, $pages_directory, &$context) {
  module_load_include('inc', 'islandora', 'includes/IslandoraTuque');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $context['results']['pages_directory'] = $pages_directory;

  // Create object.
  $object = $repository->constructObject($config['namespace']);
  $object->label = pathinfo($config['image'], PATHINFO_FILENAME);
  $ds_label = pathinfo($config['image'], PATHINFO_BASENAME);
  islandora_paged_content_update_datastream($object, $config['image'], 'OBJ', $ds_label, NULL, 'M', FALSE);
  $rels_ext = $object->relationships;
  $parent = $config['issue_pid'];
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:newspaperPageCModel');
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $parent);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $config['page_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $config['page_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $parent);
  islandora_add_object($object);
}

/**
 * Alters page and sequence number of page object to allow for inserted pages.
 *
 * @param array $page_to_reindex
 *   Array Containing page pid and current page number
 * @param int $offset
 *   Offset to determine new page number.
 */
function islandora_newspaper_insert_sequence_gap($page_to_reindex, $offset) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $new_page_num = (int) $page_to_reindex['page'] + $offset;
  $page_object = islandora_object_load($page_to_reindex['pid']);
  $rels_ext = $page_object->relationships;
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $new_page_num, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $new_page_num, TRUE);
}

/**
 * Batch 'finished' callback.
 *
 * Deletes the temporary files associated with the ingested pages.
 */
function islandora_newspaper_zipped_upload_ingest_finished($success, $results, $operations) {
  file_unmanaged_delete_recursive($results['pages_directory']);
}
