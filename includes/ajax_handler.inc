<?php

/**
 * @file
 * AJAX handler functions for the Islandora Newspaper Solution pack module.
 */

/**
 * Helper to display the issues for a given year for a newspaper.
 *
 * @param abstractObject $islandora_object
 *   The fedora newspaper model object.
 * @param string $year
 *   The year for which to render the markup.
 */
function islandora_newspaper_ajax_get_year($islandora_object, $year) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  $issues = islandora_newspaper_get_issues($islandora_object);
  $grouped_issues = islandora_newspaper_group_issues($issues);
  $output = array(
    'tabs' => array(
      '#type' => 'vertical_tabs',
    ),
  );
  ksort($grouped_issues);
  $tabs = &$output['tabs'];
  $pid = $islandora_object->id;
  $months = $grouped_issues[$year];
  foreach ($months as $month => $days) {
    $month_name = t("@date", array(
      "@date" => date("F", mktime(0, 0, 0, $month, 1, 2000)),
    ));
    $tabs[$year][$month] = array(
      '#id' => 'y_' . $year . '_' . $month,
      '#title' => $month_name,
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('collapsible', 'month'),
      ),
    );
    foreach ($days as $day => $issues) {
      foreach ($issues as $issue) {
        $tabs[$year][$month][$day][] = array(
          '#id' => 'y_' . $year . '_' . $month . '_' . $day,
          '#theme' => 'link',
          '#prefix' => '<div>',
          '#suffix' => '</div>',
          '#text' => t("@month @day, @year", array(
              '@year' => $year,
              '@month' => $month_name,
              '@day' => $day,
              )),
          '#path' => "islandora/object/{$issue['pid']}",
          '#options' => array(
            'attributes' => array(),
            'html' => FALSE,
          ),
        );
      }
    }
    ksort($tabs[$year][$month]);
  }
  $html = theme('islandora_newspaper_single_year_issues', array('tabs' => $tabs));

  print $html;
  exit(0);
}
