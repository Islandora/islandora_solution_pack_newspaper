/**
 * @file
 * Islanodora Newspaper scripts file
 */

(function ($) {
  // Triggers a pager dropdown
  Drupal.behaviors.islandora_newspaper_dropdown_pager = {
    attach: function(context, settings) {
      if (!$("#page-select").hasClass('processed')) {
        $("#page-select").change(function(e) {
          var pid = $("#page-select option:selected").attr('value');
          window.location = Drupal.settings.basePath + 'islandora/object/' + pid;
        });
        $("#page-select").addClass('processed');
      }
    }
  };
})(jQuery);