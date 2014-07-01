/**
 * @file improve user interaction with the image chunk widgets.
 */

(function($) {
  Drupal.behaviors.chunksImageRemove = {
    attach: function(context, settings) {
      // Hide the cancel button if you remove an image. If the user were to
      // press cancel it would seem as if their previously uploaded image would
      // remain in the chunk untouched. This is not the case, however, since
      // that file was actually removed from the files directory.
      $('.image-widget .form-submit[value="' + Drupal.t('Remove') + '"]', context).bind('mousedown.removeChunkImage', function(e) {
        // Only do this if we are operating on an image chunk. Other chunk types
        // may provide an image widget.
        if ($(this).parents('.image-chunk-type-configuration').length > 0) {
          $(this).parents('.chunk-wrapper').find('.chunk-cancel-button').hide();
        }
      });
    }
  };
})(jQuery);
