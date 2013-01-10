/**
 * @file
 * Islanodora Newspaper scripts file
 */

(function ($) {
  // Clone controls for full page viewer
  Drupal.behaviors.islandoraNewspaperCloneControls = {
    attach: function(context, settings) {
      if (!$(".islandora-newspaper-controls").hasClass('processed')) {
        $(".islandora-newspaper-controls").clone().appendTo("#islandora-openseadragon").css({'display': 'none'});
        $(".islandora-newspaper-controls").addClass('processed');
      }
    }
  };
  // Select page
  Drupal.behaviors.islandoraNewspaperSelectPage = {
    attach: function(context, settings) {
      if (!$(".page-select").hasClass('processed')) {
        $(".page-select").change(function(e) {
          var pid = $("option:selected", this).attr('value');
          window.location = Drupal.settings.basePath + 'islandora/object/' + pid; // check plain?
        });
        $(".page-select").addClass('processed');
      }
    }
  };
})(jQuery);